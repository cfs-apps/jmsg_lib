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
**   Manage TPLUG_WEB Discrete plugin topic
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "tplug_web_discrete.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define TPLUG_WEB_DISCRETE_CONSTRUCTOR_EID       (USR_TPLUG_BaseEid_WebDiscrete + 0)
#define TPLUG_WEB_DISCRETE_INIT_PLUGIN_TEST_EID  (USR_TPLUG_BaseEid_WebDiscrete + 1)
#define TPLUG_WEB_DISCRETE_PLUGIN_TEST_EID       (USR_TPLUG_BaseEid_WebDiscrete + 2)
#define TPLUG_WEB_DISCRETE_LOAD_JSON_DATA_EID    (USR_TPLUG_BaseEid_WebDiscrete + 3)
#define TPLUG_WEB_DISCRETE_JSON_TO_CCSDS_ERR_EID (USR_TPLUG_BaseEid_WebDiscrete + 4)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** TPLUG_WEB_DiscreteTlmMsg_t & TPLUG_WEB_DiscreteTlmMsg_Payload_t are defined in EDS
*/

typedef struct
{

   /*
   ** Discrete Telemetry
   */
   
   char            JsonMsgPayload[1024];
   CFE_SB_MsgId_t  SbMsgId;
   USR_TPLUG_WebDiscreteTlm_t  TlmMsg;

   /*
   ** The plugin test treats the 4 integers as a 4-bit integer that is
   ** incremented from 0..15 
   */
   
   uint16  TestData;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} TPLUG_WEB_DISCRETE_Class_t;


/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);
static void PluginTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

static TPLUG_WEB_DISCRETE_Class_t          TPlugWebDiscrete;
static USR_TPLUG_WebDiscreteTlm_Payload_t  DiscreteTlm;       /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                 Data                                   core-json          length of query      */
   /* Address,            Length,  Updated, Data Type,  Float,  query string,        string(exclude '\0') */
   
   { &DiscreteTlm.Item_1,   2,     false,   JSONNumber, false,  { "integer.item-1", (sizeof("integer.item-1")-1)} },
   { &DiscreteTlm.Item_2,   2,     false,   JSONNumber, false,  { "integer.item-2", (sizeof("integer.item-2")-1)} },
   { &DiscreteTlm.Item_3,   2,     false,   JSONNumber, false,  { "integer.item-3", (sizeof("integer.item-3")-1)} },
   { &DiscreteTlm.Item_4,   2,     false,   JSONNumber, false,  { "integer.item-4", (sizeof("integer.item-4")-1)} }
   
};

static const char *NullDiscreteMsg = "{\"integer\":{\"item-1\": 0,\"item-2\": 0,\"item-3\": 0,\"item-4\": 0}}";


