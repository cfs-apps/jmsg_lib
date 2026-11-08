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
**   Manage the conversion of JSON messages containing binary command
**   messages encoded with hex text
**
** Notes:
**   1. This only supports a remote commanding use case which is 
**      converting a binary encoded command message into a SB message.
**   2. JsonToCfe() performs the same EDS processing as CI_LAB.
**
*/

/*
** Includes
*/

#include "cfe_config.h"
#include "edslib_global.h"
#include "edslib_datatypedb.h"
#include "edslib_intfdb.h"
#include "cfe_missionlib_api.h"
#include "cfe_missionlib_runtime.h"
#include "cfe_mission_eds_parameters.h"
#include "cfe_mission_eds_interface_parameters.h"

#include "jmsg_topic_cmd.h"

 
/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen);
static void PluginTest(bool Init, int16 Param);

static CFE_Status_t DecodeInputHeader(CFE_SB_Buffer_t         *IngestBufPtr,
                                      const void              *SourceBuffer,
                                      const EdsLib_SizeInfo_t *MaxSize,
                                      EdsLib_SizeInfo_t       *ProcessedSize);
static CFE_Status_t DecodeInputMessage(void *SourceBuffer, size_t SourceSize, 
                                       CFE_SB_Buffer_t **DestBufferOut);
static CFE_Status_t DecodeInputPayload(CFE_SB_Buffer_t         *IngestBufPtr,
                                       EdsLib_Id_t              ParentIntfId,
                                       const void              *SourceBuffer,
                                       const EdsLib_SizeInfo_t *MaxSize,
                                       EdsLib_SizeInfo_t       *ProcessedSize);
                                       
/**********************/
/** Global File Data **/
/**********************/

static JMSG_TOPIC_CMD_Class_t* JMsgTopicCmd = NULL;


/******************************************************************************
** Function: JMSG_TOPIC_CMD_Constructor
**
** Initialize the command topic
**
** Notes:
**   1. The test command message is used for the built in test.
**
*/
void JMSG_TOPIC_CMD_Constructor(JMSG_TOPIC_CMD_Class_t *JMsgTopicCmdPtr,
                                JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl)
{

   JMsgTopicCmd = JMsgTopicCmdPtr;
   memset(JMsgTopicCmd, 0, sizeof(JMSG_TOPIC_CMD_Class_t));

   JMsgTopicCmd->SbBufPtr = NULL;

   PluginFuncTbl->CfeToJson  = CfeToJson;
   PluginFuncTbl->JsonToCfe  = JsonToCfe;  
   PluginFuncTbl->PluginTest = PluginTest;
            
} /* End JMSG_TOPIC_CMD_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Unsupported functionality.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_CfeToJson_t
**   2. This use case is unsupported because it is beyond Basecamp's scope. It
**      would mean a cFE target is sending a command to another cFS target via
**      a JSON message over some communication path like UDP or a MQTT broker.
**
*/
static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;

   CFE_MSG_ApId_t ApId;
   CFE_MSG_Size_t MsgSize;
   
   CFE_MSG_GetApId(CfeMsg, &ApId);
   CFE_MSG_GetSize(CfeMsg, &MsgSize);
   
   CFE_EVS_SendEvent(JMSG_TOPIC_CMD_CFE2JSON_EID, CFE_EVS_EventType_ERROR,
                     "Command plugin received unexpect cFS command message with ApId 0x%04X and size %d",
                     (int)ApId, (int)MsgSize);
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: DecodeInputHeader
**
** Decode the header from the incoming message. All incoming messages must
** have the standard command header.
**
** Notes:
**   1. This taken directly from CI_LAB_DecodeInputHeader() and adapted for
**      the JMSG context.
**
*/
static CFE_Status_t DecodeInputHeader(CFE_SB_Buffer_t         *IngestBufPtr,
                                      const void              *SourceBuffer,
                                      const EdsLib_SizeInfo_t *MaxSize,
                                      EdsLib_SizeInfo_t       *ProcessedSize)
{
   
   EdsLib_Id_t                  EdsId;
   int32                        EdsStatus;
   EdsLib_DataTypeDB_TypeInfo_t CmdHdrInfo;
   CFE_Status_t                 ResultStatus;

   const EdsLib_DatabaseObject_t *EDS_DB = CFE_Config_GetObjPointer(CFE_CONFIGID_MISSION_EDS_DB);

   EdsId     = EDSLIB_MAKE_ID(EDS_INDEX(CFE_HDR), EdsContainer_CFE_HDR_CommandHeader_DATADICTIONARY);
   EdsStatus = EdsLib_DataTypeDB_GetTypeInfo(EDS_DB, EdsId, &CmdHdrInfo);
   if (EdsStatus != EDSLIB_SUCCESS)
   {
        OS_printf("EdsLib_DataTypeDB_GetTypeInfo(): %d\n", (int)EdsStatus);
        ResultStatus = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
   }
   else if (MaxSize->Bits < CmdHdrInfo.Size.Bits)
   {
        /* sanity check - validate the incoming packet is at least the size of a command header */
        OS_printf("CI_LAB: Size mismatch, BitSize=%lu (packet) / %lu (min)\n",
                  (unsigned long)MaxSize->Bits,
                  (unsigned long)CmdHdrInfo.Size.Bits);

        ResultStatus = CFE_STATUS_WRONG_MSG_LENGTH;
   }
   else
   {
      /* Packet is in external wire-format byte order - unpack it and copy */
      EdsStatus = EdsLib_DataTypeDB_UnpackPartialObjectVarSize(EDS_DB,
                                                               &EdsId,
                                                               IngestBufPtr,
                                                               SourceBuffer,
                                                               MaxSize,
                                                               ProcessedSize);
      if (EdsStatus != EDSLIB_SUCCESS)
      {
         OS_printf("EdsLib_DataTypeDB_UnpackPartialObject(Header): %d\n", (int)EdsStatus);
         ResultStatus = CFE_STATUS_VALIDATION_FAILURE;
      }
      else
      {
         ResultStatus = CFE_SUCCESS;
      }
   }

   return ResultStatus;

} /* End DecodeInputHeader() */


