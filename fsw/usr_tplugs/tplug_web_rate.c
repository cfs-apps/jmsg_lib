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
**   Manage JMSG MQTT discrete plugin topic
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "tplug_web_rate.h"
#include "usr_tplug_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/


#define TPLUG_WEB_RATE_CONSTRUCTOR_EID       (USR_TPLUG_BaseEid_WebRate + 0)
#define TPLUG_WEB_RATE_INIT_SB_MSG_TEST_EID  (USR_TPLUG_BaseEid_WebRate + 1)
#define TPLUG_WEB_RATE_SB_MSG_TEST_EID       (USR_TPLUG_BaseEid_WebRate + 2)
#define TPLUG_WEB_RATE_LOAD_JSON_DATA_EID    (USR_TPLUG_BaseEid_WebRate + 3)
#define TPLUG_WEB_RATE_JSON_TO_CCSDS_ERR_EID (USR_TPLUG_BaseEid_WebRate + 4)


/**********************/
/** Type Definitions **/
/**********************/

typedef enum 
{

  TPLUG_WEB_RATE_TEST_AXIS_X = 1,
  TPLUG_WEB_RATE_TEST_AXIS_Y = 2,
  TPLUG_WEB_RATE_TEST_AXIS_Z = 3
  
} TPLUG_WEB_RATE_TestAxis_t;

/******************************************************************************
** Telemetry
** 
** TPLUG_WEB_RatePluginTlm_t & TPLUG_WEB_RatePluginTlm_Payload_t defined in EDS
*/

typedef struct
{

   /*
   ** Rate Telemetry
   */
   
   char  JsonMsgPayload[1024];
   CFE_SB_MsgId_t          SbMsgId;
   USR_TPLUG_WebRateTlm_t  TlmMsg;

   /*
   ** SB test puts rate on a single axis for N cycles
   */
   
   TPLUG_WEB_RATE_TestAxis_t  TestAxis;
   uint16   TestAxisCycleCnt;
   uint16   TestAxisCycleLim;
   float    TestAxisDefRate;
   float    TestAxisRate;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} TPLUG_WEB_RATE_Class_t;


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

static TPLUG_WEB_RATE_Class_t         TPlugWebRate;
static USR_TPLUG_WebRateTlm_Payload_t RateTlm;       /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Data         Data                                    core-json       length of query     */
   /* Address,    Length,  Updated,  Data Type,  Float,  query string,   string(exclude '\0')  */
   
   { &RateTlm.X,    4,     false,    JSONNumber, true,   { "rate.x",    (sizeof("rate.x")-1)} },
   { &RateTlm.Y,    4,     false,    JSONNumber, true,   { "rate.y",    (sizeof("rate.y")-1)} },
   { &RateTlm.Z,    4,     false,    JSONNumber, true,   { "rate.z",    (sizeof("rate.z")-1)} }
   
};

static const char *NullRateMsg = "{\"rate\":{\"x\": 0.0,\"y\": 0.0,\"z\": 0.0}}";


