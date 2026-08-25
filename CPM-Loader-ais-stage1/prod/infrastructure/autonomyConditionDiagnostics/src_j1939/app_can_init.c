/*******************************************************************************
 ***
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *** %%                                                                        %%
 *** %%  COPYRIGHT (C) 2014 CATERPILLAR INC.   ALL RIGHTS RESERVED.            %%
 *** %%      This work contains proprietary information which may              %%
 *** %%      constitute a trade secret and/or be confidential.                 %%
 *** %%                                                                        %%
 *** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 **
 *
 * FILE NAME: app_can_init.c
 *
 * DESCRIPTION: HAL CAN Interface to j1939 limited support version
 *
 * LANGUAGE: ANSI C
 *
 * FUNCTION LIST: app_can_init()
 *
 * HISTORY :
 *
 *******************************************************************************/

/*******************************************************************************
 ***
 ***    Referenced Functions, Data, & Symbols
 ***    -- #Include's --
 ***
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef   STD_TYPES_H_
#include <std_types.h>
#endif

#ifndef   HAL_J1939V2_PORT_H_
#include <hal_j1939v2_port.h>
#endif

#ifndef   SCL_J1939_H_
#include <scl_j1939.h>
#endif

// Why is this not externed anywhere?
extern void scl_j1939_set_map_timeouts(int_16, int_16);

#ifndef   APP_J1939_TABLES_H_
#include "app_j1939_tables.h"
#endif

#ifndef __APP_CAN_INIT_H__
#include "app_can_init.h"
#endif

#include <oel_rtos_posix.h>

#ifndef APP_J1939_MAP_H_
#include "app_j1939_map.h"
#endif

#include <taskPlugins/interfaces/PlatformDefinesC.h>
#include <cdl2_proto.h>

#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif

#include <scl_adt_j1939.h>
#include <scl_pgb_j1939.h>

#include <catdllib_fid_def.h>
#include <scl_health_j39.h>
#include <scl_health_j39_status.h>
#include <scl_j1939_pro.h>
#if SCL_STAT_DB_SUPPORTED
#include <scl_stat_db.h>
#endif
#include <bdt_action.h>
#include <bdt_proto.h>

#include "../src_app/app_rtos_config.h"
#include "app_health_j39.h" 
#include "../src_scl_info/app_scl_obd_es_config.h"

/*
 * These used to be in the EthernetPortStatistics plugin.
 * They were pulled out when BDT was used for more than just
 * EthernetPortStatistics
 */
#define BDT_INIT_NUM_RX_CON (4)
#define BDT_INIT_RX_BUF_SIZE (240)
#define BDT_INIT_NUM_TX_CON (4)
#define BDT_INIT_TX_BUF_SIZE (240)
#define BDT_INIT_NUM_DATALINK_CON (3)
#define BDT_ACTION_INIT_MAX_SIMULTANEOUS_CLIENTS (3)

// seconds U32_L20 -> micro-seconds
#define BDT_UPDATE_PERIOD_MICRO_SECONDS_U32 ((uint32_t)((((double)TASK_100MS_PERIOD / (1UL<<20)) * 1000000) + 0.5))
#define BDT_CALLBACK_BLOCK_COUNT (10)
#define BDT_DATATYPE_COUNT (10)

#define NONE_AVAILABLE  (0xFF)

inline static void wait_for_weigh_app_response();
static int_16 cdl_fr_tx_hdlr_weigh_app(void* tx_pie, unsigned_8* data);
static int_16 cdl_f_tx_hdlr_weigh_app(void* tx_pie, unsigned_8* data);

inline static void wait_for_jobmgr_app_response();
static int_16 cdl_fr_tx_hdlr_jobmgr_app(void* tx_pie, unsigned_8* data);
static int_16 cdl_f_tx_hdlr_jobmgr_app(void* tx_pie, unsigned_8* data);

static int_16 cdl_fr_tx_hdlr(void* tx_pie, unsigned_8* data);
static int_16 cdl_f_tx_hdlr(void* tx_pie, unsigned_8* data);
static int_16 cdl_str_tx_hdlr(void* tx_pie, unsigned_8* data, int max_str_len);
static int_16 cdl_str_tx_hdlr_24(void* tx_pie, unsigned_8* data);
static int_16 cdl_str_tx_hdlr_26(void* tx_pie, unsigned_8* data);

void app_scl_adt_cdl_server_init(void);
void app_scl_adt_j1939_server_init(scl_j1939_link_t link);

// ECM J1939 Name
j1939_name_address_t j1939_name_address;
unsigned_16 j1939_mid;
unsigned_16 st_app_num;
unsigned_16 st_chg_lvl;

#define DM13_SUSPEND_INVALID_SA (0xFE)
static unsigned_8 dm13_suspend_list[] = {
    DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA,
    DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA,
    DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA,
    DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA, DM13_SUSPEND_INVALID_SA
};

// BEM Sim - Engine derate via PGN 0 (TSC1)
#define OVR_CONTROL_MODE_PRIORITY (0x10)      /* 01b = High Priority */
#define OVR_CONTROL_CONDITIONS (0x08)         /* 10b = Stability Optimized for driveline engaged and/or in lockup condition 1 (e.g., vehicle driveline) */
#define OVR_CONTROL_MODE_LIMIT (0x03)         /* 11b = Speed/torque limit control */
#define OVR_CONTROL_BYTE_LIMITING   (OVR_CONTROL_MODE_PRIORITY | OVR_CONTROL_CONDITIONS | OVR_CONTROL_MODE_LIMIT)

bool_t bem_sim_derate_enable = FALSE;
int bem_sim_derate_rpm = 0;

bool_t dm13_receive_suspend = FALSE;

/* HAL_CANV3 resource config */
static const oel_rtos_resource_pi_vconfig_t resource_hal_canv3_vconfig = OEL_RTOS_RESOURCE_PI_VCONFIG();

const oel_rtos_resource_config_t *app_hal_canv3_resource_config = OEL_BASE(&resource_hal_canv3_vconfig);

/* BDT resource config */
static const oel_rtos_resource_pi_vconfig_t resource_bdt_vconfig = OEL_RTOS_RESOURCE_PI_VCONFIG();
const oel_rtos_resource_config_t *app_bdt_resource_config = OEL_BASE(&resource_bdt_vconfig);

static const oel_rtos_resource_pi_vconfig_t app_scl_j1939_semaphore_vconfig = OEL_RTOS_RESOURCE_PI_VCONFIG();


const scl_j1939_config_t app_scl_j1939_config =
{
    OEL_BASE(&app_scl_j1939_semaphore_vconfig),
    TRUE,
    FALSE
};
/* Stores the j1939 link information
// This is stored here because we need
// to change the tx msg control byte
// from within retarder main
 */
scl_j1939_link_t Ph_Link_app;
scl_j1939_link_t TES_Link_app;

scl_adt_j1939_link_handle_t  adt_j1939_link_handle;

void app_dm13_send_sleep_msg()
{
    /* Suspend signal with indefinite suspension of all broadcasts */
    unsigned_8 sleep_msg[SCL_J1939_MAX_1_FRAME_MSG] =
    { 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff };

    scl_j1939_que_tx_msg_by_address(Ph_Link_app,
        SCL_J1939_DM13_PGN,
        6,
        SCL_J1939_GLOBAL_ADDRESS,
        0,
        sleep_msg,
        SCL_J1939_MAX_1_FRAME_MSG);

    scl_j1939_que_tx_msg_by_address(TES_Link_app,
            SCL_J1939_DM13_PGN,
            6,
            SCL_J1939_GLOBAL_ADDRESS,
            0,
            sleep_msg,
            SCL_J1939_MAX_1_FRAME_MSG);

    return;
}

void app_j1939_build_pgn_0000_update(void)
{
    uint8_t message[8] = { 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff };

    // convert rpm limit to 0.125rpm/bit
    uint16_t engine_rpm_scaled = bem_sim_derate_rpm/0.125;

    message[0] = (uint8_t)OVR_CONTROL_BYTE_LIMITING;
    message[1] = (uint8_t)(((engine_rpm_scaled) & 0x00FF) >> 0);
    message[2] = (uint8_t)(((engine_rpm_scaled) & 0xFF00) >> 8);
    message[3] = 0xFA;   // Torque Limit High (out of the way)

    // only send the PGN0 if we need to derate the engine
    if (bem_sim_derate_enable) {

        /* Send message on datalink */
        scl_j1939_que_tx_msg_by_address( Ph_Link_app,  /* Datalink */
                0x000000,            /* CAN Identifier */
                3,                   /* Priority */
                SCL_J1939_GLOBAL_ADDRESS,  /* Destination Address */
                0,
                &message[0],
                sizeof (message) / sizeof (uint8_t) );
    }
}

/* DM13 app callback to verify whether it is safe to stop broadcast */
static boolean_t app_j1939_is_sbcast_allowed_cb(void)
{
    // Return true to indicate it is ok to stop broadcasts.
    return TRUE;
}

