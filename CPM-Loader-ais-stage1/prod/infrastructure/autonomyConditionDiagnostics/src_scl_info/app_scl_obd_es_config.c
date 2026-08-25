/*******************************************************************************
Copyright 2015 Caterpillar Inc.  All rights reserved.
--------------------------------------------------------------------------------

Filename:   app_scl_obd_es_config.c

Description
This file contains all the configuration information needed by the OBD event
system.

*******************************************************************************/
#include <stdio.h>
#include "app_scl_obd_es_config.h"
#include "../src_nvm/app_nvm_globals.h"
#include "../src_nvm/app_nvm_file_cfg.h"


#define  APP_POSE_AUTONOMY_DIAG_SCL_OBD_MAX_FAULTS  7
#define  APP_POSE_AUTONOMY_EVENT_SCL_OBD_MAX_FAULTS  4

#define  APP_SCL_OBD_MAX_FAULTS         APP_AUTONOMY_EVENT_SCL_OBD_MAX_FAULTS + \
                                        APP_AUTONOMY_DIAG_SCL_OBD_MAX_FAULTS

#define  APP_SCL_OBD_MAX_FAULT_ENTRIES     20

//NVM RAM memory
unsigned_32  nvm_array[150];

scl_obd_test_handle_t app_test_handle_CID247_FMI9;
scl_obd_test_handle_t app_test_handle_CID247_FMI14;
scl_obd_test_handle_t app_test_handle_CID5856_FMI9;
scl_obd_test_handle_t app_test_handle_CID5856_FMI14;
scl_obd_test_handle_t app_test_handle_CID2348_FMI9;
scl_obd_test_handle_t app_test_handle_CID2348_FMI14;
scl_obd_test_handle_t app_test_handle_CID248_FMI9;
scl_obd_test_handle_t app_test_handle_CID248_FMI14;


uint_least8_t app_get_security_lvl(void);

/* overall configuration */
const scl_obd_es_config_t app_scl_obd_es_config =
{
   APP_SCL_OBD_MAX_FAULTS,
   20,                           /* evt_que_size */
   30,                           /* cont_fault_que_size */
   20,                           /* max_delays */
   40,                           /* max_annunc_entries */
   5,                            /* max_readiness_grps */
   5,                            /* max_subsc_fault_clear */
   5,                            /* max_subsc_fault_rep */
   APP_SCL_OBD_MAX_FAULT_ENTRIES,/* persistent_ready_list_size */
   20,                            /* persistent_fault_list_size */
   {                             /* persist_config */
      &app_nvm_file_block_table[0],                      /* nvm_block_handle */
      NULL,                 /* persistent_mem_ptr */
      0        /* persistent_mem_size */
   },
   SCL_OBD_ES_FEAT_USE_RTC,  /* persistent_features */
   1,                            /* min_active_drv_cycles */
   100,                          /* min_active_shm_time */
   10,                           /* auto_clear_warmup_cycles */
   100,                          /* auto_clear_shm_time */
   1,
   0,
   0                             /* auto_clear_extended_shm_time*/
};


/* J1939 Diagnostics Configuration */
const scl_diag_pj1939_config_t app_scl_diag_pj1939_config =
{
   20,
   APP_SCL_OBD_MAX_FAULTS,
   6,
   5,
   app_get_security_lvl,
   1
};

uint_least8_t app_get_security_lvl(void)
{
   return 1;
}

//void app_obd_get_security_lvl(uint_least8_t *security_lvl)
//{
//   *security_lvl = sec_lvl;
//}

boolean app_scl_obd_init(scl_j1939_link_t link)
{
    boolean ret = FALSE;
    scl_obd_es_error_t es_error_code;
    scl_diag_pj1939_error_t   app_scl_diag_pj1939_error;

    /* initialize OBD Event System */
    app_scl_obd_es = scl_obd_es_init
            (
                    &app_scl_obd_es_config,
                    FALSE,
                    &es_error_code
            );

    if ( es_error_code == SCL_OBD_ES_SUCCESS )
    {
        printf("scl_obd_es_init SUCCESS %d\n", es_error_code);


        /* PJ1939 diag initialization to support EDDT on J1939 link */
        app_pj1939_diag_obj = scl_diag_pj1939_init(&app_scl_diag_pj1939_config,
                link,
                0,
                app_scl_obd_es,
                NULL,
                &app_scl_diag_pj1939_error);

        if(SCL_DIAG_PJ1939_SUCCESSFUL == app_scl_diag_pj1939_error)
        {
            
            printf("scl_diag_pj1939_init SUCCESSFUL %d\n", app_scl_diag_pj1939_error);

            /* Initialize the handle generator */
            app_handle_gen = scl_obd_handle_gen_init();
            
            if(NULL != app_handle_gen)
            {
              printf("app_scp_obd_es NOT NULL\n");
            
              /* Register CID247 FMI9/14 */
              app_scl_obd_es_register_health_bdt_faults();
              ret = TRUE;
            }
            else
            {
              printf("app_scp_obd_es NULL\n");
            } 
        }
        else
        {
           
          printf("scl_diag_pj1939_init NOT %d\n", app_scl_diag_pj1939_error);
        }

    }
    else
    {

      printf("scl_obd_es_init NOT %d\n", es_error_code);
    }  

    return ret;
}

