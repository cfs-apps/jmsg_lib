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
**   1. SB messages are not interpreted. A single JMSG topic is used
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
                                CFE_SB_MsgId_t WrappedTlmMid)
{

   JMsgTopicTlm = JMsgTopicTlmPtr;
   memset(JMsgTopicTlm, 0, sizeof(JMSG_TOPIC_TLM_Class_t));

   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
   
OS_printf(">>>>JMSG_TOPIC_TLM_Constructor()-WrappedTlmMid = %d\n",(int)CFE_SB_MsgIdToValue(WrappedTlmMid));
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicTlm->WrappedTlmMsg), WrappedTlmMid, sizeof(KIT_TO_WrappedSbMsgTlm_t));
         
} /* End JMSG_TOPIC_TLM_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a text encoded JMSG JSON message from the payload of the SB message.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. The SB message's payload is another complete SB message (includes headers)
**   3. The network app must ensure the JMsgPayload buffer is large enough
**      to hold the largest telemetry message.
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
      if (MsgSize < JMSG_PLATFORM_TOPIC_SB_MSG_MAX_LEN)
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
** TODO: Implement jmsg_topic_tlm plugin test 
**
** Notes:
**   1. The JsonToCfe() callback function is called directly so a test could
**      be performed without requiring a JMSG network app being installed and
**      KIT_TO's packet forwarding doesn't have to be configured. The downside
**      is that the test isn't aligned with a practical use case and performs
**      limited verification.  
**
*/
static void PluginTest(bool Init, int16 Param)
{

   static const char *TestTlm = APP_C_DEMO_NOOP_HEXTXT;
   CFE_MSG_Message_t *CfeMsg = NULL;
   
   
   if (Init)
   {
         
      JMsgTopicTlm->SbTestCnt = 0;
      
      CFE_EVS_SendEvent(JMSG_TOPIC_TLM_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG telemetry plugin topic test started");
   }
   else
   {
      // CfeMsg set to wrapped telemetry message
      if (JsonToCfe(&CfeMsg, TestTlm, strlen(TestTlm)))
      {
CFE_MSG_ApId_t ApId;
CFE_MSG_GetApId(CfeMsg, &ApId);
OS_printf("Sending SB message 0x%04X(%d)\n", (int)ApId, (int)ApId);
const uint8 *Buf = (const uint8 *)&CfeMsg;
OS_printf("SB message 0x%02X%02X 0x%02X%02X\n",Buf[0],Buf[1],Buf[2],Buf[3]);

         CFE_SB_TransmitMsg(CfeMsg, true);
      }
      JMsgTopicTlm->SbTestCnt++;
   }
   
} /* End PluginTest() */


