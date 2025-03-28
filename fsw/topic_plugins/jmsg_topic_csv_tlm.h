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
**   Define the JMSG topic Comma Separated Variable telemetry plugin topic
**
** Notes:
**   1. Allows a cFS app to receive JMSG telemetry from a system external 
**      to the cFS target. The telemetry parameters are contained in a CSV
**      string.
**
*/

#ifndef _jmsg_topic_csv_tlm_
#define _jmsg_topic_csv_tlm_

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

#define JMSG_TOPIC_CSV_TLM_CFE2JSON_EID         (JMSG_PLATFORM_TopicPluginBaseEid_CSV_TLM + 0)
#define JMSG_TOPIC_CSV_TLM_JSON2CFE_EID         (JMSG_PLATFORM_TopicPluginBaseEid_CSV_TLM + 1)
#define JMSG_TOPIC_CSV_TLM_LOAD_JSON_DATA_EID   (JMSG_PLATFORM_TopicPluginBaseEid_CSV_TLM + 2)
#define JMSG_TOPIC_CSV_TLM_PLUGIN_TEST_EID      (JMSG_PLATFORM_TopicPluginBaseEid_CSV_TLM + 3)


/**********************/
/** Type Definitions **/
/**********************/

/******************************************************************************
** Class
** 
*/

typedef struct
{

   /*
   ** Telemetry
   */

   JMSG_LIB_TopicCsvTlm_t  CsvTlm;

   /*
   ** JSON message data
   */
   
   char  JMsgPayload[JMSG_PLATFORM_TOPIC_NAME_MAX_LEN+4+sizeof(JMSG_LIB_TlmDateTime_String_t)+JMSG_PLATFORM_TOPIC_STRING_MAX_LEN+64]; // See EDS definitions. 64 is for keywords, quotes, spaces and commas
   
   uint16  JsonObjCnt;
   uint32  CfeToJMsgCnt;
   uint32  JMsgToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 PluginTestCnt;
   
} JMSG_TOPIC_CSV_TLM_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_CSV_TLM_Constructor
**
** Initialize the JMSG Script topic
**
** Notes:
**   None
*/
void JMSG_TOPIC_CSV_TLM_Constructor(JMSG_TOPIC_CSV_TLM_Class_t *JMsgTopicCsvTlmPtr,
                                    JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                    CFE_SB_MsgId_t CsvTlmMid);

#endif /* _jmsg_topic_csv_tlm_ */
