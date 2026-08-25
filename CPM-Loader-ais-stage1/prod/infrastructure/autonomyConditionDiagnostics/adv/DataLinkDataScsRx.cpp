/******************************Advanced********************************/
#include <chrono>
#include <interfaces/MachineSN/InterfaceTypes.h>
#include <lps_sea_defs.h>

#include "../AutonomyConditionDiagnostics.h"
#include "../src_j1939/app_et_j1939_support.h"
#include "../src_prmsw/app_prmsw_config.h"
#include "BMI_CDL_PID.h"
#include "../src_j1939/app_j1939_map.h"
#include "../src_j1939/app_health_j39.h"
#include <catdllib_private.h>   

/* Delay IMU diagnostics by 8secs on top of the 2secs configured in the J1939
 * config for bmi when calibration is not in progress. This is to address a
 * IMU reset that can cause IMU data to get delayed by few seconds. Root cause
 * of the IMU reset is not known, hence this patch.
 */
#define IMU_DELAY_DIAG_WHEN_CAL_NOT_IN_PROGRESS_SECS    8

void AutonomyConditionDiagnostics::DataLinkDataScsRx(void)
{
    DataLinkData dlData;

    /* Activate datalink diagnostic if we are missing datalink paramaters */
    isDatalinkDiagDisabled = FALSE;

    while (m_dataLinkDataInput->get(dlData)) {
        for (auto& dlParam : dlData.GetParams()) {
            if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID)
            {
                switch (dlParam.GetParamId()) {

                case 0xD010FB:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD010FB_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD010FB_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD010FB_fmi9_rcv =  %d   stat = %d ", PIDD010FB_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xD010FA:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD010FA_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD010FA_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD010FA_fmi9_rcv =  %d   stat = %d ", PIDD010FA_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xD010FD:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {

                        PIDD010FD_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD010FD_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD010FD_fmi9_rcv =  %d   stat = %d ", PIDD010FD_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xD010FC:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD010FC_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD010FC_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD010FC_fmi9_rcv =  %d   stat = %d ", PIDD010FC_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xFE5D:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDFE5D_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {     
                        PIDFE5D_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDFE5D_fmi9_rcv =  %d   stat = %d ", PIDFE5D_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xF4FD: 
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF4FD_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF4FD_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF4FD_fmi9_rcv  =  %d   stat = %d ", PIDF4FD_fmi9_rcv, dlParam.GetLastValueDsi());
                }

                break;

                case 0xD10AB5:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD10AB5_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {   
                        PIDD10AB5_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD10AB5_fmi9_rcv   =  %d   stat = %d ", PIDD10AB5_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                case 0xD01E8C:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD01E8C_fmi9_rcv  = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD01E8C_fmi9_rcv  = true;
                    }

                    AIS_LOG_INFO("PIDD01E8C_fmi9_rcv    =  %d   stat = %d ", PIDD01E8C_fmi9_rcv  , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD01F89:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD01F89_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {   
                        PIDD01F89_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD01F89_fmi9_rcv     =  %d   stat = %d ", PIDD01F89_fmi9_rcv   , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD00461:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD00461_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {   
                        PIDD00461_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD00461_fmi9_rcv      =  %d   stat = %d ", PIDD00461_fmi9_rcv    , dlParam.GetLastValueDsi());
                }

                break;


                case 0xF074:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF074_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    { 
                        PIDF074_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF074_fmi9_rcv       =  %d   stat = %d ", PIDF074_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xF25B:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF25B_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {   
                        PIDF25B_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF25B_fmi9_rcv        =  %d   stat = %d ", PIDF25B_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD0022C:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD0022C_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD0022C_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD0022C_fmi9_rcv        =  %d   stat = %d ", PIDD0022C_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD10938:
                {

                   // uint32_t sid = dlParam.GetSid();
                                     
                    // Transmission
                    if (dlParam.GetSid() == 0x03)
                    {
                         /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                         if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                              (0xFF11 == dlParam.GetLastValueDsi()) ||
                              (0xFF10 == dlParam.GetLastValueDsi()) )
                         {
                            PIDD10938_fmi9_rcv_trans = false;
                         }
                         else /* Do not activate fmi9/14 faults */
                         {
                           PIDD10938_fmi9_rcv_trans = true;
                         }

                         AIS_LOG_INFO("PIDD10938_fmi9_rcv_trans   =  %d   stat = %d ", PIDD10938_fmi9_rcv_trans , dlParam.GetLastValueDsi());   
                         AIS_LOG_INFO("PIDD10938_fmi9_rcv_trans DATA   =  %d ", dlParam.GetLastValue<uint32_t>());     
                    }
                    
                    
                    // 2nd Display
                    if (dlParam.GetSid() == 0x28)
                    {
                         /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                         if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                              (0xFF11 == dlParam.GetLastValueDsi()) ||
                              (0xFF10 == dlParam.GetLastValueDsi()) )
                         {
                            PIDD10938_fmi9_rcv = false;
                         }
                         else /* Do not activate fmi9/14 faults */
                         {
                           PIDD10938_fmi9_rcv = true;
                         }

                        AIS_LOG_INFO("PIDD10938_fmi9_rcv         =  %d   stat = %d ", PIDD10938_fmi9_rcv , dlParam.GetLastValueDsi());  
                        AIS_LOG_INFO("PIDD10938_fmi9_rcv DATA   =  %d ", dlParam.GetLastValue<uint32_t>());   
                    }
                }

                break;

                case 0xD11B8D:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD11B8D_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD11B8D_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD11B8D_fmi9_rcv         =  %d   stat = %d ", PIDD11B8D_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                case 0xD0022B:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD0022B_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD0022B_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD0022B_fmi9_rcv          =  %d   stat = %d ", PIDD0022B_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                case 0xD02408:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD02408_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD02408_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD02408_fmi9_rcv          =  %d   stat = %d ", PIDD02408_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD02409:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD02409_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD02409_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD02409_fmi9_rcv           =  %d   stat = %d ", PIDD02409_fmi9_rcv  , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD1102F:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD1102F_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD1102F_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD1102F_fmi9_rcv           =  %d   stat = %d ", PIDD1102F_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD1000E:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD1000E_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD1000E_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD1000E_fmi9_rcv            =  %d   stat = %d ", PIDD1000E_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xF84D:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) ||
                            (0xFFF0 == dlParam.GetLastValueDsi()) )  /* Aaron sd no timeout for read poll */
                    {
                        PIDF84D_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF84D_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF84D_fmi9_rcv             =  %d   stat = %d ", PIDF84D_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xF47A:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF47A_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF47A_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF47A_fmi9_rcv              =  %d   stat = %d ", PIDF47A_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0x45:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PID45_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PID45_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PID45_fmi9_rcv               =  %d   stat = %d ", PID45_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xF49C:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF49C_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF49C_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF49C_fmi9_rcv                =  %d   stat = %d ", PIDF49C_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD118CE:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD118CE_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD118CE_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD118CE_fmi9_rcv                 =  %d   stat = %d ", PIDD118CE_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;



                case 0xD10049:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD10049_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {   
                        PIDD10049_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD10049_fmi9_rcv                  =  %d   stat = %d ", PIDD10049_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD10321:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD10321_fmi9_rcv = false;
                    }
                    /* We are getting dsi16, activate fmi14 */
                    else if ( (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        // FMI14 not supported by J1939 Health!
                    }
                    /* Do not activate fmi9/14 faults */         
                    else 
                    {
                        PIDD10321_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD10321_fmi9_rcv                   =  %d   stat = %d ", PIDD10321_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                case 0xD118CA:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD118CA_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD118CA_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD118CA_fmi9_rcv                    =  %d   stat = %d ", PIDD118CA_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;


                case 0xD106D9:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD106D9_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD106D9_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD106D9_fmi9_rcv                       =  %d   stat = %d ", PIDD106D9_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                case 0xD11890:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD11890_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    { 
                        PIDD11890_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD11890_fmi9_rcv                        =  %d   stat = %d ", PIDD11890_fmi9_rcv , dlParam.GetLastValueDsi());
                }


                break;


                case 0xF1AA:
                {
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF1AA_fmi9_rcv = false;
                    }
                    else  /* Do not activate fmi9/14 faults */
                    { 
                        PIDF1AA_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF1AA_fmi9_rcv                =  %d   stat = %d ", PIDF1AA_fmi9_rcv , dlParam.GetLastValueDsi());


                    if( 0 == dlParam.GetLastValueDsi())
                    {
                        if (dlParam.GetLastValueEng() == 1) {
                            /* store button is depressed */
                            PidF1AA = 0x01;
                        }
                        else {
                            /* store button is not depressed */
                            PidF1AA = 0x00;
                        }

                    }
                    else {
                        /* DSI */
                        PidF1AA = 0xE0 + FMICNM;
                    }

                    AIS_LOG_INFO("Store Button PID 0x%X with Val =  %f   stat = %d ", dlParam.GetParamId(), dlParam.GetLastValueEng(), dlParam.GetLastValueDsi());
                }

                break;

                /* Check if Product ID has changed from what is already configured */
                case 0xF82D:
                    AIS_LOG_INFO("Inside F82D, dlParam.GetLastValueDsi() = %d, dlParam.GetVarLengthParamDsi() = %d",
                            dlParam.GetLastValueDsi(), dlParam.GetVarLengthParamDsi());
                    AIS_LOG_INFO("dlParam.GetVarLengthParamType() = %d, dlParam.GetVarParamBlockLength() = %d",
                            dlParam.GetVarLengthParamType(),dlParam.GetVarParamBlockLength());

                    if((dlParam.GetVarLengthParamType() == VarLengthDataLinkParamPool::PRODUCT_ID) &&
                            (0 == dlParam.GetLastValueDsi()) &&
                            (dlParam.GetVarParamBlockLength() == ProductIdDataLinkParam::m_ProductIdParamSize))
                    {
                        const ProductIdDataLinkParam prodIdParam(const_cast<DataLinkParam&>(dlParam));

                        char newProdId[PIDF82D_LENGTH];
                        prodIdParam.GetProductId(newProdId);

                        bool prodIdDifferent = strncmp(newProdId, pidF82D_pdata, PIDF82D_LENGTH) != 0;

                        if (prodIdDifferent) {
                            if (!save_product_id_to_nvm(newProdId, PIDF82D_LENGTH)) {
                                AIS_LOG_ERROR("Product ID is not valid, did not write to memory");
                            }
                        }
                    }

                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) ||
                            (0xFFF0 == dlParam.GetLastValueDsi()) )  /* Aaron sd no timeout for read poll */
                    {
                        PIDF82D_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF82D_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF82D_fmi9_rcv                        =  %d   stat = %d ", PIDF82D_fmi9_rcv , dlParam.GetLastValueDsi());

                    break;

                case 0xF9C0:
                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) ||
                            (0xFF13 == dlParam.GetLastValueDsi()) ||   // FF13 when not supported
                            (0xFFF0 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF9C0_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF9C0_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF9C0_fmi9_rcv                        =  %d   stat = %d ", PIDF9C0_fmi9_rcv , dlParam.GetLastValueDsi());

                    break;

                case LOAD_HOLD_CHECK_VALVE_INSTALL_STATUS_PID:
                {
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDD10F6E_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD10F6E_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDD10F6E_fmi9_rcv                     =  %d   stat = %d ", PIDD10F6E_fmi9_rcv , dlParam.GetLastValueDsi());                }
                break;

                case TRAVEL_LOADED_WHEEL_REVOLUTIONS_PID:
                {
                    if( dlParam.GetLastValueDsi() == BMI_CDL_DATA_STATUS_OK )
                    {
                        PidD026AF = dlParam.GetLastValue<uint16_t>();
                    }
                    else
                    {
                        PidD026AF = UNKNOWN2U + dlParam.GetLastValueDsi();
                    }
                }
                break;

                case PAYLOAD_CYCLE_CURRENT_DIG_TIME_PID:
                {
                    if( dlParam.GetLastValueDsi() == BMI_CDL_DATA_STATUS_OK )
                    {
                        PidD026B1 = dlParam.GetLastValue<uint16_t>();
                    }
                    else
                    {
                        PidD026B1 = UNKNOWN2U + dlParam.GetLastValueDsi();
                    }
                }
                break;

                case CPM_LVL2_FEATURE_INSTALL_STATUS_PID:
                {
                    if( dlParam.GetLastValueDsi() == BMI_CDL_DATA_STATUS_OK )
                    {
                        if (0x0010 == dlParam.GetLastValue<uint16_t>()) { /* installed */
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_INSTALL );
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_PERM_INSTALL );
                        }
                        else { /* uninstalled */
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_UNINSTALL );
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_PERM_UNINSTALL );
                        }
                    }
                    else
                    {
                        /* Do nothing since we don't know what the status is anymore */
                    }

                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetVarLengthParamDsi()) )
                    {
                        PIDD11884_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD11884_fmi9_rcv = true;
                    }
                    
                    scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_INSTALL );

                    AIS_LOG_INFO("PIDD11884_fmi9_rcv                     =  %d   stat = %d ", PIDD11884_fmi9_rcv , dlParam.GetLastValueDsi());
                }
                break;

                case CPM_FEATURE_ENABLE_STATUS_PID:
                {
                    if( dlParam.GetLastValueDsi() == BMI_CDL_DATA_STATUS_OK )
                    {
                        if (0x000C == dlParam.GetLastValue<uint16_t>()) { /* enabled */
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_ENABLE );
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_PERM_ENABLE );
                        }
                        else { /* disabled */
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_DISABLE );
                            scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_PERM_DISABLE );
                        }
                    }
                    else
                    {
                        /* Do nothing since we don't know what the status is anymore*/
                    }

                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetVarLengthParamDsi()) )
                    {
                        PIDD10EE7_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDD10EE7_fmi9_rcv = true;
                    }
                    
                    scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_149, SCL_PRMSW_FEAT_PERM_ENABLE );
                    scl_prmsw_set_feat_cmd( LPS_SEA_REASON_CODE_245, SCL_PRMSW_FEAT_PERM_ENABLE );

                    AIS_LOG_INFO("PIDD10EE7_fmi9_rcv                      =  %d   stat = %d ", PIDD10EE7_fmi9_rcv , dlParam.GetLastValueDsi());
                }
                break;

                default:
                    break;
                } /* switch (dlParam.GetParamId()) */
            } /* if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID)*/

            
            if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PUBLIC_PGN) {
                switch (dlParam.GetParamId()) {

                // EEC1
                case (61444): 
                    {
                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF10 == dlParam.GetVarLengthParamDsi()) )
                        {     
                            PGN61444_fmi9_rcv = false;
                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            PGN61444_fmi9_rcv = true;
                        }
                        
                        AIS_LOG_INFO("PGN61444_fmi9_rcv             =  %d   stat = %d ", PGN61444_fmi9_rcv , dlParam.GetVarLengthParamDsi());
                    }
                    break;

                 
                case (61451): 
                    {
                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFFF0 == dlParam.GetVarLengthParamDsi()) )
                        {     
                            PGN61451_fmi9_rcv = false;
                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            PGN61451_fmi9_rcv = true;
                        }
                        
                        AIS_LOG_INFO("PGN61451_fmi9_rcv             =  %d   stat = %d ", PGN61451_fmi9_rcv , dlParam.GetVarLengthParamDsi());
                    }
                    break;
                    

                case (65266): 
                    {
                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF10 == dlParam.GetLastValueDsi()) )
                        {     
                            PGN65266_fmi9_rcv = false;
                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            PGN65266_fmi9_rcv = true;
                        }
                        
                        AIS_LOG_INFO("PGN65266_fmi9_rcv             =  %d   stat = %d ", PGN65266_fmi9_rcv , dlParam.GetVarLengthParamDsi());
                    }
                    break;


            
                case (61485): 
                    {
                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFFF0 == dlParam.GetVarLengthParamDsi()) )  /* Remove this line after Aaron fix TES */
                        {     
                            auto steadyNow = std::chrono::steady_clock::now();
                            auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(steadyNow - imuDiagStartTime_);

                            if (PayloadCalInProgress_) {
                                PGN61485_fmi9_rcv = false;
                                PGN61485_fmi9_rcv_prev = false;
                            }
                            else if (PGN61485_fmi9_rcv_prev) {
                                /* was received previously and cal is not in progress, start a delay timer.
                                 * This is so that we can delay IMU diagnostics when cal is not in progress
                                 * due to a known reset issue on the IMU. During calibration we have to continue
                                 * to diagnose at a tighter debounce period */
                                imuDiagStartTime_ = steadyNow;
                                PGN61485_fmi9_rcv_prev = false;
                            }
                            else if (timeDiff.count() > IMU_DELAY_DIAG_WHEN_CAL_NOT_IN_PROGRESS_SECS)  {
                                    PGN61485_fmi9_rcv = false;
                                    PGN61485_fmi9_rcv_prev = false;
                            }
                            else {
                                    // do nothing, wait for delay debounce
                            }

                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            PGN61485_fmi9_rcv = true;
                            PGN61485_fmi9_rcv_prev = true;
                        }
                        
                        AIS_LOG_INFO("PGN61485_fmi9_rcv             =  %d   stat = %d ", PGN61485_fmi9_rcv , dlParam.GetVarLengthParamDsi());
                    }
                    break;

                        
                default:
                    break;

                } /* switch (dlParam.GetParamId()) */
            } /* if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PUBLIC_PGN)*/         

            if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_CAT_EXT) {
                switch (dlParam.GetParamId()) {

                
                case (0xF0B2): 
                    {
                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF10 == dlParam.GetLastValueDsi())      ||
                             (0xFFF0 == dlParam.GetVarLengthParamDsi()) ) /* Remove this line after Aaron fix TES */
                        {     
                            CATEXTIDF0B2_fmi9_rcv = false;
                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            CATEXTIDF0B2_fmi9_rcv = true;
                        }
                        
                        AIS_LOG_INFO("CATEXTIDF0B2_fmi9_rcv             =  %d   stat = %d ", CATEXTIDF0B2_fmi9_rcv , dlParam.GetVarLengthParamDsi());
                    }
                    break;
                        
                default:
                    break;

                } /* switch (dlParam.GetParamId()) */
            } /* if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_CAT_EXT)*/                               
        } /* for(auto it = dlDataMap->begin(); it != dlDataMap->end(); ++it)*/
    } /*while (m_dataLinkDataInput->get(dlData))*/
}
