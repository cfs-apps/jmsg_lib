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
**   Construct user topic plugins
**
** Notes:
**   1. Always construct plugins regardless of whether they are enabled. This
**      allows plugins to be enabled/disabled during runtime. If a topic's
**      JSON is invalid the plugin will be automatically disabled.
**   1. See jmsg_lib/docs/jmsg_topic_plugin_guide.txt for 
**      plugin creation and installation instructions
** 
*/

/*
** Include Files:
*/

#include "usr_tplug_eds_typedefs.h"
#include "tplug_web_discrete.h"
#include "tplug_web_rate.h"
#include "tplug_rpi_demo.h"

/******************************************************************************
** Function: USR_TPLUG_Constructor
**
** Construct user topic plugins
**
*/
void USR_TPLUG_Constructor(void)
{
 
   TPLUG_WEB_DISCRETE_Constructor(USR_TPLUG_Id_WebDiscrete);
   TPLUG_WEB_RATE_Constructor(USR_TPLUG_Id_WebRate);
       
   TPLUG_RPI_DEMO_Constructor(USR_TPLUG_Id_RpiDemo);
       
} /* USR_TPLUG_Constructor() */
