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
**  Purpose:
**    Provide an interface for JSON Message topic plugins.
**
**  Notes:
**    None 
**
*/

#ifndef _jmsg_topic_plugins_
#define _jmsg_topic_plugins_


/*
** Include Files:
*/

#include "lib_cfg.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Event Message IDs
*/

#define JMSG_TOPIC_PLUGIN_EID        (JMSG_PLATFORM_TOPIC_PLUGIN_BASE_EID + 0)
#define JMSG_TOPIC_PLUGIN_STUB_EID   (JMSG_PLATFORM_TOPIC_PLUGIN_BASE_EID + 1)

/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_PLUGIN_Constructor
**
** Call constructors for each topic plugin.
**
** Notes:
**   1. 
**
*/
void JMSG_TOPIC_PLUGIN_Constructor(const JMSG_TOPIC_TBL_Data_t *TopicTbl,
                                   JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                   uint32 PluginTestTlmTopicId);


#endif /* _jmsg_topic_plugins_ */