/******************************************************************************
** Function: DecodeInputMessage
**
** Translate the source buffer content to an instance of CFE_SB_Buffer_t
** that can be transmitted on the software bus.
**
** Notes:
**   1. This taken directly from CI_LAB_DecodeInputMessage() and adapted for
**      the JMSG context.
**      
*/
static CFE_Status_t DecodeInputMessage(void *SourceBuffer, size_t SourceSize, 
                                       CFE_SB_Buffer_t **DestBufferOut)
{
   
   int32                                 EdsStatus;
   CFE_SB_SoftwareBus_PubSub_Interface_t PubSubParams;
   CFE_SB_Listener_Component_t           ListenerParams;
   CFE_MissionLib_TopicInfo_t            TopicInfo;
   CFE_SB_Buffer_t                      *IngestBufPtr;
   CFE_Status_t                          ResultStatus;
   EdsLib_SizeInfo_t                     MaxSize;
   EdsLib_SizeInfo_t                     ProcessedSize;

   memset(&MaxSize, 0, sizeof(MaxSize));
   memset(&ProcessedSize, 0, sizeof(ProcessedSize));

   /* Get a SB Buffer, as a place to put the decoded data */
   IngestBufPtr = CFE_SB_AllocateMessageBuffer(sizeof(EdsNativeBuffer_CFE_HDR_CommandHeader_t));
   if (IngestBufPtr == NULL)
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_CMD_EDS_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "JMSG command topic failed to allocate a SB message buffer");

      ResultStatus = CFE_SB_BUF_ALOC_ERR;
   }
   else
   {
      MaxSize.Bits  = EdsLib_OCTETS_TO_BITS(SourceSize);
      MaxSize.Bytes = sizeof(EdsNativeBuffer_CFE_HDR_CommandHeader_t);
      ResultStatus  = DecodeInputHeader(IngestBufPtr, SourceBuffer, &MaxSize, &ProcessedSize);
   }

   if (ResultStatus == CFE_SUCCESS)
   {
      
      /* Header decoded successfully - Now need to determine the type for the rest of the payload */
      CFE_MissionLib_Get_PubSub_Parameters(&PubSubParams, &IngestBufPtr->Msg.BaseMsg);
      CFE_MissionLib_UnmapListenerComponent(&ListenerParams, &PubSubParams);

      /* This determines if there is an EDS mapping for the messge topic */
      EdsStatus = CFE_MissionLib_GetTopicInfo(&CFE_SOFTWAREBUS_INTERFACE, ListenerParams.Telecommand.TopicId, &TopicInfo);
      
      if (EdsStatus != CFE_MISSIONLIB_SUCCESS)
      {
         OS_printf("CFE_MissionLib_GetTopicInfo(): %d\n", (int)EdsStatus);
         ResultStatus = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
      }
      else if (EdsLib_Is_Valid(TopicInfo.ParentIntfId))
      {
         /* Decode the payload based on EDS */
         ResultStatus = DecodeInputPayload(IngestBufPtr, TopicInfo.ParentIntfId, SourceBuffer, &MaxSize, &ProcessedSize);
      }
      else
      {
         OS_printf("CI_LAB_DecodeInputMessage(): No EDS definition for TopicId=%u\n",
                   (unsigned int)ListenerParams.Telecommand.TopicId);
         ResultStatus = CFE_STATUS_VALIDATION_FAILURE;
      }
   }

   if (ResultStatus != CFE_SUCCESS && IngestBufPtr != NULL)
   {
      CFE_SB_ReleaseMessageBuffer(IngestBufPtr);
      IngestBufPtr = NULL;
   }

   *DestBufferOut = IngestBufPtr;

   return ResultStatus;
    
} /* End DecodeInputMessage() */


