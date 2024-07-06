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
**   Define Raspberry Pi Demo plugin topic
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "tplug_rpi_demo.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/

/*
** Event Message IDs
*/

#define TPLUG_RPI_DEMO_CONSTRUCTOR_EID       (USR_TPLUG_BaseEid_RpiDemo + 0)
#define TPLUG_RPI_DEMO_INIT_PLUGIN_TEST_EID  (USR_TPLUG_BaseEid_RpiDemo + 1)
#define TPLUG_RPI_DEMO_PLUGIN_TEST_EID       (USR_TPLUG_BaseEid_RpiDemo + 2)
#define TPLUG_RPI_DEMO_LOAD_JSON_DATA_EID    (USR_TPLUG_BaseEid_RpiDemo + 3)
#define TPLUG_RPI_DEMO_JSON_TO_CCSDS_ERR_EID (USR_TPLUG_BaseEid_RpiDemo + 4)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** TPLUG_RPI_DemoTlmMsg_t & TPLUG_RPI_DemoTlmMsg_Payload_t are defined in EDS
*/


/******************************************************************************
** Class
** 
*/

typedef struct
{
    float   RateX;
    float   RateY;
    float   RateZ;
    int32   Lux;

} TPLUG_RPI_DEMO_TestData_t;

typedef struct
{

   /*
   ** Discrete Telemetry
   */
   
   char            JsonMsgPayload[1024];
   CFE_SB_MsgId_t  SbMsgId;
   USR_TPLUG_RpiDemoTlm_t  TlmMsg;

   /*
   ** The plugin test treats the 4 integers as a 4-bit integer that is
   ** incremented from 0..15 
   */
   
   TPLUG_RPI_DEMO_TestData_t TestData;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} TPLUG_RPI_DEMO_Class_t;


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

static TPLUG_RPI_DEMO_Class_t          TPlugRpiDemo;
static USR_TPLUG_RpiDemoTlm_Payload_t  RpiDemoTlm;    /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data                 Data                                   core-json         length of query      */
   /* Address,            Length,  Updated, Data Type,  Float,  query string,       string(exclude '\0') */
   
   { &RpiDemoTlm.Rate_X,   4,     false,   JSONNumber, true,   { "rpi-demo.rate-x", (sizeof("rpi-demo.rate-x")-1)} },
   { &RpiDemoTlm.Rate_Y,   4,     false,   JSONNumber, true,   { "rpi-demo.rate-y", (sizeof("rpi-demo.rate-y")-1)} },
   { &RpiDemoTlm.Rate_Z,   4,     false,   JSONNumber, true,   { "rpi-demo.rate-z", (sizeof("rpi-demo.rate-z")-1)} },
   { &RpiDemoTlm.Lux,      4,     false,   JSONNumber, false,  { "rpi-demo.lux",    (sizeof("rpi-demo.lux")-1)} }
   
};

static const char *NullRpiDemoMsg = "{\"rpi-demo\":{\"rate-x\": 0.0,\"rate-y\": 0.0,\"rate-z\": 0.0,\"lux\": 0}}";


