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
**      converting an binary encoded command message into a SB message.
**   2. JsonToCfe() performs the same EDS processing as CI_LAB.
**
*/

/*
** Includes
*/

#include "cfe_config.h"
#include "cfe_missionlib_api.h"
#include "cfe_missionlib_runtime.h"
#include "cfe_mission_eds_parameters.h"
#include "cfe_mission_eds_interface_parameters.h"
#include "edslib_datatypedb.h"

#include "jmsg_topic_cmd.h"


/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen);
static void SbMsgTest(bool Init, int16 Param);

/**********************/
/** Global File Data **/
/**********************/

static JMSG_TOPIC_CMD_Class_t* JMsgTopicCmd = NULL;


/******************************************************************************
** Function: JMSG_TOPIC_CMD_Constructor
**
** Initialize the telemetry topic
**
** Notes:
**   1. The test telemetry message is used for the built in test.
**
*/
void JMSG_TOPIC_CMD_Constructor(JMSG_TOPIC_CMD_Class_t *JMsgTopicCmdPtr,
                                JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl)
{

   JMsgTopicCmd = JMsgTopicCmdPtr;
   memset(JMsgTopicCmd, 0, sizeof(JMSG_TOPIC_CMD_Class_t));

   JMsgTopicCmd->SbBufPtr = NULL;

   PluginFuncTbl->CfeToJson = CfeToJson;
   PluginFuncTbl->JsonToCfe = JsonToCfe;  
   PluginFuncTbl->SbMsgTest = SbMsgTest;
            
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
** Function: JsonToCfe
**
** Convert a JSON topic message to a cFS SB message. The JSON message payload
** is a complete SB command message as defined by the EDS.
**
** Notes:
**   1. Signature must match JMSG_TOPIC_TBL_JsonToCfe_t
**   2. See the CI_LAB app for EDS provcessing details
**   3. Encoded app_c_demo commands that can be used for testing. The communciation
**      mechanism depends on the app using JMSG_LIB.
**      NOOP: 185cc0000001007a
**      Start Histogram: 185cc00000010a70
**      
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JMsgPayload, uint16 PayloadLen)
{
   
   bool    RetStatus = false;
   int32   Status;
   uint32  BitSize;
   size_t  DecodedLen;
   
   EdsLib_DataTypeDB_TypeInfo_t  CmdHdrInfo;
   EdsLib_DataTypeDB_TypeInfo_t  FullCmdInfo;
   EdsLib_Id_t                   EdsId;
   
   CFE_SB_SoftwareBus_PubSub_Interface_t PubSubParams;
   CFE_SB_Listener_Component_t           ListenerParams;

   const EdsLib_DatabaseObject_t *EDS_DB = CFE_Config_GetObjPointer(CFE_CONFIGID_MISSION_EDS_DB);

   EdsId  = EDSLIB_MAKE_ID(EDS_INDEX(CFE_HDR), CFE_HDR_CommandHeader_DATADICTIONARY);
   Status = EdsLib_DataTypeDB_GetTypeInfo(EDS_DB, EdsId, &CmdHdrInfo);

   if (Status == EDSLIB_SUCCESS)
   {
      DecodedLen = PktUtil_HexDecode((uint8 *)&JMsgTopicCmd->MsgPackedBuf, JMsgPayload, PayloadLen);
      BitSize = DecodedLen * 8;

      if (BitSize >= CmdHdrInfo.Size.Bits)
      {

         if (JMsgTopicCmd->SbBufPtr == NULL)
         {
            
            JMsgTopicCmd->SbBufPtr = CFE_SB_AllocateMessageBuffer(sizeof(CFE_HDR_CommandHeader_Buffer_t));
            if (JMsgTopicCmd->SbBufPtr != NULL)
            {
               
               /* Packet is in external wire-format byte order - unpack it and copy */
               Status =
                  EdsLib_DataTypeDB_UnpackPartialObject(EDS_DB, &EdsId, JMsgTopicCmd->SbBufPtr, JMsgTopicCmd->MsgPackedBuf,
                                                        sizeof(CFE_HDR_CommandHeader_Buffer_t), BitSize, 0);
               if (Status != EDSLIB_SUCCESS)
               {
                  OS_printf("EdsLib_DataTypeDB_UnpackPartialObject(1): %d\n", (int)Status);
                  return false;
               }

               /* Header decoded successfully - Now need to determine the type for the rest of the payload */
               CFE_MissionLib_Get_PubSub_Parameters(&PubSubParams, &(JMsgTopicCmd->SbBufPtr)->Msg.BaseMsg);
               CFE_MissionLib_UnmapListenerComponent(&ListenerParams, &PubSubParams);

               Status = CFE_MissionLib_GetArgumentType(&CFE_SOFTWAREBUS_INTERFACE, CFE_SB_Telecommand_Interface_ID,
                                                       ListenerParams.Telecommand.TopicId, 1, 1, &EdsId);
               if (Status != CFE_MISSIONLIB_SUCCESS)
               {
                  OS_printf("CFE_MissionLib_GetArgumentType(): %d\n", (int)Status);
                  return false;
               }

               Status = EdsLib_DataTypeDB_UnpackPartialObject(
                  EDS_DB, &EdsId, JMsgTopicCmd->SbBufPtr, JMsgTopicCmd->MsgPackedBuf, sizeof(CFE_HDR_CommandHeader_Buffer_t),
                  BitSize, sizeof(CFE_HDR_CommandHeader_t));
               if (Status != EDSLIB_SUCCESS)
               {
                  OS_printf("EdsLib_DataTypeDB_UnpackPartialObject(2): %d\n", (int)Status);
                  return false;
               }

               /* Verify that the checksum and basic fields are correct, and recompute the length entry */
               Status = EdsLib_DataTypeDB_VerifyUnpackedObject(
                   EDS_DB, EdsId, JMsgTopicCmd->SbBufPtr, JMsgTopicCmd->MsgPackedBuf, EDSLIB_DATATYPEDB_RECOMPUTE_LENGTH);
               if (Status != EDSLIB_SUCCESS)
               {
                  OS_printf("EdsLib_DataTypeDB_VerifyUnpackedObject(): %d\n", (int)Status);
                  return false;
               }

               Status = EdsLib_DataTypeDB_GetTypeInfo(EDS_DB, EdsId, &FullCmdInfo);
               if (Status != EDSLIB_SUCCESS)
               {
                  OS_printf("EdsLib_DataTypeDB_GetTypeInfo(): %d\n", (int)Status);
                  return false;
               }

               Status = CFE_SB_TransmitBuffer(JMsgTopicCmd->SbBufPtr, false);
               if (Status == CFE_SUCCESS)
               {
                  /* Set NULL so a new buffer will be obtained next time around */
                  JMsgTopicCmd->SbBufPtr = NULL;
                  JMsgTopicCmd->JMsgToCfeCnt++;
                  RetStatus = true;
               }
               else
               {
                   CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_ERROR,
                                     "Command plugin failed to send SB message, status=%d",(int)Status);
               }
               
            } /* If allocate SB buffer */
            else
            {
               CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_ERROR,
                                 "SB buffer allocation failed");
            }
         } /* End if allocated SB buffer */
      } /* End if valid payload length */
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_ERROR,
                           "Invalid command length, bit size %d less than header bit size %d",
                           (int)BitSize, (int)CmdHdrInfo.Size.Bits);
      }
   } /* End if EDSLIB_SUCCESS */
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_CMD_JSON2CFE_EID, CFE_EVS_EventType_ERROR,
                        "Error accessing EDS database. Status = %d\n", (int)Status);
   }
      
   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: SbMsgTest
**
** Test plugin stub 
**
** Notes:
**   None
**
*/
static void SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {
         
      JMsgTopicCmd->SbTestCnt = 0;
      
      CFE_EVS_SendEvent(JMSG_TOPIC_CMD_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Command topic does not have an automated built in test");
   }
   else
   {   
      JMsgTopicCmd->SbTestCnt++;
   }
   
} /* End SbMsgTest() */

