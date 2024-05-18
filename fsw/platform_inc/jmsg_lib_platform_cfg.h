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
**    Define platform configurations for the JSON message library
**
**  Notes:
**    1. The JSON_MSG_TOPIC_LEN definition is based on KIT_TO's definition
**       of the maximum SB message that can be wrapped. The wrapped messages
**       are expected to come from JMSG_LIB.
**
*/

#ifndef _jmsg_lib_platform_cfg_
#define _jmsg_lib_platform_cfg_

/*
** Includes
*/

#include "jmsg_lib_mission_cfg.h"
#include "kit_to_eds_defines.h"

/******************************************************************************
** Platform Deployment Configurations
*/

#define JMSG_LIB_PLATFORM_REV   0
#define JMSG_LIB_INI_FILENAME   "/cf/jmsg_lib_ini.json"


/******************************************************************************
** TODO: Move JMSG_LIB_TOPIC_LEN to EDS?
*/
#define JMSG_LIB_TOPIC_LEN  KIT_TO_MAX_WRAPPED_SB_MSG_LEN


#endif /* _jmsg_lib_platform_cfg_ */
