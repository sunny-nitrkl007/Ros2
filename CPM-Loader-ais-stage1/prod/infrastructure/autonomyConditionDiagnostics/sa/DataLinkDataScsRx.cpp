/******************************Standalone********************************/
#include "../AutonomyConditionDiagnostics.h"
#include <lps_sea_defs.h>

#include "BMI_CDL_PID.h"
#include "../src_j1939/app_health_j39.h"

/* clock_time_t definition */
#include <clock_defs.h>

#define MAX_ALLOWED_SHM_DIFFERENCE_SECONDS (3.0)

/* Delay IMU diagnostics by 8secs on top of the 2secs configured in the J1939
 * config for bmi when calibration is not in progress. This is to address a
 * IMU reset that can cause IMU data to get delayed by few seconds. Root cause
 * of the IMU reset is not known, hence this patch.
 */
#define IMU_DELAY_DIAG_WHEN_CAL_NOT_IN_PROGRESS_SECS    8

void AutonomyConditionDiagnostics::DataLinkDataScsRx(void)
{
    DataLinkData dlData;

    /* Disabled datalink diagnostic if Machine Model Linkage is not selected */
    isDatalinkDiagDisabled = dl_diag_disabled;

    while (m_dataLinkDataInput->get(dlData)) {
        for (auto& dlParam : dlData.GetParams()) {
            if (dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PID) {

                switch (dlParam.GetParamId()) {

                case TOTAL_OPERATING_HOURS: {
                    if (BMI_CDL_DATA_STATUS_OK == dlParam.GetLastValueDsi()) {
                        // Get the current time in seconds (unsigned_32)
                        clock_time_t local_shm_sec = clock_getservice_sec();

                        /*
                         * Parse PID FC2D
                         * This must be put into a double because a single precision
                         * float does not contain enough range AND resolution to
                         * accommodate the value.
                         */
                        double PIDFC2D_data = dlParam.GetLastGoodValueEng();

                        AIS_LOG_INFO("PIDFC2D received from Engine RAW = %f hours.", PIDFC2D_data);

                        // Convert from hour to seconds
                        PIDFC2D_data = PIDFC2D_data * 3600;

                        // Find the difference between the service hour meters
                        double seconds_difference = static_cast<double>(local_shm_sec) - PIDFC2D_data;

                        // Get the absolute value of the difference in seconds
                        if (seconds_difference < 0) {
                            // We are behind
                            seconds_difference = -seconds_difference;
                        }

                        if (seconds_difference > MAX_ALLOWED_SHM_DIFFERENCE_SECONDS) {
                            // The two hour meters are different enough that we should sync.
                            clock_setservice_sec(PIDFC2D_data);

                            AIS_LOG_INFO("Updating local service hour meter to %f seconds.", PIDFC2D_data);
                        }
                        else {
                            // Do nothing, the hour meters are close enough
                        }
                    }
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

                case 0x45:
                {
                    const auto dataLinkType = dlParam.GetDataLinkType();

                    /* Only log CID247/248/5856 if HydOilTemp sensor is enabled */
                    AIS_LOG_INFO("HydOilTempEnabled stat = %d ", HydOilTempEnabled);
                    if (HydOilTempEnabled)
                    {
                        /* We are getting dsi9 and dsi17, activate fmi9.
                         * Added FF10 (CDL2_DSI_PARAM_NOT_AVAIL) because we get FF10 from csns
                         * for about 60secs initially before the correct dsi of FF11 is sent */
                        if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                             (0xFF11 == dlParam.GetLastValueDsi()) ||
                             (0xFF10 == dlParam.GetLastValueDsi()) )
                        {
                            if (DataLinkParamInfo::DlpDataLinkType_t::DATA_LINK_PARAM_CDL == dataLinkType)
                            {
                                CDLPID45_fmi9_rcv_sa = false;
                            }
                            else if (DataLinkParamInfo::DlpDataLinkType_t::DATA_LINK_PARAM_CAN1_1939 == dataLinkType)
                            {
                                PID45_fmi9_rcv_sa = false;
                            }
                        }
                        else  /* Do not activate fmi9/14 faults */
                        {
                            PID45_fmi9_rcv_sa = true;
                            CDLPID45_fmi9_rcv_sa = true;
                        }
                    }
                    else
                    {
                        PID45_fmi9_rcv_sa = true;
                        CDLPID45_fmi9_rcv_sa = true;
                    }

                    AIS_LOG_INFO("PID45_fmi9_rcv_sa               =  %d   stat = %d ", PID45_fmi9_rcv_sa , dlParam.GetLastValueDsi());
                    AIS_LOG_INFO("CDLPID45_fmi9_rcv_sa            =  %d   stat = %d ", CDLPID45_fmi9_rcv_sa , dlParam.GetLastValueDsi());
                }

                break;

                // Ground Speed (only supported for m-series)
                case 0xF4FD:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                // Actual Gear (only supported for m-series)
                case 0xF5D9:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDF5D9_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDF5D9_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDF5D9_fmi9_rcv =  %d   stat = %d ", PIDF5D9_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                // Tilt Lever Position (only supported for m-series)
                case 0xF49C:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                    AIS_LOG_INFO("PIDF49C_fmi9_rcv =  %d   stat = %d ", PIDF49C_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                // Machine Idle Status (only supported for m-series)
                case 0xD10AB5:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                    AIS_LOG_INFO("PIDD10AB5_fmi9_rcv  =  %d   stat = %d ", PIDD10AB5_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                // Total Fuel Low Res (only supported for m-series)
                case 0xC8:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

                    /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                    if ( (0xFF09 == dlParam.GetLastValueDsi()) ||
                            (0xFF11 == dlParam.GetLastValueDsi()) ||
                            (0xFF10 == dlParam.GetLastValueDsi()) )
                    {
                        PIDC8_fmi9_rcv = false;
                    }
                    else /* Do not activate fmi9/14 faults */
                    {
                        PIDC8_fmi9_rcv = true;
                    }

                    AIS_LOG_INFO("PIDC8_fmi9_rcv =  %d   stat = %d ", PIDC8_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                // Total Fuel High Res (only supported for m-series)
                case 0xFE5D:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                // GPS (only supported for m-series)
                case 0xF84D:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                    AIS_LOG_INFO("PIDF84D_fmi9_rcv =  %d   stat = %d ", PIDF84D_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                // External Force (only supported for m-series)
                case 0xD01E8C:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                    AIS_LOG_INFO("PIDD01E8C_fmi9_rcv =  %d   stat = %d ", PIDD01E8C_fmi9_rcv  , dlParam.GetLastValueDsi());
                }

                break;

                // Implement Operation Status (only supported for m-series)
                case 0xD106D9:
                {
                    if (!CycleSegStatus_) {
                        // If cycle segmentation not supported, do not activate faults
                        PIDF5D9_fmi9_rcv = true;
                        break;
                    }

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

                    AIS_LOG_INFO("PIDD106D9_fmi9_rcv =  %d   stat = %d ", PIDD106D9_fmi9_rcv , dlParam.GetLastValueDsi());
                }

                break;

                } /* end switch */
            } /* end if PID*/

            if(dlParam.GetParamIdentifierType() == DataLinkParam::DATA_LINK_PARAM_IDENTIFIER_PUBLIC_PGN) {

                switch (dlParam.GetParamId()) {

                case (61485):
                    {
                        if (!SEALegalForTradeInstalled_) {
                            // IF on SA and not LFT then disable diagnostics on the IMU.
                            PGN61485_fmi9_rcv_sa = true;
                            PGN61485_fmi9_rcv_prev_sa = true;
                            break;
                        }

                        /* We are getting dsi9, dsi16 and dsi17, activate fmi9 */
                        if ( (0xFF09 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF11 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFF10 == dlParam.GetVarLengthParamDsi()) ||
                             (0xFFF0 == dlParam.GetVarLengthParamDsi()) )  /* Remove this line after Aaron fix TES */
                        {
                            auto steadyNow = std::chrono::steady_clock::now();
                            auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(steadyNow - imuDiagStartTime_);

                            if (PayloadCalInProgress_) {
                                PGN61485_fmi9_rcv_sa = false;
                                PGN61485_fmi9_rcv_prev_sa = false;
                            }
                            else if (PGN61485_fmi9_rcv_prev_sa) {
                                /* was received previously and cal is not in progress, start a delay timer.
                                 * This is so that we can delay IMU diagnostics when cal is not in progress
                                 * due to a known reset issue on the IMU. During calibration we have to continue
                                 * to diagnose at a tighter debounce period */
                                imuDiagStartTime_ = steadyNow;
                                PGN61485_fmi9_rcv_prev_sa = false;
                            }
                            else if (timeDiff.count() > IMU_DELAY_DIAG_WHEN_CAL_NOT_IN_PROGRESS_SECS)  {
                                    PGN61485_fmi9_rcv_sa = false;
                                    PGN61485_fmi9_rcv_prev_sa = false;
                            }
                            else {
                                    // do nothing, wait for delay debounce
                            }

                        }
                        else /* Do not activate fmi9/14 faults */
                        {
                            PGN61485_fmi9_rcv_sa = true;
                            PGN61485_fmi9_rcv_prev_sa = true;
                        }

                        AIS_LOG_INFO("PGN61485_fmi9_rcv             =  %d   stat = %d ", PGN61485_fmi9_rcv_sa , dlParam.GetVarLengthParamDsi());
                    }
                    break;

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

                // LFE1
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

                default:
                    break;
                } /* end switch */
            } /* end PGN if*/
        } /* end for */
    } /* end while */
}
