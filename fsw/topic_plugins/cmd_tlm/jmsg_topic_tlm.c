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
**   Manage the conversion of JSON telemetry messages
**
** Notes:
**   1. SB messages are not interpretted. A single JMSG topic is used
**      to transport any SB message as its payload
**
*/

/*
** Includes
*/

#include "jmsg_topic_tlm.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen);
static void PluginTest(bool Init, int16 Param);

/**********************/
/** Global File Data **/
/**********************/

static JMSG_TOPIC_TLM_Class_t* JMsgTopicTlm = NULL;


/******************************************************************************
** Function: JMSG_TOPIC_TLM_Constructor
**
** Initialize the telemetry topic
**
** Notes:
**   1. The test telemetry message is used for the built in test.
**
*/
void JMSG_TOPIC_TLM_Constructor(JMSG_TOPIC_TLM_Class_t *JMsgTopicTlmPtr,
                                JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t WrappedTlmMid, CFE_SB_MsgId_t TestPluginTlmMid)
{

   JMsgTopicTlm = JMsgTopicTlmPtr;
   memset(JMsgTopicTlm, 0, sizeof(JMSG_TOPIC_TLM_Class_t));

   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
   JMsgTopicTlm->TestPluginTlmMsgLen = sizeof(JMSG_TEST_PluginTlmMsg_t);
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicTlm->WrappedTlmMsg), WrappedTlmMid, sizeof(KIT_TO_WrappedSbMsgTlm_t));
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicTlm->TestPluginTlmMsg), TestPluginTlmMid, JMsgTopicTlm->TestPluginTlmMsgLen);
         
} /* End JMSG_TOPIC_TLM_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a text encoded JMSG JSON message from the payload of the SB message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. The SB message's payload is another complete SB message (includes headers)
**
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   CFE_Status_t   CfeStatus;
   CFE_MSG_Size_t MsgSize;
   const KIT_TO_WrappedSbMsgTlm_Payload_t *PayloadSbMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, KIT_TO_WrappedSbMsgTlm_t);
   
   *JMsgPayload = JMsgTopicTlm->JMsgPayload;

   CfeStatus = CFE_MSG_GetSize((CFE_MSG_Message_t *)PayloadSbMsg, &MsgSize);
   if (CfeStatus == CFE_SUCCESS)
   {
       if (MsgSize < JMSG_USR_TOPIC_SB_MSG_MAX_LEN)
       {
           PktUtil_HexEncode(JMsgTopicTlm->JMsgPayload, (uint8 *)PayloadSbMsg, MsgSize, true);
           JMsgTopicTlm->CfeToJMsgCnt++;
           RetStatus = true;
       }
   }
      
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Normally this function would convert a JSON topic message to a cFS SB
** message. In this case the JMSG payload is a SB message. The SB message
** is decoded and copied into an SB message and sent to TO that expects a
** wrapped message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**   2. Encoded discrete message that can be pasted into JMSG source 
**      (eg MQTT broker) for testing:
**      00016B00030049001D0000005A890F00FB99000001000000000000000000000000000000
**      00016B00030048001D0000005A890F007D59000000000000000000000000000001000000
**      00016B00030047001D0000005A890F002A19000000000000000000000100000000000000
**      00016B00030046001D00000059890F001A9A000000000000010000000000000000000000
**
**      00016B00030000001D000000000000000000000001000000000000000000000000000000
**      00016B00030000001D000000000000000000000000000000010000000000000000000000
**      00016B00030000001D000000000000000000000000000000000000000100000000000000
**      00016B00030000001D000000000000000000000000000000000000000000000001000000
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   KIT_TO_WrappedSbMsgTlm_Payload_t *SbMsgPayload = &(JMsgTopicTlm->WrappedTlmMsg.Payload);
   size_t DecodedLen;
   
   DecodedLen = PktUtil_HexDecode((uint8 *)SbMsgPayload, JMsgPayload, PayloadLen);
   if (DecodedLen > 0)
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TLM_HEX_DECODE_EID, CFE_EVS_EventType_DEBUG,
                        "JMSG message successfully decoded. JMSG len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
      *CfeMsg = CFE_MSG_PTR(JMsgTopicTlm->WrappedTlmMsg);
      JMsgTopicTlm->JMsgToCfeCnt++;
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TLM_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "JMSG message decode failed. JMSG len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
   }
   
   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Test plugin by converting a JMSG test SB telemetry message to an JMSG test
** JSON message 
**
** Notes:
**   1. KIT_TO's packet table entry for JMSG_TEST_PLUGIN_TOPICID must have
**      the forward attribute set to true.
**   2. The jmsg_topics.json entry must be set to subscribe to
**      KIT_TO_PUB_WRAPPED_TLM_TOPICID
**   3. A walking bit pattern is used in the dsicrete data to help validation.
**
*/
static void PluginTest(bool Init, int16 Param)
{
   
   JMSG_TEST_PluginTlm_Payload_t *Payload = &JMsgTopicTlm->TestPluginTlmMsg.Payload;
   
   memset(Payload, 0, sizeof(JMSG_TEST_PluginTlmMsg_t));

   if (Init)
   {

      JMsgTopicTlm->SbTestCnt = 1;      
      CFE_EVS_SendEvent(JMSG_TOPIC_TLM_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Telemetry topic test started");
   }
   else
   {   
      JMsgTopicTlm->SbTestCnt++;
   }
  
   Payload->Int32 = JMsgTopicTlm->SbTestCnt;
   Payload->Float = (float)JMsgTopicTlm->SbTestCnt;
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicTlm->TestPluginTlmMsg.TelemetryHeader));   
   memcpy(&(JMsgTopicTlm->WrappedTlmMsg.Payload), &JMsgTopicTlm->TestPluginTlmMsg, JMsgTopicTlm->TestPluginTlmMsgLen);
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicTlm->WrappedTlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicTlm->WrappedTlmMsg.TelemetryHeader), true);

} /* End PluginTest() */


