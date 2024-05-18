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
#include "jmsg_topic_plugin.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define JSON_MAX_TOPIC_LEN  JMSG_USR_MAX_TOPIC_NAME_LEN  // Shorter & more ledgable for table use

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
   
   { &TblData.Topic[0].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[0].jmsg",     (sizeof("topic[0].jmsg")-1)}},
   { &TblData.Topic[0].Cfe,     2,                  false,   JSONNumber, false, { "topic[0].cfe",      (sizeof("topic[0].cfe")-1)}},
   { &TblData.Topic[0].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].sb-role",  (sizeof("topic[0].sb-role")-1)}},
   { &TblData.Topic[0].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].enabled",  (sizeof("topic[0].enabled")-1)}},

   { &TblData.Topic[1].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[1].jmsg",     (sizeof("topic[1].jmsg")-1)}},
   { &TblData.Topic[1].Cfe,     2,                  false,   JSONNumber, false, { "topic[1].cfe",      (sizeof("topic[1].cfe")-1)}},
   { &TblData.Topic[1].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].sb-role",  (sizeof("topic[1].sb-role")-1)}},
   { &TblData.Topic[1].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].enabled",  (sizeof("topic[1].enabled")-1)}},

   { &TblData.Topic[2].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[2].jmsg",     (sizeof("topic[2].jmsg")-1)}},
   { &TblData.Topic[2].Cfe,     2,                  false,   JSONNumber, false, { "topic[2].cfe",      (sizeof("topic[2].cfe")-1)}},
   { &TblData.Topic[2].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].sb-role",  (sizeof("topic[2].sb-role")-1)}},
   { &TblData.Topic[2].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].enabled",  (sizeof("topic[2].enabled")-1)}},

   { &TblData.Topic[3].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[3].jmsg",     (sizeof("topic[3].jmsg")-1)}},
   { &TblData.Topic[3].Cfe,     2,                  false,   JSONNumber, false, { "topic[3].cfe",      (sizeof("topic[3].cfe")-1)}},
   { &TblData.Topic[3].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].sb-role",  (sizeof("topic[3].sb-role")-1)}},
   { &TblData.Topic[3].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].enabled",  (sizeof("topic[3].enabled")-1)}},

   { &TblData.Topic[4].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[4].jmsg",     (sizeof("topic[4].jmsg")-1)}},
   { &TblData.Topic[4].Cfe,     2,                  false,   JSONNumber, false, { "topic[4].cfe",      (sizeof("topic[4].cfe")-1)}},
   { &TblData.Topic[4].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].sb-role",  (sizeof("topic[4].sb-role")-1)}},
   { &TblData.Topic[4].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].enabled",  (sizeof("topic[4].enabled")-1)}},

   { &TblData.Topic[5].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[5].jmsg",     (sizeof("topic[5].jmsg")-1)}},
   { &TblData.Topic[5].Cfe,     2,                  false,   JSONNumber, false, { "topic[5].cfe",      (sizeof("topic[5].cfe")-1)}},
   { &TblData.Topic[5].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].sb-role",  (sizeof("topic[5].sb-role")-1)}},
   { &TblData.Topic[5].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].enabled",  (sizeof("topic[5].enabled")-1)}},

   { &TblData.Topic[6].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[6].jmsg",     (sizeof("topic[6].jmsg")-1)}},
   { &TblData.Topic[6].Cfe,     2,                  false,   JSONNumber, false, { "topic[6].cfe",      (sizeof("topic[6].cfe")-1)}},
   { &TblData.Topic[6].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].sb-role",  (sizeof("topic[6].sb-role")-1)}},
   { &TblData.Topic[6].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].enabled",  (sizeof("topic[6].enabled")-1)}},

   { &TblData.Topic[7].JMsg,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[7].jmsg",     (sizeof("topic[7].jmsg")-1)}},
   { &TblData.Topic[7].Cfe,     2,                  false,   JSONNumber, false, { "topic[7].cfe",      (sizeof("topic[7].cfe")-1)}},
   { &TblData.Topic[7].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].sb-role",  (sizeof("topic[7].sb-role")-1)}},
   { &TblData.Topic[7].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].enabled",  (sizeof("topic[7].enabled")-1)}}
   
};


// Table is populated by topic plugin constructors
static JMSG_TOPIC_TBL_PluginFuncTbl_t PluginFuncTbl[JMSG_USR_TopicPlugin_Enum_t_MAX];

/******************************************************************************
** Function: JMSG_TOPIC_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void JMSG_TOPIC_TBL_Constructor(JMSG_TOPIC_TBL_Class_t *JMsgTopicTblPtr,
                                uint32 TestTlmTopicId)
{

   enum JMSG_USR_TopicPlugin i;
   JMsgTopicTbl = JMsgTopicTblPtr;

   CFE_PSP_MemSet(JMsgTopicTbl, 0, sizeof(JMSG_TOPIC_TBL_Class_t));

   JMsgTopicTbl->TestTlmTopicId = TestTlmTopicId;
   JMsgTopicTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   for (i=0; i < JMSG_USR_TopicPlugin_Enum_t_MAX; i++)
   {
      TblData.Topic[i].Enabled = false;
      JMsgTopicTbl->Data.Topic[i].Enabled = false;
      JMsgTopicTbl->Data.Topic[i].SbRole = JMSG_LIB_TopicSbRole_UNDEF;
   }
   
   // Plugin stubs loaded for disabled entries
   JMSG_TOPIC_PLUGIN_Constructor(&JMsgTopicTbl->Data, PluginFuncTbl,
                                  JMsgTopicTbl->TestTlmTopicId);
                                    

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
bool JMSG_TOPIC_TBL_DisablePlugin(enum JMSG_USR_TopicPlugin TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_USR_TopicPlugin_Enum_t_MAX)
   {
      if (JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled = false;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to disable topic plugin ID %d(index %d) that is already disabled",
                           (TopicPlugin+1), TopicPlugin);
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                        "Attempt to disable a topic plugin with an invalid topic plugin ID %d(index %d)",
                        (TopicPlugin+1), TopicPlugin);             
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

   for (i=0; i < JMSG_USR_TopicPlugin_Enum_t_MAX; i++)
   {
      if (JMsgTopicTbl->Data.Topic[i].Enabled)
      {
         if (i > 0)
         {
            sprintf(DumpRecord,",\n");
            OS_write(FileHandle,DumpRecord,strlen(DumpRecord));      
         }
         sprintf(DumpRecord,"   {\n         \"mqtt\": \"%s\",\n         \"cfe\": %d,\n         \"sb-role\": \"%s\",\n         \"enabled\": \"%s\"\n      }",
                 JMsgTopicTbl->Data.Topic[i].JMsg, JMsgTopicTbl->Data.Topic[i].Cfe, JMsgTopicTbl->Data.Topic[i].SbStr, JMsgTopicTbl->Data.Topic[i].EnaStr);
         OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
      }
   }

   sprintf(DumpRecord,"   ]\n");
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
bool JMSG_TOPIC_TBL_EnablePlugin(enum JMSG_USR_TopicPlugin TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_USR_TopicPlugin_Enum_t_MAX)
   {      
      if (JMsgTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         CFE_EVS_SendEvent(JMSG_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to enable topic plugin %d(index %d) that is already enabled",
                           (TopicPlugin+1), TopicPlugin);
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
                              "Attempt to enable topic plugin ID %d(index %d) with an invalid sb-role definition",
                              (TopicPlugin+1), TopicPlugin);             
         }
      }
   }
   else
   {
      CFE_EVS_SendEvent(JMSG_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR,
                        "Attempt to enable a topic plugin with an invalid topic plugin ID %d(index %d)",
                        (TopicPlugin+1), TopicPlugin);           
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
**   1. TopicPlugin must be less than JMSG_USR_TopicPlugin_Enum_t_MAX
**
*/
JMSG_TOPIC_TBL_CfeToJson_t JMSG_TOPIC_TBL_GetCfeToJson(enum JMSG_USR_TopicPlugin TopicPlugin, 
                                                       const char **JsonMsgTopic)
{

   JMSG_TOPIC_TBL_CfeToJson_t CfeToJsonFunc = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
         *JsonMsgTopic = JMsgTopicTbl->Data.Topic[TopicPlugin].JMsg;
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
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetDisabledTopic(enum JMSG_USR_TopicPlugin TopicPlugin)
{

   JMSG_TOPIC_TBL_Topic_t *Topic = NULL;

   if (TopicPlugin < JMSG_USR_TopicPlugin_Enum_t_MAX)
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
                        "Table topic plugin ID %d(index %d) is out of range. It must less than %d",
                        (TopicPlugin+1), TopicPlugin, JMSG_USR_TopicPlugin_Enum_t_MAX);
   }
   
   return Topic;
   
} /* End JMSG_TOPIC_TBL_GetDisabledTopic() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetTopic
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_TopicPlugin_Enum_t_MAX
**
*/
const JMSG_TOPIC_TBL_Topic_t *JMSG_TOPIC_TBL_GetTopic(enum JMSG_USR_TopicPlugin TopicPlugin)
{

   JMSG_TOPIC_TBL_Topic_t *Topic = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      Topic = &JMsgTopicTbl->Data.Topic[TopicPlugin];
   }

   return Topic;
   
} /* End JMSG_TOPIC_TBL_GetTopic() */



/******************************************************************************
** Function: JMSG_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than JMSG_TopicPlugin_Enum_t_MAX
**
*/
JMSG_TOPIC_TBL_JsonToCfe_t JMSG_TOPIC_TBL_GetJsonToCfe(enum JMSG_USR_TopicPlugin TopicPlugin)
{

   JMSG_TOPIC_TBL_JsonToCfe_t JsonToCfeFunc = NULL;
   
   if (JMSG_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      JsonToCfeFunc = PluginFuncTbl[TopicPlugin].JsonToCfe;
   }

   return JsonToCfeFunc;
   
} /* End JMSG_TOPIC_TBL_GetJsonToCfe() */


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

   if (CJSON_ProcessFile(Filename, JMsgTopicTbl->JsonBuf, JMSG_TOPIC_TBL_JSON_FILE_MAX_CHAR, LoadJsonData))
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
**   1. JMSG_USR_TopicPlugin_UNDEF is returned if the message ID isn't found.
**
*/
uint8 JMSG_TOPIC_TBL_MsgIdToTopicPlugin(CFE_SB_MsgId_t MsgId)
{
   
   uint8  TopicPlugin = JMSG_USR_TopicPlugin_UNDEF;
   uint32 MsgIdValue = CFE_SB_MsgIdToValue(MsgId);
  
   for (enum JMSG_USR_TopicPlugin i=0; i < JMSG_USR_TopicPlugin_Enum_t_MAX; i++)
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
** Function: JMSG_TOPIC_TBL_ResetStatus
**
*/
void JMSG_TOPIC_TBL_ResetStatus(void)
{

   JMsgTopicTbl->LastLoadCnt = 0;
 
} /* End JMSG_TOPIC_TBL_ResetStatus() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_RunSbMsgTest
**
** Notes:
**   1. Assumes Plugin ID enumeration value is a PluginFuncTbl index and that
**      the PluginFuncTbl array does not have any NULL pointers 
**
*/
void JMSG_TOPIC_TBL_RunSbMsgTest(enum JMSG_USR_TopicPlugin TopicPlugin, 
                                 bool Init, int16 Param)
{

   (PluginFuncTbl[TopicPlugin].SbMsgTest)(Init, Param);

} /* End JMSG_TOPIC_TBL_RunSbMsgTest() */


/******************************************************************************
** Function: JMSG_TOPIC_TBL_ValidId
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool JMSG_TOPIC_TBL_ValidTopicPlugin(enum JMSG_USR_TopicPlugin TopicPlugin)
{

   bool RetStatus = false;
   
   if (TopicPlugin < JMSG_USR_TopicPlugin_Enum_t_MAX)
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
                        "Table topic plugin ID %d(index %d) is out of range. ID must be less than %d",
                        (TopicPlugin+1), TopicPlugin, (JMSG_USR_TopicPlugin_Enum_t_MAX+1));
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

      for (int i=0; i < JMSG_USR_TopicPlugin_Enum_t_MAX; i++)
      {
         TblData.Topic[i].Enabled = (strcmp(TblData.Topic[i].EnaStr, "true") == 0);
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
      }
      memcpy(&JMsgTopicTbl->Data,&TblData, sizeof(JMSG_TOPIC_TBL_Data_t));
      JMsgTopicTbl->LastLoadCnt = ObjLoadCnt;

      JMSG_TOPIC_PLUGIN_Constructor(&JMsgTopicTbl->Data,PluginFuncTbl,
                                    JMsgTopicTbl->TestTlmTopicId);

      RetStatus = true;
      
   } /* End if valid JSON object count */
   
   return RetStatus;
   
} /* End LoadJsonData() */


