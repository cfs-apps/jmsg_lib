/*
**  Copyright 2022 bitValence, Inc.
**  All Rights Reserved.
**
**  This program is free software; you can modify and/or redistribute it
**  under the terms of the GNU Affero General Public License
**  as published by the Free Software Foundation; version 3 with
**  attribution addendums as found in the LICENSE.txt
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  Purpose:
**    Entry point function for JSON Message library
**
**  Notes:
**    None
**
*/

/*
** Includes
*/

#include "lib_cfg.h"
#include "jmsg_lib.h"


/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ   (&JMsgLib.IniTbl)


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in lib_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/

DEFINE_ENUM(Config, LIB_CONFIG)  


/*****************/
/** Global Data **/
/*****************/

JMSG_LIB_Class_t  JMsgLib;


/*******************************/
/** Local Function Prototypes **/
/*******************************/


/******************************************************************************
** Entry function
**
** Notes:
**   1. This is called by the cFE during lib/app loading and initialization and
**      serves as the object constructor in the Basecamp design model.
**   2. RetStatus is used to notify the cFE if there was a critical error 
**      loading the library. If this happens then the symbols required by the
**      app using the library will fail to load because the symbols won't be
**      defined. JMsgLib.Initialized indicates to the app whether the library
**      is ready for use by an app.  
*/
uint32 JMSG_LibInit(void)
{

   uint32 RetStatus = OS_SUCCESS; 
   
   memset((void*)&JMsgLib, 0, sizeof(JMSG_LIB_Class_t));
   
   /* IniTbl will report errors */
   if (INITBL_Constructor(INITBL_OBJ, JMSG_LIB_INI_FILENAME, &IniCfgEnum))
   {
      
      JMsgLib.ToPubWrappedTlmTopicId = INITBL_GetIntConfig(INITBL_OBJ, CFG_KIT_TO_PUB_WRAPPED_TLM_TOPICID);

      JMSG_TOPIC_TBL_Constructor(&JMsgLib.TopicTbl, 
                                 INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_LIB_TOPIC_TBL_TLM_TOPICID));

      JMsgLib.Initialized = true;

      OS_printf("JSON Message(JMSG) Library Initialized. Version %d.%d.%d\n",
                JMSG_LIB_MAJOR_VER, JMSG_LIB_MINOR_VER, JMSG_LIB_PLATFORM_REV);
   }
   else
   {

      // Keep library loaded: RetStatus = OS_ERROR;
      JMsgLib.Initialized = false;
      OS_printf("Error initializing JSON Message(JMSG) Library. Version %d.%d.%d\n",
                JMSG_LIB_MAJOR_VER, JMSG_LIB_MINOR_VER, JMSG_LIB_PLATFORM_REV);      
            
   }
   
   return RetStatus;

} /* End JMSG_LibInit() */

/******************************************************************************
** Function: JMSG_LIB_GetTopicTbl
**
** Return a pointer to library's instance of JMSG_TOPICC_TBL
**
** Notes:
**   1. Only one JMSG_LIB can exist in a cFS target
**
*/
const JMSG_TOPIC_TBL_Class_t *JMSG_LIB_GetTopicTbl(void)
{
   
   return  &JMsgLib.TopicTbl;
   
} /* End JMSG_LIB_GetTopicTbl() */