/* Receive DM13 suspend signal from other ECMS */
void app_j1939_suspend_cb_1(
    scl_j1939_link_t link,
    unsigned_8 ecm_index,
    unsigned_8 remote_address,
    scl_j1939_suspend_t status,
    unsigned_16 suspend_duration,
    boolean_t hold_signal,
    void *context)
{
    (void)context;

    if (SCL_J1939_SUSPEND_INDEF_ALL == status ||
        SCL_J1939_SUSPEND_INDEF_SOME == status ||
        SCL_J1939_SUSPEND_TEMP_ALL == status ||
        SCL_J1939_SUSPEND_TEMP_SOME == status)
    {
        // Make sure this remote address is in the suspend list.
        int ii;
        for (ii = 0; ii < sizeof(dm13_suspend_list)/sizeof(dm13_suspend_list[0]); ++ii) {
            if (remote_address == dm13_suspend_list[ii]) {
                // Already in the list.
                break;
            }
            else if (DM13_SUSPEND_INVALID_SA == dm13_suspend_list[ii]) {
                // Not already in the list, and we have room for it, add it.
                dm13_suspend_list[ii] = remote_address;
                break;
            }
        }

        /* Suspend CID247/5856*/
        // scl_health_j39_link_pause(app_health_11bit_link_CAN_A);
        // scl_health_j39_link_pause(app_health_11bit_link_CAN_B);
    }
    else if (SCL_J1939_SUSPEND_RESUME == status)
    {
        // Remove this remote address from the suspend list.
        int ii;
        for (ii = 0; ii < sizeof(dm13_suspend_list)/sizeof(dm13_suspend_list[0]); ++ii) {
            // If the source address matches, remove it.
            if (remote_address == dm13_suspend_list[ii]) {
                dm13_suspend_list[ii] = DM13_SUSPEND_INVALID_SA;
            }

            // Shift the list back to fill in the gaps
            if (DM13_SUSPEND_INVALID_SA == dm13_suspend_list[ii]) {
                int next_ii = ii + 1;
                if (next_ii < sizeof(dm13_suspend_list)/sizeof(dm13_suspend_list[0])) {
                    // Move the next item back one to fill in the gap.
                    dm13_suspend_list[ii] = dm13_suspend_list[next_ii];
                    dm13_suspend_list[next_ii] = DM13_SUSPEND_INVALID_SA;
                }
            }
        }

        /* Resume CID247/5856*/
        // scl_health_j39_link_start(app_health_11bit_link_CAN_A);
        // scl_health_j39_link_start(app_health_11bit_link_CAN_A);
    }

    // If any remote address is in the suspend list, then indicate that suspend is active.
    if (DM13_SUSPEND_INVALID_SA != dm13_suspend_list[0]) {
        // We have at least one ECM in the suspend list
        dm13_receive_suspend = TRUE;
    }
    else {
        // The suspend list is empty
        dm13_receive_suspend = FALSE;
    }
}

/*
 *****************************************************************************
 *
 *Name:         app_can_init
 *Description:
 * This function checks if the CAN chip is supported and sets the number of
 * objects needed by the application. It then calls each component init
 * function, passing the appropriate object numbers.
 *
 *****************************************************************************
 */


/*HAL CAN V3 Packet & Packet Reader Heap Limits
//     This operation pre-allocates memory in heap for the number of
//     packet objects and packet reader objects allowed.
//
//     It also limits the memory that can be allocated to this amount.
//
//     If we don't do this, then the HAL CANV3 layer will continue to
//     allocate memory every time a new packet occurs.  This can cause
//     us to use all of the available heap space during times when
//     we would rather the packets just be lost (e.g. initialization).
//
//     The pools of packets are used for transmit as well as receive.
//
//     Each time a packet comes in, a packet reader is associated with
//     that packet for each HAL CANV3 client that exists in the system.
//     For this application, here is a list of the clients:
//       *  SCL J1939 (Port A/B) (Extended Ids Only)
//       *  SCL CCP              (Extended Ids Only)
//       *  Network Module       (Standard Ids Only)
//       *  Bridge (Port A/B)    (Extended Ids Only)
//     We need to provide a packet reader for the worst case.  But
//     there is filtering available in the HALCANV3 port update function, so
//     some packets will never get attached to the client, if the client
//     does not care about it.
//
//     Here is a rough estimate of the limits required:
//                   +--------+--------+--------+--------+
//                   |    Transmit     |    Receive      |
//                   | 29-bit | 11-bit | 29-bit | 11-bit |
//          +--------+--------+--------+--------+--------+
//          | Port A |    6   |    0   |    6   |    0   |
//          | Port B |    1   |    0   |    4   |    1   |
//          +--------+--------+--------+--------+--------+
//     If we assume that all of these messages go out every
//     loop (they don't), and we use an outrageous safety
//     factor (times 2), then we come up with the values below.
//
//     We also need to provide a little margin for transmit messages
//     during initialization.  The address claimed message can get
//     choked out if all of the packets get used up because of the
//     other stuff on the link.  Reserve 20 packets for this (usually 10
//     pre-allocation errors end up occuring).  Note that we don't need
//     Packet Readers for these packets, as they are only used for transmit.
 */


/*SCL J1939 Maximum Number Of Nodes
//     This operation reserves memory for the J1939 Name/Address Table.
//     In order for the address claim process to work properly, there
//     needs to be space reserved in this list so that the worst case
//     number of nodes can be stored.  
//
 */

#define SCL_J1939_MAX_PORTA_NODE_NUMBER  ( 2+18 )
#define SCL_J1939_MAX_PORTB_NODE_NUMBER  ( 2+18 )

volatile hal_canv3_port_t* P_port1;
volatile hal_canv3_port_t* P_port2;


/** Legal For Trade **/
unsigned_16 PidD11F99 = UNKNOWN2U + FMI14;//Payload Legal For Trade Measurement Seal Status
int_16      PidD02B3B = UNKNOWN2S + FMI14;//Payload Zero Adjustment Weight
unsigned_16 PidD023EA = UNKNOWN2U + FMI14;//Payload Correction Factor
unsigned_16 PidD1207A = UNKNOWN2U + FMI14;//Payload Legal For Trade Measurement Sealed Flash Enable Status

/** Totals [130] **/
unsigned_32	PidFC06 = UNKNOWN4U + FMI14; // - Load Count [3]  (RECM)
unsigned_32	PidFCF8 = UNKNOWN4U + FMI14; // - Lifetime Total Payload Weight [1]  (RECM)
unsigned_32	PidFCF7 = UNKNOWN4U + FMI14; // - Total Pass Count [1]  (RECM)
uint32_t PidFE22 = UNKNOWN4U + FMI14; // Total Machine Operation Cycle Count

unsigned_32	PidWrFC06; // - Load Count [3]  (RECM)
unsigned_32	PidWrFCF8; // - Lifetime Total Payload Weight [1]  (RECM)
unsigned_32	PidWrFCF7; // - Total Pass Count [1]  (RECM)

boolean	PidWrFC06Flag; // - Load Count [3]  (RECM)
boolean	PidWrFCF8Flag; // - Lifetime Total Payload Weight [1]  (RECM)
boolean	PidWrFCF7Flag; // - Total Pass Count [1]  (RECM)

int_16 LiftCylinderVelocity_val = 0;
unsigned_16 ParkingBrkStatus_val =  IS_KNOWN2S(FMIDNI);
unsigned_8 ImplLockoutSwitchPos =  IS_KNOWN2S(FMIDNI);

//System Parameters
unsigned_16 PidF42A = UNKNOWN2U + FMI14;//System Voltage
unsigned_8 PidF2C4 = NONE_AVAILABLE;//ECM Location Code
unsigned_8 PidF2C5 = NONE_AVAILABLE;//Desired ECM Location Code

//Production Measurement Loading Status
unsigned_8 PidF2C6 = UNKNOWN1U + FMI14;//Bucket Payload Data
int_16 PidD00C8D = UNKNOWN2S + FMI14;//Bucket Payload
unsigned_16 PidD00D67 = UNKNOWN2U + FMI14;//Loader Bucket Payload Target Weight Percentage
unsigned_32 PidFD39 = UNKNOWN4U + FMI14;//Weigh Cycle Total Truck Payload
unsigned_32 PidFD38 = UNKNOWN4U + FMI14;//Truck Payload Target Weight
int_32 PidFE2E = UNKNOWN4S + FMI14;//Remaining Payload to Load
unsigned_16 Pid00D00BD3 = UNKNOWN2U + FMI14;//Load Cycle Pass Count
unsigned_16 PidD10748 = UNKNOWN2U + FMI14;//Payload Operating Mode
unsigned_16 PidD01788 = UNKNOWN2U + FMI14;//Payload Calculation Method

// ECM Parameters
boolean PidWrF81AFlag = 1;// - Equipment ID (initialize to true to force an update at startup)

//Payload Operating Mode Write
unsigned_16 PidWrD10748; //Payload Operating Mode
boolean PidWrD10748Flag; //Payload Operating Mode

//Production Measurement Weigh Status
unsigned_16 PidD00E9B = UNKNOWN2U + FMI14;//Payload Weigh Status - Insufficient Data

//Production Measurement Sensor Status
unsigned_16 PidF5AE = UNKNOWN2U + FMI14;//Lift Cylinder Position
unsigned_16 PidF47B = UNKNOWN2U + FMI14;//Lift Cylinder Head End Pressure
unsigned_16 PidF5AF = UNKNOWN2U + FMI14;//Lift Cylinder Rod End Pressure
unsigned_16 PidF4E0 = UNKNOWN2U + FMI14;//Tilt Cylinder Head End Pressure
int_16      PidF47A = UNKNOWN2S + FMI14;//Loader Tilt Linkage Angle
unsigned_16 PidF47C = UNKNOWN2U + FMI14;//Tilt Cylinder Rod End Pressure
int_16 Pid0045 = UNKNOWN2S + FMI14;//Hydraulic Oil Temperature
unsigned_16 Pid00D0018B = UNKNOWN2U + FMI14;//Lift Tilt Cylinder Extension (Absolute)
unsigned_16 Pid00D0018C = UNKNOWN2U + FMI14;//Tilt Cylinder Extension (Absolute)
unsigned_16 Pid00D00639 = UNKNOWN2U + FMI14;//Tilt Cylinder Position Sensor Duty Cycle
int_16      Pid00D00A70 = UNKNOWN2S + FMI14;//Loader Bucket Angle