/******************************************************************************
** Function: TPLUG_RPI_DEMO_Constructor
**
** Initialize the TPLUG_RPI demo topic plugin 
**
** Notes:
**   None
**
*/
void TPLUG_RPI_DEMO_Constructor(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   memset(&TPlugRpiDemo, 0, sizeof(TPLUG_RPI_DEMO_Class_t));

   TPlugRpiDemo.JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));

   /* TODO: Confirm no race ocndition if JSON being produced while this initialization is taking place becuase message init occurs after the callback subscription */ 
   
   TPlugRpiDemo.SbMsgId = JMSG_TOPIC_TBL_RegisterPlugin(TopicPlugin, CfeToJson, JsonToCfe, PluginTest);

   CFE_MSG_Init(CFE_MSG_PTR(TPlugRpiDemo.TlmMsg), TPlugRpiDemo.SbMsgId, sizeof(USR_TPLUG_RpiDemoTlm_t));
      
   CFE_EVS_SendEvent(TPLUG_RPI_DEMO_CONSTRUCTOR_EID, CFE_EVS_EventType_INFORMATION,
                     "Constructed TPLUG RPI Demo topic 0x%04X(%d)",
                     CFE_SB_MsgIdToValue(TPlugRpiDemo.SbMsgId),CFE_SB_MsgIdToValue(TPlugRpiDemo.SbMsgId));   
      
} /* End TPLUG_RPI_DEMO_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE RPI Demo message to a UDP JSON topic message 
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_CfeToJson_t
**
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const USR_TPLUG_RpiDemoTlm_Payload_t *RpiDemoMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, USR_TPLUG_RpiDemoTlm_t);

   *JsonMsgPayload = NullRpiDemoMsg;
   
   PayloadLen = sprintf(TPlugRpiDemo.JsonMsgPayload,
                "{\"rpi-demo\":{\"rate-x\": %f,\"rate-y\": %f,\"rate-z\": %f,\"lux\": %1d}}",
                RpiDemoMsg->Rate_X, RpiDemoMsg->Rate_Y, RpiDemoMsg->Rate_Z, RpiDemoMsg->Lux);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = TPlugRpiDemo.JsonMsgPayload;
   
      ++TPlugRpiDemo.CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a UDP JSON RPI Demo topic message to a cFE SB RPI Demo message 
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
**   2. Messages that can be sent over UDP for testing
**      {"rpi-demo":{"rate-x": 1.0, "rate-y": 2.0, "rate-z": 3.0, "lux": 456}}
**
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&TPlugRpiDemo.TlmMsg;

      ++TPlugRpiDemo.JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send SB RPI Demo topic messages on SB that are read back by
** JMSG_UDP and cause JMSG's to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
*/
static void PluginTest(bool Init, int16 Param)
{

   if (Init)
   {
      
      TPlugRpiDemo.TestData.RateX = 1.0;
      TPlugRpiDemo.TestData.RateY = 2.0;
      TPlugRpiDemo.TestData.RateZ = 3.0;
      TPlugRpiDemo.TestData.Lux   = 100;

      CFE_EVS_SendEvent(TPLUG_RPI_DEMO_INIT_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "TPLUG RPI Demo plugin topic test started");

   }
   else
   {
                    
      TPlugRpiDemo.TestData.RateX += 1.0;
      TPlugRpiDemo.TestData.RateY += 1.0;
      TPlugRpiDemo.TestData.RateZ += 1.0;
      TPlugRpiDemo.TestData.Lux++;
      
   }

   
   CFE_EVS_SendEvent(TPLUG_RPI_DEMO_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                        "TPLUG RPI Demo plugin topic test sending: %.4f, %.4f, %.4f, %d ", 
                        TPlugRpiDemo.TestData.RateX, TPlugRpiDemo.TestData.RateY,
                        TPlugRpiDemo.TestData.RateZ, TPlugRpiDemo.TestData.Lux);
                        
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(TPlugRpiDemo.TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(TPlugRpiDemo.TlmMsg.TelemetryHeader), true);
   
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

   memset(&TPlugRpiDemo.TlmMsg.Payload, 0, sizeof(USR_TPLUG_RpiDemoTlm_Payload_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, TPlugRpiDemo.JsonObjCnt, JsonMsgPayload, PayloadLen);
   
   CFE_EVS_SendEvent(TPLUG_RPI_DEMO_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "TPLUG RPI Demo LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == TPlugRpiDemo.JsonObjCnt)
   {
      memcpy(&TPlugRpiDemo.TlmMsg.Payload, &RpiDemoTlm, sizeof(USR_TPLUG_RpiDemoTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(TPLUG_RPI_DEMO_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing TPLUG RPI Demo, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)TPlugRpiDemo.JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

