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
**   Manage JSON message topic table
**
** Notes:
**   1. This header can't include lib_cfg.h because it would cause an initbl
**      CFG definition conflict with apps that include this header. The initbl
**      design assumes the the CFG_ definitions are unique to an app or a lib.
**   2. The JSON message topic table defines each supported EDS topic. This
**      library supports the following data flows:
**      - Translated from JSON to binary cFE message and published on the SB
**      - Read from the SB and translated from binary cFE message to JSON
**   3. Apps using this library manage the JSON network routing
**   4. The plugin tests provided by JMSG_LIB are "one shots" which means they
**      cause a single message to be generated. these are intended as integration
**      to demonstrate the topic table and plugin functions are oeprational. The
**      JSON network apps (e.g. JSMG_UDP) provide functional and performance tests. 
**   5. See jsmg_lib/topic_plugins/jmsg_topic_plugin_guide.txt for 
**      customization details. 
**
*/

#ifndef _jmsg_topic_tbl_
#define _jmsg_topic_tbl_

/*
** Includes
*/

#include "app_c_fw.h"
#include "jmsg_usr_eds_defines.h"
#include "jmsg_usr_eds_typedefs.h"
#include "jmsg_lib_eds_defines.h"
#include "jmsg_lib_eds_typedefs.h"
#include "jmsg_test_eds_defines.h"
#include "jmsg_test_eds_typedefs.h"

/***********************/
/** Macro Definitions **/
/***********************/


#define JSON_MAX_KW_LEN  10  // Maximum length of short keywords

#define JMSG_TOPIC_TBL_NAME "JMSG Topics"

/*
** Event Message IDs
*/

#define JMSG_TOPIC_TBL_INDEX_ERR_EID     (JMSG_USR_TOPIC_TBL_BASE_EID + 0)
#define JMSG_TOPIC_TBL_DUMP_ERR_EID      (JMSG_USR_TOPIC_TBL_BASE_EID + 1)
#define JMSG_TOPIC_TBL_LOAD_ERR_EID      (JMSG_USR_TOPIC_TBL_BASE_EID + 2)
#define JMSG_TOPIC_TBL_STUB_EID          (JMSG_USR_TOPIC_TBL_BASE_EID + 3)
#define JMSG_TOPIC_TBL_ENA_PLUGIN_EID    (JMSG_USR_TOPIC_TBL_BASE_EID + 4)
#define JMSG_TOPIC_TBL_DIS_PLUGIN_EID    (JMSG_USR_TOPIC_TBL_BASE_EID + 5)
#define JMSG_TOPIC_TBL_SUBSCRIBE_EID     (JMSG_USR_TOPIC_TBL_BASE_EID + 6)


/**********************/
/** Type Definitions **/
/**********************/


typedef enum
{
   JMSG_TOPIC_TBL_SUB_SB     = 1,
   JMSG_TOPIC_TBL_SUB_JMSG   = 2,
   JMSG_TOPIC_TBL_UNSUB_SB   = 3,
   JMSG_TOPIC_TBL_UNSUB_JMSG = 4,
   JMSG_TOPIC_TBL_SUB_ERR    = 5,
   JMSG_TOPIC_TBL_SUB_UNDEF  = 6

} JMSG_TOPIC_TBL_SubscriptionOptEnum_t;

typedef enum
{
   JMSG_TOPIC_TBL_SUB_TO_ROLE  = 1,  // Subscribe to the role defined in the plugin table
   JMSG_TOPIC_TBL_SUB_TO_JMSG  = 2,
   JMSG_TOPIC_TBL_SUB_TO_SB    = 3
   
} JMSG_TOPIC_TBL_TopicSubscribeToEnum_t;

                               
/******************************************************************************
** Topic Table 
** 
*/

typedef struct
{

   char    Name[JMSG_USR_TOPIC_NAME_MAX_LEN];
   uint16  Cfe;
   char    SbStr[JSON_MAX_KW_LEN];
   char    EnaStr[JSON_MAX_KW_LEN];
   
   // These are set based on JSON strings
   bool    Enabled;
   JMSG_LIB_PluginSbRole_Enum_t PluginSbRole;
   
} JMSG_TOPIC_TBL_Topic_t;

