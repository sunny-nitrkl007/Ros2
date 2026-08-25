
#include<scl_j1939_proto.h>
#include <scl_health_j39.h>
#include <string.h>
#include <oel_assert.h>
#include "app_health_j39.h"
#include <stdio.h>
#include "app_scl_dlinfo.h"
#include <scl_health_j39_prop_link.h>
#include <scl_health_j39_ext_param.h>
#include <scl_health_j39_peer.h>
#include "../src_scl_info/app_scl_obd_es_config.h"
#include "../src_app/app_rtos_config.h"
#include "src_scl_info/app_scl_obd_es_config.h" 

/* Transmission */
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PID45;

/* CAN 1: Ph_Link_app */ 
scl_health_j39_ecm_t *app_health_11bit_ecm_CAN_A;


/* CAN 2: TES_Link_app */ 
scl_health_j39_ecm_t *app_health_11bit_ecm_CAN_B;

/* CAN 2: Standalone */
scl_health_j39_ecm_t *app_health_11bit_ecm_CAN_B_SA;

/* CDL: CDL_link app */ 
scl_health_j39_ecm_t *app_health_11bit_ecm_CDL;


scl_health_j39_link_t *app_health_11bit_link_CAN_A;
scl_health_j39_peer_t *app_health_11bit_peer_can_a_engine;
scl_health_j39_peer_t *app_health_11bit_peer_can_a_transmission;
scl_health_j39_peer_t *app_health_11bit_peer_can_a_2nd_display;
scl_health_j39_peer_t *app_health_11bit_peer_can_a_prod_link;
scl_health_j39_peer_t *app_health_11bit_peer_can_a_steering;


scl_health_j39_link_t *app_health_11bit_link_CAN_B;
scl_health_j39_link_t *app_health_11bit_link_CAN_B_SA;
scl_health_j39_peer_t *app_health_11bit_peer_can_b_implement;
scl_health_j39_peer_t *app_health_11bit_peer_can_b_imu;
scl_health_j39_peer_t *app_health_11bit_peer_can_b_sa_imu;

scl_health_j39_link_t *app_health_11bit_link_CDL;
scl_health_j39_peer_t *app_health_11bit_peer_cdl_mon_sys;
scl_health_j39_peer_t *app_health_11bit_peer_cdl_mcm;
scl_health_j39_peer_t *app_health_11bit_peer_cdl_elec_implement;

/* Mon Sys */
scl_health_j39_ext_param_t* app_peer1_ext_param_mon_sys_PID45;


/* MCM */
scl_health_j39_ext_param_t* app_peer1_ext_param_mcm_PID45;


/* Elec Implement */
scl_health_j39_ext_param_t* app_peer1_ext_param_elec_implement_PID45;


/* Engine */
scl_health_j39_ext_param_t* app_peer1_ext_param_engine_PGN61444;
scl_health_j39_ext_param_t* app_peer1_ext_param_engine_PGN65266;
scl_health_j39_ext_param_t* app_peer1_ext_param_engine_PIDC8;
scl_health_j39_ext_param_t* app_peer1_ext_param_engine_PIDFE5D;


/* Transmission */
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDF4FD;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDD10AB5;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDD01E8C;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDD01F89;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDD00461;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDF074;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_CATEXTIDF0B2;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDD10938;
scl_health_j39_ext_param_t* app_peer1_ext_param_transmission_PIDF5D9;



/* Secondary Display */
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDF25B;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD0022C;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD10938;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD11B8D;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD0022B;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD02408;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD02409;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD1102F;
scl_health_j39_ext_param_t* app_peer1_ext_param_2nd_display_PIDD1000E;



/* Product Link */
scl_health_j39_ext_param_t* app_peer1_ext_param_prod_link_PIDF84D;


/* Steering */
scl_health_j39_ext_param_t* app_peer1_ext_param_steering_PGN61451;


/* Implement */
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDF47A;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PID45;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDF1AA;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDF49C;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD118CE;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD10049;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD10321;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD118CA;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD11884;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD10EE7;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD106D9;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDF82D;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDF9C0;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD10F6E;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD11890;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD010FA;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD010FB;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD010FC;
scl_health_j39_ext_param_t* app_peer1_ext_param_implement_PIDD010FD;

/* IMU */
//scl_health_j39_ext_param_list_t* app_peer1_ext_param_imu_PGN61485; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5347; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5348; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5349; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5350; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5351; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5352; 
scl_health_j39_ext_param_t* app_peer1_ext_param_imu_PGN61485_SPN5353; 

/* IMU for Standalone */
//scl_health_j39_ext_param_list_t* app_peer2_ext_param_imu_PGN61485;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5347;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5348;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5349;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5350;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5351;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5352;
scl_health_j39_ext_param_t* app_peer2_ext_param_imu_PGN61485_SPN5353;

/********************************************************/
//
//                 FMI9
//
/********************************************************/

/* PID45 on all ECMs on CDL */
bool_t CDLPID45_fmi9_rcv_sa = TRUE;


/* Transmission */
bool_t PID45_fmi9_rcv_sa = TRUE;


/* Engine */
bool_t PIDC8_fmi9_rcv = TRUE;
bool_t PIDFE5D_fmi9_rcv = TRUE;
bool_t PGN65266_fmi9_rcv = TRUE;
bool_t PGN61444_fmi9_rcv = TRUE;


/* Transmission */
bool_t PIDF4FD_fmi9_rcv = TRUE;
bool_t PIDD10AB5_fmi9_rcv = TRUE;
bool_t PIDD01E8C_fmi9_rcv = TRUE;
bool_t PIDD01F89_fmi9_rcv = TRUE;
bool_t PIDD00461_fmi9_rcv = TRUE;
bool_t PIDF074_fmi9_rcv = TRUE;
bool_t CATEXTIDF0B2_fmi9_rcv = TRUE;
bool_t PIDD10938_fmi9_rcv_trans = TRUE;
bool_t PIDF5D9_fmi9_rcv = TRUE;


/* Secondary Display */
bool_t PIDF25B_fmi9_rcv = TRUE;
bool_t PIDD0022C_fmi9_rcv = TRUE;
bool_t PIDD10938_fmi9_rcv = TRUE;
bool_t PIDD11B8D_fmi9_rcv = TRUE;
bool_t PIDD0022B_fmi9_rcv = TRUE;
bool_t PIDD02408_fmi9_rcv = TRUE;
bool_t PIDD02409_fmi9_rcv = TRUE;
bool_t PIDD1102F_fmi9_rcv = TRUE;
bool_t PIDD1000E_fmi9_rcv = TRUE;


/* Product Link */
bool_t PIDF84D_fmi9_rcv = TRUE;


/* Steering */
bool_t PGN61451_fmi9_rcv = TRUE;


