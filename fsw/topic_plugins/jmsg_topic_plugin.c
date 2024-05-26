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
**   Manage topic plugins
**
** Notes:
**   1. See jsmg_lib/topic_plugins/jmsg_topic_plugin_guide.txt for 
**      plugin creation and installation instructions
**   2. SC_SIM is commented out to serve as an example topic plugin.
**      To use SC_SIM the '**' comment lines can be commented out
**      using the '//' syntax.
** 
*/

/*
** Include Files:
*/

#include <string.h>
#include "jmsg_topic_plugin.h"

/************************/
/** Topic header files **/
/************************/

#include "jmsg_topic_cmd.h"
#include "jmsg_topic_tlm.h"
#include "jmsg_topic_test.h"
/*@@ Example user Plugins
#include "jmsg_topic_discrete.h"
#include "jmsg_topic_rate.h"
@@*/

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool StubCfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static void StubSbMsgTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

// JMSG_LIB topic plugin objects
static JMSG_TOPIC_CMD_Class_t   JMsgTopicCmd;
static JMSG_TOPIC_TLM_Class_t   JMsgTopicTlm;
static JMSG_TOPIC_TEST_Class_t  JMsgTopicTest;

// User topic plugin objects

/*@@ Example user Plugins
static JMSG_TOPIC_DISCRETE_Class_t  JMsgTopicDiscrete;
static JMSG_TOPIC_RATE_Class_t      JMsgTopicRate;
@@*/


/******************************************************************************
** Function: JMSG_TOPIC_PLUGIN_Constructor
**
**
** Notes:
**   1. The design convention is for topic objects to receive a
**      CFE_SB_MsgId_t type so TopicIds defined in the JSON table
**      must be converted to message IDs. 
**   2. Always construct plugins regardless of whether they are enabled. This
**      allows plugins to be enabled/disabled during runtime. If a topic's
**      JSON is invalid the plugin will be automatically disabled.
**
*/
void JMSG_TOPIC_PLUGIN_Constructor(const JMSG_TOPIC_TBL_Data_t *TopicTbl,
                                   JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                   uint32 TestTlmTopicId)
{

   for (enum JMSG_USR_TopicPlugin i=0; i < JMSG_USR_TopicPlugin_Enum_t_MAX; i++)
   {
      switch (i)
      {
         case JMSG_USR_TopicPlugin_CMD:
            JMSG_TOPIC_CMD_Constructor(&JMsgTopicCmd, &PluginFuncTbl[i]);
            break;
            
         case JMSG_USR_TopicPlugin_TLM:
            JMSG_TOPIC_TLM_Constructor(&JMsgTopicTlm, &PluginFuncTbl[i],
                                       CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe),
                                       CFE_SB_ValueToMsgId(TestTlmTopicId));         
            break;

         case JMSG_USR_TopicPlugin_TEST:
            JMSG_TOPIC_TEST_Constructor(&JMsgTopicTest, &PluginFuncTbl[i],
                                        CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
            break;

/*@@ Example user Plugins
         case JMSG_USR_TopicPlugin_USR_1:
            JMSG_TOPIC_DISCRETE_Constructor(&JMsgTopicDiscrete, &PluginFuncTbl[i],
                                            CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
            break;

         case JMSG_USR_TopicPlugin_USR_2:
            JMSG_TOPIC_RATE_Constructor(&JMsgTopicRate, &PluginFuncTbl[i],
                                        CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
            break;
@@*/
         default:
            // Plugin should be disabled
            PluginFuncTbl[i].CfeToJson = StubCfeToJson;
            PluginFuncTbl[i].JsonToCfe = StubJsonToCfe;
            PluginFuncTbl[i].SbMsgTest = StubSbMsgTest;
            CFE_EVS_SendEvent(JMSG_TOPIC_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                              "Plugin topic ID %d(index %d) is enabled in the topic table, but doesn't have a constructor.",
                              (i+1), i);
            break;
                        
      } // End switch

   } // End plugin loop
    
} /* JMSG_TOPIC_PLUGIN_Constructor() */


/******************************************************************************
** Function: StubCfeToJson
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static bool StubCfeToJson(const char **JsonMsgPayload, 
                          const CFE_MSG_Message_t *CfeMsg)
{

   CFE_EVS_SendEvent(JMSG_TOPIC_PLUGIN_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "CfeToJson stub");

   return false;
   
} /* End StubCfeToJson() */


/******************************************************************************
** Function: StubJsonToCfe
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                          const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   CFE_EVS_SendEvent(JMSG_TOPIC_PLUGIN_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "JsonToCfe stub");

   return false;
   
} /* End StubJsonToCfe() */


/******************************************************************************
** Function: StubSbMsgTest
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static void StubSbMsgTest(bool Init, int16 Param)
{

   CFE_EVS_SendEvent(JMSG_TOPIC_PLUGIN_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "SbMsgTest stub");
   
} /* End StubSbMsgTest() */



