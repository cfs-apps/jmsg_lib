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
**    Define the JSON Message library 
**
**  Notes:
**    None
**
*/
#ifndef _jmsg_lib_
#define _jmsg_lib_

/*
** Includes
*/

#include "jmsg_topic_tbl.h"


/***********************/
/** Macro Definitions **/
/***********************/


/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{

   /* 
   ** App Framework
   */ 
   
   INITBL_Class_t  IniTbl;
   
   /* 
   ** State 
   */ 

   bool Initialized;

   uint32  ToPubWrappedTlmTopicId;

   /*
   ** Contained Objects
   */

   JMSG_TOPIC_TBL_Class_t TopicTbl;
   
} JMSG_LIB_Class_t;


/************************/
/** Exported Functions **/
/************************/

/******************************************************************************
** Function: JMSG_LibInit
**
** Initialize the JSON Message library to a known state
**
** Notes:
**   1. This is called by the cFE during lib/app loading and initialization and
**      serves as the object constructor in the Basecamp design model.
**
*/
uint32 JMSG_LibInit(void);


/******************************************************************************
** Function: JMSG_LIB_
**
** Return a pointer to library's instance of JMSG_TOPICC_TBL
**
** Notes:
**   1. Only one JMSG_LIB can exist in a cFS target
**
*/
JMSG_TOPIC_TBL_Class_t *JMSG_LIB_GetInstance(void);

#endif /* _jmsg_lib_ */
