/*******************************************************************************
***     COPYRIGHT (C) 2011-2015 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** ****************************************************************************
***
***  FILE:  app_et_j1939_support.h
***
***  DESCRIPTION: ET support over J1939
******************************************************************************/
#ifndef APP_ET_J1939_SUPPORT_H_
#define APP_ET_J1939_SUPPORT_H_

#ifndef SCL_J1939_H_
#include <scl_j1939.h>
#endif

#ifndef SCL_J1939_PROTO_H_
#include <scl_j1939_proto.h>
#endif

#ifndef APPLIB_PUBLIC_H_
#include <applib_public.h>
#endif

#ifndef ECMINFOLIB_PRIVATE_H_
#include <ecminfolib_private.h>
#endif

#include <scl_pgb_j1939.h>

#include <oel_pack.h>
/******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
******************************************************************************/
#define ECM_SERIAL_NUMBER_MAX_SIZE  16
#define ECM_PART_NUMBER_MAX_SIZE 12

/* ET J1939 support Information */
#define PROP_PGN_CAT_EXT_ID_READ                  0x80
#define SCL_J1939_TEST_DEVICE_ID_VER              0x00
#define SCL_J1939_DEVICE_INFO_EXT_ID              0xF016
#define SCL_J1939_FLASH_INFORMATION_MESSAGE       0xF003
#define SCL_J1939_ECM_INFO_CAT_ID                 0xF01A
#define SCL_J1939_SW_GRP_INFO_EXT_ID              0xF019
#define SCL_J1939_ECM_COMM_PREF_EXT_ID            0xF0A6
#define SCL_J1939_SW_NAME_CODE_EXT_ID             0xF0BD

