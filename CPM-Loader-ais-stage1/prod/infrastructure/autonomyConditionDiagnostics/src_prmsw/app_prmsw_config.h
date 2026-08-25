/******************************************************************************************************************
 Copyright 2008 Caterpillar Inc. All rights reserved.
 -------------------------------------------------------------------------------------------------------------------
 File name: app_prmsw_config.h

Description:    This file contains the public interface to scl_prmsw.
 ******************************************************************************************************************/
#ifndef APP_PRMSW_CONFIG_H_
#define APP_PRMSW_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <scl_prmsw.h>
#include <oel_rtos_posix.h>
#include "../src_nvm/app_nvm_globals.h"
#include "../src_j1939/app_can_init.h"
#include "app_scl_fps_j1939_config.h"
#include "app_scl_fps_config.h"
#include <scl_fps_key_externs_am1.h>
#include <scl_fps_key_externs_am2.h>

extern int prmsw_feat_size;
extern scl_prmsw_free_use_cfg_t* prmsw_free_use_cfg;
extern scl_prmsw_feat_cfg_t* prmsw_feat_cfg;

extern void app_prmsw_init( void );
extern void app_prmsw_update( void );
extern void app_prmsw_get_feat_status
(
	void* context,
	uint_least16_t reason_code,
	scl_prmsw_feat_status_t feat_status,
	scl_prmsw_feat_info_t feat_info
);

/* Enumeration for Premium Software Command Status */
enum scl_prmsw_feat_status_cmd_e
{
  SCL_PRMSW_FEAT_STATUS_CMD_PERM_UNINSTALL=0x01,
  SCL_PRMSW_FEAT_STATUS_CMD_PERM_INSTALL = 0x02,
  SCL_PRMSW_FEAT_STATUS_CMD_PERM_ENABLE=0x04,
  SCL_PRMSW_FEAT_STATUS_CMD_PERM_DISABLE=0x08,
  SCL_PRMSW_FEAT_STATUS_CMD_FU_INSTALL=0x10,
  SCL_PRMSW_FEAT_STATUS_CMD_FU_UNINSTALL=0x20
};

typedef uint_least8_t scl_prmsw_feat_status_cmd_t;

/******************************************************************************************************************
Function name: scl_prmsw_set_feat_cmd_status()

Description:
This operation will set the feature status by getting the feature pointer for the given reason code.

Parameter Description: uint_least16_t reason_code: This parameter contains the reason_code of a feature.
                       scl_prmsw_feat_status_cmd_t feat_status:This parameter contains the feature statusto be set.


Return Description: void
******************************************************************************************************************/
extern void scl_prmsw_set_feat_cmd_status(uint_least16_t reason_code,
                                                     scl_prmsw_feat_status_cmd_t feat_status);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef APP_PRMSW_CONFIG_H_ */