/* Implement */
bool_t PIDF47A_fmi9_rcv = TRUE;
bool_t PID45_fmi9_rcv = TRUE;
bool_t PIDF1AA_fmi9_rcv = TRUE;
bool_t PIDF49C_fmi9_rcv = TRUE;
bool_t PIDD118CE_fmi9_rcv = TRUE;
bool_t PIDD10049_fmi9_rcv = TRUE;
bool_t PIDD10321_fmi9_rcv = TRUE;
bool_t PIDD118CA_fmi9_rcv = TRUE;
bool_t PIDD11884_fmi9_rcv = TRUE;
bool_t PIDD10EE7_fmi9_rcv = TRUE;
bool_t PIDD106D9_fmi9_rcv = TRUE;
bool_t PIDF82D_fmi9_rcv = TRUE;
bool_t PIDF9C0_fmi9_rcv = TRUE;
bool_t PIDD10F6E_fmi9_rcv = TRUE;
bool_t PIDD11890_fmi9_rcv = TRUE;
bool_t PIDD010FA_fmi9_rcv = TRUE;
bool_t PIDD010FB_fmi9_rcv = TRUE;
bool_t PIDD010FC_fmi9_rcv = TRUE;
bool_t PIDD010FD_fmi9_rcv = TRUE;

/* IMU */ 
bool_t PGN61485_fmi9_rcv = TRUE;
bool_t PGN61485_fmi9_rcv_prev = TRUE;

/* IMU */
bool_t PGN61485_fmi9_rcv_sa = TRUE;
bool_t PGN61485_fmi9_rcv_prev_sa = TRUE;


/*NOTE: If you move the call to app_health_j39_update to a different task period, 
      this value needs to be updated. Currently, this task is running at the same
      ACD cycleRate_hz set in AutonomyConditionDiagnostics.rb
*/      
const uint_least16_t app_health_11bit_tsk_period = (uint_least16_t) 100; /*100ms*/


/* link structure initialization for CAN A (Ph_app)*/
scl_health_j39_link_config_t link_config_CAN_A =
{
   SCL_J1939_LINK_VER_0,      
   {  
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      247, /* CID */
      9    /* FMI */ 
   },
   {
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      247, /* CID */
      14,  /* FMI */ 
   },
   FALSE, /* NOT USED */
   SCL_HEALTH_J39_LINK_TYPE_PROP_CAN
};   


/* link structure initialization for CAN B (TES_link) */
scl_health_j39_link_config_t link_config_CAN_B =
{
   SCL_J1939_LINK_VER_0,      
   {  
      SCL_HEALTH_J39_TRIG_CDL_DIAG, 
      5856, /* CID */
      9     /* FMI */ 
   },
   {
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      5856, /* CID */
      14,   /* FMI */ 
   },
   FALSE,     /* NOT USED */
   SCL_HEALTH_J39_LINK_TYPE_PROP_CAN
};   

/* link structure initialization for CAN B (Standalone) */
scl_health_j39_link_config_t link_config_CAN_B_SA =
{
   SCL_J1939_LINK_VER_0,
   {
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      2348, /* CID */
      9     /* FMI */
   },
   {
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      2348, /* CID */
      14,   /* FMI */
   },
   FALSE,     /* NOT USED */
   SCL_HEALTH_J39_LINK_TYPE_PROP_CAN
};

/* link structure initialization for CDL */
scl_health_j39_link_config_t link_config_CDL =
{
   SCL_J1939_LINK_VER_0,      
   {  
      SCL_HEALTH_J39_TRIG_CDL_DIAG, 
      248, /* CID */
      9     /* FMI */ 
   },
   {
      SCL_HEALTH_J39_TRIG_CDL_DIAG,
      248, /* CID */
      14,   /* FMI */ 
   },
   FALSE,     /* NOT USED */
   SCL_HEALTH_J39_LINK_TYPE_PROP_CAN
};   

/* Set eval_delay to 10000ms timeout since PGN 65266 has 5s timeout */
scl_health_j39_peer_config_t app_peer_config_engine = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F1);
scl_health_j39_peer_config_t app_peer_config_transmission = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x00000252);
scl_health_j39_peer_config_t app_peer_config_2nd_display = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F6);
scl_health_j39_peer_config_t app_peer_config_prod_link = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x00000260);
scl_health_j39_peer_config_t app_peer_config_implement = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F3);
scl_health_j39_peer_config_t app_peer_config_steering = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000004EF);
scl_health_j39_peer_config_t app_peer_config_imu = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F7);


/* NOTE: BOGUS GIDs for the following config for SA */
scl_health_j39_peer_config_t app_peer_config_mon_sys = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F4);
scl_health_j39_peer_config_t app_peer_config_mcm = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F5);
scl_health_j39_peer_config_t app_peer_config_elec_implement = SCL_HEALTH_J39_11BIT_CAN_PEER_CONFIG(10000,0x000000F6);


/* The default macro sets address claim timeout to 0 since we are not using Health library to report datalink health */
scl_health_j39_ecm_config_t ecm_config = SCL_HEALTH_J39_11BIT_DEFAULT_ECM_CONFIG();


/* This definition defines fault manager interface to use for the scl_info event
   system. It should be passed in scl_health_j39_link_set_fault_mgr(). */
scl_health_j39_fault_mgr_ifc_t app_scl_health_j39_scl_info_struct =
{
   app_scl_health_j39_scl_info_report_status,
   app_scl_health_j39_scl_info_get_status
};


scl_health_j39_fault_mgr_ifc_t *app_health_j39_fault_mgr_ifc =
                                                &app_scl_health_j39_scl_info_struct;




/*==============================================================================
** Function: 
**    app_scl_health_j39_scl_info_get_status
**
**  Description:
**    Get the status of a fault from the fault manager. Wrapper function for 
**    scl_obd_es_get_fault_rep(). Library user should initialize 
**    get_active_status of scl_health_j39_fault_mgr_ifc_t with this function if 
**    lib_scl\scl_info lib is used for fault logging
**
**  Parameter:
**    fault_mgr_ptr: The library user supplied pointer to the fault 
**                   manager passed to scl_health_j39_link_set_fault_mgr. This
**                   should be a pointer to scl_obd_es_t
**    fault_ptr: The library user supplied pointer to the fault information
**               passed to scl_health_j39_link_set_fault_mgr. This should be
**               a pointer to scl_obd_test_handle_t
**
** Return:
**    TRUE if the fault is currently active. FALSE if the fault is currently
**    not active or fault_ptr is NULL or fault_mgr_ptr is NULL or 
**    scl_obd_es_get_fault_rep() doesn't return SCL_OBD_ES_SUCCESS.
**============================================================================*/
bool_t app_scl_health_j39_scl_info_get_status(
   void *fault_mgr_ptr,
   void *fault_ptr)
{
   scl_obd_es_fault_rep_t fault_report = 0;

   scl_obd_es_error_t error = scl_obd_es_get_fault_rep(
                                         (scl_obd_es_t *)fault_mgr_ptr, 
                                         *((scl_obd_test_handle_t *)fault_ptr), 
                                          &fault_report);
   if(error == SCL_OBD_ES_SUCCESS)
   {
      return (SCL_OBD_ES_REP_ACTIVE_DIAG & fault_report);
   }
   else
   {
      return FALSE;
   }
}