//Production Measurement Switches
unsigned_8 PidF1AA = UNKNOWN1U + FMI14;// Payload Store Switch
//Production Measurement System Status
unsigned_16 PidD106DF = UNKNOWN2U + FMI14;//Loader Payload State
unsigned_16 PidD10972 = UNKNOWN2U + FMI14;//Requested Payload Control System Status
unsigned_16 PidD10933 = UNKNOWN2U + FMI14;//Payload System Zero Requirement Status
unsigned_16 PidD10822 = UNKNOWN2U + FMI14;//Payload System Zeroed Status
unsigned_16 PidD109A5 = UNKNOWN2U + FMI14;//Payload System Calibration Weight Entry Requirement Status
unsigned_16 PidD10FAD = UNKNOWN2U + FMI14;//Payload Remove Last Pass Button Display Status
unsigned_16 PidD10FAC = UNKNOWN2U + FMI14;//Payload Clear Button Display Status

const uint16_t PidD106E7 = 0x000E; // Requested Payload Control System (PCS) Display Status (Active)

//Production Measurement Simple Calibration

 unsigned_16 PidD01686 = UNKNOWN2U + FMI14;// - Truck History List Selected Index Number
 unsigned_32 PidFE25 = UNKNOWN4U + FMI14; // -  Truck History List Truck Payload Weight



// Linkage Sensor Calibrated Limits
unsigned_8 PidF13D = UNKNOWN1U + FMI14;//Lift Linkage Position Sensor Duty Cycle
unsigned_16 PidD010FA = UNKNOWN2U + FMI14;//Lift Linkage Position Sensor Full Raise Duty Cycle
unsigned_16 PidD010FB = UNKNOWN2U + FMI14;//Lift Linkage Position Sensor Full Lower Duty Cycle
unsigned_8 PidF13E = UNKNOWN1U + FMI14;//Tilt Linkage Position Sensor Duty Cycle
unsigned_16 PidD010FC = UNKNOWN2U + FMI14;//Tilt Linkage Position Sensor Full Rackback Duty Cycle
unsigned_16 PidD010FD = UNKNOWN2U + FMI14;//Tilt Linkage Position Sensor Full Dump Duty Cycle

//Configurable Parameters
//Production Measurement Settings
int_16 PidD01B24 = UNKNOWN2S + FMI14;// - Last Payload Weight
unsigned_16 PidD009A7 = UNKNOWN2U + FMI14;// - Payload Control System Calibration Weight
unsigned_16 PidD009F5 = UNKNOWN2U + FMI14;// - Loader Bucket Payload Target Weigh
unsigned_16 PidD112B2 = UNKNOWN2U + FMI14;// - Tip-Off Trigger Type Configuration
unsigned_16 PidD10E4E = UNKNOWN2U + FMI14;;// - Tip-Off Mode [1] (RECM)

//Production Measurement Advanced Settings
unsigned_16 PidD00C9F = UNKNOWN2U + FMI14;// - Payload Weigh Activate Lift Position [1] (RECM)
unsigned_16 PidD0102D = UNKNOWN2U + FMI14;// - Payload Weigh Range Configuration [1] (RECM)
unsigned_16 PidD10ACC = UNKNOWN2U + FMI14;// - Payload Overload Warning Enable [1] (RECM)
unsigned_16 PidD10F96 = UNKNOWN2U + FMI14;// - Payload Weight Calculation Acceptable Audible Alarm Configuration [1] (RECM)
unsigned_16 PidD02923 = UNKNOWN2U + FMI14;// - Desired Audible Annunciation Priority

//Display Settings
unsigned_8 PidF25B = DSI9_1U;//Information Units Status 
unsigned_16 PidD00144 = DSI9_2U;// Service Mode Enable Code 
unsigned_16 PidD0022C = DSI9_2U;//Display Language Status
unsigned_16 PidD106C4 = DSI9_2U;//Store Payload Data Horn Configuration  //TODO: what is the default value?

//Travel Rev
unsigned_16 PidD026AF = UNKNOWN2U + FMI14;// - Travelling Loaded Wheel Revs
unsigned_16 PidD026B1 = UNKNOWN2U + FMI14;// - Payload Cycle <Current> Dig Time

//Cycle seg
unsigned_16 PidD11907 = UNKNOWN2U + FMICNM;// - Payload Cycle Dig State
unsigned_16 PidD0272F = UNKNOWN2U + FMICNM;// - Productivity Measurement Active Segment ID

//Production Measurement Settings
unsigned_16 PidWrD009A7;// - Payload Control System Calibration Weight [1]
unsigned_16 PidWrD009F5;// - Loader Bucket Payload Target Weight [1]
unsigned_16 PidWrD112B2;// - Tip-Off Trigger Type Configuration [1] (RECM)
unsigned_16 PidWrD10E4E;// - Tip-Off Mode [1] (RECM)

//Production Measurement Advanced Settings
unsigned_16 PidWrD00C9F;// - Payload Weigh Activate Lift Position [1] (RECM)
unsigned_16 PidWrD0102D;// - Payload Weigh Range Configuration [1] (RECM)
unsigned_16 PidWrD10ACC;// - Payload Overload Warning Enable [1] (RECM)
unsigned_16 PidWrD10F96;// - Payload Weight Calculation Acceptable Audible Alarm Configuration
unsigned_16 PidWrD106C4;// - Store Payload Data Horn Configuration
unsigned_16 PidWrD11F99;// - Payload Legal For Trade Seal Status
unsigned_16 PidWrD1207A;// - Payload Legal For Trade Measurement Sealed Flash Enable Status

//Production Measurement System Status
unsigned_16 PidWrD10972;// - Requested Payload Control System Status

//Configurable Parameters
boolean PidWrD009A7Flag ;// - Payload Control System Calibration Weight [1]
boolean PidWrD009F5Flag ;// - Loader Bucket Payload Target Weight [1]
boolean PidWrD112B2Flag ;// - Tip-Off Trigger Type Configuration [1] (RECM)
boolean PidWrD10E4EFlag ;// - Tip-Off Mode [1] (RECM)
boolean PidWrD11F99Flag ;// - Payload Legal For Trade Seal Status
boolean PidWrD1207AFlag ;// - Payload Legal For Trade Measurement Sealed Flash Enable Status

//Production Measurement Advanced Settings
boolean PidWrD00C9FFlag ;// - Payload Weigh Activate Lift Position [1] (RECM)
boolean PidWrD0102DFlag ;// - Payload Weigh Range Configuration [1] (RECM)
boolean PidWrD10ACCFlag ;// - Payload Overload Warning Enable [1] (RECM)
boolean PidWrD10F96Flag ;// - Payload Weight Calculation Acceptable Audible Alarm Configuration [1] (RECM)
//Display Settings
boolean PidWrF25BFlag ;// - Information Units Status [1] (RECM)
boolean PidWrD00144Flag ;// - Service Mode Enable Code [1] (RECM)
boolean PidWrD0022CFlag ;// - Display Language Status [1] (RECM)
boolean PidWrD106C4Flag;// - Store Payload Data Horn Configuration

//Production Measurement System Status
boolean PidWrD10972Flag;// - Requested Payload Control System Status

unsigned_8 PidWrF25B;
unsigned_16 PidWrD00144;
unsigned_16 PidWrD0022C;

AppPidU16_t PidD10938 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Display Weight Units Configuration
AppPidU16_t PidD11B8D  = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Display Weight Precision Configuration

AppPidU8_t PidF2CA = { 0xC1, 0xC1, 0 }; // PCS Features Configuration #2

AppPidU16_t PidD11B9A = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Payload Hydraulic Oil Temperature Compensation Enable Status [Payload / PCS / CPM]

// This is hard coded to "Normal" and is not writable.
const unsigned_16 PidD113BD = 0x0009; // Payload System Display Mode

// Production Measurement Printer Settings
AppPidU16_t PidD10E4D = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Installation Status [1] (RECM)
AppPidU16_t PidD00CA0 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Payload Control System Number of Printer Tickets Configuration [1] (RECM)
AppPidU16_t PidD01D2A = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Number of Blank Lines Preceding Output [1] (RECM)
AppPidU16_t PidD01D2B = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Number of Blank Lines Following Output [1] (RECM)
AppPidU16_t PidD01D2C = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Truck Ticket Number [1] (RECM)
AppPidU16_t PidD11132 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Header Enable Status [1] (RECM)
AppPidString_t PidF933 = { {0}, {0}, 0 }; // Onboard Payload Printer Ticket Header Line #1 [1] (RECM)
AppPidString_t PidF934 = { {0}, {0}, 0 }; // Onboard Payload Printer Ticket Header Line #2 [1] (RECM)
AppPidString_t PidF935 = { {0}, {0}, 0 }; // Onboard Payload Printer Ticket Header Line #3 [1] (RECM)
AppPidU16_t PidD11133 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Bucket Weights Enable Status [1] (RECM)
AppPidU16_t PidD11134 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Truck Payload Weight Enable Status [1] (RECM)
AppPidU16_t PidD11135 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Truck ID Enable Status [1] (RECM)
AppPidU16_t PidD11136 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Material ID Enable Status [1] (RECM)
AppPidU16_t PidD11137 = { UNKNOWN2U+FMIDNI, UNKNOWN2U+FMIDNI, 0 }; // Onboard Payload Printer Ticket Operator ID Enable Status [1] (RECM)
AppPidU16_t PidD11130 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Date Format Configuration [1] (RECM)
AppPidU16_t PidD11131 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Date Format Separator Character [1] (RECM)
AppPidU16_t PidD11138 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Ticket Payload Data Store Time Enable Status [1] (RECM)
AppPidU16_t PidD11139 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Material Report Reset Time Enable Status [1] (RECM)
AppPidU16_t PidD1113A = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Onboard Payload Printer Truck Report Reset Time Enable Status [1] (RECM)
AppPidU16_t PidD03093 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Payload Memory Ticket Retention Period

