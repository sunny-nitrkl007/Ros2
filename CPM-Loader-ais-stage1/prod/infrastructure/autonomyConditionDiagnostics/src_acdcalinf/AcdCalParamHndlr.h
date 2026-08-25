/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: AcdCalParamHndlr.h
DESCRIPTION:
*******************************************************************************/
#ifndef ACDCALPARAMHNDLR_H_
#define ACDCALPARAMHNDLR_H_
/*******************************************************************************
** -- #Include's --
*******************************************************************************/

#include <scl_j1939_struct.h>			// for scl_j1939_link_t 

#if defined (__cplusplus)
extern "C"
{
#endif

void app_scl_qr_j1939_server_init(scl_j1939_link_t link);
void app_scl_pgb_cdl_server_init(void);
void app_scl_qr_j1939_server_add_entry(void);
void app_scl_qr_j1939_server_remove_entry(void);
int_least32_t app_qualified_data_clbk(void *context);
void app_scl_qr_j1939_server_set_values(int_16 set_HydOilTempMin, int_16 set_LiftCylVelMin, int_16 set_LiftCylVelCalValue);

#if defined (__cplusplus)
}
#endif

#endif /* ACDCALPARAMHNDLR_H_ */