/*==============================================================================
** Function: 
**    app_scl_health_j39_scl_info_report_status
**
**  Description:
**    Wrapper function for scl_obd_es_notify_test_results(). Library user should 
**    initialize report_status of scl_health_j39_fault_mgr_ifc_t  with this 
**    function if lib_scl\scl_info lib is being used for fault logging
**
**  Parameter:
**    fault_mgr_ptr: The library user supplied pointer to the fault 
**                   manager passed to scl_health_j39_link_set_fault_mgr.
**    fault_ptr: The library user supplied pointer to the fault information
**               passed to scl_health_j39_ecm_add_peer.
**
** Return:
**    None       
**============================================================================*/
void app_scl_health_j39_scl_info_report_status(
   void *fault_mgr_ptr,
   void *fault_ptr,
   bool_t faulted)
{
   if(faulted)
   {
      scl_obd_es_notify_test_results((scl_obd_es_t *)fault_mgr_ptr, 
                                     *((scl_obd_test_handle_t *)fault_ptr), 
                                     SCL_OBD_ES_TEST_RESULTS_FAIL);
   }
   else
   {
      scl_obd_es_notify_test_results((scl_obd_es_t *)fault_mgr_ptr, 
                                     *((scl_obd_test_handle_t *)fault_ptr), 
                                     SCL_OBD_ES_TEST_RESULTS_PASS);
   }
}




