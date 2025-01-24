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
**   Define the JMSG script plugin topic
**
** Notes:
**   1. The JMSG script interface allows a cFS app to manage the excecution
**      of scripts on a remote system.  Scripts can be embedded in the JSMG
**      or resident on the remote system.
**
*/

#ifndef _jmsg_topic_script_
#define _jmsg_topic_script_

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

#define JMSG_TOPIC_SCRIPT_CFE2JSON_EID         (JMSG_PLATFORM_TopicPluginBaseEid_CMD + 0)
#define JMSG_TOPIC_SCRIPT_JSON2CFE_EID         (JMSG_PLATFORM_TopicPluginBaseEid_CMD + 1)
#define JMSG_TOPIC_SCRIPT_LOAD_JSON_DATA_EID   (JMSG_PLATFORM_TopicPluginBaseEid_CMD + 2)
#define JMSG_TOPIC_SCRIPT_PLUGIN_TEST_EID      (JMSG_PLATFORM_TopicPluginBaseEid_CMD + 3)


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

   JMSG_LIB_ExecScriptTlm_t  ExecScriptTlm;

   /*
   ** JSON message data
   */
   
   char  JMsgPayload[128+OS_MAX_PATH_LEN+JMSG_PLATFORM_TOPIC_SCRIPT_STRING_MAX_LEN]; // 128 covers characters beyond script filename and in-message script text
   
   uint16  JsonObjCnt;
   uint32  CfeToJMsgCnt;
   uint32  JMsgToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 PluginTestCnt;
   
} JMSG_TOPIC_SCRIPT_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_SCRIPT_Constructor
**
** Initialize the JMSG Script topic
**
** Notes:
**   None
*/
void JMSG_TOPIC_SCRIPT_Constructor(JMSG_TOPIC_SCRIPT_Class_t *JMsgTopicScriptPtr,
                                   JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                   CFE_SB_MsgId_t ScriptTlmMid);

#endif /* _jmsg_topic_script_ */