/******************************************************************************
** Function: DecodeInputPayload
**
** Decode the payload from the incoming message
**
** Notes:
**   1. This taken directly from CI_LAB_DecodeInputPayload() and adapted for
**      the JMSG context.
**
*/
static CFE_Status_t DecodeInputPayload(CFE_SB_Buffer_t         *IngestBufPtr,
                                       EdsLib_Id_t              ParentIntfId,
                                       const void              *SourceBuffer,
                                       const EdsLib_SizeInfo_t *MaxSize,
                                       EdsLib_SizeInfo_t       *ProcessedSize)
{
   
   EdsLib_Id_t  EdsId;
   int32        EdsStatus;
   CFE_Status_t ResultStatus;

   const EdsLib_DatabaseObject_t *EDS_DB = CFE_Config_GetObjPointer(CFE_CONFIGID_MISSION_EDS_DB);

   static const EdsLib_Id_t CFE_SB_TELECOMMAND_ID =
        EDSLIB_INTF_ID(EDS_INDEX(CFE_SB), EdsCommand_CFE_SB_Telecommand_indication_DECLARATION);

   ResultStatus = CFE_SUCCESS;

   EdsStatus = EdsLib_IntfDB_FindAllArgumentTypes(EDS_DB, CFE_SB_TELECOMMAND_ID, ParentIntfId, &EdsId, 1);
   if (EdsStatus != EDSLIB_SUCCESS)
   {
      OS_printf("EdsLib_IntfDB_FindAllArgumentTypes(): %d\n", (int)EdsStatus);
      ResultStatus = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
   }
   else
   {
      /* Note, this is invoked even if the message is already fully decoded, as it also serves
       * to identify the message and check against defined constraints.
       * For example, on the CI_LAB_CMD interface, the previous lookup would have
       * yielded the EdsId of CI_LAB/CommandBase, but this will update the EdsId
       * to e.g. CI_LAB/NoopCmd (with NoopCmd being the same size as CommandBase) */
      EdsStatus = EdsLib_DataTypeDB_UnpackPartialObjectVarSize(EDS_DB,
                                                               &EdsId,
                                                               IngestBufPtr,
                                                               SourceBuffer,
                                                               MaxSize,
                                                               ProcessedSize);
      if (EdsStatus != EDSLIB_SUCCESS)
      {
         OS_printf("EdsLib_DataTypeDB_UnpackPartialObject(Payload): %d\n", (int)EdsStatus);
         ResultStatus = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
      }
   }

   if (ResultStatus == CFE_SUCCESS)
   {
      /* Verify that the checksum and basic fields are correct, and recompute the length entry */
      EdsStatus = EdsLib_DataTypeDB_VerifyUnpackedObjectVarSize(EDS_DB,
                                                                EdsId,
                                                                IngestBufPtr,
                                                                SourceBuffer,
                                                                EDSLIB_DATATYPEDB_RECOMPUTE_LENGTH,
                                                                ProcessedSize);
      if (EdsStatus != EDSLIB_SUCCESS)
      {
         OS_printf("EdsLib_DataTypeDB_VerifyUnpackedObject(): %d\n", (int)EdsStatus);
         ResultStatus = CFE_STATUS_VALIDATION_FAILURE;
      }
   }

   return ResultStatus;
    
} /* End DecodeInputPayload() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON topic message to a cFS SB message. The JSON message payload
** is a complete SB command message as defined by the EDS.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**   2. Since JSON payload contains the entire message and the the decoded
**      message is sent on the SB, the CfeMsg parameter is not used. 
**      JMsgTopicCmd->MsgPackedBuf is used for temporary storage.
**   3. The EDS processing code was derived from CI_LAB. This function plays
**      a role analogous to CI_LAB_ReadUpLink() and DecodeInputMessage() is
**      dwerived from CI_LAB_DecodeInputMessage()        
**   4. Encoded APP_C_DEMO commands that can be used for testing. The
**      communication mechanism depends on the app using JMSG_LIB.
**      NOOP: 185cc0000001007a
**      Start Histogram: 185cc00000010a70
**      
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool         RetStatus = false;
   CFE_Status_t CfeStatus;
   size_t       DecodedLen;


   DecodedLen = PktUtil_HexDecode((uint8 *)&JMsgTopicCmd->MsgPackedBuf, JMsgPayload, PayloadLen);

   CfeStatus = DecodeInputMessage(&JMsgTopicCmd->MsgPackedBuf, DecodedLen, &JMsgTopicCmd->SbBufPtr);
   if (CfeStatus == CFE_SUCCESS)
   {

      CfeStatus = CFE_SB_TransmitBuffer(JMsgTopicCmd->SbBufPtr, false);
      if (CfeStatus == CFE_SUCCESS)
      {
         const uint8 *Buf = (const uint8 *)&JMsgTopicCmd->SbBufPtr;
         CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_INFORMATION,
                           "Command plugin successfully sent SB message 0x%02X%02X 0x%02X%02X",
                           Buf[0],Buf[1],Buf[2],Buf[3]);
         JMsgTopicCmd->SbBufPtr = NULL;  // Set NULL so a new buffer will be obtained next time around
         JMsgTopicCmd->JMsgToCfeCnt++;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_ERROR,
                           "Command plugin failed to send SB message, status=%d",(int)CfeStatus);
      }
      
   } /* End if decoded EDS message */

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: PluginTest
**
** Notes:
**   1. The JsonToCfe() callback function is called directly so a meaningful
**      test can be performed without requiring a JMSG network app being
**      installed.
*/
static void PluginTest(bool Init, int16 Param)
{
   static const char *TestCmd = APP_C_DEMO_NOOP_HEXTXT;
   CFE_MSG_Message_t *CfeMsg = NULL;
   
   if (Init)
   {
         
      JMsgTopicCmd->SbTestCnt = 0;
      
      CFE_EVS_SendEvent(JMSG_TOPIC_CMD_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "JMSG command plugin topic test started");

   }
   else
   {   
      JsonToCfe(&CfeMsg, TestCmd, strlen(APP_C_DEMO_NOOP_HEXTXT));
      JMsgTopicCmd->SbTestCnt++;
   }
   
} /* End PluginTest() */