/*================================================================================================================
** Function:
**  app_health_j39_init_CAN_A
**
** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_init_CAN_A(void)
{

   /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
   scl_j1939_link_t j1939_link_a = 0;
   
   /* According to Mustafiz, this is just a dummy variable and can be set to anything */
   uint_least8_t ecm_idx = 1;  

   bool_t start = 1;

   scl_health_j39_error_t health_error_CAN_A;
   scl_health_j39_error_t health_error_can_a_engine;
   scl_health_j39_error_t health_error_can_a_transmission;
   scl_health_j39_error_t health_error_can_a_prod_link;
   scl_health_j39_error_t health_error_can_a_2nd_display;
   scl_health_j39_error_t health_error_can_a_steering;

              
   /* Register J1939 Link */
   app_health_11bit_link_CAN_A = scl_health_j39_add_link(j1939_link_a,
                                                         &link_config_CAN_A,
                                                         start,
                                                         &health_error_CAN_A);



  if(health_error_CAN_A == SCL_HEALTH_J39_SUCCESS)
  {
      printf("scl_health_j39_add_link SUCCESS\n");
        
      /* Set health periodic period */
      scl_health_j39_prop_link_set_task_period(app_health_11bit_link_CAN_A,app_health_11bit_tsk_period);    
 
 
      /* Register CPM ecm */ 
      app_health_11bit_ecm_CAN_A = scl_health_j39_link_add_ecm(app_health_11bit_link_CAN_A,
                                                           ecm_idx,
                                                           &ecm_config,
                                                           start,
                                                           &health_error_CAN_A);
    
      
      if(health_error_CAN_A == SCL_HEALTH_J39_SUCCESS)
      {
        
          printf("scl_health_j39_link_add_ecm SUCCESS\n");

          /* Initialize fault manager interface */
          health_error_CAN_A = scl_health_j39_link_set_fault_mgr(app_health_11bit_link_CAN_A,
                                                                 app_health_j39_fault_mgr_ifc,
                                                                 app_scl_obd_es, /* fault_mgr_ptr */
                                                                 &app_test_handle_CID247_FMI9, /* no_comm_fault_ptr */
                                                                 &app_test_handle_CID247_FMI14); /* config_err_fault_ptr */ 


            if(health_error_CAN_A == SCL_HEALTH_J39_SUCCESS)
            {
                printf("scl_health_j39_link_set_fault_mgr SUCCESS\n");   
                    
                /* A "peer" is an ECU from which you expect messages.
                   Called for each peer ECU, i.e. each ECU from which the application
                   expects to receive messages periodically 
                */   

                app_health_11bit_peer_can_a_engine = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_A,
                                                                &app_peer_config_engine,
                                                                TRUE,
                                                                &health_error_can_a_engine);     
          
    
                if(health_error_can_a_engine == SCL_HEALTH_J39_SUCCESS)
                {   
                    printf("scl_health_j39_ecm_add_peer ENGINE SUCCESS\n");
             

                    app_peer1_ext_param_engine_PGN61444 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_engine,
                                                                             SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                             190,
                                                                             start,
                                                                             &health_error_can_a_engine);

                    if(health_error_can_a_engine == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PGN61444 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PGN61444 NOT %d \n", health_error_can_a_engine);
                    }      


                    app_peer1_ext_param_engine_PGN65266 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_engine,
                                                                             SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                             183,
                                                                             start,
                                                                             &health_error_can_a_engine);

                    if(health_error_can_a_engine == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PPGN65266 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PGN65266 NOT %d \n", health_error_can_a_engine);
                    }      


                    app_peer1_ext_param_engine_PIDC8 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_engine,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xC8,
                                                                             start,
                                                                             &health_error_can_a_engine);

                    if(health_error_can_a_engine == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PIDC8 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PIDC8 NOT %d \n", health_error_can_a_engine);
                    }  


                    app_peer1_ext_param_engine_PIDFE5D = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_engine,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xFE5D,
                                                                             start,
                                                                             &health_error_can_a_engine);

                    if(health_error_can_a_engine == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PIDFE5D SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param ENGINE_PIDFE5D NOT %d \n", health_error_can_a_engine);
                    }       
       
                }
                
                else
                {
                    printf("scl_health_j39_ecm_add_peer ENGINE NOT %d \n", health_error_can_a_engine);
                }

   

                app_health_11bit_peer_can_a_transmission = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_A,
                                                                &app_peer_config_transmission,
                                                                TRUE,
                                                                &health_error_can_a_transmission);         
  

               if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
               {
                   printf("scl_health_j39_ecm_add_peer TRANSMISSION SUCCESS\n");    

                   app_peer1_ext_param_transmission_PID45 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0x45,
                                                                                              start,
                                                                                              &health_error_can_a_transmission);

                   if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PID45 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PID45 NOT %d \n", health_error_can_a_transmission);
                   }      


                              
                   app_peer1_ext_param_transmission_PIDF4FD = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xF4FD,
                                                                             start,
                                                                             &health_error_can_a_transmission);


                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF4FD SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF4FD NOT %d \n", health_error_can_a_transmission);
                    }         


                    app_peer1_ext_param_transmission_PIDD10AB5 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xD10AB5,
                                                                             start,
                                                                             &health_error_can_a_transmission);

                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD10AB5 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD10AB5 NOT %d \n", health_error_can_a_transmission);
                    }         


                    app_peer1_ext_param_transmission_PIDD01E8C = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xD01E8C,
                                                                             start,
                                                                             &health_error_can_a_transmission);

                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD01E8C SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD01E8C NOT %d \n", health_error_can_a_transmission);
                    }      

                    app_peer1_ext_param_transmission_PIDD01F89 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xD01F89,
                                                                             start,
                                                                             &health_error_can_a_transmission);

                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD01F89 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD01F89 NOT %d \n", health_error_can_a_transmission);
                    }      

                    app_peer1_ext_param_transmission_PIDD00461 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xD00461,
                                                                             start,
                                                                             &health_error_can_a_transmission);
                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD00461 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD00461 NOT %d \n", health_error_can_a_transmission);
                    }      
                    

                    app_peer1_ext_param_transmission_PIDF074 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0xF074,
                                                                             start,
                                                                             &health_error_can_a_transmission);

                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF074 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF074 NOT %d \n", health_error_can_a_transmission);
                    }      



                    app_peer1_ext_param_transmission_CATEXTIDF0B2 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                                                      SCL_HEALTH_J39_PTYPE_GID,
                                                                                                      0x10D,
                                                                                                      start,
                                                                                                      &health_error_can_a_transmission);

                    if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_CATEXTIDF0B2 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_CATEXTIDF0B2 NOT %d \n", health_error_can_a_transmission);
                    }      


                    app_peer1_ext_param_transmission_PIDD10938 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                                                   SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                                   0xD10938,
                                                                                                   start,
                                                                                                   &health_error_can_a_transmission);

                     if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                     {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD10938 SUCCESS\n");
                     }
                     else
                     {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDD10938 NOT %d \n", health_error_can_a_transmission);
                      }       
                   


                    app_peer1_ext_param_transmission_PIDF5D9 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_transmission,
                                                                                                   SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                                   0xF5D9,
                                                                                                   start,
                                                                                                   &health_error_can_a_transmission);

                     if(health_error_can_a_transmission == SCL_HEALTH_J39_SUCCESS)
                     {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF5D9 SUCCESS\n");
                     }
                     else
                     {
                        printf("scl_health_j39_peer_add_ext_param TRANSMISSION_PIDF5D9 NOT %d \n", health_error_can_a_transmission);
                      }                  

              }
              
              else
              { 
                  printf("scl_health_j39_ecm_add_peer TRANSMISSION NOT %d \n", health_error_can_a_transmission);
              }  
              


               /* A "peer" is an ECU from which you expect messages.
               Called for each peer ECU, i.e. each ECU from which the application
               expects to receive messages periodically 
               */   

              app_health_11bit_peer_can_a_2nd_display = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_A,
                                                                &app_peer_config_2nd_display,
                                                                TRUE,
                                                                &health_error_can_a_2nd_display);   


              if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
              {
                 printf("scl_health_j39_ecm_add_peer 2nd 2nd_display SUCCESS\n");    
             
                 app_peer1_ext_param_2nd_display_PIDF25B = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xF25B,
                                                                                              start,
                                                                                              &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDF25B SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDF25B NOT %d \n", health_error_can_a_2nd_display);
                 }        


                 app_peer1_ext_param_2nd_display_PIDD0022C = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                                SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                                0xD0022C,
                                                                                                start,
                                                                                                &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD0022C SUCCESS\n");
                 }
                 else
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD0022C NOT %d \n", health_error_can_a_2nd_display);
                 }       

                   
                 app_peer1_ext_param_2nd_display_PIDD10938 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD10938,
                                                                                               start,
                                                                                               &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD10938 SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD10938 NOT %d \n", health_error_can_a_2nd_display);
                 }       
                   

                 app_peer1_ext_param_2nd_display_PIDD11B8D = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD11B8D,
                                                                                               start,
                                                                                               &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD11B8D SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD11B8D NOT %d \n", health_error_can_a_2nd_display);
                 }       



                 app_peer1_ext_param_2nd_display_PIDD02408 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD02408,
                                                                                               start,
                                                                                               &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD02408 SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD02408 NOT %d \n", health_error_can_a_2nd_display);
                 }

                 app_peer1_ext_param_2nd_display_PIDD0022B = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                                                SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                                                0xD0022B,
                                                                                                                start,
                                                                                                                &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD0022B SUCCESS\n");
                 }
                 else
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD0022B NOT %d \n", health_error_can_a_2nd_display);
                 }

                   
                 app_peer1_ext_param_2nd_display_PIDD02409 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD02409,
                                                                                               start,
                                                                                              &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD02409 SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD02409 NOT %d \n", health_error_can_a_2nd_display);
                 }       

                    
                 app_peer1_ext_param_2nd_display_PIDD1102F = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD1102F,
                                                                                               start,
                                                                                               &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD1102F SUCCESS\n");
                 }
                 else
                 {
                    printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD1102F NOT %d \n", health_error_can_a_2nd_display);
                 }       


                 app_peer1_ext_param_2nd_display_PIDD1000E = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_2nd_display,
                                                                                               SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                               0xD1000E,
                                                                                               start,
                                                                                               &health_error_can_a_2nd_display);

                 if(health_error_can_a_2nd_display == SCL_HEALTH_J39_SUCCESS)
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD1000E SUCCESS\n");
                 }
                 else
                 {
                     printf("scl_health_j39_peer_add_ext_param 2nd 2nd_display_PIDD1000E NOT %d \n", health_error_can_a_2nd_display);
                 }       
              }
              else
              {
                 printf("scl_health_j39_ecm_add_peer 2nd 2nd_display NOT %d \n", health_error_can_a_2nd_display);
              }

              /* A "peer" is an ECU from which you expect messages.
                 Called for each peer ECU, i.e. each ECU from which the application
                 expects to receive messages periodically 
              */   

              app_health_11bit_peer_can_a_prod_link = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_A,
                                                                                   &app_peer_config_prod_link,
                                                                                   TRUE,
                                                                                   &health_error_can_a_prod_link);     
  

               if(health_error_can_a_prod_link == SCL_HEALTH_J39_SUCCESS)
               {
                  printf("scl_health_j39_ecm_add_peer PROD_LINK SUCCESS\n");   
             
                  app_peer1_ext_param_prod_link_PIDF84D = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_prod_link,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xF84D,
                                                                                            start,
                                                                                            &health_error_can_a_prod_link);

                  if(health_error_can_a_prod_link == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param PROD_LINK PIDF84D SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param PROD_LINK PIDF84D NOT %d \n", health_error_can_a_prod_link);
                  }                  
               }
               else
               {
                  printf("scl_health_j39_ecm_add_peer PROD_LINK NOT %d \n", health_error_can_a_prod_link);
               }

               
               /* A "peer" is an ECU from which you expect messages.
                  Called for each peer ECU, i.e. each ECU from which the application
                  expects to receive messages periodically 
               */   
               app_health_11bit_peer_can_a_steering = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_A,
                                                                                  &app_peer_config_steering,
                                                                                  TRUE,
                                                                                  &health_error_can_a_steering);     


               if(health_error_can_a_steering == SCL_HEALTH_J39_SUCCESS)
               {
                  printf("scl_health_j39_ecm_add_peer STEERING SUCCESS\n");   
             
                  app_peer1_ext_param_steering_PGN61451 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_a_steering,
                                                                                   SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                   2927,
                                                                                   start,
                                                                                   &health_error_can_a_steering);

                  if(health_error_can_a_steering == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param STEERING PGN61451 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param STEERING PGN61451 NOT %d \n", health_error_can_a_steering);
                  }                  
               }
               else
               {
                  printf("scl_health_j39_ecm_add_peer STEERING NOT %d \n", health_error_can_a_steering);
               }
          
            


            /* ADD MORE ECMS HERE */
            }       
             
            // Can't set fault mgr 
            else
            { 
               printf("scl_health_j39_link_set_fault_mgr NOT %d \n", health_error_CAN_A);
            }
 
          }
    
          // Can't add CPM ECM
          else
          {
             printf("scl_health_j39_link_add_ecm NOT %d \n", health_error_CAN_A);
          }
       }

    // Can't add J1939 Link
    else
    {
        printf("scl_health_j39_add_link NOT %d \n", health_error_CAN_A);
    }          
         

    app_scl_dlinfo_init(app_health_11bit_link_CAN_A, &app_dlinfo_j39_link_config_CAN_A);
         
    return;

}  


