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
**   Export TPLUG_WEB Discrete constructor
**
** Notes:
**   1. Other than the constructor, no other plugin definitions should be
**      exported. Plugin interfaces are through the callback functions
**      that are registered by the plugin's constructor.
**
*/

#ifndef _tplug_web_discrete_
#define _tplug_web_discrete_

/*
** Includes
*/

#include "jmsg_platform_eds_typedefs.h"


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: TPLUG_WEB_DISCRETE_Constructor
**
** Initialize the JMSG MQTT discrete plugin topic
*/
void TPLUG_WEB_DISCRETE_Constructor(JMSG_PLATFORM_TopicPlugin_Enum_t TopicPlugin);


#endif /* _tplug_web_discrete_ */
