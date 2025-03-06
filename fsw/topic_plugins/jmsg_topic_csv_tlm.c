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
**   Define the JMSG topic Comma Separated Variable telemetry plugin topic
**
** Notes:
**   1. Allows a cFS app to receive JMSG telemetry from a system external 
**      to the cFS target. The telemetry parameters are contained in a CSV
**      string.
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_csv_tlm.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define NAME_LEN        JMSG_PLATFORM_TOPIC_NAME_MAX_LEN
#define SEQ_COUNT_LEN   sizeof(uint32)
#define DATE_TIME_LEN   sizeof(JMSG_LIB_TlmDateTime_String_t)
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

static JMSG_TOPIC_CSV_TLM_Class_t  *JMsgTopicCsvTlm = NULL;

static JMSG_LIB_TopicCsvTlm_Payload_t  CsvTlmPayload;    /* Working buffer for JSON message parsing */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                           Data                                          core-json       length of query      */
   /* Address,                       Length,          Updated, Data Type,  Float,  query string,   string(exclude '\0') */
   
   { &CsvTlmPayload.Name,         NAME_LEN,        false,   JSONString, false,  { "name",       (sizeof("name")-1)} },
   { &CsvTlmPayload.SeqCount,     SEQ_COUNT_LEN,   false,   JSONNumber, false,  { "seq-count",  (sizeof("seq-count")-1)} },
   { &CsvTlmPayload.DateTime,     DATE_TIME_LEN,   false,   JSONString, false,  { "date-time",  (sizeof("date-time")-1)} },
   { &CsvTlmPayload.ParamText,    SCRIPT_TXT_LEN,  false,   JSONString, false,  { "parameters", (sizeof("parameters")-1)} }
   
}; 

static const char *NullCsvTlm = "{\"name\": \"null\", \"seq-count\": 99, \"date-time\": \"\0\", \"parameters\": \"\none\"}";


/******************************************************************************
** Function: JMSG_TOPIC_CSV_TLM_Constructor
**
** Initialize the JMSG Script topic plugin 
**
** Notes:
**   None
**
*/
void JMSG_TOPIC_CSV_TLM_Constructor(JMSG_TOPIC_CSV_TLM_Class_t *JMsgTopicCsvTlmPtr,
                                    JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
								            CFE_SB_MsgId_t CsvTlmMid)

{

   JMsgTopicCsvTlm = JMsgTopicCsvTlmPtr;
   memset(JMsgTopicCsvTlm, 0, sizeof(JMSG_TOPIC_CSV_TLM_Class_t));

   JMsgTopicCsvTlm->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicCsvTlm->CsvTlm), CsvTlmMid, sizeof(JMSG_LIB_TopicCsvTlm_t));  
      
} /* End JMSG_TOPIC_CSV_TLM_Constructor() */


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
   const JMSG_LIB_TopicCsvTlm_Payload_t *ScriptMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_LIB_TopicCsvTlm_t);

   *JMsgPayload = NullCsvTlm;
   
   //TODO: Add string protection?
   PayloadLen = sprintf(JMsgTopicCsvTlm->JMsgPayload,
                "{\"name\": \"%s\", \"seq-count\": %d, \"date-time\": \"%s\",  \"parameters\": \"%s\"}",
                ScriptMsg->Name, ScriptMsg->SeqCount, ScriptMsg->DateTime, ScriptMsg->ParamText);

   if (PayloadLen > 0)
   {
      *JMsgPayload = JMsgTopicCsvTlm->JMsgPayload;
   
      ++JMsgTopicCsvTlm->CfeToJMsgCnt;
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
**      {"name": "null", "seq-count": 0, "date-time": "00/00/0000 00:00:00",  "parameters": "none" }
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&JMsgTopicCsvTlm->CsvTlm;

      ++JMsgTopicCsvTlm->JMsgToCfeCnt;
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

   memset(&JMsgTopicCsvTlm->CsvTlm.Payload, 0, sizeof(JMSG_LIB_TopicCsvTlm_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicCsvTlm->JsonObjCnt, JMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(JMSG_TOPIC_CSV_TLM_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG Script Command Topic LoadJsonData() processed %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == JMsgTopicCsvTlm->JsonObjCnt)
   {
      memcpy(&JMsgTopicCsvTlm->CsvTlm.Payload, &CsvTlmPayload, sizeof(JMSG_LIB_TopicCsvTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_CSV_TLM_LOAD_JSON_DATA_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing JMSG Script Command Topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicCsvTlm->JsonObjCnt);
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

   JMSG_LIB_TopicCsvTlm_Payload_t *Payload = &JMsgTopicCsvTlm->CsvTlm.Payload;
   

   if (Init)
   {

      JMsgTopicCsvTlm->PluginTestCnt = 1;
	  

      strcpy(Payload->Name, "Test");
      strcpy(Payload->DateTime, "00/00/0000 12:34:56");
      strcpy(Payload->ParamText, "\"one\": 1");

      CFE_EVS_SendEvent(JMSG_TOPIC_CSV_TLM_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG CSV command plugin topic test started");

   }
   else
   {                 
      JMsgTopicCsvTlm->PluginTestCnt++;      
   }

   Payload->SeqCount = JMsgTopicCsvTlm->PluginTestCnt;
      
   CFE_EVS_SendEvent(JMSG_TOPIC_CSV_TLM_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "JMSG CSV telemetry plugin topic test text payload: %s", Payload->ParamText);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicCsvTlm->CsvTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicCsvTlm->CsvTlm.TelemetryHeader), true);
   
} /* End PluginTest() */