/*****************************************************************************************************************
Name:       app_scl_obd_es_register_CID247_faults()

Arguments:  void

Return:     void

Description
This operation will register all faults with the OBD Event System and J1939
Diagnostics.
*****************************************************************************************************************/
void app_scl_obd_es_register_CID247_faults(void)
{
  
    scl_obd_es_error_t    error;

    app_test_handle_CID247_FMI9 = scl_obd_handle_gen_get(app_handle_gen);
    
    error =  scl_obd_es_register_test(app_scl_obd_es,
                                      app_test_handle_CID247_FMI9,
                                      &app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_es
                                     );

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID247_FMI9 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID247_FMI9,
                                           &app_diagcfg_DL_J1939_1_CID_247_FMI_9_lv1_dl);
       
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID247_FMI9 [%d], \n", app_test_handle_CID247_FMI9); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID247_FMI9 %d, %d\n", app_test_handle_CID247_FMI9, error);
      }
   }
   else
   {
      printf("scl_obd_es_register_test CID247_FMI9 NOT\n"); 
   }
    

   app_test_handle_CID247_FMI14 = scl_obd_handle_gen_get(app_handle_gen);

   error =  scl_obd_es_register_test(app_scl_obd_es,
                              app_test_handle_CID247_FMI14,
                              &app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_es);

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID247_FMI14 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID247_FMI14,
                                           &app_diagcfg_DL_J1939_1_CID_247_FMI_14_lv1_dl);
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID247_FMI14 [%d]\n", app_test_handle_CID247_FMI14); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID247_FMI14 %d, %d\n", app_test_handle_CID247_FMI14, error);
    }
  }
  else
  {
    printf("scl_obd_es_register_test CID247_FMI14 NOT\n"); 
  }



           
}


/*****************************************************************************************************************
Name:       app_scl_obd_es_register_CID5856_faults()

Arguments:  void

Return:     void

Description
This operation will register all faults with the OBD Event System and J1939
Diagnostics.
*****************************************************************************************************************/
void app_scl_obd_es_register_CID5856_faults(void)
{
  
    scl_obd_es_error_t    error;

    app_test_handle_CID5856_FMI9 = scl_obd_handle_gen_get(app_handle_gen);
    
    error =  scl_obd_es_register_test(app_scl_obd_es,
                                      app_test_handle_CID5856_FMI9,
                                      &app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_es
                                     );


   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID5856_FMI9 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID5856_FMI9,
                                           &app_diagcfg_DL_J1939_3_CID_5856_FMI_9_lv1_dl);
       
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID5856_FMI9 [%d], \n", app_test_handle_CID5856_FMI9); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID5856_FMI9 %d, %d\n", app_test_handle_CID5856_FMI9, error);
      }
   }
   else
   {
      printf("scl_obd_es_register_test CID5856_FMI9 NOT\n"); 
   }
    

   app_test_handle_CID5856_FMI14 = scl_obd_handle_gen_get(app_handle_gen);

   error =  scl_obd_es_register_test(app_scl_obd_es,
                              app_test_handle_CID5856_FMI14,
                              &app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_es);

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID5856_FMI14 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID5856_FMI14,
                                           &app_diagcfg_DL_J1939_3_CID_5856_FMI_14_lv1_dl);
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID5856_FMI14 [%d]\n", app_test_handle_CID5856_FMI14); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID5856_FMI14 %d, %d\n", app_test_handle_CID5856_FMI14, error);
    }
  }
  else
  {
    printf("scl_obd_es_register_test CID5856_FMI14 NOT\n"); 
  }    
}


