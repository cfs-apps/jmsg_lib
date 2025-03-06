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

   /* Data                           Data                                          core-json       length of query      */
   /* Address,                       Length,          Updated, Data Type,  Float,  query string,   string(exclude '\0') */
   
   { &CsvCmdPayload.Name,         NAME_LEN,        false,   JSONNumber, false,  { "name",       (sizeof("name")-1)} },
   { &CsvCmdPayload.ParamText,    SCRIPT_TXT_LEN,  false,   JSONString, false,  { "parameters", (sizeof("parameters")-1)} }
   
}; 

static const char *NullCsvCmd = "{\"name\": \"null\", \"parameters\": \"\none\"}";


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
**
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const JMSG_LIB_TopicCsvCmd_Payload_t *CsvMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_LIB_TopicCsvCmd_t);

   *JMsgPayload = NullCsvCmd;
   
   //TODO: Add string protection?
   PayloadLen = sprintf(JMsgTopicCsvCmd->JMsgPayload,
                "{\"name\": \"%s\", \"parameters\": \"%s\"}",
                CsvMsg->Name, CsvMsg->ParamText);

   if (PayloadLen > 0)
   {
      *JMsgPayload = JMsgTopicCsvCmd->JMsgPayload;
   
      ++JMsgTopicCsvCmd->CfeToJMsgCnt;
      RetStatus = true;
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
**   2. Test messages:
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
**  1. See file prologue for full/partial table load scenarios
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
** Generate and send JMSG Script topic messages on SB that are read back by
** JMSG_UDP and cause JMSG's to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
*/
static void PluginTest(bool Init, int16 Param)
{

   JMSG_LIB_TopicCsvCmd_Payload_t *Payload = &JMsgTopicCsvCmd->CsvCmd.Payload;
   

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

   sprintf(Payload->ParamText, "\"Param\": %d", JMsgTopicCsvCmd->PluginTestCnt);
 
   CFE_EVS_SendEvent(JMSG_TOPIC_CSV_CMD_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG CSV telemetry plugin topic test text payload: %s", Payload->ParamText);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicCsvCmd->CsvCmd.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicCsvCmd->CsvCmd.TelemetryHeader), true);
   
} /* End PluginTest() */
