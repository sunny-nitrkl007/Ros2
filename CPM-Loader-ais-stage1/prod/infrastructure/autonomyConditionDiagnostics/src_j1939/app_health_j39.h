#include <scl_j1939_proto.h>
#include <scl_health_j39.h>
#include <oel_assert.h>
#include <scl_obd_es.h>
#include <scl_health_j39_link.h>
#include <scl_flex.h>
#include <scl_flex_def_structs.h>
#include "app_scl_dlinfo.h"


#if defined (__cplusplus)
extern "C"
{
#endif

extern uint_least32_t scl_dlinfo_bdt_write_gid_encoding(const scl_flex_pid_t * pid, uint_least8_t **buffer);
extern void app_bdt_init(const oel_rtos_resource_config_t *app_bdt_resource_config);

extern void app_health_j39_init(void);
extern void app_health_j39_init_CAN_A(void);
extern void app_health_j39_init_CAN_B(void);
extern void app_health_j39_init_CDL(void);

extern void app_health_j39_update(bool_t inhibit, bool_t disable);
extern void app_health_j39_update_CAN_A(bool_t disable);
extern void app_health_j39_update_CAN_B(bool_t disable);
extern void app_health_j39_update_CDL(bool_t disable);

extern bool_t app_scl_health_j39_scl_info_get_status(void *fault_mgr_ptr, void *fault_ptr);
extern void app_scl_health_j39_scl_info_report_status(void *fault_mgr_ptr, void *fault_ptr, bool_t faulted);

#if defined (__cplusplus)
}
#endif

/* CAN 1: Ph_Link_app */ 
extern scl_health_j39_link_t *app_health_11bit_link_CAN_A;


/* CAN 2: TES_Link_app */ 
extern scl_health_j39_link_t *app_health_11bit_link_CAN_B;
extern scl_health_j39_link_config_t link_config_CAN_B;

/* CDL: CDL_Link_app */ 
extern scl_health_j39_link_t *app_health_11bit_link_CDL;

/********* SA BEGIN ********/

/* All ECMS */
extern bool_t CDLPID45_fmi9_rcv_sa;

/* Transmission */
extern bool_t PID45_fmi9_rcv_sa;


/********* SA END ********/

/* Engine */
extern bool_t PIDC8_fmi9_rcv;
extern bool_t PIDFE5D_fmi9_rcv;
extern bool_t PGN65266_fmi9_rcv;
extern bool_t PGN61444_fmi9_rcv;


/* Transmission */
extern bool_t PIDF4FD_fmi9_rcv;
extern bool_t PIDD10AB5_fmi9_rcv;
extern bool_t PIDD01E8C_fmi9_rcv;
extern bool_t PIDD01F89_fmi9_rcv;
extern bool_t PIDD00461_fmi9_rcv;
extern bool_t PIDF074_fmi9_rcv;
extern bool_t CATEXTIDF0B2_fmi9_rcv;
extern bool_t PIDD10938_fmi9_rcv_trans;
extern bool_t PIDF5D9_fmi9_rcv;



/* Secondary Display */
extern bool_t PIDF25B_fmi9_rcv;
extern bool_t PIDD0022C_fmi9_rcv;
extern bool_t PIDD10938_fmi9_rcv;
extern bool_t PIDD11B8D_fmi9_rcv;
extern bool_t PIDD0022B_fmi9_rcv;
extern bool_t PIDD02408_fmi9_rcv;
extern bool_t PIDD02409_fmi9_rcv;
extern bool_t PIDD1102F_fmi9_rcv;
extern bool_t PIDD1000E_fmi9_rcv;


/* Product Link */
extern bool_t PIDF84D_fmi9_rcv;


/* Steering */
extern bool_t PGN61451_fmi9_rcv;


/* Implement */
extern bool_t PIDF47A_fmi9_rcv; 
extern bool_t PID45_fmi9_rcv;
extern bool_t PIDF1AA_fmi9_rcv;
extern bool_t PIDF49C_fmi9_rcv;
extern bool_t PIDD118CE_fmi9_rcv;
extern bool_t PIDD10049_fmi9_rcv;
extern bool_t PIDD10321_fmi9_rcv;
extern bool_t PIDD118CA_fmi9_rcv;
extern bool_t PIDD11884_fmi9_rcv;
extern bool_t PIDD10EE7_fmi9_rcv;
extern bool_t PIDD106D9_fmi9_rcv;
extern bool_t PIDF82D_fmi9_rcv;
extern bool_t PIDF9C0_fmi9_rcv;
extern bool_t PIDD10F6E_fmi9_rcv;
extern bool_t PIDD11890_fmi9_rcv;
extern bool_t PIDD010FA_fmi9_rcv;
extern bool_t PIDD010FB_fmi9_rcv;
extern bool_t PIDD010FC_fmi9_rcv;
extern bool_t PIDD010FD_fmi9_rcv;

/* IMU */ 
extern bool_t PGN61485_fmi9_rcv;
extern bool_t PGN61485_fmi9_rcv_prev;

/* IMU - Standalone */
extern bool_t PGN61485_fmi9_rcv_sa;
extern bool_t PGN61485_fmi9_rcv_prev_sa;

extern bool_t isDatalinkDiagInhibited;
extern bool_t isDatalinkDiagDisabled;
extern bool_t dl_diag_disabled;