/*================================================================================================================
** Function:
**  app_health_j39_init_CAN_B

** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_init_CAN_B(void)
{

   /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
   scl_j1939_link_t j1939_link_b = 0; 
   
   /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
   uint_least8_t ecm_idx = 2;  

   bool_t start = 1;

   scl_health_j39_error_t health_error_CAN_B;
   scl_health_j39_error_t health_error_can_b_implement;
   scl_health_j39_error_t health_error_can_b_imu;

              
   /* Register J1939 Link */
   app_health_11bit_link_CAN_B = scl_health_j39_add_link(j1939_link_b,
                                                         &link_config_CAN_B,
                                                         start,
                                                         &health_error_CAN_B);



  if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
  {
      printf("scl_health_j39_add_link SUCCESS\n");
        
      /* Set health periodic period */
      scl_health_j39_prop_link_set_task_period(app_health_11bit_link_CAN_B,app_health_11bit_tsk_period);    
 
 
      /* Register CPM ecm */ 
      app_health_11bit_ecm_CAN_B = scl_health_j39_link_add_ecm(app_health_11bit_link_CAN_B,
                                                               ecm_idx,
                                                               &ecm_config,
                                                               start,
                                                               &health_error_CAN_B);
    
      
      if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
      {
          printf("scl_health_j39_link_add_ecm SUCCESS\n");

          /* Initialize fault manager interface */
          health_error_CAN_B = scl_health_j39_link_set_fault_mgr(app_health_11bit_link_CAN_B,
                                                                 app_health_j39_fault_mgr_ifc,
                                                                 app_scl_obd_es, /* fault_mgr_ptr */
                                                                 &app_test_handle_CID5856_FMI9, /* no_comm_fault_ptr */
                                                                 &app_test_handle_CID5856_FMI14); /* config_err_fault_ptr */ 

          if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
          {
              /* A "peer" is an ECU from which you expect messages.
                 Called for each peer ECU, i.e. each ECU from which the application
                 expects to receive messages periodically 
              */   
              app_health_11bit_peer_can_b_implement = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_B,
                                                                                  &app_peer_config_implement,
                                                                                  TRUE,
                                                                                  &health_error_can_b_implement);     

  
              if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS) 
              {
                  printf("scl_health_j39_ecm_add_peer IMPLEMENT SUCCESS\n");   
              
                    
                  app_peer1_ext_param_implement_PIDF47A = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xF47A,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF47A SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF47A NOT %d \n", health_error_can_b_implement);
                  }       


                  app_peer1_ext_param_implement_PIDD010FA = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xD010FA,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FA SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FA NOT %d \n", health_error_can_b_implement);
                  }

                  app_peer1_ext_param_implement_PIDD010FB = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xD010FB,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FB SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FB NOT %d \n", health_error_can_b_implement);
                  }

                  app_peer1_ext_param_implement_PIDD010FC = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xD010FC,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FC SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FC NOT %d \n", health_error_can_b_implement);
                  }

                  app_peer1_ext_param_implement_PIDD010FD = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xD010FD,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FD SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD010FD NOT %d \n", health_error_can_b_implement);
                  }

                  
                  app_peer1_ext_param_implement_PID45 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                          SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                          0x45,
                                                                                          start,
                                                                                          &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PID45 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PID45 NOT %d \n", health_error_can_b_implement);
                  }       


                  
                  app_peer1_ext_param_implement_PIDF1AA = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xF1AA,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF1AA SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF1AA NOT %d \n", health_error_can_b_implement);
                  }       

                  
                  app_peer1_ext_param_implement_PIDF49C = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xF49C,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF49C SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF49C NOT %d \n", health_error_can_b_implement);
                  }       


                  app_peer1_ext_param_implement_PIDD118CE = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD118CE,
                                                                                              start,
                                                                                              &health_error_can_b_implement);                

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD118CE SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD118CE NOT %d \n", health_error_can_b_implement);
                  }       



                  app_peer1_ext_param_implement_PIDD10049 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD10049,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10049 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10049 NOT %d \n", health_error_can_b_implement);
                  }       


                  
                  app_peer1_ext_param_implement_PIDD10321 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD10321,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10321 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10321 NOT %d \n", health_error_can_b_implement);
                  }       



                  app_peer1_ext_param_implement_PIDD118CA = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD118CA,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD118CA SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD118CA NOT %d \n", health_error_can_b_implement);
                  }       


                  
                  app_peer1_ext_param_implement_PIDD11884 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD11884,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD11884 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD11884 NOT %d \n", health_error_can_b_implement);
                  }       



                  app_peer1_ext_param_implement_PIDD10EE7 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                             0xD10EE7,
                                                                                             start,
                                                                                             &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10EE7 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10EE7 NOT %d \n", health_error_can_b_implement);
                  }       


                  
                  app_peer1_ext_param_implement_PIDD106D9 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD106D9,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD106D9 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD106D9 NOT %d \n", health_error_can_b_implement);
                  }       



                  app_peer1_ext_param_implement_PIDF82D = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xF82D,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF82D SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF82D NOT %d \n", health_error_can_b_implement);
                  }       

                  /* 0xF9C0 - Payload Legal For Trade Measurement Software Identifier */
                  app_peer1_ext_param_implement_PIDF9C0 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                          SCL_HEALTH_J39_PTYPE_CDL_PID,
                          0xF9C0,
                          start,
                          &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF9C0 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDF9C0 NOT %d \n", health_error_can_b_implement);
                  }

                  app_peer1_ext_param_implement_PIDD10F6E = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                            SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                            0xD10F6E,
                                                                                            start,
                                                                                            &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10F6E SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD10F6E NOT %d \n", health_error_can_b_implement);
                  }


                  
                  app_peer1_ext_param_implement_PIDD11890 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_implement,
                                                                                              SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                                              0xD11890,
                                                                                              start,
                                                                                              &health_error_can_b_implement);

                  if(health_error_can_b_implement == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD11890 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMPLEMENT PIDD11890 NOT %d \n", health_error_can_b_implement);
                  }      

              
              }
              else
              {
                 printf("scl_health_j39_ecm_add_peer IMPLEMENT NOT %d \n", health_error_can_b_implement);
              }



              /* A "peer" is an ECU from which you expect messages.
                 Called for each peer ECU, i.e. each ECU from which the application
                 expects to receive messages periodically 
              */   
              app_health_11bit_peer_can_b_imu = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_B,
                                                                            &app_peer_config_imu,
                                                                            TRUE,
                                                                            &health_error_can_b_imu);     
  

              if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS) 
              {
                  printf("scl_health_j39_ecm_add_peer IMU SUCCESS\n");   

                  app_peer1_ext_param_imu_PGN61485_SPN5347 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5347,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5347 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5347 NOT %d \n", health_error_can_b_imu);
                  }       
              
                    
                  app_peer1_ext_param_imu_PGN61485_SPN5348 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5348,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5348 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5348 NOT %d \n", health_error_can_b_imu);
                  }       


                  app_peer1_ext_param_imu_PGN61485_SPN5349 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5349,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5349 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5349 NOT %d \n", health_error_can_b_imu);
                  }       


                  app_peer1_ext_param_imu_PGN61485_SPN5350 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5350,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5350 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5350 NOT %d \n", health_error_can_b_imu);
                  }       


                  app_peer1_ext_param_imu_PGN61485_SPN5351 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5351,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5351 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5351 NOT %d \n", health_error_can_b_imu);
                  }       


                  app_peer1_ext_param_imu_PGN61485_SPN5352 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5352,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5352 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5352 NOT %d \n", health_error_can_b_imu);
                  }


                  app_peer1_ext_param_imu_PGN61485_SPN5353 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_imu,
                                                                                       SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                       5353,
                                                                                       start,
                                                                                       &health_error_can_b_imu);

                  if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5353 SUCCESS\n");
                  }
                  else
                  {
                      printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5353 NOT %d \n", health_error_can_b_imu);
                  }       


              }
              else
              {
                 printf("scl_health_j39_ecm_add_peer IMU NOT %d \n", health_error_can_b_imu);
              }

            /* ADD MORE ECMS HERE */
            

           }                    
           // Can't set fault mgr 
           else
           { 
              printf("scl_health_j39_link_set_fault_mgr NOT %d \n", health_error_CAN_B);
           }
       }
       // Can't add CPM ECM
       else
       {
           printf("scl_health_j39_link_add_ecm NOT %d \n", health_error_CAN_B);
       }
    }
    // Can't add J1939 Link
    else
    {
        printf("scl_health_j39_add_link NOT %d \n", health_error_CAN_B);
    }          
         

    app_scl_dlinfo_init(app_health_11bit_link_CAN_B, &app_dlinfo_j39_link_config_CAN_B);
          
    return;

}  

