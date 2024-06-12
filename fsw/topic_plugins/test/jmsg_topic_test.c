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
**   Manage JSON Message Test topic
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "jmsg_topic_test.h"
#include "jmsg_test_eds_typedefs.h"

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

static JMSG_TOPIC_TEST_Class_t* JMsgTopicTest = NULL;

static JMSG_TEST_PluginTlm_Payload_t  Test; /* Working buffer for loads */
   
static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Data                                 core-json       length of query      */
   /* Data Address,   Length,   Updated, Data Type,  Float,  query string, string(exclude '\0') */
   
   { &Test.Int32,       4,      false,   JSONNumber, false,  { "int32",    (sizeof("int32")-1)} },
   { &Test.Float,       4,      false,   JSONNumber, true,   { "float",    (sizeof("float")-1)} }
   
};

static const char *NullTestMsg = "{\"int32\": 0,\"float\": 0.0}";


/******************************************************************************
** Function: JMSG_TOPIC_TEST_Constructor
**
** Initialize the JSON Test topic
**
** Notes:
**   None
**
*/
void JMSG_TOPIC_TEST_Constructor(JMSG_TOPIC_TEST_Class_t *JMsgTopicTestPtr, 
                                 JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                 CFE_SB_MsgId_t TlmMsgMid)
{

   JMsgTopicTest = JMsgTopicTestPtr;
   memset(JMsgTopicTest, 0, sizeof(JMSG_TOPIC_TEST_Class_t));

   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   JMsgTopicTest->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicTest->TlmMsg), TlmMsgMid, sizeof(JMSG_TEST_PluginTlmMsg_t));
      
} /* End JMSG_TOPIC_TEST_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE test message to a JSON topic message 
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. Messages that can be pasted into external data source for testing:
**      {"int32": 1, "float": 2.0}
**
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const JMSG_TEST_PluginTlm_Payload_t *TestMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, JMSG_TEST_PluginTlmMsg_t);

   *JsonMsgPayload = NullTestMsg;
   
   PayloadLen = sprintf(JMsgTopicTest->JsonMsgPayload,
                "{\"int32\": %1d,\"float\": %f}",
                TestMsg->Int32, TestMsg->Float);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = JMsgTopicTest->JsonMsgPayload;
   
      ++JMsgTopicTest->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON test topic message to a cFE test message 
**
** Notes:
**   1.  Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&JMsgTopicTest->TlmMsg;

      ++JMsgTopicTest->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Generate and send plugin test topic messages on the SB that are read back
** by JMSG and cause JSON messages to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
** Test plugin by converting a JMSG test SB telemetry message to an JMSG
** test message 
**
** Notes:
**   1. KIT_TO's packet table entry for JMSG_TEST_PLUGIN_TOPICID must have
**      the forward attribute set to true.
**   2. The jmsg_topics.json entry must be set to subscribe to
**      KIT_TO_PUB_WRAPPED_CMD_TOPICID
**   3. Incrementing values are used in the test data to help validation.
*/
static void PluginTest(bool Init, int16 Param)
{

   JMSG_TEST_PluginTlm_Payload_t *Payload = &JMsgTopicTest->TlmMsg.Payload;
   
   memset(Payload, 0, sizeof(JMSG_TEST_PluginTlmMsg_t));

   if (Init)
   {

      JMsgTopicTest->PluginTestCnt = 1;      
      CFE_EVS_SendEvent(JMSG_TOPIC_TEST_INIT_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Test topic test started");
   }
   else
   {   
      JMsgTopicTest->PluginTestCnt++;
   }
  
   Payload->Int32 = JMsgTopicTest->PluginTestCnt;
   Payload->Float = (float)JMsgTopicTest->PluginTestCnt;
   
   CFE_EVS_SendEvent(JMSG_TOPIC_TEST_PLUGIN_TEST_EID, CFE_EVS_EventType_DEBUG,
                     "Test topic test sending %d, %f", Payload->Int32, Payload->Float);
                           
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicTest->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicTest->TlmMsg.TelemetryHeader), true);
   
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

   memset(&JMsgTopicTest->TlmMsg.Payload, 0, sizeof(JMSG_TEST_PluginTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicTest->JsonObjCnt, 
                                   JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(JMSG_TOPIC_TEST_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Test LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == JMsgTopicTest->JsonObjCnt)
   {
      memcpy(&JMsgTopicTest->TlmMsg.Payload, &Test, sizeof(JMSG_TEST_PluginTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TEST_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing test topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicTest->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