typedef struct
{

  JMSG_TOPIC_TBL_Topic_t Topic[JMSG_USR_TOPIC_PLUGIN_MAX];
   
} JMSG_TOPIC_TBL_Data_t;


/*
** Callback function that is called when a topic plugin's configuration is changed. This allows the table
** owner to subscribe/unsubscribe when a topic is enabled/disabled.
*/ 
typedef bool (*JMSG_TOPIC_TBL_ConfigSubscription_t)(const JMSG_TOPIC_TBL_Topic_t *Topic, JMSG_TOPIC_TBL_SubscriptionOptEnum_t ConfigOpt);

/******************************************************************************
** Topic 'virtual' function signatures
** - Separate JMSG_TOPIC_xxx files are used for each topic plugin. The
**   JMSG_TOPIC_TBL_PluginFuncTbl_t is used to hold pointers to each conversion
**   function so you can think of a plugin type as an abstract base class and
**   each JMSG_TOPIC_xxx as concrete classes that provide the plugin conversion
**   methods.
*/

typedef bool (*JMSG_TOPIC_TBL_JsonToCfe_t)(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
typedef bool (*JMSG_TOPIC_TBL_CfeToJson_t)(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
typedef void (*JMSG_TOPIC_TBL_PluginTest_t)(bool Init, int16 Param);

typedef struct
{

   JMSG_TOPIC_TBL_CfeToJson_t   CfeToJson;
   JMSG_TOPIC_TBL_JsonToCfe_t   JsonToCfe;  
   JMSG_TOPIC_TBL_PluginTest_t  PluginTest;

} JMSG_TOPIC_TBL_PluginFuncTbl_t; 


/******************************************************************************
** Class
*/

typedef struct
{

   /*
   ** JMSG Library
   */

   JMSG_LIB_TopicPluginTlm_t   TopicPluginTlm;
      
   /*
   ** Topic Table
   */
   
   uint32  PluginTestTlmTopicId;

   int16   PluginTestParam;
   JMSG_USR_TopicPlugin_Enum_t  PluginTestId;

   
   JMSG_TOPIC_TBL_Data_t  Data;
   
   JMSG_TOPIC_TBL_ConfigSubscription_t  ConfigSubscription;
   
   /*
   ** Standard CJSON table data
   */
   
   bool        Loaded;   /* Has entire table been loaded? */
   uint16      LastLoadCnt;
   
   size_t      JsonObjCnt;
   char        JsonBuf[JMSG_USR_MAX_JSON_TOPIC_TBL_CHAR];   
   size_t      JsonFileLen;
   
} JMSG_TOPIC_TBL_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_TBL_Constructor
**
** Initialize the JMSG topic table.
**
** Notes:
**   1. The table values are not populated. This is done when the table is 
**      registered with the table manager.
**
*/
void JMSG_TOPIC_TBL_Constructor(JMSG_TOPIC_TBL_Class_t *JMsgTopicTblPtr,
                                uint32 TopicTblTlmTopicId, 
                                uint32 PluginTestTlmTopicId);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_DisablePlugin
**
** Disable a topic plugin
**
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
bool JMSG_TOPIC_TBL_DisablePlugin(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_DumpCmd
**
** Command to write the table data from memory to a JSON file.
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**
*/
bool JMSG_TOPIC_TBL_DumpCmd(osal_id_t  FileHandle);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_EnablePlugin
**
** Command to write the table data from memory to a JSON file.
**
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
bool JMSG_TOPIC_TBL_EnablePlugin(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetCfeToJson
**
** Return a pointer to the CfeToJson conversion function for 'TopicPlugin'
** and return a pointer to the JSON topic string in JsonMsgTopic.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_M
**
*/
JMSG_TOPIC_TBL_CfeToJson_t JMSG_TOPIC_TBL_GetCfeToJson(JMSG_USR_TopicPlugin_Enum_t TopicPlugin,
                                                       const char **JsonMsgTopic);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetDisabledTopic
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. A special case arose for getting a disabled topic pointer. Rather than
**      add a parameter JMSG_TOPIC_TBL_GetTopic() in which this one case would
**      be the exception, it seemed cleaner to add another function that has
**      some duplicate functionality.
**
*/
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetDisabledTopic(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetTopic
**
** Return a pointer to the table entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetTopic(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
JMSG_TOPIC_TBL_JsonToCfe_t JMSG_TOPIC_TBL_GetJsonToCfe(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_LoadCmd
**
** Command to copy the table data from a JSON file to memory.
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager that verified the filename.
**
*/
bool JMSG_TOPIC_TBL_LoadCmd(APP_C_FW_TblLoadOptions_Enum_t LoadType, const char *Filename);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_MsgIdToTopicPlugin
**
** Return a topic table index for a message ID
** 
** Notes:
**   1. JMSG_LIB_TopicPlugin_UNDEF is returned if the message ID isn't found.
**
*/
uint8 JMSG_TOPIC_TBL_MsgIdToTopicPlugin(CFE_SB_MsgId_t MsgId);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback
**
** Register the callback function that will be called when a topic is enabled
** or disabled and the table owner needs to manage netwrok layer subscriptions. 
**
*/
void JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback(JMSG_TOPIC_TBL_ConfigSubscription_t ConfigSubscription);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RegisterPlugin
**
** Register a user topic plugin.
** TODO: Verify the TopicPlugin ID is in the USR range.
**
*/
 CFE_SB_MsgId_t JMSG_TOPIC_TBL_RegisterPlugin(JMSG_USR_TopicPlugin_Enum_t TopicPlugin,
                                              JMSG_TOPIC_TBL_CfeToJson_t  CfeToJson,
                                              JMSG_TOPIC_TBL_JsonToCfe_t  JsonToCfe,
                                              JMSG_TOPIC_TBL_PluginTest_t PluginTest);

                                
/******************************************************************************
** Function: JMSG_TOPIC_TBL_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of
** the table manager should not be impacted. The intent is to clear counters
** and flags to a known default state for telemetry.
**
*/
void JMSG_TOPIC_TBL_ResetStatus(void);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RunTopicPluginTest
**
** Execute a topic's SB message test.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
void JMSG_TOPIC_TBL_RunTopicPluginTest(JMSG_USR_TopicPlugin_Enum_t TopicPlugin, 
                                       bool Init, int16 Param);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SendTopicTPluginTlmCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used.
**
*/
bool JMSG_TOPIC_TBL_SendTopicTPluginTlmCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SubscribeToAll
**
** Subscribe to all topics defined in the topic table. A topic's SbRole
** determines which type of subscription will be performed.
**
** Notes:
**   1. This should only be used when only one JMSG network app is using
**      JMSG_LIB because when it is called the app's ConfigSubscription()
**      function is called for every enabled topic table entry.
**   2. JMSG_TOPIC_TBL_SubscribeToTopicMsg() to configure individual entries.
**   TODO: Current design requires code changes to the JMSG network app's
**   TODO: constructor. Create a scheme that is table or EDS based.
**
*/
void JMSG_TOPIC_TBL_SubscribeToAll(JMSG_TOPIC_TBL_TopicSubscribeToEnum_t SubscribeTo);


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SubscribeToTopicMsg
**
** Subscribe to a topic defined in the topic table. A topic's SbRole
** determines which type of subscription will be performed.
**
*/
JMSG_TOPIC_TBL_SubscriptionOptEnum_t JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_USR_TopicPlugin_Enum_t TopicPlugin,
                                                                        JMSG_TOPIC_TBL_TopicSubscribeToEnum_t SubscribeTo);
                                                                               
                             
/******************************************************************************
** Function: JMSG_TOPIC_TBL_UnsubscribeFromTopicMsg
**
** Unsubscribe from a topic message
**
** Notes:
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
bool JMSG_TOPIC_TBL_UnsubscribeFromTopicMsg(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);

                             
/******************************************************************************
** Function: JMSG_TOPIC_TBL_ValidTopicPlugin
**
** In addition to being in range, valid means that the TopicPlugin has been
** defined.
*/
bool JMSG_TOPIC_TBL_ValidTopicPlugin(JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


#endif /* _jmsg_topic_tbl_ */