/******************************************************************************
** Function: TPLUG_WEB_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
#define DEG_PER_SEC_IN_RADIANS 0.0174533
void TPLUG_WEB_RATE_Constructor(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   memset(&TPlugWebRate, 0, sizeof(TPLUG_WEB_RATE_Class_t));

   TPlugWebRate.JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));

   TPlugWebRate.SbMsgId = JMSG_TOPIC_TBL_RegisterPlugin(TopicPlugin, CfeToJson, JsonToCfe, PluginTest);

   CFE_MSG_Init(CFE_MSG_PTR(TPlugWebRate.TlmMsg), TPlugWebRate.SbMsgId, sizeof(USR_TPLUG_WebRateTlm_t));
   
   /*
   ** Cycle counts are used by sim to switch axis
   ** Need to coordinate with topic message SB pend time
   */
   TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_X;
   TPlugWebRate.TestAxisCycleCnt = 0;
   TPlugWebRate.TestAxisDefRate  = 0.0087265;  /* 0.5 deg/sec in radians with 250ms pend */
   TPlugWebRate.TestAxisDefRate  = 0.0174533;  /* 1.0 deg/sec in radians with 250ms pend */
   TPlugWebRate.TestAxisDefRate  *= 7.5;       /* 7.5 deg/s so 24 cycles for 90 deg */
   TPlugWebRate.TestAxisRate     = TPlugWebRate.TestAxisDefRate;
   TPlugWebRate.TestAxisCycleLim = 12*4;       /* Rotate 90 deg on each axis */

   CFE_EVS_SendEvent(TPLUG_WEB_RATE_CONSTRUCTOR_EID, CFE_EVS_EventType_INFORMATION,
                     "Constructed TPLUG Web Rate topic 0x%04X(%d)",
                     CFE_SB_MsgIdToValue(TPlugWebRate.SbMsgId), CFE_SB_MsgIdToValue(TPlugWebRate.SbMsgId)); 
                     
} /* End TPLUG_WEB_RATE_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
** Notes:
**   1.  Signature must match TPLUG_WEB_TBL_CfeToJson_t
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const USR_TPLUG_WebRateTlm_Payload_t *RateMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, USR_TPLUG_WebRateTlm_t);

   *JsonMsgPayload = NullRateMsg;
   
   PayloadLen = sprintf(TPlugWebRate.JsonMsgPayload,
                "{\"rate\":{\"x\": %0.6f,\"y\": %0.6f,\"z\": %0.6f}}",
                RateMsg->X, RateMsg->Y, RateMsg->Z);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = TPlugWebRate.JsonMsgPayload;
      TPlugWebRate.CfeToJsonCnt++;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON rate topic message to a cFE rate message 
**
** Notes:
**   1. Signature must match TPLUG_WEB_TBL_JsonToCfe_t
**   2. Messages that can be pasted into MQTT broker for testing
**      {"rate":{"x": 1.1, "y": 2.2, "z": 3.3}}
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&TPlugWebRate.TlmMsg;

      ++TPlugWebRate.JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send SB rate topic messages on SB that are read back by MQTT
** and cause MQTT messages to be generated from the SB messages. 
**
** Notes:
**   1. Param is used to scale the default test rate and change the sign
**      Increase rate:   2 <= Param <= 10
**      Deccrease rate: 12 <= Param <= 20
**
*/
static void PluginTest(bool Init, int16 Param)
{

   if (Init)
   {

      TPlugWebRate.TestAxisRate = TPlugWebRate.TestAxisDefRate;
      
      if (Param >= 2 && Param <= 10)
      {
         TPlugWebRate.TestAxisRate *= (float)Param;
      }
      else if (Param >= 12 && Param <= 20)
      {         
         TPlugWebRate.TestAxisRate /= ((float)Param - 10.0);
      }
      TPlugWebRate.TlmMsg.Payload.X = TPlugWebRate.TestAxisRate;
      TPlugWebRate.TlmMsg.Payload.Y = 0.0;
      TPlugWebRate.TlmMsg.Payload.Z = 0.0;
      TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_X;
      TPlugWebRate.TestAxisCycleCnt = 0;

      CFE_EVS_SendEvent(TPLUG_WEB_RATE_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "TPLUG_WEB Rate topic test started with axis rate %6.2f", TPlugWebRate.TestAxisRate);

   }
   else
   {
      switch (TPlugWebRate.TestAxis)
      {
         case TPLUG_WEB_RATE_TEST_AXIS_X:
            if (++TPlugWebRate.TestAxisCycleCnt > TPlugWebRate.TestAxisCycleLim)
            {
               TPlugWebRate.TestAxisCycleCnt = 0;
               TPlugWebRate.TlmMsg.Payload.X = 0.0;
               TPlugWebRate.TlmMsg.Payload.Y = TPlugWebRate.TestAxisRate;
               TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_Y;
            }
            break;
         case TPLUG_WEB_RATE_TEST_AXIS_Y:
            if (++TPlugWebRate.TestAxisCycleCnt > TPlugWebRate.TestAxisCycleLim)
            {
               TPlugWebRate.TestAxisCycleCnt = 0;
               TPlugWebRate.TlmMsg.Payload.Y = 0.0;
               TPlugWebRate.TlmMsg.Payload.Z = TPlugWebRate.TestAxisRate;
               TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_Z;
            }
            break;
         case TPLUG_WEB_RATE_TEST_AXIS_Z:
            if (++TPlugWebRate.TestAxisCycleCnt > TPlugWebRate.TestAxisCycleLim)
            {
               TPlugWebRate.TestAxisCycleCnt = 0;
               TPlugWebRate.TlmMsg.Payload.Z = 0.0;
               TPlugWebRate.TlmMsg.Payload.X = TPlugWebRate.TestAxisRate;
               TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_X;
            }
            break;
         default:
            TPlugWebRate.TestAxisCycleCnt = 0;
            TPlugWebRate.TlmMsg.Payload.X = TPlugWebRate.TestAxisRate;
            TPlugWebRate.TlmMsg.Payload.Y = 0.0;
            TPlugWebRate.TlmMsg.Payload.Z = 0.0;
            TPlugWebRate.TestAxis         = TPLUG_WEB_RATE_TEST_AXIS_X;
            break;
         
      } /* End axis switch */
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(TPlugWebRate.TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(TPlugWebRate.TlmMsg.TelemetryHeader), true);
   
} /* End PluginTest() */


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

   memset(&TPlugWebRate.TlmMsg.Payload, 0, sizeof(USR_TPLUG_WebRateTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, TPlugWebRate.JsonObjCnt, 
                                  JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(TPLUG_WEB_RATE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "TPLUG_WEB Rate LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == TPlugWebRate.JsonObjCnt)
   {
      memcpy(&TPlugWebRate.TlmMsg.Payload, &RateTlm, sizeof(USR_TPLUG_WebRateTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(TPLUG_WEB_RATE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing TPLUG_WEB Rate message, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)TPlugWebRate.JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

