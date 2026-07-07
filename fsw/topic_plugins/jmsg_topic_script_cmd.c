/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Define the JMSG topic script command plugin topic
**
** Notes:
**   1. Allows a cFS app to manage the excecution of scripts external to
**      the cFS target. The scripts could be on the same processor or on
**      a remote system.  Scripts can be embedded in the JSMG or in a file
**      on the external system.
**   2. The JsonToCfe() function is provided however there currently aren't
**      use cases for the cFS target to execute python scripts. 
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_script_cmd.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define SCRIPT_TXT_LEN  sizeof(JMSG_LIB_CmdScriptString_String_t)

/**********************/
/** Type Definitions **/
/**********************/


/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen);
static bool LoadJsonData(const char *JMsgPayload, uint16 PayloadLen);
static void PluginTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

static JMSG_TOPIC_SCRIPT_CMD_Class_t  *JMsgTopicScriptCmd = NULL;

static JMSG_LIB_TopicScriptCmd_Payload_t  ScriptCmdPayload;    /* Working buffer for JSON message parsing */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                           Data                                          core-json           length of query      */
   /* Address,                       Length,          Updated, Data Type,  Float,  query string,       string(exclude '\0') */
   
   { &ScriptCmdPayload.Command,      2,               false,   JSONNumber, false,  { "command",     (sizeof("command")-1)} },
   { &ScriptCmdPayload.ScriptFile,   OS_MAX_PATH_LEN, false,   JSONString, false,  { "script-file", (sizeof("script-file")-1)} },
   { &ScriptCmdPayload.ScriptText,   SCRIPT_TXT_LEN,  false,   JSONString, false,  { "script-text", (sizeof("script-text")-1)} }
   
};

static const char *NullScriptCmdMsg = "{\"command\": 0, \"script-file\": \"\0\", \"script-text\": \"\0\"}";


/******************************************************************************
** Function: JMSG_TOPIC_SCRIPT_CMD_Constructor
**
** Initialize the JMSG Script topic plugin 
**
** Notes:
**   1. A telemetry MID is used because a script command message uses a cFE
**      telemetry message.
**
*/
void JMSG_TOPIC_SCRIPT_CMD_Constructor(JMSG_TOPIC_SCRIPT_CMD_Class_t *JMsgTopicScriptCmdPtr,
                                       JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
								               CFE_SB_MsgId_t ScriptTlmMid)

{

   JMsgTopicScriptCmd = JMsgTopicScriptCmdPtr;
   memset(JMsgTopicScriptCmd, 0, sizeof(JMSG_TOPIC_SCRIPT_CMD_Class_t));

   JMsgTopicScriptCmd->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicScriptCmd->ScriptCmd), ScriptTlmMid, sizeof(JMSG_LIB_TopicScriptCmd_t));  
      
} /* End JMSG_TOPIC_SCRIPT_CMD_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a JMSG JSON message from the payload of the JMSG Script SB message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. It is the user's responsibility to ensure that the JMsgPayload buffer
**      length is adequate. An event message when a memory overrrun/corruption
**      occurs. 
**
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const JMSG_LIB_TopicScriptCmd_Payload_t *ScriptMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_LIB_TopicScriptCmd_t);

   *JMsgPayload = NullScriptCmdMsg;
   
   PayloadLen = sprintf(JMsgTopicScriptCmd->JMsgPayload,
                "{\"command\": %d, \"script-file\": \"%s\", \"script-text\": \"%s\"}",
                ScriptMsg->Command, ScriptMsg->ScriptFile, ScriptMsg->ScriptText);

   if (PayloadLen > 0)
   {
      if (PayloadLen <= sizeof(JMsgTopicScriptCmd->JMsgPayload))
      {
         *JMsgPayload = JMsgTopicScriptCmd->JMsgPayload;
      
         ++JMsgTopicScriptCmd->CfeToJMsgCnt;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_CFE2JSON_EID, CFE_EVS_EventType_ERROR,
                           "JMSG Script Command payload %d byte buffer overrun (memory corrupted)",
                           (uint16)(PayloadLen-sizeof(JMsgTopicScriptCmd->JMsgPayload)));
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_CFE2JSON_EID, CFE_EVS_EventType_ERROR,
                        "JMSG Script Command payload conversion error");      
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON script topic message to a cFE SB script message
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**   2. Below is a null test message that theoritically could be used in a 
**      test. However, it isn't very practical since there currently aren't
**      use cases for the cFS target to execute python scripts. 
**      {"command": 0, "script-file": "null-file", "script-text": "null-script" }
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&JMsgTopicScriptCmd->ScriptCmd;

      ++JMsgTopicScriptCmd->JMsgToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. All JSON objects must be defined even if they are unused.
*/
static bool LoadJsonData(const char *JMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&JMsgTopicScriptCmd->ScriptCmd.Payload, 0, sizeof(JMSG_LIB_TopicScriptCmd_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicScriptCmd->JsonObjCnt, JMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG Script Command Topic LoadJsonData() processed %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == JMsgTopicScriptCmd->JsonObjCnt)
   {
      memcpy(&JMsgTopicScriptCmd->ScriptCmd.Payload, &ScriptCmdPayload, sizeof(JMSG_LIB_TopicScriptCmd_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_LOAD_JSON_DATA_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing JMSG Script Command Topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicScriptCmd->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send JMSG Script topic messages on SB.
**
** Notes:
**   1. Param is not used
**   2. The test message can be viewed in the JMSG_LIB_TOPIC_SCRIPT_CMD
**      telemetry window.
**   3. See JMSG_DEMO for examples on how to send the JMSG_LIB_TOPIC_SCRIPT_CMD
**      for a protocol like UDP and run scripts remote to the cFS target
*/
static void PluginTest(bool Init, int16 Param)
{

   JMSG_LIB_TopicScriptCmd_Payload_t *Payload = &JMsgTopicScriptCmd->ScriptCmd.Payload;
   

   if (Init)
   {

      JMsgTopicScriptCmd->PluginTestCnt = 1;
	   CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG script command plugin topic test started");

   }
   else
   {
                    
      if (JMsgTopicScriptCmd->PluginTestCnt % 2 == 0)
      {
         Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_TEXT;
         strncpy(Payload->ScriptFile, "Undefined", OS_MAX_PATH_LEN);
         sprintf(Payload->ScriptText, "print(\"Hello World %d\")", JMsgTopicScriptCmd->PluginTestCnt);
         
      }
      else
      {
         Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_FILE;
         strncpy(Payload->ScriptFile, "hello_world.py", OS_MAX_PATH_LEN);         
         sprintf(Payload->ScriptText, "Undefined");
      }
      
      JMsgTopicScriptCmd->PluginTestCnt++;
   }


   CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_CMD_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG script command plugin topic test text payload: %s", Payload->ScriptText);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicScriptCmd->ScriptCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicScriptCmd->ScriptCmd.TelemetryHeader), true);
   
} /* End PluginTest() */