void app_scl_obd_es_register_CID2348_faults(void)
{

    scl_obd_es_error_t    error;

    app_test_handle_CID2348_FMI9 = scl_obd_handle_gen_get(app_handle_gen);

    error =  scl_obd_es_register_test(app_scl_obd_es,
                                      app_test_handle_CID2348_FMI9,
                                      &app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_es
                                     );


   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID2348_FMI9 SUCCESS\n");

        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID2348_FMI9,
                                           &app_diagcfg_DL_J1939_2_CID_2348_FMI_9_lv1_dl);

       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID2348_FMI9 [%d], \n", app_test_handle_CID2348_FMI9);
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID2348_FMI9 %d, %d\n", app_test_handle_CID2348_FMI9, error);
      }
   }
   else
   {
      printf("scl_obd_es_register_test CID2348_FMI9 NOT\n");
   }


   app_test_handle_CID2348_FMI14 = scl_obd_handle_gen_get(app_handle_gen);

   error =  scl_obd_es_register_test(app_scl_obd_es,
                              app_test_handle_CID2348_FMI14,
                              &app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_es);

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID2348_FMI14 SUCCESS\n");

        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID2348_FMI14,
                                           &app_diagcfg_DL_J1939_2_CID_2348_FMI_14_lv1_dl);
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID2348_FMI14 [%d]\n", app_test_handle_CID2348_FMI14);
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID2348_FMI14 %d, %d\n", app_test_handle_CID2348_FMI14, error);
    }
  }
  else
  {
    printf("scl_obd_es_register_test CID2348_FMI14 NOT\n");
  }
}

/*****************************************************************************************************************
Name:       app_scl_obd_es_register_CID248_faults()

Arguments:  void

Return:     void

Description
This operation will register all faults with the OBD Event System and J1939
Diagnostics.
*****************************************************************************************************************/
void app_scl_obd_es_register_CID248_faults(void)
{
  
    scl_obd_es_error_t    error;

    app_test_handle_CID248_FMI9 = scl_obd_handle_gen_get(app_handle_gen);
    
    error =  scl_obd_es_register_test(app_scl_obd_es,
                                      app_test_handle_CID248_FMI9,
                                      &app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_es
                                     );

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID248_FMI9 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID248_FMI9,
                                           &app_diagcfg_DL_CDL_CID_248_FMI_9_lv1_dl);
       
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID248_FMI9 [%d], \n", app_test_handle_CID248_FMI9); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID248_FMI9 %d, %d\n", app_test_handle_CID248_FMI9, error);
      }
   }
   else
   {
      printf("scl_obd_es_register_test CID248_FMI9 NOT\n"); 
   }
    

   app_test_handle_CID248_FMI14 = scl_obd_handle_gen_get(app_handle_gen);

   error =  scl_obd_es_register_test(app_scl_obd_es,
                              app_test_handle_CID248_FMI14,
                              &app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_es);

   if (error == SCL_OBD_ES_SUCCESS)
   {
        printf("scl_obd_es_register_test CID247_FMI14 SUCCESS\n"); 
        
        error = scl_diag_pj1939_register_fault(app_pj1939_diag_obj,
                                           app_test_handle_CID248_FMI14,
                                           &app_diagcfg_DL_CDL_CID_248_FMI_14_lv1_dl);
       if (error == SCL_OBD_ES_SUCCESS)
       {
          printf("SUCCESS!! registering app_test_handle_CID248_FMI14 [%d]\n", app_test_handle_CID248_FMI14); 
       }
       else
       {
          printf("ERROR!! registering app_test_handle_CID248_FMI14 %d, %d\n", app_test_handle_CID248_FMI14, error);
    }
  }
  else
  {
    printf("scl_obd_es_register_test CID248_FMI14 NOT\n"); 
  }           
}


/*****************************************************************************************************************
Name:       app_scl_obd_es_register_health_bdt_faults()

Arguments:  void

Return:     void

Description
This operation will register all faults with the OBD Event System and J1939
Diagnostics.
*****************************************************************************************************************/
void app_scl_obd_es_register_health_bdt_faults(void)
{

    app_scl_obd_es_register_CID247_faults();
    app_scl_obd_es_register_CID5856_faults();
    app_scl_obd_es_register_CID2348_faults();
    app_scl_obd_es_register_CID248_faults();

    return;
}