// Dispatch (Work Order Assist) Settings
// - $000C = Enabled
// - $000D = Disabled
AppPidU16_t PidD11878 = { 0x000D, 0x000D, 0 }; // Dispatch Enable Status

// - $0010 = Installed
// - $0011 = Not Installed
AppPidU16_t PidD11879 = { 0x0011, 0x0011, 0 }; // Dispatch Installation Status

// Payload Legal For Trade Measurement Installation Status
// - $0010 = Installed
// - $0011 = Not Installed
unsigned_16 PidD11E8D = UNKNOWN2U+FMICNM; // Payload Legal For Trade Measurement Installation Status

char PidF9C0[PID_F9C0_STRING_BUF_SIZE] = { 0 }; // Payload Legal For Trade Measurement Software Identifier

// Linkage Sensor Configurations
// - $01C3 = Rotary Position Sensor
// - $01C4 = In-Cylinder Position Sensor
AppPidU16_t PidD10045 = { 0x1C3, 0x1C3, 0 }; // Loader Lift Linkage Position Sensor Configuration
AppPidU16_t PidD10046 = { 0x1C3, 0x1C3, 0 }; // Loader Tilt Linkage Position Sensor Configuration

// IMU
// Rear (EEF - Engine End Frame)
int16_t PidD019A1 = UNKNOWN2S + FMICNM; // $D019A1 Machine Rear Lateral Acceleration
int16_t PidD019A3 = UNKNOWN2S + FMICNM; // $D019A3 Machine Rear Longitudinal Acceleration
int16_t PidD019A5 = UNKNOWN2S + FMICNM; // $D019A5 Machine Rear Vertical Acceleration

// Rear (EEF - Engine End Frame)
int16_t PidF58B = UNKNOWN2S + FMICNM; // $F58B Machine Pitch
int16_t PidF533 = UNKNOWN2S + FMICNM; // $F533 Machine Slope

// Rear (EEF - Engine End Frame)
int16_t PidD01AFF = UNKNOWN2S + FMICNM; // $D01AFF Machine Rear Roll
int16_t PidD01FC9 = UNKNOWN2S + FMICNM; // $D01FC9 Machine Rear Side Slope

// Ambiguously defined, populating as Rear (EEF - Engine End Frame)
int16_t PidD00A64 = UNKNOWN2S + FMICNM; // $D00A64 Machine Roll
int16_t PidD00409 = UNKNOWN2S + FMICNM; // $D00409 Machine Side Slope

// Display Screen Code
uint16_t PidD01396 = UNKNOWN2U + FMICNM; // Display Module Screen Selection Code

// Payload Manual Add Configuration
AppPidU16_t PidD11919 = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Payload Manual Add Configuration

// Payload Auto Store Pass Count Threshold
AppPidU16_t PidD0273B = { UNKNOWN2U+FMICNM, UNKNOWN2U+FMICNM, 0 }; // Payload Auto Store Pass Count Threshold

// Payload Memory Remaining Status
uint16_t PidD001A4 = UNKNOWN2U + FMICNM; // $D001A4 Payload Memory Remaining Status

/* ECM 01 is transmitting PGB groups.  ECM 02 is receiving PGB groups.  */
 static const scl_pgb_j1939_ecm_config_t app_scl_pgb_j1939_ecm_config_01 =
 {
    0,       /* ecm_idx */
    20,       /* num rx connections */
    40,       /* num tx connections */
    20,       /* num rx_groups */
    80,      /* num tx_groups */
    80,       /* num bcasts */
 };

  scl_pgb_j1939_t *app_scl_pgb_j1939_1, *app_scl_pgb_j1939_2;

