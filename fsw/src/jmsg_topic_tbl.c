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
**   Manage JMSG Topic table
**
** Notes:
**   1. Each supported JMSG topic is listed in a JSON file and each
**      topic has a JSON file that defines the topic's content.
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "jmsg_lib.h"
#include "lib_cfg.h"
#include "jmsg_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/

#define JSON_MAX_TOPIC_LEN  JMSG_PLATFORM_TOPIC_NAME_MAX_LEN  // Shorter & more ledgable for table use


/********************************** **/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(size_t JsonFileLen);


/**********************/
/** Global File Data **/
/**********************/

static JMSG_TOPIC_TBL_Class_t *JMsgTopicTbl = NULL;

static JMSG_TOPIC_TBL_Data_t TblData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{
 
   /* Table                     Data                                             core-json             length of query       */
   /* Data Address,             Len,                Updated, Data Type,  Float,  query string,         string(exclude '\0')  */
   
   { &TblData.Topic[0].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[0].name",     (sizeof("topic[0].name")-1)}},
   { &TblData.Topic[0].Cfe,     2,                  false,   JSONNumber, false, { "topic[0].cfe",      (sizeof("topic[0].cfe")-1)}},
   { &TblData.Topic[0].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].protocol", (sizeof("topic[0].protocol")-1)}},
   { &TblData.Topic[0].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].sb-role",  (sizeof("topic[0].sb-role")-1)}},
   { &TblData.Topic[0].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].enabled",  (sizeof("topic[0].enabled")-1)}},

   { &TblData.Topic[1].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[1].name",     (sizeof("topic[1].name")-1)}},
   { &TblData.Topic[1].Cfe,     2,                  false,   JSONNumber, false, { "topic[1].cfe",      (sizeof("topic[1].cfe")-1)}},
   { &TblData.Topic[1].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].protocol", (sizeof("topic[1].protocol")-1)}},
   { &TblData.Topic[1].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].sb-role",  (sizeof("topic[1].sb-role")-1)}},
   { &TblData.Topic[1].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].enabled",  (sizeof("topic[1].enabled")-1)}},

   { &TblData.Topic[2].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[2].name",     (sizeof("topic[2].name")-1)}},
   { &TblData.Topic[2].Cfe,     2,                  false,   JSONNumber, false, { "topic[2].cfe",      (sizeof("topic[2].cfe")-1)}},
   { &TblData.Topic[2].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].protocol", (sizeof("topic[2].protocol")-1)}},
   { &TblData.Topic[2].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].sb-role",  (sizeof("topic[2].sb-role")-1)}},
   { &TblData.Topic[2].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].enabled",  (sizeof("topic[2].enabled")-1)}},

   { &TblData.Topic[3].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[3].name",     (sizeof("topic[3].name")-1)}},
   { &TblData.Topic[3].Cfe,     2,                  false,   JSONNumber, false, { "topic[3].cfe",      (sizeof("topic[3].cfe")-1)}},
   { &TblData.Topic[3].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].protocol", (sizeof("topic[3].protocol")-1)}},
   { &TblData.Topic[3].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].sb-role",  (sizeof("topic[3].sb-role")-1)}},
   { &TblData.Topic[3].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].enabled",  (sizeof("topic[3].enabled")-1)}},

   { &TblData.Topic[4].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[4].name",     (sizeof("topic[4].name")-1)}},
   { &TblData.Topic[4].Cfe,     2,                  false,   JSONNumber, false, { "topic[4].cfe",      (sizeof("topic[4].cfe")-1)}},
   { &TblData.Topic[4].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].protocol", (sizeof("topic[4].protocol")-1)}},
   { &TblData.Topic[4].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].sb-role",  (sizeof("topic[4].sb-role")-1)}},
   { &TblData.Topic[4].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].enabled",  (sizeof("topic[4].enabled")-1)}},

   { &TblData.Topic[5].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[5].name",     (sizeof("topic[5].name")-1)}},
   { &TblData.Topic[5].Cfe,     2,                  false,   JSONNumber, false, { "topic[5].cfe",      (sizeof("topic[5].cfe")-1)}},
   { &TblData.Topic[5].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].protocol", (sizeof("topic[5].protocol")-1)}},
   { &TblData.Topic[5].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].sb-role",  (sizeof("topic[5].sb-role")-1)}},
   { &TblData.Topic[5].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].enabled",  (sizeof("topic[5].enabled")-1)}},

   { &TblData.Topic[6].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[6].name",     (sizeof("topic[6].name")-1)}},
   { &TblData.Topic[6].Cfe,     2,                  false,   JSONNumber, false, { "topic[6].cfe",      (sizeof("topic[6].cfe")-1)}},
   { &TblData.Topic[6].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].protocol", (sizeof("topic[6].protocol")-1)}},
   { &TblData.Topic[6].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].sb-role",  (sizeof("topic[6].sb-role")-1)}},
   { &TblData.Topic[6].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].enabled",  (sizeof("topic[6].enabled")-1)}},

   { &TblData.Topic[7].Name,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[7].name",     (sizeof("topic[7].name")-1)}},
   { &TblData.Topic[7].Cfe,     2,                  false,   JSONNumber, false, { "topic[7].cfe",      (sizeof("topic[7].cfe")-1)}},
   { &TblData.Topic[7].ProtStr, JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].protocol", (sizeof("topic[7].protocol")-1)}},
   { &TblData.Topic[7].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].sb-role",  (sizeof("topic[7].sb-role")-1)}},
   { &TblData.Topic[7].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].enabled",  (sizeof("topic[7].enabled")-1)}}
   
};


// Table is populated by topic plugin constructors & plugin regitrations
static JMSG_TOPIC_TBL_PluginFuncTbl_t PluginFuncTbl[JMSG_PLATFORM_TOPIC_PLUGIN_MAX];

/******************************************************************************
** Function: JMSG_TOPIC_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void JMSG_TOPIC_TBL_Constructor(JMSG_TOPIC_TBL_Class_t *JMsgTopicTblPtr,
                                uint32 TopicTblTlmTopicId)
{

   JMSG_PLATFORM_TopicPlugin_Enum_t i;
   JMsgTopicTbl = JMsgTopicTblPtr;

   CFE_PSP_MemSet(JMsgTopicTbl, 0, sizeof(JMSG_TOPIC_TBL_Class_t));

   JMsgTopicTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));

   
   for (i=0; i < JMSG_PLATFORM_TOPIC_PLUGIN_MAX; i++)
   {
      TblData.Topic[i].Enabled = false;
      JMsgTopicTbl->Data.Topic[i].Enabled  = false;
      JMsgTopicTbl->Data.Topic[i].Protocol = JMSG_LIB_TopicProtocol_UNDEF;
      JMsgTopicTbl->Data.Topic[i].SbRole   = JMSG_LIB_TopicSbRole_UNDEF;
      JMsgTopicTbl->Data.Topic[i].SubscriptionCallback = NULL;
   }
   
   // Plugin stubs loaded for disabled entries
   JMSG_TOPIC_PLUGIN_Constructor(&JMsgTopicTbl->Data, PluginFuncTbl,
                                  JMsgTopicTbl->PluginTestTlmTopicId);
                                    
   CFE_MSG_Init(CFE_MSG_PTR(JMsgTopicTbl->TopicTblTlm.TelemetryHeader), 
                CFE_SB_ValueToMsgId(TopicTblTlmTopicId), 
                sizeof(JMSG_LIB_TopicTblTlm_t));

} /* End JMSG_TOPIC_TBL_Constructor() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_DisablePlugin
**
** Disable a topic plugin
** 
** Notes:
**   1. This function operates at the table scope so it is only concerned with
**      validating table parameters. If this returns true then the calling
**      function can take action such as managing subscriptions.
**
*/
bool JMSG_TOPIC_TBL_DisablePlugin(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_PLATFORM_TOPIC_PLUGIN_MAX)
   {
      if (JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled = false;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to disable topic plugin ID %d that is already disabled",
                           TopicPlugin);
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                        "Attempt to disable a topic plugin with an invalid topic plugin ID %d",
                        TopicPlugin);             
   }
   
   return RetStatus;
   
} /* JMSG_TOPIC_TBL_DisablePlugin() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. File is formatted so it can be used as a load file.
*/
bool JMSG_TOPIC_TBL_DumpCmd(osal_id_t  FileHandle)
{

   uint8  i;
   char   DumpRecord[256];


   sprintf(DumpRecord,"   \"topic\": [\n");
   OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

   for (i=0; i < JMSG_PLATFORM_TOPIC_PLUGIN_MAX; i++)
   {
      if (JMsgTopicTbl->Data.Topic[i].Enabled)
      {
         if (i > 0)
         {
            sprintf(DumpRecord,",\n");
            OS_write(FileHandle,DumpRecord,strlen(DumpRecord));      
         }
         sprintf(DumpRecord,"   {\n      \"name\": \"%s\",\n      \"cfe\": %d,\n      \"protocol\": \"%s\",\n      \"sb-role\": \"%s\",\n      \"enabled\": \"%s\"\n   }",
                 JMsgTopicTbl->Data.Topic[i].Name, JMsgTopicTbl->Data.Topic[i].Cfe, JMsgTopicTbl->Data.Topic[i].ProtStr, JMsgTopicTbl->Data.Topic[i].SbStr, JMsgTopicTbl->Data.Topic[i].EnaStr);
         OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
      }
   }

   sprintf(DumpRecord,"]");
   OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

   return true;
   
} /* End of JMSG_TOPIC_TBL_DumpCmd() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_EnablePlugin
**
** Enable a topic plugin
** 
** Notes:
**   1. This function operates at the table scope so it is only concerned with
**      validating table parameters. If this returns true then the calling
**      function can take action such as managing subscriptions. 
**
*/
bool JMSG_TOPIC_TBL_EnablePlugin(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_PLATFORM_TOPIC_PLUGIN_MAX)
   {      
      if (JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to enable topic plugin %d that is already enabled",
                            TopicPlugin);
      }
      else
      {
         if ((JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_PUBLISH) ||
             (JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_SUBSCRIBE))
         {
            JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled = true;
            RetStatus = true;   
         }
         else
         {
            CFE_EVS_SendEvent(JMSG_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                              "Attempt to enable topic plugin ID %d with an invalid sb-role definition",
                              TopicPlugin);             
         }
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR,
                        "Attempt to enable a topic plugin with an invalid topic plugin ID %d",
                        TopicPlugin);           
   }
   
   return RetStatus;
   
} /* JMSG_TOPIC_TBL_EnablePlugin() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetCfeToJson
**
** Return a pointer to the CfeToJson conversion function for 'TopicPlugin' and return a
** pointer to the JSON topic string in JsonMsgTopic.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_PLATFORM_TOPIC_PLUGIN_MAX
**
*/
JMSG_TOPIC_TBL_CfeToJson_t JMSG_TOPIC_TBL_GetCfeToJson(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin, 
                                                       const char **JsonMsgTopic)
{

   JMSG_TOPIC_TBL_CfeToJson_t CfeToJsonFunc = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
         *JsonMsgTopic = JMsgTopicTbl->Data.Topic[TopicPlugin].Name;
         CfeToJsonFunc = PluginFuncTbl[TopicPlugin].CfeToJson;
   }

   return CfeToJsonFunc;
   
} /* End JMSG_TOPIC_TBL_GetCfeToJson() */


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
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetDisabledTopic(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   JMSG_TOPIC_TBL_Topic_t *Topic = NULL;

   if (TopicPlugin < JMSG_PLATFORM_TOPIC_PLUGIN_MAX)
   {
      if ((JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_PUBLISH)||
          (JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_SUBSCRIBE))
      {
         Topic = &JMsgTopicTbl->Data.Topic[TopicPlugin];
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table topic plugin ID %d is out of range. It must less than %d",
                        TopicPlugin, JMSG_PLATFORM_TOPIC_PLUGIN_MAX);
   }
   
   return Topic;
   
} /* End JMSG_TOPIC_TBL_GetDisabledTopic() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_TopicPlugin_Enum_t_MAX
**
*/
JMSG_TOPIC_TBL_JsonToCfe_t JMSG_TOPIC_TBL_GetJsonToCfe(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   JMSG_TOPIC_TBL_JsonToCfe_t JsonToCfeFunc = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      JsonToCfeFunc = PluginFuncTbl[TopicPlugin].JsonToCfe;
   }

   return JsonToCfeFunc;
   
} /* End JMSG_TOPIC_TBL_GetJsonToCfe() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetTopic
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_TopicPlugin_Enum_t_MAX
**
*/
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetTopic(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   JMSG_TOPIC_TBL_Topic_t *Topic = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      Topic = &JMsgTopicTbl->Data.Topic[TopicPlugin];
   }

   return Topic;
   
} /* End JMSG_TOPIC_TBL_GetTopic() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetTopicProtocol
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_TopicPlugin_Enum_t_MAX
**
*/
JMSG_LIB_TopicProtocol_Enum_t JMSG_TOPIC_TBL_GetTopicProtocol(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   JMSG_LIB_TopicProtocol_Enum_t TopicProtocol = JMSG_LIB_TopicProtocol_UNDEF;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      TopicProtocol = JMsgTopicTbl->Data.Topic[TopicPlugin].Protocol;
   }

   return TopicProtocol;
   
} /* End JMSG_TOPIC_TBL_GetTopicProtocol() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. This could migrate into table manager but I think I'll keep it here so
**     user's can add table processing code if needed.
*/
bool JMSG_TOPIC_TBL_LoadCmd(APP_C_FW_TblLoadOptions_Enum_t LoadType, const char *Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, JMsgTopicTbl->JsonBuf, JMSG_PLATFORM_JSON_TOPIC_TBL_MAX_CHAR, LoadJsonData))
   {
      JMsgTopicTbl->Loaded = true;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JMSG_TOPIC_TBL_LoadCmd() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_MsgIdToTopicPlugin
**
** Return a topic plugin ID for a message ID
** 
** Notes:
**   1. JMSG_PLATFORM_TOPIC_PLUGIN_UNDEF is returned if the message ID isn't found.
**
*/
uint8 JMSG_TOPIC_TBL_MsgIdToTopicPlugin(CFE_SB_MsgId_t MsgId)
{
   
   uint8  TopicPlugin = JMSG_PLATFORM_TOPIC_PLUGIN_UNDEF;
   uint32 MsgIdValue = CFE_SB_MsgIdToValue(MsgId);
  
   for (JMSG_PLATFORM_TopicPlugin_Enum_t i=0; i < JMSG_PLATFORM_TOPIC_PLUGIN_MAX; i++)
   {
      if (MsgIdValue == JMsgTopicTbl->Data.Topic[i].Cfe)
      {
         TopicPlugin = i;
         break;
      }
   }

   return TopicPlugin;
   
} /* End JMSG_TOPIC_TBL_MsgIdToTopicPlugin() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback
**
** Register the callback function that will be called when a topic is enabled
** or disabled and the table owner needs to manage netwrok layer subscriptions. 
**
*/
bool JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin,
                                                       JMSG_TOPIC_TBL_SubscriptionCallback_t SubscriptionCallback)
{
   
   bool RetStatus = false;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback = SubscriptionCallback;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RegisterPlugin
**
** Register a user topic plugin.
** TODO: Verify the TopicPlugin ID is in the USR range.
**
*/
 CFE_SB_MsgId_t JMSG_TOPIC_TBL_RegisterPlugin(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin,
                                              JMSG_TOPIC_TBL_CfeToJson_t  CfeToJson,
                                              JMSG_TOPIC_TBL_JsonToCfe_t  JsonToCfe,
                                              JMSG_TOPIC_TBL_PluginTest_t PluginTest)
{

   const JMSG_TOPIC_TBL_Class_t *TopicTbl = JMSG_LIB_GetTopicTbl();
 
   PluginFuncTbl[TopicPlugin].CfeToJson  = CfeToJson;
   PluginFuncTbl[TopicPlugin].JsonToCfe  = JsonToCfe;
   PluginFuncTbl[TopicPlugin].PluginTest = PluginTest;

   return  CFE_SB_ValueToMsgId(TopicTbl->Data.Topic[TopicPlugin].Cfe);
 
  
} /* End JMSG_TOPIC_TBL_RegisterPlugin() */
   
   
/******************************************************************************
** Function: JMSG_TOPIC_TBL_ResetStatus
**
*/
void JMSG_TOPIC_TBL_ResetStatus(void)
{

   JMsgTopicTbl->LastLoadCnt = 0;
 
} /* End JMSG_TOPIC_TBL_ResetStatus() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RunTopicPluginTest
**
** Notes:
**   1. Assumes Plugin ID enumeration value is a PluginFuncTbl index and that
**      the PluginFuncTbl array does not have any NULL pointers 
**
*/
void JMSG_TOPIC_TBL_RunTopicPluginTest(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin, 
                                       bool Init, int16 Param)
{
   
   JMsgTopicTbl->PluginTestId    = TopicPlugin;
   JMsgTopicTbl->PluginTestParam = Param;
   (PluginFuncTbl[TopicPlugin].PluginTest)(Init, Param);

} /* End JMSG_TOPIC_TBL_RunTopicPluginTest() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SendTlmCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used.
**
*/
bool JMSG_TOPIC_TBL_SendTlmCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   JMSG_LIB_TopicTblTlm_Payload_t *Payload = &JMsgTopicTbl->TopicTblTlm.Payload;

   for (JMSG_PLATFORM_TopicPlugin_Enum_t i=JMSG_PLATFORM_TopicPlugin_Enum_t_MIN; i <= JMSG_PLATFORM_TopicPlugin_Enum_t_MAX; i++)
   {
      JMSG_LIB_PluginDescr_t *PluginDescr  = &Payload->TopicPlugin[i];
      const JMSG_TOPIC_TBL_Topic_t *Topic = &JMsgTopicTbl->Data.Topic[i];
      
      strncpy(PluginDescr->Name, Topic->Name, JMSG_PLATFORM_TOPIC_NAME_MAX_LEN);
      PluginDescr->Enabled  = Topic->Enabled;
      PluginDescr->Protocol = Topic->Protocol;
      PluginDescr->SbRole   = Topic->SbRole;
   }

   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgTopicTbl->TopicTblTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgTopicTbl->TopicTblTlm.TelemetryHeader), true);

   return true;
   
} /* End JMSG_TOPIC_TBL_SendTlmCmd() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SubscribeToAll
**
** Subscribe to all topics defined in the topic table. A topic's SbRole
** determines which type of subscription will be performed.
**
** Notes:
**   1. This should only be used when only one JMSG network app is using
**      JMSG_LIB because when it is called the app's SubscriptionCallback()
**      function is called for every enabled topic table entry.
**   2. JMSG_TOPIC_TBL_SubscribeToTopicMsg() to configure individual entries.
**   TODO: Current design requires code changes to the JMSG network app's
**   TODO: constructor. Create a scheme that is table or EDS based.
*/
void JMSG_TOPIC_TBL_SubscribeToAll(JMSG_TOPIC_TBL_TopicSubscribeToEnum_t SubscribeTo)
{

   JMSG_TOPIC_TBL_SubscriptionOptEnum_t TopicSubscription;
   
   uint16 SbSubscribeCnt = 0;
   uint16 JMsgSubscribeCnt = 0;
   uint16 SubscribeErr = 0;
   
 
   for (JMSG_PLATFORM_TopicPlugin_Enum_t i=JMSG_PLATFORM_TopicPlugin_Enum_t_MIN; i <= JMSG_PLATFORM_TopicPlugin_Enum_t_MAX; i++)
   {
      TopicSubscription = JMSG_TOPIC_TBL_SubscribeToTopicMsg(i, SubscribeTo);
      switch (TopicSubscription)
      {
         case JMSG_TOPIC_TBL_SUB_SB:
            SbSubscribeCnt++;
            break;
         case JMSG_TOPIC_TBL_SUB_JMSG:
            JMsgSubscribeCnt++;
            break;
         case JMSG_TOPIC_TBL_SUB_ERR:
            SubscribeErr++;
            break;
         default:
            break;
      } /* End switch */
   } /* End topic loop */

   CFE_EVS_SendEvent(JMSG_TOPIC_TBL_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                     "Topic table subscriptions performed: SB %d, JMSG %d, Errors %d",
                     SbSubscribeCnt, JMsgSubscribeCnt, SubscribeErr);
 
} /* End JMSG_TOPIC_TBL_SubscribeToAll() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_SubscribeToTopicMsg
**
** Performs all processing relevant to the topic table and calls the table
** owner's callback function so it can perform network level subscription
** functions.
*/
JMSG_TOPIC_TBL_SubscriptionOptEnum_t JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin,
                                                                        JMSG_TOPIC_TBL_TopicSubscribeToEnum_t SubscribeTo)
{
   
   const JMSG_TOPIC_TBL_Topic_t *Topic;
   JMSG_TOPIC_TBL_SubscriptionOptEnum_t SubscriptionOpt = JMSG_TOPIC_TBL_SUB_ERR;

   Topic = JMSG_TOPIC_TBL_GetTopic(TopicPlugin);
   if (Topic != NULL)   
   {
      if (Topic->Enabled)
      {
         // Table load logic doesn't enable an invalid SbRole so don't report invalid
         if (Topic->SbRole == JMSG_LIB_TopicSbRole_PUBLISH)
         {
            if (SubscribeTo == JMSG_TOPIC_TBL_SUB_TO_ROLE || SubscribeTo == JMSG_TOPIC_TBL_SUB_TO_JMSG)
            {
               if (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback != NULL)
               {
                  if ((JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback)(Topic, JMSG_TOPIC_TBL_SUB_JMSG))
                  {         
                     SubscriptionOpt = JMSG_TOPIC_TBL_SUB_JMSG;
                  }
               }
            }
         } /* End Publish */
         else if (Topic->SbRole == JMSG_LIB_TopicSbRole_SUBSCRIBE)
         {
            if (SubscribeTo == JMSG_TOPIC_TBL_SUB_TO_ROLE || SubscribeTo == JMSG_TOPIC_TBL_SUB_TO_SB)
            {
               if (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback != NULL)
               {
                  if ((JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback)(Topic, JMSG_TOPIC_TBL_SUB_SB))
                  {         
                    SubscriptionOpt = JMSG_TOPIC_TBL_SUB_SB;
                  }
               }
            }
         } /* End Subscribe */
      } /* End if enabled */
      else
      {
         SubscriptionOpt = JMSG_TOPIC_TBL_SUB_UNDEF;
      }
   } /* End if not NULL */

   return SubscriptionOpt;
   
} /* End JMSG_TOPIC_TBL_SubscribeToTopicMsg() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_UnsubscribeFromTopicMsg
**
** Performs all processing relevant to the topic table and calls the table
** owner's callback function so it can perform network level unsubscribe
** functions.
**
*/
bool JMSG_TOPIC_TBL_UnsubscribeFromTopicMsg(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   const JMSG_TOPIC_TBL_Topic_t *Topic;
   bool RetStatus = false;


   Topic = JMSG_TOPIC_TBL_GetDisabledTopic(TopicPlugin);
   if (Topic != NULL)   
   {
      if (Topic->Enabled == false)
      {
         if (Topic->SbRole == JMSG_LIB_TopicSbRole_PUBLISH)
         {
            if (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback != NULL)
            {
               RetStatus = (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback)(Topic, JMSG_TOPIC_TBL_UNSUB_JMSG);
            }
         }
         else if (Topic->SbRole == JMSG_LIB_TopicSbRole_SUBSCRIBE)
         {
            if (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback != NULL)
            {
               RetStatus = (JMsgTopicTbl->Data.Topic[TopicPlugin].SubscriptionCallback)(Topic, JMSG_TOPIC_TBL_UNSUB_SB);
            }
         }
      } /* End if disabled */
   } /* End if not NULL */

   return RetStatus;
 
} /* End JMSG_TOPIC_TBL_UnsubscribeFromTopicMsg() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_ValidTopicPlugin
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool JMSG_TOPIC_TBL_ValidTopicPlugin(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin)
{

   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_PLATFORM_TOPIC_PLUGIN_MAX)
   {
      if (JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled &&
         ((JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_PUBLISH)||
          (JMsgTopicTbl->Data.Topic[TopicPlugin].SbRole == JMSG_LIB_TopicSbRole_SUBSCRIBE)))
      {
         RetStatus = true;
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table topic plugin ID %d is out of range. ID must be less than %d",
                        TopicPlugin, JMSG_PLATFORM_TOPIC_PLUGIN_MAX);
   }

   return RetStatus;
   

} /* End JMSG_TOPIC_TBL_ValidTopicPlugin() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**   None
*/
static bool LoadJsonData(size_t JsonFileLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;


   JMsgTopicTbl->JsonFileLen = JsonFileLen;

   /* 
   ** 1. Copy table owner data into local table buffer
   ** 2. Process JSON file which updates local table buffer with JSON supplied values
   ** 3. If valid, copy local buffer over owner's data 
   */
   
   memcpy(&TblData, &JMsgTopicTbl->Data, sizeof(JMSG_TOPIC_TBL_Data_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, JMsgTopicTbl->JsonObjCnt, JMsgTopicTbl->JsonBuf, JMsgTopicTbl->JsonFileLen);

   if (!JMsgTopicTbl->Loaded && (ObjLoadCnt != JMsgTopicTbl->JsonObjCnt))
   {

      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table has never been loaded and new table only contains %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)JMsgTopicTbl->JsonObjCnt);
   
   }
   else
   {

      for (JMSG_PLATFORM_TopicPlugin_Enum_t i=JMSG_PLATFORM_TopicPlugin_Enum_t_MIN; i <= JMSG_PLATFORM_TopicPlugin_Enum_t_MAX; i++)
      {

         TblData.Topic[i].Enabled = (strcmp(TblData.Topic[i].EnaStr, "true") == 0);

         if (strcmp(TblData.Topic[i].ProtStr,"mqtt") == 0)
         {
             TblData.Topic[i].Protocol = JMSG_LIB_TopicProtocol_MQTT;
         }
         else if (strcmp(TblData.Topic[i].ProtStr,"udp") == 0)
         {
             TblData.Topic[i].Protocol = JMSG_LIB_TopicProtocol_UDP;
         }            
         else
         {
            TblData.Topic[i].Enabled  = false; 
            TblData.Topic[i].Protocol = JMSG_LIB_TopicProtocol_INVALID;
            CFE_EVS_SendEvent(JMSG_TOPIC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "Invalid protocol '%s'. It must be either 'mqtt' or 'udp'", TblData.Topic[i].ProtStr);
         }

         if (strcmp(TblData.Topic[i].SbStr,"pub") == 0)
         {         
             TblData.Topic[i].SbRole = JMSG_LIB_TopicSbRole_PUBLISH;
         }
         else if (strcmp(TblData.Topic[i].SbStr,"sub") == 0)
         {
             TblData.Topic[i].SbRole = JMSG_LIB_TopicSbRole_SUBSCRIBE;
         }            
         else
         {
            TblData.Topic[i].Enabled = false; 
            TblData.Topic[i].SbRole  = JMSG_LIB_TopicSbRole_INVALID;
            CFE_EVS_SendEvent(JMSG_TOPIC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "Invalid sb-role '%s'. It must be either 'pub' or 'sub'", TblData.Topic[i].SbStr);
         }
         
      } /* End topic loop */
      
      memcpy(&JMsgTopicTbl->Data,&TblData, sizeof(JMSG_TOPIC_TBL_Data_t));
      JMsgTopicTbl->LastLoadCnt = ObjLoadCnt;

      JMSG_TOPIC_PLUGIN_Constructor(&JMsgTopicTbl->Data,PluginFuncTbl,
                                    JMsgTopicTbl->PluginTestTlmTopicId);

      RetStatus = true;
      
   } /* End if valid JSON object count */
   
   return RetStatus;
   
} /* End LoadJsonData() */
