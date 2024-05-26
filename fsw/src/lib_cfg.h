/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Define platform configurations for the JSOM Message library
**
** Notes:
**   1. These macros can only be built with the library and can't
**      have a platform scope because the same lib_cfg.h filename is used
**      for all libraries following the Basecamp app design patterns.
**
*/

#ifndef _lib_cfg_
#define _lib_cfg_

/*
** Includes
*/

#include "jmsg_usr_eds_defines.h"
#include "jmsg_usr_eds_typedefs.h"
#include "jmsg_lib_eds_defines.h"
#include "jmsg_lib_eds_typedefs.h"
#include "jmsg_test_eds_defines.h"
#include "jmsg_test_eds_typedefs.h"

#include "jmsg_lib_platform_cfg.h"
#include "app_c_fw.h"


/******************************************************************************
** Application Macros
*/

/*
** Versions:
**
** 1.0 - Initial release tested with json_gw
*/

#define  JMSG_LIB_MAJOR_VER      0
#define  JMSG_LIB_MINOR_VER      9


/******************************************************************************
** Init File declarations create:
**
**  typedef enum {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigEnum;
**    
**  typedef struct {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigStruct;
**
**   const char *GetConfigStr(value);
**   ConfigEnum GetConfigVal(const char *str);
**
** XX(name,type)
*/

#define CFG_JMSG_LIB_TLM_TOPICID            JMSG_LIB_TLM_TOPICID
#define CFG_JMSG_TEST_PLUGIN_TOPICID        JMSG_TEST_PLUGIN_TOPICID
#define CFG_KIT_TO_PUB_WRAPPED_TLM_TOPICID  KIT_TO_PUB_WRAPPED_TLM_TOPICID

#define LIB_CONFIG(XX) \
   XX(JMSG_LIB_TLM_TOPICID,uint32) \
   XX(JMSG_TEST_PLUGIN_TOPICID,uint32) \
   XX(KIT_TO_PUB_WRAPPED_TLM_TOPICID,uint32)


DECLARE_ENUM(Config, LIB_CONFIG)


/******************************************************************************
** JMSG Topic CCSDS
**
** The CCSDS topic contains a CCSDS message as its payload so this length must
** be large enough to accomodate the largest CCSDS packet that will be sent as
** a JSGM payload.
**
** JMSG_TOPIC_SB_MSG_MAX_LEN can't be defined in EDS because it is used for 
** data structures.
*/

#define JMSG_TOPIC_SB_MSG_MAX_LEN  4096  

#endif /* _lib_cfg_ */