const app_scl_pgb_cdl_pie_t app_scl_pgb_cdl_pie[] =
{
    { sizeof(PidF42A), (unsigned_32*)&PidF42A, 0xF42A, &cdl_fr_tx_hdlr },
    { sizeof(PidF2C4), (unsigned_32*)&PidF2C4, 0xF2C4, &cdl_fr_tx_hdlr },
    { sizeof(PidF2C5), (unsigned_32*)&PidF2C5, 0xF2C5, &cdl_fr_tx_hdlr },
    { sizeof(PidF2C6), (unsigned_32*)&PidF2C6, 0xF2C6, &cdl_fr_tx_hdlr },
    { sizeof(PidD00C8D), (unsigned_32*)&PidD00C8D, 0x00D00C8D, &cdl_fr_tx_hdlr },
    { sizeof(PidD00D67), (unsigned_32*)&PidD00D67, 0xD00D67, &cdl_fr_tx_hdlr },
    { sizeof(PidFD39 ), (unsigned_32*)&PidFD39 , 0xFD39, &cdl_fr_tx_hdlr },
    { sizeof(PidFD38), (unsigned_32*)&PidFD38, 0xFD38, &cdl_fr_tx_hdlr },
    { sizeof(PidFE2E), (unsigned_32*)&PidFE2E, 0xFE2E, &cdl_fr_tx_hdlr },
    { sizeof(Pid00D00BD3), (unsigned_32*)&Pid00D00BD3, 0x00D00BD3, &cdl_fr_tx_hdlr },
    { sizeof(PidD10748), (unsigned_32*)&PidD10748, 0xD10748, &cdl_f_tx_hdlr },
    { sizeof(PidD01788), (unsigned_32*)&PidD01788, 0xD01788, &cdl_fr_tx_hdlr },
    { sizeof(PidD00E9B), (unsigned_32*)&PidD00E9B, 0xD00E9B, &cdl_f_tx_hdlr },
    { sizeof(PidFC06), &PidFC06, 0xFC06, &cdl_fr_tx_hdlr },
    { sizeof(PidFCF8), &PidFCF8, 0xFCF8, &cdl_fr_tx_hdlr },
    { sizeof(PidFCF7), &PidFCF7, 0xFCF7, &cdl_fr_tx_hdlr },
    { sizeof(PidFE22), &PidFE22, 0xFE22, &cdl_fr_tx_hdlr },
    { sizeof(PidD00C9F), (unsigned_32*)&PidD00C9F, 0x00D00C9F, &cdl_fr_tx_hdlr_weigh_app },
	{ sizeof(PidD0102D), (unsigned_32*)&PidD0102D, 0xD0102D, &cdl_fr_tx_hdlr_weigh_app },
    { 2, (unsigned_32*)&LiftCylinderVelocity_val, 0xD01961, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&ParkingBrkStatus_val, 0xF602, &cdl_fr_tx_hdlr },
    { 1, (unsigned_32*)&ImplLockoutSwitchPos, 0xF0B8, &cdl_fr_tx_hdlr },
    { sizeof(PidF13D), (unsigned_32*)&PidF13D, 0xF13D, &cdl_fr_tx_hdlr },
    { sizeof(PidF5AE), (unsigned_32*)&PidF5AE, 0xF5AE, &cdl_fr_tx_hdlr },
    { sizeof(PidF47B), (unsigned_32*)&PidF47B, 0xF47B, &cdl_fr_tx_hdlr },
    { sizeof(PidF5AF), (unsigned_32*)&PidF5AF, 0xF5AF, &cdl_fr_tx_hdlr },
    { sizeof(PidF4E0), (unsigned_32*)&PidF4E0, 0xF4E0, &cdl_fr_tx_hdlr },
    { sizeof(PidF47A), (unsigned_32*)&PidF47A, 0xF47A, &cdl_fr_tx_hdlr },
    { sizeof(PidF47C), (unsigned_32*)&PidF47C, 0xF47C, &cdl_fr_tx_hdlr },
    { sizeof(PidF13E), (unsigned_32*)&PidF13E, 0xF13E, &cdl_fr_tx_hdlr },
    { sizeof(Pid0045), (unsigned_32*)&Pid0045, 0x0045, &cdl_fr_tx_hdlr },
    { sizeof(Pid00D0018B), (unsigned_32*)&Pid00D0018B, 0x00D0018B, &cdl_fr_tx_hdlr },
    { sizeof(Pid00D0018C), (unsigned_32*)&Pid00D0018C, 0x00D0018C, &cdl_fr_tx_hdlr },
    { sizeof(Pid00D00639), (unsigned_32*)&Pid00D00639, 0x00D00639, &cdl_fr_tx_hdlr },
    { sizeof(Pid00D00A70), (unsigned_32*)&Pid00D00A70, 0x00D00A70, &cdl_fr_tx_hdlr },
    { sizeof(PidF1AA), (unsigned_32*)&PidF1AA, 0xF1AA, &cdl_fr_tx_hdlr },
    { sizeof(PidD106DF), (unsigned_32*)&PidD106DF, 0x00D106DF, &cdl_f_tx_hdlr },
    { sizeof(PidD10972), (unsigned_32*)&PidD10972, 0x00D10972, &cdl_f_tx_hdlr_jobmgr_app },
    { sizeof(PidD10933), (unsigned_32*)&PidD10933, 0x00D10933, &cdl_f_tx_hdlr },
    { sizeof(PidD10822), (unsigned_32*)&PidD10822, 0x00D10822, &cdl_f_tx_hdlr },
    { sizeof(PidD109A5), (unsigned_32*)&PidD109A5, 0x00D109A5, &cdl_f_tx_hdlr },
    { sizeof(PidD10FAD), (unsigned_32*)&PidD10FAD, 0x00D10FAD, &cdl_f_tx_hdlr },
    { sizeof(PidD10FAC), (unsigned_32*)&PidD10FAC, 0x00D10FAC, &cdl_f_tx_hdlr },
    { sizeof(PidD01686), (unsigned_32*)&PidD01686, 0x00D01686, &cdl_fr_tx_hdlr },
    { sizeof(PidFE25), (unsigned_32*)&PidFE25, 0x00FE25, &cdl_fr_tx_hdlr },
    { sizeof(PidD010FA), (unsigned_32*)&PidD010FA, 0x00D010FA, &cdl_fr_tx_hdlr },
    { sizeof(PidD010FB), (unsigned_32*)&PidD010FB, 0x00D010FB, &cdl_fr_tx_hdlr },
    { sizeof(PidD010FC), (unsigned_32*)&PidD010FC, 0x00D010FC, &cdl_fr_tx_hdlr },
    { sizeof(PidD010FD), (unsigned_32*)&PidD010FD, 0x00D010FD, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD106E7, 0xD106E7, &cdl_f_tx_hdlr },
	/* Write Config Data */
    { sizeof(PidD01B24), (unsigned_32*)&PidD01B24, 0x00D01B24, &cdl_fr_tx_hdlr_weigh_app },
    { sizeof(PidD009A7), (unsigned_32*)&PidD009A7, 0x00D009A7, &cdl_fr_tx_hdlr_weigh_app },
    { sizeof(PidD009F5), (unsigned_32*)&PidD009F5, 0x00D009F5, &cdl_fr_tx_hdlr_weigh_app },
    { sizeof(PidD112B2), (unsigned_32*)&PidD112B2, 0x00D112B2, &cdl_f_tx_hdlr_jobmgr_app },
    { sizeof(PidD10E4E), (unsigned_32*)&PidD10E4E, 0x00D10E4E, &cdl_f_tx_hdlr_jobmgr_app },
    { sizeof(PidD10ACC), (unsigned_32*)&PidD10ACC, 0x00D10ACC, &cdl_f_tx_hdlr_weigh_app },
    { sizeof(PidD10F96), (unsigned_32*)&PidD10F96, 0x00D10F96, &cdl_f_tx_hdlr_weigh_app },
    { sizeof(PidF25B), (unsigned_32*)&PidF25B, 0x00F25B, &cdl_fr_tx_hdlr },
    { sizeof(PidD02923), (unsigned_32*)&PidD02923, 0x00D02923, &cdl_fr_tx_hdlr },
    { sizeof(PidD00144), (unsigned_32*)&PidD00144, 0x00D00144, &cdl_f_tx_hdlr },
    { sizeof(PidD0022C), (unsigned_32*)&PidD0022C, 0x00D0022C, &cdl_f_tx_hdlr },
    { sizeof(PidD106C4), (unsigned_32*)&PidD106C4, 0x00D106C4, &cdl_f_tx_hdlr_jobmgr_app },
    { 2, (unsigned_32*)&PidD11919.tx, 0x00D11919, &cdl_f_tx_hdlr_jobmgr_app },
    { 2, (unsigned_32*)&PidD0273B.tx, 0x00D0273B, &cdl_fr_tx_hdlr_jobmgr_app },
    { sizeof(PidD026AF), (unsigned_32*)&PidD026AF, 0x00D026AF, &cdl_fr_tx_hdlr },
    { sizeof(PidD026B1), (unsigned_32*)&PidD026B1, 0x00D026B1, &cdl_fr_tx_hdlr },
    { sizeof(PidD11907), (unsigned_32*)&PidD11907, 0x00D11907, &cdl_f_tx_hdlr },
    { sizeof(PidD0272F), (unsigned_32*)&PidD0272F, 0x00D0272F, &cdl_f_tx_hdlr },
    { 1, (unsigned_32*)&PidF2CA.tx, 0xF2CA, &cdl_f_tx_hdlr_jobmgr_app },
    { 2, (unsigned_32*)&PidD113BD, 0xD113BD, &cdl_f_tx_hdlr },
	{ 2, (unsigned_32*)&PidD10E4D.tx, 0xD10E4D, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD00CA0.tx, 0xD00CA0, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD01D2A.tx, 0xD01D2A, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD01D2B.tx, 0xD01D2B, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD01D2C.tx, 0xD01D2C, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD11132.tx, 0xD11132, &cdl_f_tx_hdlr },
    { 25, (unsigned_32*)&(PidF933.tx[0]), 0xF933, &cdl_str_tx_hdlr_24 },
    { 25, (unsigned_32*)&(PidF934.tx[0]), 0xF934, &cdl_str_tx_hdlr_24 },
    { 25, (unsigned_32*)&(PidF935.tx[0]), 0xF935, &cdl_str_tx_hdlr_24 },
    { 2, (unsigned_32*)&PidD11133.tx, 0xD11133, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11134.tx, 0xD11134, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11135.tx, 0xD11135, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11136.tx, 0xD11136, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11137.tx, 0xD11137, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11130.tx, 0xD11130, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11131.tx, 0xD11131, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11138.tx, 0xD11138, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11139.tx, 0xD11139, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD1113A.tx, 0xD1113A, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11878.tx, 0xD11878, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11879.tx, 0xD11879, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11E8D, 0xD11E8D, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD10045.tx, 0xD10045, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD10046.tx, 0xD10046, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD019A1, 0xD019A1, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD019A3, 0xD019A3, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD019A5, 0xD019A5, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidF58B, 0xF58B, &cdl_fr_tx_hdlr }, // $F58B Machine Pitch
    { 2, (unsigned_32*)&PidF533, 0xF533, &cdl_fr_tx_hdlr }, // $F533 Machine Slope
    { 2, (unsigned_32*)&PidD01AFF, 0xD01AFF, &cdl_fr_tx_hdlr }, // $D01AFF Machine Rear Roll
    { 2, (unsigned_32*)&PidD01FC9, 0xD01FC9, &cdl_fr_tx_hdlr }, // $D01FC9 Machine Rear Side Slope
    { 2, (unsigned_32*)&PidD00A64, 0xD00A64, &cdl_fr_tx_hdlr }, // $D00A64 Machine Roll
    { 2, (unsigned_32*)&PidD00409, 0xD00409, &cdl_fr_tx_hdlr }, // $D00409 Machine Side Slope
    { 2, (unsigned_32*)&PidD01396, 0xD01396, &cdl_fr_tx_hdlr },
    { 2, (unsigned_32*)&PidD10938.tx, 0xD10938, cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11B8D.tx, 0xD11B8D, cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD11B9A.tx, 0xD11B9A, &cdl_f_tx_hdlr },
    { 2, (unsigned_32*)&PidD001A4, 0xD001A4, &cdl_fr_tx_hdlr }, // $D001A4 Payload Memory Remaining Status
    { 27, (unsigned_32*)&(PidF9C0[0]), 0xF9C0, &cdl_str_tx_hdlr_26 },
    { 2, (unsigned_32*)&PidD11F99, 0xD11F99, &cdl_f_tx_hdlr_weigh_app }, // Payload Legal For Trade Measurement Seal Status - MSB
    { 2, (unsigned_32*)&PidD1207A, 0xD1207A, &cdl_f_tx_hdlr_weigh_app }, // Payload Legal For Trade Measurement Sealed Flash Enable Status - MSB
    { 2, (unsigned_32*)&PidD023EA, 0xD023EA, &cdl_fr_tx_hdlr }, // Payload Correction Factor
    { 2, (unsigned_32*)&PidD02B3B, 0xD02B3B, &cdl_fr_tx_hdlr }, // Payload Zero Adjustment Weight - LSB
    { 2, (unsigned_32*)&PidD03093.tx, 0xD03093, &cdl_fr_tx_hdlr }, // Payload Memory Ticket Retention Period - LSB
};