/*================================================================================================================
** Function:
**  app_health_j39_init_CAN_B_SA

** Description:
**  Initialize the J1939 Health library for CANB for Standalone.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_init_CAN_B_SA(void)
{

    /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
    scl_j1939_link_t j1939_link_b = 0;

    /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
    uint_least8_t ecm_idx = 2;

    bool_t start = 1;

    scl_health_j39_error_t health_error_CAN_B;
    scl_health_j39_error_t health_error_can_b_imu;


    /* Register J1939 Link */
    app_health_11bit_link_CAN_B_SA = scl_health_j39_add_link(j1939_link_b,
                                                          &link_config_CAN_B_SA,
                                                          start,
                                                          &health_error_CAN_B);



   if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
   {
       printf("scl_health_j39_add_link SUCCESS\n");

       /* Set health periodic period */
       scl_health_j39_prop_link_set_task_period(app_health_11bit_link_CAN_B_SA,app_health_11bit_tsk_period);


       /* Register CPM ecm */
       app_health_11bit_ecm_CAN_B_SA = scl_health_j39_link_add_ecm(app_health_11bit_link_CAN_B_SA,
                                                                ecm_idx,
                                                                &ecm_config,
                                                                start,
                                                                &health_error_CAN_B);


       if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
       {
           printf("scl_health_j39_link_add_ecm SUCCESS\n");

           /* Initialize fault manager interface */
           health_error_CAN_B = scl_health_j39_link_set_fault_mgr(app_health_11bit_link_CAN_B_SA,
                                                                  app_health_j39_fault_mgr_ifc,
                                                                  app_scl_obd_es, /* fault_mgr_ptr */
                                                                  &app_test_handle_CID2348_FMI9, /* no_comm_fault_ptr */
                                                                  &app_test_handle_CID2348_FMI14); /* config_err_fault_ptr */

           if(health_error_CAN_B == SCL_HEALTH_J39_SUCCESS)
           {
               /* A "peer" is an ECU from which you expect messages.
                  Called for each peer ECU, i.e. each ECU from which the application
                  expects to receive messages periodically
               */
               app_health_11bit_peer_can_b_sa_imu = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CAN_B_SA,
                                                                             &app_peer_config_imu,
                                                                             TRUE,
                                                                             &health_error_can_b_imu);


               if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
               {
                   printf("scl_health_j39_ecm_add_peer IMU SUCCESS\n");

                   app_peer2_ext_param_imu_PGN61485_SPN5347 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5347,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5347 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5347 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5348 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5348,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5348 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5348 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5349 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5349,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5349 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5349 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5350 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5350,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5350 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5350 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5351 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5351,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5351 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5351 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5352 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5352,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5352 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5352 NOT %d \n", health_error_can_b_imu);
                   }


                   app_peer2_ext_param_imu_PGN61485_SPN5353 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_can_b_sa_imu,
                                                                                        SCL_HEALTH_J39_PTYPE_J1939_SPN,
                                                                                        5353,
                                                                                        start,
                                                                                        &health_error_can_b_imu);

                   if(health_error_can_b_imu == SCL_HEALTH_J39_SUCCESS)
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5353 SUCCESS\n");
                   }
                   else
                   {
                       printf("scl_health_j39_peer_add_ext_param IMU PGN61485_SPN5353 NOT %d \n", health_error_can_b_imu);
                   }


               }
               else
               {
                  printf("scl_health_j39_ecm_add_peer IMU NOT %d \n", health_error_can_b_imu);
               }

             /* ADD MORE ECMS HERE */


            }
            // Can't set fault mgr
            else
            {
               printf("scl_health_j39_link_set_fault_mgr NOT %d \n", health_error_CAN_B);
            }
        }
        // Can't add CPM ECM
        else
        {
            printf("scl_health_j39_link_add_ecm NOT %d \n", health_error_CAN_B);
        }
     }
     // Can't add J1939 Link
     else
     {
         printf("scl_health_j39_add_link NOT %d \n", health_error_CAN_B);
     }


     app_scl_dlinfo_init(app_health_11bit_link_CAN_B_SA, &app_dlinfo_j39_link_config_CAN_B_SA);

     return;

}


