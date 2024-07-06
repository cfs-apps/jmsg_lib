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
**   Manage the conversion of telemetry messages
**
** Notes:
**   1. Depends on KIT_TO's wrapped telemetry message definitions.
**   2. Telemetry messages are not interpretted, they are treated 
**      as payloads in wrapped messages.
**
*/

#ifndef _jmsg_topic_tlm_
#define _jmsg_topic_tlm_

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_plugin.h"
#include "kit_to_eds_typedefs.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define JMSG_TOPIC_TLM_INIT_SB_MSG_TEST_EID  (JMSG_PLATFORM_TopicPluginBaseEid_TLM + 0)
#define JMSG_TOPIC_TLM_HEX_DECODE_EID        (JMSG_PLATFORM_TopicPluginBaseEid_TLM + 1)

/**********************/
/** Type Definitions **/
/**********************/

/******************************************************************************
** Telemetry
** 
** Since this topic is a wrapper for existing telemetry messages, no new
** telemetry definitions are required.
*/

typedef struct
{

   /*
   ** Telemetry
   */

   KIT_TO_WrappedSbMsgTlm_t  WrappedTlmMsg;

   /*
   ** JSON message data
   */
   
   char  JMsgPayload[JMSG_PLATFORM_TOPIC_SB_MSG_MAX_LEN*2]; /* Endcoded hex is twice as long as the binary */

   uint32  CfeToJMsgCnt;
   uint32  JMsgToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 SbTestCnt;
   
} JMSG_TOPIC_TLM_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_TLM_Constructor
**
** Initialize the JMSG Telemetry topic
**
** Notes:
**   1. The test telemetry message is used for the built in test
**
*/
void JMSG_TOPIC_TLM_Constructor(JMSG_TOPIC_TLM_Class_t *JMsgTopicTlmPtr,
                                JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                CFE_SB_MsgId_t WrappedTlmMid);

#endif /* _jmsg_topic_tlm_ */