/* Write PID Config Array */
const app_scl_cdl_wr_pie_t app_scl_cdl_wr_pie[] =
{
    { sizeof(PidWrD00C9F), (unsigned_8*)&PidWrD00C9F, 0xD00C9F, CdlWrPidD00C9FWrHdlr },
	{ sizeof(PidWrD0102D), (unsigned_8*)&PidWrD0102D, 0xD0102D, CdlWrPidD0102DWrHdlr },
    { sizeof(PidWrFC06), (unsigned_8*)&PidWrFC06, 0xFC06, CdlWrPidWrFC06WrHdlr },
    { sizeof(PidWrFCF8), (unsigned_8*)&PidWrFCF8, 0xFCF8, CdlWrPidWrFCF8WrHdlr },
    { sizeof(PidWrFCF7), (unsigned_8*)&PidWrFCF7, 0xFCF7, CdlWrPidWrFCF7WrHdlr },
    { sizeof(PidWrD009A7), (unsigned_8*)&PidWrD009A7, 0xD009A7, CdlWrPidWrD009A7WrHdlr },
    { sizeof(PidWrD009F5), (unsigned_8*)&PidWrD009F5, 0xD009F5, CdlWrPidWrD009F5WrHdlr },
    { sizeof(PidWrD112B2), (unsigned_8*)&PidWrD112B2, 0xD112B2, CdlWrPidWrD112B2WrHdlr },
    { sizeof(PidWrD10E4E), (unsigned_8*)&PidWrD10E4E, 0xD10E4E, CdlWrPidWrD10E4EWrHdlr },
    { sizeof(PidWrD10ACC), (unsigned_8*)&PidWrD10ACC, 0xD10ACC, CdlWrPidWrD10ACCWrHdlr },
    { sizeof(PidWrD10F96), (unsigned_8*)&PidWrD10F96, 0xD10F96, CdlWrPidWrD10F96WrHdlr },
    { sizeof(PidWrF25B), (unsigned_8*)&PidWrF25B, 0xF25B, CdlWrPidWrF25BWrHdlr },
    { sizeof(PidWrD00144), (unsigned_8*)&PidWrD00144, 0xD00144, CdlWrPidWrD00144WrHdlr },
    { sizeof(PidWrD0022C), (unsigned_8*)&PidWrD0022C, 0xD0022C, CdlWrPidWrD0022CWrHdlr },
    { sizeof(PidWrD106C4), (unsigned_8*)&PidWrD106C4, 0xD106C4, CdlWrPidWrD106C4WrHdlr },
    { sizeof(PidWrD10748), (unsigned_8*)&PidWrD10748, 0xD10748, CdlWrPidWrD10748WrHdlr },
    { 2, (unsigned_8*)&PidD11919, 0xD11919, &CdlWrPidWrD11919WrHdlr },
    { 2, (unsigned_8*)&PidD0273B, 0xD0273B, &CdlWrPidWrD0273BWrHdlr },
    { sizeof(PidWrD10972), (unsigned_8*)&PidWrD10972, 0xD10972, CdlWrPidWrD10972WrHdlr },
    { 1, (unsigned_8*)&PidF2CA, 0xF2CA, &CdlPidF2CAWrHdlr },
    { 2, (unsigned_8*)&PidD10E4D, 0xD10E4D, &CdlPidPrinterInstallWrHdlr },
    { 2, (unsigned_8*)&PidD00CA0, 0xD00CA0, &CdlPidPrinterNumTicketsWrHdlr },
    { 2, (unsigned_8*)&PidD01D2A, 0xD01D2A, &CdlPidPrinterBlankLinesWrHdlr },
    { 2, (unsigned_8*)&PidD01D2B, 0xD01D2B, &CdlPidPrinterBlankLinesWrHdlr },
    { 2, (unsigned_8*)&PidD01D2C, 0xD01D2C, &CdlPidPrinterTicketNumberWrHdlr },
    { 2, (unsigned_8*)&PidD11132, 0xD11132, &CdlPidPrinterEnableStatusWrHdlr },
    { 25, (unsigned_8*)&PidF933, 0xF933, &CdlPidPrinterHeaderLineWrHdlr },
    { 25, (unsigned_8*)&PidF934, 0xF934, &CdlPidPrinterHeaderLineWrHdlr },
    { 25, (unsigned_8*)&PidF935, 0xF935, &CdlPidPrinterHeaderLineWrHdlr },
    { 2, (unsigned_8*)&PidD11133, 0xD11133, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD11134, 0xD11134, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD11135, 0xD11135, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD11136, 0xD11136, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD11130, 0xD11130, &CdlPidPrinterDateFormatWrHdlr },
    { 2, (unsigned_8*)&PidD11131, 0xD11131, &CdlPidPrinterDateSeparatorWrHdlr },
    { 2, (unsigned_8*)&PidD11138, 0xD11138, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD11139, 0xD11139, &CdlPidPrinterEnableStatusWrHdlr },
    { 2, (unsigned_8*)&PidD1113A, 0xD1113A, &CdlPidPrinterEnableStatusWrHdlr },
    { 9, (unsigned_8*)pidF82D_pdata, 0xF82D, &app_j1939_f82d_wr_hndl},
    { 18, (unsigned_8*)pidF81A_pdata, 0xF81A, &app_j1939_f81a_wr_hndl},
    { 2, (unsigned_8*)&PidD10938, 0xD10938, &CdlWrPidWrD10938WrHdlr },
    { 2, (unsigned_8*)&PidD11B8D, 0xD11B8D, &CdlWrPidWrD11B8DWrHdlr },
    { 2, (unsigned_8*)&PidD11B9A, 0xD11B9A, &CdlWrPidWrD11B9AWrHdlr },
    { 2, (unsigned_8*)&PidD11878, 0xD11878, &CdlWrPidWrD11878WrHdlr },
    { 2, (unsigned_8*)&PidD11F99, 0xD11F99, &CdlWrPidWrD11F99WrHdlr },
    { 2, (unsigned_8*)&PidD1207A, 0xD1207A, &CdlWrPidWrD1207AWrHdlr },
    { 2, (unsigned_8*)&PidD03093, 0xD03093, &CdlPidTicketRetentionWrHdlr },
};

/*
 * Waits for new data from the Weigh App if is was flushed
 *  Waits a maximum of 1 second.
 */
inline static void wait_for_weigh_app_response() {
    // Wait until a new value has been received since the last flush
    static const int timeout_us = 1000000; /* 1 sec */
    int counter_us = 0;
    while (((0 == NewLpsSaWeighScsTxIn_) || (0 != PendingLpsSaWeighScsReqOut_)) && counter_us < timeout_us ) {
        usleep(1000);
        counter_us+=1000;
    }
}

/*
 * This tx handler waits for a new Weigh App Tx input first before getting the data
 */
static int_16 cdl_fr_tx_hdlr_weigh_app(void* tx_pie, unsigned_8* data) {
    wait_for_weigh_app_response();
    return cdl_fr_tx_hdlr(tx_pie, data);
}

/*
 * This tx handler waits for a new Weigh App Tx input first before getting the data
 */
static int_16 cdl_f_tx_hdlr_weigh_app(void* tx_pie, unsigned_8* data) {
    wait_for_weigh_app_response();
    return cdl_f_tx_hdlr(tx_pie, data);
}

/*
 * Waits for new data from the JobMgr App if is was flushed
 *  Waits a maximum of 1 second.
 */
inline static void wait_for_jobmgr_app_response() {
    // Wait until a new value has been received since the last flush
    static const int timeout_us = 1000000; /* 1 sec */
    int counter_us = 0;
    while ((( NewLpsSaJobMgrScsTxIn_ < 3) || (0 != PendingLpsSaJobMgrScsReqOut_)) && counter_us < timeout_us ) {
        usleep(1000);
        counter_us+=1000;
    }
}

/*
 * This tx handler waits for a new JobMgr App Tx input first before getting the data
 */
static int_16 cdl_fr_tx_hdlr_jobmgr_app(void* tx_pie, unsigned_8* data) {
    wait_for_jobmgr_app_response();
    return cdl_fr_tx_hdlr(tx_pie, data);
}

/*
 * This tx handler waits for a new JobMgr App Tx input first before getting the data
 */
static int_16 cdl_f_tx_hdlr_jobmgr_app(void* tx_pie, unsigned_8* data) {
    wait_for_jobmgr_app_response();
    return cdl_f_tx_hdlr(tx_pie, data);
}

/*
 * tx handler for fixed length reversed (LSB first)
 */
static int_16 cdl_fr_tx_hdlr(void *tx_pie, unsigned_8 *data)
{
    app_scl_pgb_cdl_pie_t *temp_tx_pie = tx_pie;

    switch (temp_tx_pie->len) {
    case 1: {
        OEL_PACK_LE_8_NO_INCR(data, (unsigned_8)(*(temp_tx_pie->data)));
        break;
    }
    case 2: {
        OEL_PACK_LE_16_NO_INCR(data, (unsigned_16)(*(temp_tx_pie->data)));
        break;
    }
    case 4: {
        OEL_PACK_LE_32_NO_INCR(data, *(temp_tx_pie->data));
        break;
    }
    default: {
        break;
    }
    }

    return temp_tx_pie->len;
}

/*
 * tx handler for fixed length (MSB first)
 */
static int_16 cdl_f_tx_hdlr(void* tx_pie, unsigned_8* data) {
    app_scl_pgb_cdl_pie_t *temp_tx_pie = tx_pie;

    switch (temp_tx_pie->len) {
    case 1: {
        OEL_PACK_BE_8_NO_INCR(data, (unsigned_8)(*(temp_tx_pie->data)));
        break;
    }
    case 2: {
        OEL_PACK_BE_16_NO_INCR(data, (unsigned_16)(*(temp_tx_pie->data)));
        break;
    }
    case 4: {
        OEL_PACK_BE_32_NO_INCR(data, *(temp_tx_pie->data));
        break;
    }
    default: {
        break;
    }
    }

    return temp_tx_pie->len;
}

// Generic string tx handler
// Generic string tx handler
static int_16 cdl_str_tx_hdlr(void* tx_pie, unsigned_8* data, int max_str_len) {
    const app_scl_pgb_cdl_pie_t* pie = tx_pie;
    const uint8_t* src = (const uint8_t*)pie->data;
    uint8_t* dest = data;
    int_16 len = 0;

    // Maximum possible CDL string length is 26 bytes, leaving 1 byte for the length for a total of 27 bytes.
    if (max_str_len > 26) {
        max_str_len = 26;
    }

    while (len < max_str_len) {
        if (*src != '\0') {
            *dest = *src;
            ++dest;
            ++src;
            ++len;
        }
        else {
            break;
        }
    }

    /*
     * scl_j1939_process_ma_param_request doesn't support 0 length
     * multi-byte parameters due to a bug.  The workaround is to
     * add a null character as a length 1 string.
     */
    if (0 == len) {
        *dest = '\0';
        len = 1;
    }

    return len;
}

/*
 * tx handler for multi-byte string data
 */
static int_16 cdl_str_tx_hdlr_24(void* tx_pie, unsigned_8* data) {
    return cdl_str_tx_hdlr(tx_pie, data, 24);
}

/*
 * tx handler for multi-byte string data
 */
static int_16 cdl_str_tx_hdlr_26(void* tx_pie, unsigned_8* data) {
    return cdl_str_tx_hdlr(tx_pie, data, 26);
}