/*================================================================================================================
** Function:
**  app_health_j39_init_CDL

** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_init_CDL(void)
{

   /* According to Mustafiz, this is just a dummy variable and can be set to anything for propritery */
   scl_j1939_link_t cdl_link_a = 0;
   
   /* According to Mustafiz, this is just a dummy variable and can be set to anything */
   uint_least8_t ecm_idx = 1;  

   bool_t start = 1;

   scl_health_j39_error_t health_error_CDL;
   scl_health_j39_error_t health_error_cdl_mon_sys;
   scl_health_j39_error_t health_error_cdl_mcm;
   scl_health_j39_error_t health_error_cdl_elec_implement;
              
   /* Register J1939 Link */
   app_health_11bit_link_CDL = scl_health_j39_add_link(cdl_link_a,
                                                       &link_config_CDL,
                                                       start,
                                                       &health_error_CDL);



  if(health_error_CDL == SCL_HEALTH_J39_SUCCESS)
  {
      printf("scl_health_j39_add_link SUCCESS\n");
        
      /* Set health periodic period */
      scl_health_j39_prop_link_set_task_period(app_health_11bit_link_CDL,app_health_11bit_tsk_period);    
 
 
      /* Register CPM ecm */ 
      app_health_11bit_ecm_CDL = scl_health_j39_link_add_ecm(app_health_11bit_link_CDL,
                                                             ecm_idx,
                                                             &ecm_config,
                                                             start,
                                                             &health_error_CDL);
    
      
      if(health_error_CDL == SCL_HEALTH_J39_SUCCESS)
      {
        
          printf("scl_health_j39_link_add_ecm SUCCESS\n");

          /* Initialize fault manager interface */
          health_error_CDL = scl_health_j39_link_set_fault_mgr(app_health_11bit_link_CDL,
                                                                 app_health_j39_fault_mgr_ifc,
                                                                 app_scl_obd_es, /* fault_mgr_ptr */
                                                                 &app_test_handle_CID248_FMI9, /* no_comm_fault_ptr */
                                                                 &app_test_handle_CID248_FMI14); /* config_err_fault_ptr */ 


            if(health_error_CDL == SCL_HEALTH_J39_SUCCESS)
            {
                printf("scl_health_j39_link_set_fault_mgr SUCCESS\n");   
                    
                /* A "peer" is an ECU from which you expect messages.
                   Called for each peer ECU, i.e. each ECU from which the application
                   expects to receive messages periodically 
                */   

                app_health_11bit_peer_cdl_mon_sys = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CDL,
                                                                &app_peer_config_mon_sys,
                                                                TRUE,
                                                                &health_error_cdl_mon_sys);     
          
    
                if(health_error_cdl_mon_sys == SCL_HEALTH_J39_SUCCESS)
                {   
                    printf("scl_health_j39_ecm_add_peer MON_SYS SUCCESS\n");
             

                    app_peer1_ext_param_mon_sys_PID45 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_cdl_mon_sys,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0x45,
                                                                             start,
                                                                             &health_error_cdl_mon_sys);

                    if(health_error_cdl_mon_sys == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param MON SYS PID45 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param MON_SYS PID45 NOT %d \n", health_error_cdl_mon_sys);
                    }      
                }
                
                else
                {
                    printf("scl_health_j39_ecm_add_peer MON_SYS NOT %d \n", health_error_cdl_mon_sys);
                }

   
                /* A "peer" is an ECU from which you expect messages.
                   Called for each peer ECU, i.e. each ECU from which the application
                   expects to receive messages periodically 
                */   

                app_health_11bit_peer_cdl_mcm = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CDL,
                                                                &app_peer_config_mcm,
                                                                TRUE,
                                                                &health_error_cdl_mcm);     
          
    
                if(health_error_cdl_mcm == SCL_HEALTH_J39_SUCCESS)
                {   
                    printf("scl_health_j39_ecm_add_peer MON_SYS SUCCESS\n");
             

                    app_peer1_ext_param_mcm_PID45 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_cdl_mcm,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0x45,
                                                                             start,
                                                                             &health_error_cdl_mcm);

                    if(health_error_cdl_mcm == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param MCM PID45 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param MCM PID45 NOT %d \n", health_error_cdl_mcm);
                    }      
                }
                
                else
                {
                    printf("scl_health_j39_ecm_add_peer MCM NOT %d \n", health_error_cdl_mcm);
                }

   
                /* A "peer" is an ECU from which you expect messages.
                   Called for each peer ECU, i.e. each ECU from which the application
                   expects to receive messages periodically 
                */   

                app_health_11bit_peer_cdl_elec_implement = scl_health_j39_ecm_add_peer(app_health_11bit_ecm_CDL,
                                                                &app_peer_config_elec_implement,
                                                                TRUE,
                                                                &health_error_cdl_elec_implement);     
          
    
                if(health_error_cdl_elec_implement == SCL_HEALTH_J39_SUCCESS)
                {   
                    printf("scl_health_j39_ecm_add_peer MON_SYS SUCCESS\n");
             

                    app_peer1_ext_param_elec_implement_PID45 = scl_health_j39_peer_add_ext_param(app_health_11bit_peer_cdl_elec_implement,
                                                                             SCL_HEALTH_J39_PTYPE_CDL_PID,
                                                                             0x45,
                                                                             start,
                                                                             &health_error_cdl_elec_implement);

                    if(health_error_cdl_elec_implement == SCL_HEALTH_J39_SUCCESS)
                    {
                        printf("scl_health_j39_peer_add_ext_param ELEC_IMPLEMENT PID45 SUCCESS\n");
                    }
                    else
                    {
                        printf("scl_health_j39_peer_add_ext_param ELEC_IMPLEMENT PID45 NOT %d \n", health_error_cdl_elec_implement);
                    }      
                }
                
                else
                {
                    printf("scl_health_j39_ecm_add_peer ELEC_IMPLEMENT NOT %d \n", health_error_cdl_elec_implement);
                }

   

                


            /* ADD MORE ECMS HERE */
            }       
             
            // Can't set fault mgr 
            else
            { 
               printf("scl_health_j39_link_set_fault_mgr NOT %d \n", health_error_CDL);
            }
 
          }
    
          // Can't add CPM ECM
          else
          {
             printf("scl_health_j39_link_add_ecm NOT %d \n", health_error_CDL);
          }
       }

    // Can't add J1939 Link
    else
    {
        printf("scl_health_j39_add_link NOT %d \n", health_error_CDL);
    }          
        
         
    /* NOTE: We are not calling app_scl_dlinfo_init for CDL because Health System is not supposed
             to support CDL and have yet to be tested by CSNS. But, for SA, we do want CID248 to 
             activated if we are missing PIDs and logged it on ET. Thus, not calling  
             app_scl_dlinfo_init() for CDL, CID248 missing PIDs will not show up in System Diagnostic
             status screen on ET.
    */


    return;
}  


/*================================================================================================================
** Function:
**  app_health_j39_init
**
** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/
void app_health_j39_init(void)
{
   
   scl_health_j39_error_t health_error;

   scl_dlinfo_config_t*   app_dlinfo_config;

   /* LINK1 is on CAN Port 1 and LINK2 is on CAN Port 2*/;
   scl_dlinfo_error_t dlinfo_error;

   /* Initiliaze J1939 Health BDT */
   /* Must be called before any calls to scl_dlinfo_add_j39_link */
   health_error = scl_health_j39_init();

   if(health_error != SCL_HEALTH_J39_SUCCESS)
   {
       printf("scl_health_j39_init NOT, %d \n", health_error);
       return;
   }

   printf("scl_health_j39_init SUCCESS\n");
   
   /* RTC configuration */
   app_dlinfo_config= &app_dlinfo_config_wrtc;
   
   /* Initialize dlinfo for BDT Health Support */
   dlinfo_error = scl_dlinfo_init(app_dlinfo_config);

   /* IF dlinfo Init not successful, return */ 
   if (dlinfo_error != SCL_DLINFO_SUCCESS)
   {
       printf("scl_dlinfo_init NOT, %d \n", dlinfo_error);
       return;
   }
   printf("scl_dlinfo_init SUCCESS\n");   
   

   app_health_j39_init_CAN_A();
   app_health_j39_init_CAN_B();
   app_health_j39_init_CAN_B_SA();
   app_health_j39_init_CDL();
    
   return;
}



/*================================================================================================================
** Function:
**  app_health_j39_update_CAN_A
**
** Description:
**  Periodically process J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    void
**==============================================================================================================*/

