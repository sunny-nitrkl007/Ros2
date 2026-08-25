/*******************************************************************************
 ***
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** %%                                                                       %%
 *** %%  COPYRIGHT (C) 2011 CATERPILLAR INC.   ALL RIGHTS RESERVED.           %%
 *** %%      This work contains proprietary information which may             %%
 *** %%      constitute a trade secret and/or be confidential.                %%
 *** %%                                                                       %%
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 ***
 *** FILE NAME: passwd_supt.c
 ***
 *** DESCRIPTION: This file provides the password support to the application
 ***
 ***
 *******************************************************************************/
/*******************************************************************************
 ***
 ***    Referenced Functions, Data, & Symbols
 ***    -- #Include's --
 ***
 *******************************************************************************/
#ifndef   SCL_J1939_H_
#include <scl_j1939.h>
#endif
#include "passwd.h"
#include <cat_std_types.h>
#include <util_proto.h>
#include <string.h>
#include <hal_std_types.h>
#include <hal_boot_proto.h>
#include <scl_security.h>
#include "../src_app/app_rtos_config.h"
#include "../src_j1939/app_j1939_map.h"
#include "../src_j1939/app_et_j1939_support.h"
/*******************************************************************************
 ***
 ***    Constants defined for this file
 ***    -- #Define's --
 ***
 *******************************************************************************/
/*******************************************************************************
 ***
 ***    Internal Functions defined in this file
 ***    -- Function Prototypes --
 ***
 *******************************************************************************/
/*********** Password Package Support Hook Function Prototype ***********/
PASSWD_HOOK_RSP_E   passwd_hook_proc(   PASSWD_HOOK_CMD_E cmd,
		PASSWD_Pkg_Work_t *work,
		PASSWD_Pkg_Config_t *config );

/*******************************************************************************
 ***
 ***    Internal Data for this file
 ***    -- Global Symbols --
 ***
 *******************************************************************************/

static PASSWD_NVM_t passwd_nvm  = { {"ROCKWOOD", "PASSWORD"} };

static PASSWD_Pkg_Config_t   passwd_pkg_config = {
		passwd_hook_proc,       /* Application Package Support Hook Function */
};
PASSWD_Pkg_Work_t   passwd_pkg_work;

/*******************************************************************************
 ***
 *** FUNCTION NAME: app_passwd_pkg_main()
 ***
 *** DESCRIPTION: Wrapper function to allow main function to be called elsewhere
 ***
 *** INPUTS: None
 ***
 *** RETURNS: none
 ******************************************************************************/
void app_passwd_pkg_main(void)
{
	passwd_pkg_j1939_main(&passwd_pkg_work, &passwd_pkg_config);
}
/*******************************************************************************
 ***
 *** FUNCTION NAME: app_passwd_pkg_init()
 ***
 *** DESCRIPTION: Wrapper function to allow init function to be called elsewhere
 ***
 *** INPUTS: None
 ***
 *** RETURNS: none
 ******************************************************************************/
void app_passwd_pkg_init(scl_j1939_link_t h_link, uint_least8_t ecm_idx)
{
	/* specify the j1939 link the password library will operate on */
	passwd_set_j1939_link(h_link);

	/* specify the j1939 ecm index the password library will operate on */
	passwd_set_j1939_ecm_index(ecm_idx);

	passwd_pkg_init(&passwd_pkg_work, &passwd_pkg_config);

	scl_security_install_passwd();

}

/*******************************************************************************
 ***
 *** FUNCTION NAME: passwd_hook_proc()
 ***
 *** DESCRIPTION: password Hook process function
 ***
 *** INPUTS: cmd - init command, work- work structure,config- configuration
 ***
 *** RETURNS:  PASSWD_HOOK_RSP_E - PASSWD_HOOK_SUCCESS_RSP
 ***                               PASSWD_HOOK_FAIL_RSP
 ******************************************************************************/

PASSWD_HOOK_RSP_E passwd_hook_proc(PASSWD_HOOK_CMD_E cmd,
        PASSWD_Pkg_Work_t *work,
        PASSWD_Pkg_Config_t *config ) {

    static boolean writing = FALSE;

	{ // Why do we do this?  Isn't this done in processHardwareHealthStatus?
	    hal_ecm_serialno_t app_boot_ecm_sn;
	    app_boot_ecm_sn = hal_boot_get_ecm_serialno();
	    int ii;
        for (ii = 0; ii < ECM_SERIAL_NUMBER_MAX_SIZE; ++ii) {
            ecm_serial_number[ii] = app_boot_ecm_sn.string[ii];
        }
	}

	(void)config; /*Touch the argument to avoid compiler warning*/

    switch (cmd) {
    case (PASSWD_MAIN_TASK_INIT): {
        return( PASSWD_HOOK_SUCCESS_RSP );
    }

    case (PASSWD_NVM_INIT): {
        /* return the address of the NVM image  */
        /* for Password package to use */
        work->nvm_image = &passwd_nvm;
        return PASSWD_HOOK_SUCCESS_RSP;
    }

	case (PASSWD_NVM_WRITE_CMD): {
        /* Continue writing */
        if (writing) {
            /* Check to see if writing is complete */
            if (1 /*writing_complete()*/) {
                writing = FALSE;
                return PASSWD_HOOK_SUCCESS_RSP;
            }
        }

        /* Start writing - Copy from work->image to NVM */
        else if (work->nvm_wr_req) {
            writing = TRUE;
            passwd_nvm = *(work->nvm_image);    /* testing */
        }

        break;
    }

    case (PASSWD_GET_TTT): {
        static int x = 0;
        if (x == 0) {
            work->ttt =0;
            x = 1; // Done initializing ttt
        }

        return PASSWD_HOOK_SUCCESS_RSP;
    }

    case (PASSWD_INC_TTT): {
        work->ttt += 1;
        return PASSWD_HOOK_SUCCESS_RSP;
    }

    case (PASSWD_GET_PRODID): {
        work->prod_id = (unsigned_8*)pidF82D_pdata;
        return PASSWD_HOOK_SUCCESS_RSP;
    }

    case (PASSWD_GET_ECMSN): {
        work->ecm_sn = (unsigned_8*)ecm_serial_number;
        return PASSWD_HOOK_SUCCESS_RSP;
    }
    }

    return PASSWD_HOOK_FAIL_RSP;
}                             