/******************************************************************************
FUNCTION app_scl_pgb_cdl_server_init
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
/* Add all the pids to tx pit */
void app_scl_pgb_cdl_server_init(void)
{
   unsigned_8 idx;
   unsigned_8 pid_num = sizeof(app_scl_pgb_cdl_pie)/sizeof(app_scl_pgb_cdl_pie[0]);

   for(idx = 0; idx < pid_num ; idx++)
   {
      cdl2_add_to_tx_pit(app_scl_pgb_cdl_pie[idx].pid,
                                         app_scl_pgb_cdl_pie[idx].tx_hdlr,
                         &app_scl_pgb_cdl_pie[idx]);
   }


   /* Configuring  Write PID's and handles */
   pid_num = sizeof(app_scl_cdl_wr_pie)/sizeof(app_scl_cdl_wr_pie[0]);

   for(idx = 0; idx < pid_num ; idx++)
   {
           cdl2_add_to_wr_pit(app_scl_cdl_wr_pie[idx].pid,
                                                  app_scl_cdl_wr_pie[idx].tx_hdlr,
                              &app_scl_cdl_wr_pie[idx]
                             );
   }
}/*------------------------------------------------------------------------------
Name:       app_scl_pgb_j1939_server_init()

Arguments:  scl_j1939_link_t link: This parameter contains the link that will
               use scl_pgb_j1939.

Return:     void

Description
This operation will initialize a server for scl_pgb_j1939 for the application
to be able to broadcast PIDs.
*/
void app_scl_pgb_j1939_server_init(scl_j1939_link_t link)
{
   app_scl_pgb_j1939_1 = scl_pgb_j1939_init(link);

   oel_assert( app_scl_pgb_j1939_1 != NULL );

   {
      if ( scl_pgb_j1939_add_ecm
           (
              app_scl_pgb_j1939_1,
              &app_scl_pgb_j1939_ecm_config_01
           ) == FALSE )
      {
         oel_assert(0);
         return;
      }
      else
      {
    	  /* add pids to tx pit */
         app_scl_pgb_cdl_server_init();

      }
   }

 }

/******************************************************************************
FUNCTION app_scl_adt_cdl_server_init
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
/* Add all the pids to tx pit */
void app_scl_adt_cdl_server_init(void)
{
   unsigned_8 idx;
   unsigned_8 pid_num = sizeof(app_scl_pgb_cdl_pie)/sizeof(app_scl_pgb_cdl_pie[0]);

   for(idx = 0; idx < pid_num ; idx++)
   {
      cdl2_add_to_tx_pit(app_scl_pgb_cdl_pie[idx].pid,
                         app_scl_pgb_cdl_pie[idx].tx_hdlr,
                         &app_scl_pgb_cdl_pie[idx]);
   }


   /* Configuring  Write PID's and handles */
   pid_num = sizeof(app_scl_cdl_wr_pie)/sizeof(app_scl_cdl_wr_pie[0]);

   for(idx = 0; idx < pid_num ; idx++)
   {
       cdl2_add_to_wr_pit(app_scl_cdl_wr_pie[idx].pid,
                          app_scl_cdl_wr_pie[idx].tx_hdlr,
                          &app_scl_cdl_wr_pie[idx]
                         );
   }

}

/*------------------------------------------------------------------------------
Name:       app_scl_adt_j1939_server_init()

Arguments:  scl_j1939_link_t link: This parameter contains the link that will
               use scl_pgb_j1939.

Return:     void

Description
This operation will initialize a server for scl_pgb_j1939 for the application
to be able to broadcast PIDs.
*/
void app_scl_adt_j1939_server_init(scl_j1939_link_t link)
{

   adt_j1939_link_handle = scl_adt_j1939_link_init(link);

   oel_assert( 1 );

   {
      if ( scl_adt_j1939_link_add_ecm(adt_j1939_link_handle,0   ) == FALSE )
      {
         printf("adt data");
         oel_assert(0);
         return;
      }
      else
      {
         /* add pids to tx pit */
          app_scl_adt_cdl_server_init();

      }
   }

 }

void app_can_init()
{
    J1939Table_t *table_ptr;
    unsigned_8 i;
    bool_t eddt_error = FALSE;
    
    /* Try to find a match in the array */
    table_ptr = application_search(0,
            j1939_directory,
            j1939_max_applications
    );

    if ( !table_ptr )
    {
        /* Match was not found, return */
        return;
    }

    P_port1 = HAL_CANV3_PORT_01;
    P_port2 = HAL_CANV3_PORT_02;

    hal_canv3_port_set_resource(P_port1, app_hal_canv3_resource_config);
    hal_canv3_port_set_resource(P_port2, app_hal_canv3_resource_config);

    /*This operation pre-allocates memory in heap for the number of
   //packet objects specified.
   //
   //It also limits the memory that can be allocated to this amount.
   //
   //If we don't do this, then the HAL CANV3 layer will continue to
   //allocate memory every time a new packet occurs.  This can cause
   //us to use all of the available heap space during times when
   //we would rather the packets just be lost (e.g. initialization).
   //
   //This pool of packets are used for transmit as well as receive.
     */

    hal_canv3_pckt_prealloc_objects(P_port1, 1000);
    hal_canv3_pckt_prealloc_objects(P_port2, 1000);

    /*This operation pre-allocates memory in heap for the number of
   //packet reader objects specified.
   //
   //It also limits the memory that can be allocated to this amount.
   //
   //If we don't do this, then the HAL CANV3 layer will continue to
   //allocate memory every time a new packet occurs.  This can cause
   //us to use all of the available heap space during times when
   //we would rather the packets just be lost (e.g. initialization).
   //
   //Each time a packet comes in, a packet reader is associated with
   //that packet for each HAL CANV3 client that exists in the system.
     */

    hal_canv3_preader_prealloc_objects(P_port1, 1000);
    hal_canv3_preader_prealloc_objects(P_port2, 1000);

    scl_j1939_set_num_links(2); // Set to maximum available ports

    Ph_Link_app =
            scl_j1939_establish_link
            (
                    HAL_CANV3_PORT_01,                 /* hal_j1939v2 port */
                    &(table_ptr->can_a_address_table[0]),

                    /*Maximum Number Of Virtual ECMs*/
                    1,

                    100,                                /* number of mbufs */
                    40,                                /* number of extentions */
                    15,                                 /* number of tpcbs */

                    /*Maximum Number Of Nodes On The Network [Including Virtual ECMs]*/
                    (unsigned_8)(1 + SCL_J1939_MAX_PORTA_NODE_NUMBER),

                    /* addr status callback */
                    (scl_j1939_ac_status_cb_t *)(app_scl_j1939_ac_status_callback),
                    &app_scl_j1939_config              /* app config for J1939 */
            );

    TES_Link_app =
            scl_j1939_establish_link
            (
                    HAL_CANV3_PORT_02,                 /* hal_j1939v2 port */
                    &(table_ptr->can_b_address_table[0]),

                    /*Maximum Number Of Virtual ECMs*/
                    1,

                    100,                                /* number of mbufs */
                    40,                                /* number of extentions */
                    15,                                 /* number of tpcbs */

                    /*Maximum Number Of Nodes On The Network [Including Virtual ECMs]*/
                    (unsigned_8)(1 + SCL_J1939_MAX_PORTA_NODE_NUMBER),

                    /* addr status callback */
                    (scl_j1939_ac_status_cb_t *)(app_scl_j1939_ac_status_callback),
                    &app_scl_j1939_config              /* app config for J1939 */
            );

    /*Proprietary messaging support */
    scl_j1939_set_ppgn_enable_table(Ph_Link_app,                 /* scl_j1939_link_t Ph_link */
            table_ptr->can_a_ppgn_enable_num,       /* Pi8_ppgn_enable_size */
            table_ptr->can_a_ppgn_enable);          /* scl_j1939_prop_enable_t* Ppast_ppgn_enable_tbl */

    scl_j1939_set_ppgn_enable_table(TES_Link_app,                 /* scl_j1939_link_t Ph_link */
            table_ptr->can_b_ppgn_enable_num,       /* Pi8_ppgn_enable_size */
            table_ptr->can_b_ppgn_enable);          /* scl_j1939_prop_enable_t* Ppast_ppgn_enable_tbl */

    scl_j1939_set_enable_global_ppgna(Ph_Link_app, TRUE );
    scl_j1939_set_enable_global_ppgna(TES_Link_app, TRUE );

    /*Register Proprietary PGN Command With Extended Ids for CAN A*/
    for (i = 0; i < table_ptr->can_a_ppgn_ext_msg_num; i++)
    {
        scl_j1939_cat_ppgn_id_install_command_id
        (
                /* Identify J1939 link to application */
                Ph_Link_app,

                /* Pointer to a Proprietary PGN Command entry */
                &(table_ptr->can_a_ppgn_ext_msg[i]));
    }

    /*Register Proprietary PGN Command With Extended Ids for CAN B*/
    for (i = 0; i < table_ptr->can_b_ppgn_ext_msg_num; i++)
    {
        scl_j1939_cat_ppgn_id_install_command_id
        (
                /* Identify J1939 link to application */
                TES_Link_app,

                /* Pointer to a Proprietary PGN Command entry */
                &(table_ptr->can_b_ppgn_ext_msg[i]));
    }

    /*
     * Making memory access protocol timeouts longer for testing.
     */
    //scl_j1939_set_map_timeouts(5000, 10000);

    scl_j1939_set_ma_configuration(
            Ph_Link_app, /* scl_j1939_link_t Ph_link */
            app_scl_j1939_get_ma_num(), /* Pi8_ma_param_size */
            app_scl_j1939_ma_param_tbl, /* scl_j1939_ma_param_list_t* Ppast_ma_param_tbl */
            app_scl_j1939_max_ma_sessions); /* unsigned_8 Pu8_max_ma_sessions */

    scl_j1939_set_ma_configuration(
            TES_Link_app, /* scl_j1939_link_t Ph_link */
            app_scl_j1939_get_ma_num(), /* Pi8_ma_param_size */
            app_scl_j1939_ma_param_tbl, /* scl_j1939_ma_param_list_t* Ppast_ma_param_tbl */
            app_scl_j1939_max_ma_sessions); /* unsigned_8 Pu8_max_ma_sessions */

    app_j1939_map_init(Ph_Link_app, 0);

    /* Proprietary j1939 connection management is required for SCL PGN J1939 */
    scl_j1939_cmgr_init(Ph_Link_app, 10,10);
    scl_j1939_cmgr_init(TES_Link_app, 10,10);

    app_scl_pgb_j1939_server_init(Ph_Link_app);
    app_scl_pgb_j1939_server_init(TES_Link_app);

    /* Initialize server for J1939 read/write protocol*/
    scl_rw_j1939_server_init(Ph_Link_app);
    scl_rw_j1939_server_init(TES_Link_app);

    /*Initializes CDL R/W protocol over J1939 for CAN A */
    scl_rw_j1939_client_init(Ph_Link_app);
    scl_rw_j1939_client_init(TES_Link_app);

    scl_j1939_install_dm13(Ph_Link_app, 0);
    scl_j1939_install_dm13(TES_Link_app, 0);
    scl_j1939_set_max_dm13_clients(0, 1, app_j1939_is_sbcast_allowed_cb);
    scl_j1939_add_suspend_client(app_j1939_suspend_cb_1, NULL);

    /*initialize eddt */
    eddt_error = app_scl_obd_init(Ph_Link_app);
    printf("app_scl_obd_init: %d \n", eddt_error);
   
    /* Initialization done */
    scl_j1939_init_tasks(app_scl_j1939_tsk_period);

    /* PGB and ADT protocol init  */
    app_scl_adt_j1939_server_init(Ph_Link_app);
    app_scl_adt_j1939_server_init(TES_Link_app);

    { // Initialize J1939 BDT
        static const bdt_action_config_t action_config = { BDT_ACTION_INIT_MAX_SIMULTANEOUS_CLIENTS };

        // Initialize the bdt layer
        if (FALSE == bdt_init(BDT_INIT_NUM_RX_CON, BDT_INIT_RX_BUF_SIZE,
                BDT_INIT_NUM_TX_CON, BDT_INIT_TX_BUF_SIZE,
                BDT_INIT_NUM_DATALINK_CON)) {
            printf("bdt_init failed\n");
        }
        else if (FALSE == bdt_action_init(&action_config)) {
            printf("bdt_action_init failed\n");
        }
        else {
            // This operation will set the BDT J1939 resource.
            bdt_j1939_set_resource(app_bdt_resource_config);

            // This assumes that BDT will be on the 1st J1939 link.
            bdt_j1939_init(Ph_Link_app);

            // A function to define the period (in microsec) at which the periodic BDT processing function will be called at
            bdt_set_period(BDT_UPDATE_PERIOD_MICRO_SECONDS_U32);

            // A function to reserve space for the number of callback blocks needed.
            bdt_set_callback_block_count(BDT_CALLBACK_BLOCK_COUNT);

            // A function to reserve space for the number of datatypes to be defined for BDT
            bdt_set_datatype_count(BDT_DATATYPE_COUNT);
        }
    }

    /*
     * Initialize Ethernet Port Stats
     */
#if SCL_STAT_DB_SUPPORTED
    if (FALSE == scl_stat_db_read_cs_bdt_init()) {
        printf("scl_stat_db_read_cs_bdt_init failed\n");
    }
#endif

    /* Initialize Health J1939:  
       NOTE: Uses external link because we are not using Health library to 
       log diagnostics. We are using Health library only for 
       System Communciation Status feature on ET
    */ 
    app_health_j39_init();

    /*CAl CAN Init starts*/
    init_cal_can();

    can_init_completed = TRUE;

    return;
}


