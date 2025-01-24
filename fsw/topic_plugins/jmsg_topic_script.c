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
**   Define the JMSG script plugin topic
**
** Notes:
**   1. The JMSG script interface allows a cFS app to manage the excecution
**      of scripts on a remote system.  Scripts can be embedded in the JSMG
**      or resident on the remote system.
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_script.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define SCRIPT_TXT_LEN  sizeof(JMSG_LIB_ScriptString_String_t)

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

static JMSG_TOPIC_SCRIPT_Class_t  *JMsgTopicScript = NULL;

static JMSG_LIB_ExecScriptTlm_Payload_t  ExecScriptTlmPayload;    /* Working buffer for JSON message parsing */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                               Data                                          core-json           length of query      */
   /* Address,                           Length,          Updated, Data Type,  Float,  query string,       string(exclude '\0') */
   
   { &ExecScriptTlmPayload.Command,      2,               false,   JSONNumber, false,  { "command",     (sizeof("command")-1)} },
   { &ExecScriptTlmPayload.ScriptFile,   OS_MAX_PATH_LEN, false,   JSONString, false,  { "script-file", (sizeof("script-file")-1)} },
   { &ExecScriptTlmPayload.ScriptText,   SCRIPT_TXT_LEN,  false,   JSONString, false,  { "script-text", (sizeof("script-text")-1)} }
   
};

static const char *NullExecScriptMsg = "{\"command\": 0, \"script-file\": \"\0\", \"script-text\": \"\0\"}";


/******************************************************************************
** Function: JMSG_TOPIC_SCRIPT_Constructor
**
** Initialize the JMSG Script topic plugin 
**
** Notes:
**   None
**
*/
void JMSG_TOPIC_SCRIPT_Constructor(JMSG_TOPIC_SCRIPT_Class_t *JMsgTopicScriptPtr,
                                   JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
								           CFE_SB_MsgId_t ScriptTlmMid)

{

   JMsgTopicScript = JMsgTopicScriptPtr;
   memset(JMsgTopicScript, 0, sizeof(JMSG_TOPIC_SCRIPT_Class_t));

   JMsgTopicScript->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicScript->ExecScriptTlm), ScriptTlmMid, sizeof(JMSG_LIB_ExecScriptTlm_t));  
      
} /* End JMSG_TOPIC_SCRIPT_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a JMSG JSON message from the payload of the JMSG Script SB message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const JMSG_LIB_ExecScriptTlm_Payload_t *ExecScriptMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_LIB_ExecScriptTlm_t);

   *JMsgPayload = NullExecScriptMsg;

   //TODO: Add string protection
   PayloadLen = sprintf(JMsgTopicScript->JMsgPayload,
                "{\"command\": %d, \"script-file\": \"%s\", \"script-text\": \"%s\"}",
                ExecScriptMsg->Command, ExecScriptMsg->ScriptFile, ExecScriptMsg->ScriptText);

   if (PayloadLen > 0)
   {
      *JMsgPayload = JMsgTopicScript->JMsgPayload;
   
      ++JMsgTopicScript->CfeToJMsgCnt;
      RetStatus = true;
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
**   2. Test messages:
**      {"command": 0, "script-file": "null-file", "script-text": "null-script" }
**      {"command": 1, "script-file": "null-file", "script-text": "null-script" }  //TODO
**      {"command": 2, "script-file": "null-file", "script-text": "null-script" }  //TODO
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&JMsgTopicScript->ExecScriptTlm;

      ++JMsgTopicScript->JMsgToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static bool LoadJsonData(const char *JMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&JMsgTopicScript->ExecScriptTlm.Payload, 0, sizeof(JMSG_LIB_ExecScriptTlm_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicScript->JsonObjCnt, JMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG Script Topic LoadJsonData() processed %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == JMsgTopicScript->JsonObjCnt)
   {
      memcpy(&JMsgTopicScript->ExecScriptTlm.Payload, &ExecScriptTlmPayload, sizeof(JMSG_LIB_ExecScriptTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_LOAD_JSON_DATA_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing JMSG Script Topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicScript->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send JMSG Script topic messages on SB that are read back by
** JMSG_UDP and cause JMSG's to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
*/
static void PluginTest(bool Init, int16 Param)
{

   JMSG_LIB_ExecScriptTlm_Payload_t *Payload = &JMsgTopicScript->ExecScriptTlm.Payload;
   

   if (Init)
   {

      JMsgTopicScript->PluginTestCnt = 1;
	  
      Payload->Command = JMSG_LIB_ExecScriptCmd_RUN_SCRIPT_TEXT;
      strncpy(Payload->ScriptFile, "Undefined", OS_MAX_PATH_LEN);
      sprintf(Payload->ScriptText, "print(\"Hello World %d\")", JMsgTopicScript->PluginTestCnt);

      CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG script plugin topic test started");

   }
   else
   {
                    
      JMsgTopicScript->PluginTestCnt++;
      sprintf(Payload->ScriptText, "print(\"Hello World %d\")", JMsgTopicScript->PluginTestCnt);
      
   }

   CFE_EVS_SendEvent(JMSG_TOPIC_SCRIPT_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG script plugin topic test text payload: %s", Payload->ScriptText);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicScript->ExecScriptTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicScript->ExecScriptTlm.TelemetryHeader), true);
   
} /* End PluginTest() */
