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
**    1. Can't include lib_cfg.h because the "CFG_" macro definitions
**       will conflict
**
*/

#ifndef _usr_tplug_
#define _usr_tplug_


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: USR_TPLUG_Constructor
**
** Call constructors for each user topic plugin.
**
*/
void USR_TPLUG_Constructor(void);


#endif /* _usr_tplug_ */