void app_health_j39_update_CAN_A(bool_t disable)
{
    /* IF CPM is not installed, clear datalink diagnostics CID247 and CID5856 by letting BDT Health knows we have
     * received all requested params
     */
    if (disable)
    {
        /* Notify J39_health ext param received */
        scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDC8,SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDFE5D,SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN65266, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN61444, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PID45, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF4FD, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10AB5, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01E8C, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01F89, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD00461, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF074, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_CATEXTIDF0B2, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10938, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF5D9, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDF25B, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022C, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD10938, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD11B8D, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022B, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02408, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02409, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1102F, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1000E, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_prod_link_PIDF84D, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_steering_PGN61451, SCL_HEALTH_J39_PARAM_RXED);
    }

    /* ELSE CPM is installed, check datalink diagnostics CID247 and CID5856 for activation or deactivation */
    else
    {
        /* Notify J39_health ext param received */
        if (PIDC8_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDC8,SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDC8,SCL_HEALTH_J39_PARAM_TIMEOUT);
        }
        
        if (PIDFE5D_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDFE5D,SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PIDFE5D,SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN65266_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN65266, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN65266, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61444_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN61444, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_engine_PGN61444, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PID45_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PID45, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PID45, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF4FD_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF4FD, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF4FD, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10AB5_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10AB5, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10AB5, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD01E8C_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01E8C, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01E8C, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD01F89_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01F89, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD01F89, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD00461_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD00461, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD00461, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF074_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF074, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF074, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (CATEXTIDF0B2_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_CATEXTIDF0B2, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_CATEXTIDF0B2, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10938_fmi9_rcv_trans )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10938, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDD10938, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF5D9_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF5D9, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_transmission_PIDF5D9, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF25B_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDF25B, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDF25B, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD0022C_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022C, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022C, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10938_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD10938, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD10938, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD11B8D_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD11B8D, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD11B8D, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD02408_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02408, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02408, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD0022B_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022B, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD0022B, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD02409_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02409, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD02409, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD1102F_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1102F, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1102F, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD1000E_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1000E, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_2nd_display_PIDD1000E, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF84D_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_prod_link_PIDF84D, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_prod_link_PIDF84D, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }


        if (PGN61451_fmi9_rcv )
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_steering_PGN61451, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_steering_PGN61451, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }
    }
}


/*================================================================================================================
** Function:
**  app_health_j39_update_CAN_B
**
** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_update_CAN_B(bool_t disable)
{
    /* IF CPM is not installed, clear datalink diagnostics CID247 and CID5856 by letting BDT Health knows we have
     * received all requested params
     */
    if (disable)
    {
      /* Notify J39_health ext param received */
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF47A, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PID45, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF1AA, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF49C, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CE, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10049, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10321, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CA, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11884, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10EE7, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD106D9, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF82D, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF9C0, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10F6E, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11890, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FA, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FB, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FC, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FD, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_RXED);
    }

    /* ELSE CPM is installed, check datalink diagnostics CID247 and CID5856 for activation or deactivation */
    else
    {
        /* Notify J39_health ext param received or timedout */
        if (PIDD010FA_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FA, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FA, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD010FB_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FB, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FB, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD010FC_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FC, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FC, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD010FD_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FD, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD010FD, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF47A_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF47A, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF47A, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PID45_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PID45, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PID45, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF1AA_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF1AA, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF1AA, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF49C_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF49C, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF49C, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD118CE_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CE, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CE, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10049_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10049, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10049, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10321_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10321, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10321, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD118CA_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CA, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD118CA, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD11884_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11884, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11884, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10EE7_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10EE7, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10EE7, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD106D9_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD106D9, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD106D9, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF82D_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF82D, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF82D, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDF9C0_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF9C0, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDF9C0, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD10F6E_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10F6E, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD10F6E, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PIDD11890_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11890, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_implement_PIDD11890, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }
    }
}

/*================================================================================================================
** Function:
**  app_health_j39_update_CAN_B_SA
**
** Description:
**  Initialize the J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/

void app_health_j39_update_CAN_B_SA(bool_t disable)
{
    /* IF CPM is not installed, clear datalink diagnostics CID247 and CID5856 by letting BDT Health knows we have
     * received all requested params
     */
    if (disable)
    {
      /* Notify J39_health ext param received */
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_RXED);
    }

    /* ELSE CPM is installed, check datalink diagnostics CID247 and CID5856 for activation or deactivation */
    else
    {

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5347, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5348, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5349, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5350, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5351, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5352, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }

        if (PGN61485_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer2_ext_param_imu_PGN61485_SPN5353, SCL_HEALTH_J39_PARAM_TIMEOUT);
        }
    }
}

/*================================================================================================================
** Function:
**  app_health_j39_update_CDL
**
** Description:
**  Periodically process J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    void
**==============================================================================================================*/

void app_health_j39_update_CDL(bool_t disable)
{
    /* IF CPM is not installed, clear datalink diagnostics CID248 by letting BDT Health knows we have
     * received all requested params
     */
    if (disable)
    {
        /* Notify J39_health ext param received */
        scl_health_j39_ext_param_rx( app_peer1_ext_param_mon_sys_PID45,SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_mcm_PID45,SCL_HEALTH_J39_PARAM_RXED);
        scl_health_j39_ext_param_rx( app_peer1_ext_param_elec_implement_PID45,SCL_HEALTH_J39_PARAM_RXED);
    }

    /* ELSE CPM is installed, check datalink diagnostics CID248 for activation or deactivation */
    else
    {
        /* Notify J39_health ext param received */
        if (CDLPID45_fmi9_rcv_sa)
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_mon_sys_PID45,SCL_HEALTH_J39_PARAM_RXED);
            scl_health_j39_ext_param_rx( app_peer1_ext_param_mcm_PID45,SCL_HEALTH_J39_PARAM_RXED);
            scl_health_j39_ext_param_rx( app_peer1_ext_param_elec_implement_PID45,SCL_HEALTH_J39_PARAM_RXED);
        }
        else
        {
            scl_health_j39_ext_param_rx( app_peer1_ext_param_mon_sys_PID45,SCL_HEALTH_J39_PARAM_TIMEOUT);
            scl_health_j39_ext_param_rx( app_peer1_ext_param_mcm_PID45,SCL_HEALTH_J39_PARAM_TIMEOUT);
            scl_health_j39_ext_param_rx( app_peer1_ext_param_elec_implement_PID45,SCL_HEALTH_J39_PARAM_TIMEOUT);
        }
        
    }

}
/*================================================================================================================
** Function:
**  app_health_j39_update
**
** Description:
**  Periodically process J1939 Health library.
**
** Parameter:
**  None
**
** Return:
**    Returns an error code of type scl_health_j39_error_t
**==============================================================================================================*/
void app_health_j39_update(bool_t inhibit, bool_t disable)
{
    static bool_t previousInhibit = FALSE;
    if (inhibit && !previousInhibit) {
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_engine, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_transmission, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_2nd_display, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_prod_link, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_steering, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_b_implement, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_b_imu, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_mon_sys, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_mcm, TRUE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_elec_implement, TRUE);
    }
    else if (!inhibit && previousInhibit) {
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_engine, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_transmission, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_2nd_display, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_prod_link, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_a_steering, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_b_implement, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_can_b_imu, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_mon_sys, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_mcm, FALSE);
        scl_health_j39_peer_inhibit(app_health_11bit_peer_cdl_elec_implement, FALSE);
    }

    previousInhibit = inhibit;

    {
        /* NOTE: The following functions are not thread safe and needs to be called in the same task */
        scl_health_j39_prop_link_update();
        app_health_j39_update_CAN_A(disable);
        app_health_j39_update_CAN_B(disable);
        app_health_j39_update_CAN_B_SA(disable);
        app_health_j39_update_CDL(disable);
    }
}