/******************************************************************************
** Function: TPLUG_WEB_DISCRETE_Constructor
**
** Initialize the TPLUG_WEB discrete object
**
** Notes:
**   None
**
*/
void TPLUG_WEB_DISCRETE_Constructor(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   memset(&TPlugWebDiscrete, 0, sizeof(TPLUG_WEB_DISCRETE_Class_t));

   TPlugWebDiscrete.JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   TPlugWebDiscrete.SbMsgId = JMSG_TOPIC_TBL_RegisterPlugin(TopicPlugin, CfeToJson, JsonToCfe, PluginTest);

   CFE_MSG_Init(CFE_MSG_PTR(TPlugWebDiscrete.TlmMsg), TPlugWebDiscrete.SbMsgId, sizeof(USR_TPLUG_WebDiscreteTlm_t));

   CFE_EVS_SendEvent(TPLUG_WEB_DISCRETE_CONSTRUCTOR_EID, CFE_EVS_EventType_INFORMATION,
                     "Constructed TPLUG Web Discrete topic 0x%04X(%d)",
                     CFE_SB_MsgIdToValue(TPlugWebDiscrete.SbMsgId), CFE_SB_MsgIdToValue(TPlugWebDiscrete.SbMsgId)); 
                     
} /* End TPLUG_WEB_DISCRETE_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE discrete message to a MQTT JSON topic message 
**
** Notes:
**   1. Signature must match TPLUG_WEB_TBL_CfeToJson_t
**
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const USR_TPLUG_WebDiscreteTlm_Payload_t *DiscreteMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, USR_TPLUG_WebDiscreteTlm_t);

   *JsonMsgPayload = NullDiscreteMsg;
   
   PayloadLen = sprintf(TPlugWebDiscrete.JsonMsgPayload,
                "{\"integer\":{\"item-1\": %1d,\"item-2\": %1d,\"item-3\": %1d,\"item-4\": %1d}}",
                DiscreteMsg->Item_1, DiscreteMsg->Item_2, DiscreteMsg->Item_3, DiscreteMsg->Item_4);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = TPlugWebDiscrete.JsonMsgPayload;
   
      ++TPlugWebDiscrete.CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a MQTT JSON discrete topic message to a cFE discrete message 
**
** Notes:
**   1. Signature must match TPLUG_WEB_TBL_JsonToCfe_t
**   2. Messages that can be pasted into MQTT broker for testing
**      {"integer":{"item-1": 1,"item-2": 2,"item-3": 3,"item-4": 4}}
**
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&TPlugWebDiscrete.TlmMsg;

      ++TPlugWebDiscrete.JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send SB discrete topic messages on SB that are read back by MQTT
** and cause MQTT messages to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
** Test plugin by converting a JMSG discrete SB telemetry message to an JMSG
** discrete message 
**
** Notes:
**   1. KIT_TO's packet table entry for JMSG_DISCRETE_PLUGIN_TOPICID must have
**      the forward attribute set to true.
**   2. The jmsg_topics.json entry must be set to subscribe to
**      KIT_TO_PUB_WRAPPED_CMD_TOPICID
**   3. A walking bit pattern is used in the discrete data to help validation.
*/
static void PluginTest(bool Init, int16 Param)
{

   if (Init)
   {
      TPlugWebDiscrete.TestData = 0;  
      CFE_EVS_SendEvent(TPLUG_WEB_DISCRETE_INIT_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "MQTT Discrete plugin topic test started");

   }
   else
   {
   
      CFE_EVS_SendEvent(TPLUG_WEB_DISCRETE_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                        "MQTT Discrete plugin topic test sending 0x%04X", TPlugWebDiscrete.TestData);
                        
      TPlugWebDiscrete.TlmMsg.Payload.Item_1 =  (TPlugWebDiscrete.TestData & 0x01);
      TPlugWebDiscrete.TlmMsg.Payload.Item_2 = ((TPlugWebDiscrete.TestData & 0x02) >> 1);
      TPlugWebDiscrete.TlmMsg.Payload.Item_3 = ((TPlugWebDiscrete.TestData & 0x04) >> 2);
      TPlugWebDiscrete.TlmMsg.Payload.Item_4 = ((TPlugWebDiscrete.TestData & 0x08) >> 3);
      
      if (++TPlugWebDiscrete.TestData > 15)
      {
         TPlugWebDiscrete.TestData = 0;
      }
      
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(TPlugWebDiscrete.TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(TPlugWebDiscrete.TlmMsg.TelemetryHeader), true);
   
} /* End SbMsgTest() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&TPlugWebDiscrete.TlmMsg.Payload, 0, sizeof(USR_TPLUG_WebDiscreteTlm_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, TPlugWebDiscrete.JsonObjCnt, JsonMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(TPLUG_WEB_DISCRETE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "TPLUG WEB Discrete LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == TPlugWebDiscrete.JsonObjCnt)
   {
      memcpy(&TPlugWebDiscrete.TlmMsg.Payload, &DiscreteTlm, sizeof(USR_TPLUG_WebDiscreteTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(TPLUG_WEB_DISCRETE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing TPLUG WEB discrete, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)TPlugWebDiscrete.JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

