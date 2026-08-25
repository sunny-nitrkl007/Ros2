/*******************************************************************************
 ***
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** %%                                                                       %%
 *** %%  COPYRIGHT (C) 2006 CATERPILLAR INC.   ALL RIGHTS RESERVED.           %%
 *** %%      This work contains proprietary information which may             %%
 *** %%      constitute a trade secret and/or be confidential.                %%
 *** %%                                                                       %%
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** FILE: passwd.h
 ***
 *** DESCRIPTION:
 ***     This is the definition include file for the passwd module.
 ***     All of the contents in this file are intended for other tasks/modules
 ***     to use.
 ***
 ***     IMPORTANT NOTE:  All global variables should be considered read-only
 ***                      by any external module that accesses them.
 ***                      It is forbidden for any routines besides those
 ***                      local to this module to alter these variables.
 ******************************************************************************/
#ifndef __PASSWD_H__
#define __PASSWD_H__
/******************************************************************************
 ***
 *** $Source: $
 ***
 *** $Author: $
 ***
 *** $Locker: $
 ***
 *** $Date: $
 ***
 *** $Revision: $
 ***
 *** $View: $
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/

#include <passwd_struct.h>
#include <passwd_defs.h>
#include <passwd_proto.h>


/******************************************************************************
 ***
 ***    Constants defined for this file
 ***    -- #Define's --
 ***
 ******************************************************************************/
/******************************************************************************
 ***
 ***    Data types defined for this file
 ***    -- Struct's, Typedef's, Enum's --
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    File scope functions defined for this file
 ***    -- Function Prototypes --
 ***
 ******************************************************************************/
PASSWD_HOOK_RSP_E	passwd_hook_proc(	PASSWD_HOOK_CMD_E cmd,
                 	                 	PASSWD_Pkg_Work_t *work,
                 	                 	PASSWD_Pkg_Config_t *config );

extern void app_passwd_pkg_init(scl_j1939_link_t h_link, uint_least8_t ecm_idx);
extern void app_passwd_pkg_main(void);

/******************************************************************************
 ***
 ***    File scope symbols defined for this file
 ***    -- Symbols --
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    Global data symbols defined in this file
 ***    -- Global Symbols --
 ***
 ******************************************************************************/
/*prmsw workspace variables*/
extern PASSWD_Pkg_Work_t passwd_pkg_work;
#endif /* __PASSWD_H__ */
