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
**   Define the JMSG topic Comma Separated Variable command plugin topic
**
** Notes:
**   1. Allows a cFS app to send JMSG commands to a system external 
**      to the cFS target. The command parameters are contained in a CSV
**      string.
**   2. The JsonToCfe() function is provided for teh less common situation
**      when a an external system to the cFS needs to command a cFS
**      using the JMSG CSV interface.
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_csv_cmd.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define NAME_LEN        JMSG_PLATFORM_TOPIC_NAME_MAX_LEN
#define SCRIPT_TXT_LEN  sizeof(JMSG_LIB_TlmParamString_String_t)


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

static JMSG_TOPIC_CSV_CMD_Class_t  *JMsgTopicCsvCmd = NULL;

static JMSG_LIB_TopicCsvCmd_Payload_t  CsvCmdPayload;    /* Working buffer for JSON message parsing */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                        Data                                          core-json       length of query      */
   /* Address,                    Length,          Updated, Data Type,  Float,  query string,   string(exclude '\0') */
   
   { &CsvCmdPayload.Name,         NAME_LEN,        false,   JSONNumber, false,  { "name",       (sizeof("name")-1)} },
   { &CsvCmdPayload.ParamText,    SCRIPT_TXT_LEN,  false,   JSONString, false,  { "parameters", (sizeof("parameters")-1)} }
   
}; 

static const char *NullCsvCmd = "{\"name\": \"null\", \"parameters\": \"\none\"}";

static const char *TestParamStr[] = {"zero","one"};

/******************************************************************************
** Function: JMSG_TOPIC_CSV_CMD_Constructor
**
** Initialize the JMSG Script topic plugin 
**
** Notes:
**   None
**
*/
void JMSG_TOPIC_CSV_CMD_Constructor(JMSG_TOPIC_CSV_CMD_Class_t *JMsgTopicCsvCmdPtr,
                                    JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
								            CFE_SB_MsgId_t CsvCmdMid)

{

   JMsgTopicCsvCmd = JMsgTopicCsvCmdPtr;
   memset(JMsgTopicCsvCmd, 0, sizeof(JMSG_TOPIC_CSV_CMD_Class_t));

   JMsgTopicCsvCmd->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicCsvCmd->CsvCmd), CsvCmdMid, sizeof(JMSG_LIB_TopicCsvCmd_t));  
      
} /* End JMSG_TOPIC_CSV_CMD_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a JMSG JSON message from the payload of the JMSG CSV SB message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. It is the user's responsibility to ensure that the JMsgPayload buffer
**      length is adequate. An event message when a memory overrrun/corruption
**      occurs. 
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const JMSG_LIB_TopicCsvCmd_Payload_t *CsvMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_LIB_TopicCsvCmd_t);

   *JMsgPayload = NullCsvCmd;
   
   PayloadLen = sprintf(JMsgTopicCsvCmd->JMsgPayload,
                "{\"name\": \"%s\", \"parameters\": \"%s\"}",
                CsvMsg->Name, CsvMsg->ParamText);

   if (PayloadLen > 0)
   {
      if (PayloadLen <= sizeof(JMsgTopicCsvCmd->JMsgPayload))
      {
         *JMsgPayload = JMsgTopicCsvCmd->JMsgPayload;
      
         ++JMsgTopicCsvCmd->CfeToJMsgCnt;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_CFE2JSON_EID, CFE_EVS_EventType_ERROR,
                           "JMSG CSV Command payload %d byte buffer overrun (memory corrupted)",
                           (uint16)(PayloadLen-sizeof(JMsgTopicCsvCmd->JMsgPayload)));
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_CFE2JSON_EID, CFE_EVS_EventType_ERROR,
                        "JMSG CSV Command payload conversion error");      
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON CSV topic message to a cFE SB CSV message
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**   2. Below is a null test message that theoritically could be used in a 
**      test. However, it isn't very practical. Also a network transport app
**      like UDP/MQTT is required.
**      {"name": "null", "parameters": "none" }
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&JMsgTopicCsvCmd->CsvCmd;

      ++JMsgTopicCsvCmd->JMsgToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**   1. Each JSON message must contain all of the objects defined in 
**      JsonTblObjs[]
*/
static bool LoadJsonData(const char *JMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&JMsgTopicCsvCmd->CsvCmd.Payload, 0, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicCsvCmd->JsonObjCnt, JMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG CSV Command Topic LoadJsonData() processed %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == JMsgTopicCsvCmd->JsonObjCnt)
   {
      memcpy(&JMsgTopicCsvCmd->CsvCmd.Payload, &CsvCmdPayload, sizeof(JMSG_LIB_TopicCsvCmd_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_LOAD_JSON_DATA_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing JMSG CSV Command Topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicCsvCmd->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send JMSG CSV topic command messages on SB that are read back
** by JMSG_UDP and cause JMSG's to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**   2. The test message can be viewed in the JMSG_LIB_TOPIC_CSV_CMD
**      telemetry window.
**   3. See JMSG_DEMO for a functional example based on practical use cases. 
**
*/
static void PluginTest(bool Init, int16 Param)
{

   JMSG_LIB_TopicCsvCmd_Payload_t *Payload = &JMsgTopicCsvCmd->CsvCmd.Payload;
   
   uint16 TestParam;
   
   if (Init)
   {

      JMsgTopicCsvCmd->PluginTestCnt = 1;
     
      strcpy(Payload->Name, "Test");
     
      CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG CSV command plugin topic test started");

   }
   else
   {                 
      JMsgTopicCsvCmd->PluginTestCnt++;      
   }

   TestParam = JMsgTopicCsvCmd->PluginTestCnt % 2;
   
   sprintf(Payload->ParamText, "\"cmd-code\": %d, \"ex_int\": %d, \"ex_str\": \"%s\"",
           JMsgTopicCsvCmd->PluginTestCnt, TestParam, TestParamStr[TestParam]);
    
   CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG CSV telemetry plugin topic test text payload: %s", Payload->ParamText);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicCsvCmd->CsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicCsvCmd->CsvCmd.TelemetryHeader), true);
   
} /* End PluginTest() */
