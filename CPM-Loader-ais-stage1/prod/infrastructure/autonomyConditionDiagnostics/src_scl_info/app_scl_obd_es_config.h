/*******************************************************************************
Copyright 2015 Caterpillar Inc.  All rights reserved.

--------------------------------------------------------------------------------	
Filename:   app_scl_obd_es_config.h

Description
This file contains configuration information for the OBD event system.	
*******************************************************************************/

#ifndef APP_SCL_OBD_ES_CONFIG_H_
#define APP_SCL_OBD_ES_CONFIG_H_

#include <scl_obd_es.h>
#include <scl_diag_pj1939.h>
#include "LoaderdiagnosticEventConfig/Loader_autonomy_diagnostics_config.h"
#include "LoaderdiagnosticEventConfig/Loader_autonomy_event_config.h"


extern const scl_obd_es_config_t         app_scl_obd_es_config;
extern const scl_diag_pj1939_config_t    app_scl_diag_pj1939_config;

extern scl_obd_es_t          *app_scl_obd_es;
extern scl_diag_pj1939_t     *app_pj1939_diag_obj;
extern scl_obd_handle_gen_t      *app_handle_gen;
extern scl_obd_test_handle_t     app_test_handle_CID247_FMI9;
extern scl_obd_test_handle_t     app_test_handle_CID247_FMI14;
extern scl_obd_test_handle_t     app_test_handle_CID5856_FMI9;
extern scl_obd_test_handle_t     app_test_handle_CID5856_FMI14;
extern scl_obd_test_handle_t     app_test_handle_CID2348_FMI9;
extern scl_obd_test_handle_t     app_test_handle_CID2348_FMI14;
extern scl_obd_test_handle_t     app_test_handle_CID248_FMI9;
extern scl_obd_test_handle_t     app_test_handle_CID248_FMI14;


extern boolean app_scl_obd_init(scl_j1939_link_t link);
extern void app_scl_obd_es_register_health_bdt_faults(void);
extern void app_scl_obd_es_register_CID247_faults(void);
extern void app_scl_obd_es_register_CID5856_faults(void);
extern void app_scl_obd_es_register_CID2348_faults(void);
extern void app_scl_obd_es_register_CID248_faults(void);

#endif /* APP_SCL_OBD_ES_CONFIG_H_ */