int_16 CdlWrPidWrF25BWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
	if (len != sizeof(PidWrF25B)) {
	    return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    PidF25B = PidWrF25B = *src_data;

    printf("PidWrF25B = %x\n",PidWrF25B);

    /* Set flag to send request to Weigh App */
    PidWrF25BFlag = 1;
    printf("PidWrF25BFlag = %d\n",PidWrF25BFlag);
    return(0);
}

int_16 CdlWrPidWrD00144WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    if (len != sizeof(PidWrD00144)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data,PidWrD00144);
    PidD00144 = PidWrD00144;

    printf("PidWrD00144 = %x\n",PidWrD00144);

    /* Set flag to send request to Weigh App */
    PidWrD00144Flag = 1;
    printf("PidWrD00144Flag = %d\n",PidWrD00144Flag);
    return(0);
}

int_16 CdlWrPidWrD0022CWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
	  if (len != sizeof(PidWrD0022C))
	     {
	         return(1);
	     }
	  OEL_UNPACK_BE_16_NO_INCR(src_data,PidWrD0022C);
	  PidD0022C = PidWrD0022C;
	 // getLogger().log_error("\nPidWrD0022C = %x\n",PidWrD0022C);
	  printf("PidWrD0022C = %x\n",PidWrD0022C);

	  /* Set flag to send request to Weigh App */
	  PidWrD0022CFlag = 1;
	  printf("PidWrD0022CFlag = %d\n",PidWrD0022CFlag);
	  return(0);
}

int_16 CdlPidF2CAWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    uint8_t value;
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU8_t* data = (AppPidU8_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_8_NO_INCR(src_data, value);

    // 1 1 0 x x 0 0 1
    // | | | | | | | |
    // | | | | | | | - Zero Reminder Feature (1 - Always Enabled)
    // | | | | | | - Warmup Lifts Feature (0 - Always Disabled)
    // | | | | | - Automatic Operator ID Feature (0 - Always Disabled)
    // | | | | - Automatic Material ID Feature (Configurable)
    // | | | - Automatic Truck ID Feature (Configurable)
    // | | - Not Used (0)
    // | - Lift Cylinder Rod End Pressure Sensor (1 - Always Enabled)
    // - Tilt Sensor (1 - Always Enabled for now)
    value |= 0xC1;  // Set 11-----1
    value &= ~0x26; // Clr --0--00-

    data->wr = value;
    data->tx = value;
    data->wrFlag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return 0;
}

int_16 CdlWrPidWrD0273BWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, data->wr);

    data->tx = data->wr;
    data->wrFlag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return 0;
}

// Display Weight Units Configuration
int_16 CdlWrPidWrD10938WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x03DB): // Kilogram
    case (0x03DC): // Pound
    case (0x03DD): // Ton
    case (0x03DE): // Tonne (metric)
    case (0x07D7): { // Kilopound
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

// Display Weight Precision Configuration
int_16 CdlWrPidWrD11B8DWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x0003): // High
    case (0x002C): // Not Configured
    case (0x003D): { // Standard
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlWrPidWrD11B9AWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    // disable write if lft is sealed
    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x000C): // Enabled
    case (0x000D): { // Disabled
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlWrPidWrD11878WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x000C): // Enabled
    case (0x000D): { // Disabled
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}


int_16 CdlWrPidWrD11F99WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD11F99)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD11F99);
    PidD11F99 = PidWrD11F99;

    PidWrD11F99Flag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD1207AWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD1207A)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD1207A);
    PidD1207A = PidWrD1207A;

    PidWrD1207AFlag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlPidPrinterInstallWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x0010):
    case (0x0011):
    case (0x003D):
    case (0x067A): {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlPidPrinterNumTicketsWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, data->wr);

    if (data->wr <= 3) {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    else {
        data->wr = data->tx;
        return 1;
    }
}

int_16 CdlPidPrinterBlankLinesWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, data->wr);

    if (data->wr <= 3) {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    else {
        data->wr = data->tx;
        return 1;
    }
}

int_16 CdlPidPrinterTicketNumberWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, data->wr);

    if (data->wr <= 65503) {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    else {
        data->wr = data->tx;
        return 1;
    }
}

int_16 CdlPidPrinterEnableStatusWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x000C):
    case (0x000D): {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlPidPrinterHeaderLineWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidString_t* data = (AppPidString_t*)wr_pie->data;
    uint8_t src_len = *src_data;
    if (len > wr_pie->len) {
        return 1;
    }

    if (src_len < APP_PID_STRING_BUF_SIZE) {
        const uint8_t* src = src_data+1;
        char* dest = &(data->wr[0]);
        uint8_t ii = 0;
        while (ii < APP_PID_STRING_BUF_SIZE) {
            if (ii < src_len) {
                *dest = *src;
                ++src;
            }
            else {
                *dest = '\0';
            }
            ++dest;
            ++ii;
        }
        memcpy(&(data->tx[0]), &(data->wr[0]), APP_PID_STRING_BUF_SIZE);
        data->wrFlag = 1;
        return 0;
    }

    return 1;
}

int_16 CdlPidPrinterDateFormatWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x0670):
    case (0x0671):
    case (0x0672):
    case (0x0673):
    case (0x0674):
    case (0x0675):
    case (0x0676):
    case (0x0677): {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlPidPrinterDateSeparatorWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x0678):
    case (0x0679): {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlWrPidWrD11919WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, data->wr);

    switch (data->wr) {
    case (0x000C):
    case (0x000D): {
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    default: {
        data->wr = data->tx;
        return 1;
    }
    }
}

int_16 CdlPidTicketRetentionWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    const app_scl_cdl_wr_pie_t* wr_pie = pie;
    AppPidU16_t* data = (AppPidU16_t*)wr_pie->data;
    if (len != wr_pie->len) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, PidD03093.wr);

    if (data->wr <= 65503) {
        /*
         * Limit to between 1 and 10 years
         */
        if (data->wr < 365) {
            data->wr = 365;
        }
        else if (data->wr > 3660) {
            data->wr = 3660;
        }
        data->tx = data->wr;
        data->wrFlag = 1;
        return 0;
    }
    else {
        data->wr = data->tx;
        return 1;
    }
}