#ifdef __cplusplus
extern "C" {
#endif

void app_scl_j1939_build_extid_f016(scl_j1939_mbuf_hdr_t *mbuf_ptr);
void app_scl_j1939_build_extid_f003(scl_j1939_mbuf_hdr_t *mbuf_ptr);
void app_scl_j1939_build_extid_f01a(scl_j1939_mbuf_hdr_t *mbuf_ptr);
void app_scl_j1939_build_extid_f019(scl_j1939_mbuf_hdr_t *mbuf_ptr);
void app_scl_j1939_build_extid_f0a6(scl_j1939_mbuf_hdr_t *mbuf_ptr);
void app_scl_j1939_build_extid_f0bd(scl_j1939_mbuf_hdr_t *mbuf_ptr);

extern volatile boolean AppStartupInitFlag;

extern char app_description_dl_tx_pie[];
extern char ecm_serial_number[]; /* ECM Serial Number */
extern char ecm_part_number[]; /* ECM Part Number */

extern scl_pgb_j1939_tx_pid_config_t app_tx_pid_config[];


/** Legal For Trade **/
extern int_16      PidD02B3B;//Payload Zero Adjustment Weight
extern unsigned_16 PidD023EA;//Payload Correction Factor
extern bool_t lft_disable_write_pids;


/** Totals [130] **/

extern unsigned_32 PidFC06; // - Load Count [3]  (RECM)
extern unsigned_32 PidFCF8; // - Lifetime Total Payload Weight [1]  (RECM)
extern unsigned_32 PidFCF7; // - Total Pass Count [1]  (RECM)
extern uint32_t PidFE22; // Total Machine Operation Cycle Count

extern unsigned_32 PidWrFC06; // - Load Count [3]  (RECM)
extern unsigned_32 PidWrFCF8; // - Lifetime Total Payload Weight [1]  (RECM)
extern unsigned_32 PidWrFCF7; // - Total Pass Count [1]  (RECM)

extern boolean PidWrFC06Flag; // - Load Count [3]  (RECM)
extern boolean PidWrFCF8Flag; // - Lifetime Total Payload Weight [1]  (RECM)
extern boolean PidWrFCF7Flag; // - Total Pass Count [1]  (RECM)

//System Parameters [4361]
extern unsigned_16 PidF42A;//System Voltage
extern unsigned_8 PidF2C4;//ECM Location Code
extern unsigned_8 PidF2C5;//Desired ECM Location Code

//Production Measurement Loading Status [-75716]
extern unsigned_8 PidF2C6;//Bucket Payload Data
extern int_16 PidD00C8D;//Bucket Payload
extern unsigned_16 PidD00D67;//Loader Bucket Payload Target Weight Percentage
extern unsigned_32 PidFD39 ;//Weigh Cycle Total Truck Payload
extern unsigned_32 PidFD38;//Truck Payload Target Weight
extern int_32 PidFE2E;//Remaining Payload to Load
extern unsigned_16 Pid00D00BD3;//Load Cycle Pass Count
extern unsigned_16 PidD01788;//Payload Calculation Method

//Payload Operating Mode
extern unsigned_16 PidD10748;
extern unsigned_16 PidWrD10748;
extern boolean PidWrD10748Flag;

//Production Measurement Weigh Status [-75717]
extern unsigned_16 PidD00E9B;//Payload Weigh Status - Insufficient Data

//Production Measurement Sensor Status
extern unsigned_8 PidF13D;//Lift Linkage Position Sensor Duty Cycle
extern unsigned_16 PidF5AE;//Lift Cylinder Position
extern unsigned_16 PidF47B;//Lift Cylinder Head End Pressure
extern unsigned_16 PidF5AF;//Lift Cylinder Rod End Pressure
extern unsigned_16 PidF4E0;//Tilt Cylinder Head End Pressure
extern int_16 PidF47A;//Loader Tilt Linkage Angle
extern unsigned_16 PidF47C;//Tilt Cylinder Rod End Pressure
extern unsigned_8 PidF13E ;//Tilt Linkage Position Sensor Duty Cycle
extern int_16 Pid0045;//Hydraulic Oil Temperature
extern unsigned_16 Pid00D0018B;//Lift Tilt Cylinder Extension (Absolute)
extern unsigned_16 Pid00D0018C;//Tilt Cylinder Extension (Absolute)
extern unsigned_16 Pid00D00639;//Tilt Cylinder Position Sensor Duty Cycle
extern int_16      Pid00D00A70;//Loader Bucket Angle

//Production Measurement Switches [-75719]
extern unsigned_8 PidF1AA;// Payload Store Switch

//Production Measurement System Status [-75720]
extern unsigned_16 PidD106DF;//Loader Payload State
extern unsigned_16 PidD10972;//Requested Payload Control System Status
extern unsigned_16 PidD10933;//Payload System Zero Requirement Status
extern unsigned_16 PidD10822;//Payload System Zeroed Status
extern unsigned_16 PidD109A5;//Payload System Calibration Weight Entry Requirement Status

//extern unsigned_16 PidD10765;//Payload Control System Print Command
extern unsigned_16 PidD10FAD;//Payload Remove Last Pass Button Display Status
extern unsigned_16 PidD10FAC;//Payload Clear Button Display Status

// Linkage Sensor Calibrated Limits [-75722]
extern unsigned_8 PidF13D;//Lift Linkage Position Sensor Duty Cycle
extern unsigned_16 PidD010FA;//Lift Linkage Position Sensor Full Raise Duty Cycle
extern unsigned_16 PidD010FB;//Lift Linkage Position Sensor Full Lower Duty Cycle
extern unsigned_8 PidF13E;//Tilt Linkage Position Sensor Duty Cycle
extern unsigned_16 PidD010FC;//Tilt Linkage Position Sensor Full Rackback Duty Cycle
extern unsigned_16 PidD010FD;//Tilt Linkage Position Sensor Full Dump Duty Cycle

//Configurable Parameters
//Production Measurement Settings
extern int_16 PidD01B24;// - Last Payload Weight [255]
extern unsigned_16 PidD009A7;// - Payload Control System Calibration Weight [1]
extern unsigned_16 PidD009F5;// - Loader Bucket Payload Target Weight [1]
extern unsigned_16 PidD112B2;// - Tip-Off Trigger Type Configuration [1] (RECM)
extern unsigned_16 PidD10E4E;// - Tip-Off Mode [1] (RECM)

//Production Measurement Advanced Settings
extern unsigned_16 PidD00C9F;// - Payload Weigh Activate Lift Position [1] (RECM)
extern unsigned_16 PidD0102D;// - Payload Weigh Range Configuration [1] (RECM)
extern unsigned_16 PidD10ACC;// - Payload Overload Warning Enable [1] (RECM)
extern unsigned_16 PidD10F96;// - Payload Weight Calculation Acceptable Audible Alarm Configuration [1] (RECM)

//Display Settings
extern unsigned_8 PidF25B;// - Information Units Status [1] (RECM)
extern unsigned_16 PidD00144;// - Service Mode Enable Code [1] (RECM)
extern unsigned_16 PidD0022C;// - Display Language Status [1] (RECM)

//Production Measurement Simple Calibration

extern unsigned_16 PidD01686;// - Truck History List Selected Index Number
extern unsigned_32 PidFE25;// -  Truck History List Truck Payload Weight


//Store Payload Data
extern boolean PidWrD106C4Flag;// - Store Payload Data Horn Configuration Write Flag
extern unsigned_16 PidWrD106C4;// - Store Payload Data Horn Configuration Write Data
extern unsigned_16 PidD106C4; // - Store Payload Data Horn Configuration

//Configurable Parameters
//Production Measurement Settings
extern unsigned_16 PidWrD009A7;// - Payload Control System Calibration Weight [1]
extern unsigned_16 PidWrD009F5;// - Loader Bucket Payload Target Weight [1]
extern unsigned_16 PidWrD112B2;// - Tip-Off Trigger Type Configuration [1] (RECM)
extern unsigned_16 PidWrD10E4E;// - Tip-Off Mode [1] (RECM)

//Production Measurement Advanced Settings
extern unsigned_16 PidWrD00C9F;// - Payload Weigh Activate Lift Position [1] (RECM)
extern unsigned_16 PidWrD0102D;// - Payload Weigh Range Configuration [1] (RECM)
extern unsigned_16 PidWrD10ACC;// - Payload Overload Warning Enable [1] (RECM)
extern unsigned_16 PidWrD10F96;// - Payload Weight Calculation Acceptable Audible Alarm Configuration
extern unsigned_16 PidWrD11F99;// - Payload Legal For Trade Seal Status
extern unsigned_16 PidWrD1207A;// - Payload Legal For Trade Measurement Sealed Flash Enable Status

//Display Settings
extern unsigned_8 PidWrF25B;// - Information Units Status [1] (RECM)
extern unsigned_16 PidWrD00144;// - Service Mode Enable Code [1] (RECM)
extern unsigned_16 PidWrD0022C;// - Display Language Status [1] (RECM)

//Production Measurement System Status
extern unsigned_16 PidWrD10972;// - Requested Payload Control System Status Write Data

//Configurable Parameters
//Production Measurement Settings
extern boolean PidWrD009A7Flag ;// - Payload Control System Calibration Weight [1]
extern boolean PidWrD009F5Flag ;// - Loader Bucket Payload Target Weight [1]
extern boolean PidWrD112B2Flag ;// - Tip-Off Trigger Type Configuration [1] (RECM)
extern boolean PidWrD10E4EFlag ;// - Tip-Off Mode [1] (RECM)

// ECM Parameters
extern boolean PidWrF81AFlag;   // Equipment ID

//Production Measurement Advanced Settings
extern boolean PidWrD00C9FFlag ;// - Payload Weigh Activate Lift Position [1] (RECM)
extern boolean PidWrD0102DFlag ;// - Payload Weigh Range Configuration [1] (RECM)
extern boolean PidWrD10ACCFlag ;// - Payload Overload Warning Enable [1] (RECM)
extern boolean PidWrD10F96Flag ;// - Payload Weight Calculation Acceptable Audible Alarm Configuration [1] (RECM)
extern boolean PidWrD11F99Flag ;// - Payload Legal For Trade Seal Status
extern boolean PidWrD1207AFlag ;// - Payload Legal For Trade Measurement Sealed Flash Enable Status

//Display Settings
extern boolean PidWrF25BFlag ;// - Information Units Status [1] (RECM)
extern boolean PidWrD00144Flag ;// - Service Mode Enable Code [1] (RECM)
extern boolean PidWrD0022CFlag ;// - Display Language Status [1] (RECM)

//Production Measurement System Status
extern boolean PidWrD10972Flag;// - Requested Payload Control System Status Write Flag

//Travel Revs Parameter
extern unsigned_16 PidD026AF;// - Travelling Loaded Wheel Revs
extern unsigned_16 PidD026B1;// - Payload Cycle <Current> Dig Time

//Cycle seg
extern unsigned_16 PidD11907; // - Payload Cycle Dig State
extern unsigned_16 PidD0272F; // - Productivity Measurement Active Segment ID

// Audible Annunciation Command
extern unsigned_16 PidD02923;// - Desired Audible Annunciation Priority

#ifdef __cplusplus
}
#endif

#endif /* APP_ET_J1939_SUPPORT_H_*/
