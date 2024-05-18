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

#ifndef _jmsg_topic_cmd_
#define _jmsg_topic_cmd_

/*
** Includes
*/

#include "cfe_hdr_eds_typedefs.h"
#include "lib_cfg.h"
#include "jmsg_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define JMSG_TOPIC_CMD_INIT_SB_MSG_TEST_EID  (JMSG_USR_TopicPluginBaseEid_CMD + 0)
#define JMSG_TOPIC_CMD_CFE2JSON_EID          (JMSG_USR_TopicPluginBaseEid_CMD + 1)
#define JMSG_TOPIC_CMD_JSON2CFE_EID          (JMSG_USR_TopicPluginBaseEid_CMD + 2)

/**********************/
/** Type Definitions **/
/**********************/

typedef struct
{

   /*
   ** JMsg message data
   */
   
   CFE_SB_Buffer_t                 *SbBufPtr;
   CFE_HDR_Message_PackedBuffer_t  MsgPackedBuf;

   uint32  CfeToJMsgCnt;
   uint32  JMsgToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 SbTestCnt;
   
} JMSG_TOPIC_CMD_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_CMD_Constructor
**
** Initialize the JMSG Command plugin
**
** Notes:
**   1. No message ID's are required because the encoded binary message conatins
**      the CCSDS header that contains the message ID.
**
*/
void JMSG_TOPIC_CMD_Constructor(JMSG_TOPIC_CMD_Class_t *JMsgTopicCmdPtr,
                                JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl);

#endif /* _jmsg_topic_cmd_ */
