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
**   Manage JMSG test topic
**
** Notes:
**   None
**
*/

#ifndef _jmsg_topic_test_
#define _jmsg_topic_test_

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define JMSG_TOPIC_TEST_INIT_SB_MSG_TEST_EID  (JMSG_USR_TopicPluginBaseEid_TEST + 0)
#define JMSG_TOPIC_TEST_SB_MSG_TEST_EID       (JMSG_USR_TopicPluginBaseEid_TEST + 1)
#define JMSG_TOPIC_TEST_LOAD_JSON_DATA_EID    (JMSG_USR_TopicPluginBaseEid_TEST + 2)
#define JMSG_TOPIC_TEST_JSON_TO_CCSDS_ERR_EID (JMSG_USR_TopicPluginBaseEid_TEST + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** JMSG_TestPluginTlm_t & JMSG_TestPluginTlm_Payload_t are defined in EDS
*/

typedef struct
{

   /*
   ** Test Telemetry
   */
   
   JMSG_TEST_PluginTlmMsg_t  TlmMsg;
   char                      JsonMsgPayload[1024];
   
   uint32  SbTestCnt;
   int32   IntData;
   float   FloatData;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} JMSG_TOPIC_TEST_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_TEST_Constructor
**
** Initialize the JMSG test topic

** Notes:
**   None
**
*/
void JMSG_TOPIC_TEST_Constructor(JMSG_TOPIC_TEST_Class_t *JMsgTopicTestPtr,
                                 JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                 CFE_SB_MsgId_t TlmMsgMid);


#endif /* _jmsg_topic_test_ */
