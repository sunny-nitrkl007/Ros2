/*******************************************************************************
***
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%                                                                        %%
*** %%  COPYRIGHT (C) 2013 CATERPILLAR INC.   ALL RIGHTS RESERVED.            %%
*** %%      This work contains proprietary information which may              %%
*** %%      constitute a trade secret and/or be confidential.                 %%
*** %%                                                                        %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
***
*
*  File: app_can_init.h
*
*/

#ifndef __APP_CAN_INIT_H__
#define __APP_CAN_INIT_H__

#ifndef   APP_J1939_TABLES_H_
#include "app_j1939_tables.h"
#endif


#ifndef   SCL_RW_J1939_H_
#include <scl_rw_j1939.h>
#endif

#include <scl_j1939.h>
#include <cdl2_proto.h>

#ifndef   OEL_RTOS_POSIX_H_
#include <oel_rtos_posix.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t tx;
    uint8_t wr;
    boolean wrFlag;
} AppPidU8_t;

typedef struct {
    uint16_t tx;
    uint16_t wr;
    boolean wrFlag;
} AppPidU16_t;

#define APP_PID_STRING_BUF_SIZE (25)
typedef struct {
    char tx[APP_PID_STRING_BUF_SIZE]; // null terminated, max 24 characters
    char wr[APP_PID_STRING_BUF_SIZE]; // null terminated, max 24 characters
    boolean wrFlag;
} AppPidString_t;


#define DSI9_2U (UNKNOWN2U+FMIIUA)
#define DSI9_1U (UNKNOWN1U+FMIIUA)
/*********** Global procedures *******************************************/

 /* tx pit configuration for j1939 pgb */
 typedef struct app_scl_pgb_cdl_pie_s
 {
    unsigned_8 len;
    unsigned_32 *data;
    unsigned_32 pid;
    cdl2_tx_hdlr_t *tx_hdlr;
 }app_scl_pgb_cdl_pie_t;

 typedef struct
 {
	unsigned_8 len;
	unsigned_8 *data;
	unsigned_32 pid;
	Cdl_wr_hdlr_t *tx_hdlr;
 }app_scl_cdl_wr_pie_t;

void app_can_init();
void app_dm13_send_sleep_msg();

// PGN 0 for BEM Sim engine derate
void app_j1939_build_pgn_0000_update(void);

extern const oel_rtos_resource_config_t *app_bdt_resource_config;

extern int can_port1;
extern int can_port2;

// ECM J1939 Name
extern j1939_name_address_t j1939_name_address;
extern bool_t can_init_completed;
extern unsigned_8 app_master_mid;
extern unsigned_8 rtc_master_mid;
extern unsigned_8 rtc_type;
extern unsigned_8 rtc_security;
extern unsigned_16 j1939_mid;
extern unsigned_16 st_app_num;
extern unsigned_16 st_chg_lvl;
extern bool_t dm13_receive_suspend;
extern scl_j1939_link_t Ph_Link_app;

extern bool_t bem_sim_derate_enable;
extern int bem_sim_derate_rpm;

extern int_16 CdlWrPidWrD009A7WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD009F5WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD112B2WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD10E4EWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD00C9FWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD0102DWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD10ACCWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD10F96WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrF25BWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD00144WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD0022CWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD106C4WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD11919WrHdlr(void* pie, unsigned_16 len,unsigned_8* src_data);
extern int_16 CdlWrPidWrD10972WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrD0273BWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlWrPidWrD10748WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlPidTicketRetentionWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

extern char NewLpsSaWeighScsTxIn_;
extern int NewLpsSaJobMgrScsTxIn_;
extern char PendingLpsSaWeighScsReqOut_;
extern char PendingLpsSaJobMgrScsReqOut_;
extern char WeighRangeBottomRqstFlg;
extern char WeighRangeSizeRqstFlg;

extern int_16 CdlWrPidD00C9FWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidD0102DWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);

extern int_16 CdlWrPidWrFC06WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrFCF8WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);
extern int_16 CdlWrPidWrFCF7WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data);

extern AppPidU8_t PidF2CA; // PCS Features Configuration #2
extern int_16 CdlPidF2CAWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

extern AppPidU16_t PidD11B9A; // Payload Hydraulic Oil Temperature Compensation Enable Status [Payload / PCS / CPM]
extern int_16 CdlWrPidWrD11B9AWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

extern const unsigned_16 PidD113BD; // Payload System Display Mode

extern int_16 CdlPidPrinterInstallWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterNumTicketsWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterBlankLinesWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterTicketNumberWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterEnableStatusWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterHeaderLineWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterDateFormatWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern int_16 CdlPidPrinterDateSeparatorWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

// Display Settings
extern AppPidU16_t PidD10938; // Display Weight Units Configuration
extern int_16 CdlWrPidWrD10938WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

extern AppPidU16_t PidD11B8D; // Display Weight Precision Configuration
extern int_16 CdlWrPidWrD11B8DWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

// Production Measurement Printer Settings
extern AppPidU16_t PidD10E4D; // Onboard Payload Printer Installation Status [1] (RECM)
extern AppPidU16_t PidD00CA0; // Payload Control System Number of Printer Tickets Configuration [1] (RECM)
extern AppPidU16_t PidD01D2A; // Onboard Payload Printer Number of Blank Lines Preceding Output [1] (RECM)
extern AppPidU16_t PidD01D2B; // Onboard Payload Printer Number of Blank Lines Following Output [1] (RECM)
extern AppPidU16_t PidD01D2C; // Onboard Payload Printer Truck Ticket Number [1] (RECM)
extern AppPidU16_t PidD11132; // Onboard Payload Printer Ticket Header Enable Status [1] (RECM)
extern AppPidString_t PidF933; // Onboard Payload Printer Ticket Header Line #1 [1] (RECM)
extern AppPidString_t PidF934; // Onboard Payload Printer Ticket Header Line #2 [1] (RECM)
extern AppPidString_t PidF935; // Onboard Payload Printer Ticket Header Line #3 [1] (RECM)
extern AppPidU16_t PidD11133; // Onboard Payload Printer Ticket Bucket Weights Enable Status [1] (RECM)
extern AppPidU16_t PidD11134; // Onboard Payload Printer Ticket Truck Payload Weight Enable Status [1] (RECM)
extern AppPidU16_t PidD11135; // Onboard Payload Printer Ticket Truck ID Enable Status [1] (RECM)
extern AppPidU16_t PidD11136; // Onboard Payload Printer Ticket Material ID Enable Status [1] (RECM)
extern AppPidU16_t PidD11137; // Onboard Payload Printer Ticket Operator ID Enable Status [1] (RECM)
extern AppPidU16_t PidD11130; // Onboard Payload Printer Date Format Configuration [1] (RECM)
extern AppPidU16_t PidD11131; // Onboard Payload Printer Date Format Separator Character [1] (RECM)
extern AppPidU16_t PidD11138; // Onboard Payload Printer Ticket Payload Data Store Time Enable Status [1] (RECM)
extern AppPidU16_t PidD11139; // Onboard Payload Printer Material Report Reset Time Enable Status [1] (RECM)
extern AppPidU16_t PidD1113A; // Onboard Payload Printer Truck Report Reset Time Enable Status [1] (RECM)
extern AppPidU16_t PidD03093; // Payload Memory Ticket Retention Period

// Work Order Assist (WOA) Settings
extern AppPidU16_t PidD11878; // Dispatch Enable Status
extern int_16 CdlWrPidWrD11878WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern AppPidU16_t PidD11879; // Dispatch Installation Status

// Payload Legal For Trade Measurement
extern unsigned_16 PidD11E8D; // Payload Legal For Trade Measurement Installation Status
extern unsigned_16 PidD11F99; // Payload Legal for Trade Seal Status
extern int_16 CdlWrPidWrD11F99WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);
extern unsigned_16 PidD1207A; // Payload Legal For Trade Measurement Sealed Flash Enable Status
extern int_16 CdlWrPidWrD1207AWrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data);

// null terminated, max 26 characters
#define PID_F9C0_STRING_BUF_SIZE (27)
extern char PidF9C0[]; // Payload Legal For Trade Measurement Software Identifier

// Linkage Sensor Configurations
extern AppPidU16_t PidD10045; // Loader Lift Linkage Position Sensor Configuration
extern AppPidU16_t PidD10046; // Loader Tilt Linkage Position Sensor Configuration

// IMU
// Rear (EEF - Engine End Frame)
extern int16_t PidD019A1; // $D019A1 Machine Rear Lateral Acceleration
extern int16_t PidD019A3; // $D019A3 Machine Rear Longitudinal Acceleration
extern int16_t PidD019A5; // $D019A5 Machine Rear Vertical Acceleration

// Rear (EEF - Engine End Frame)
extern int16_t PidF58B; // $F58B Machine Pitch
extern int16_t PidF533; // $F533 Machine Slope

// Rear (EEF - Engine End Frame)
extern int16_t PidD01AFF; // $D01AFF Machine Rear Roll
extern int16_t PidD01FC9; // $D01FC9 Machine Rear Side Slope

// Ambiguously defined, populating as Rear (EEF - Engine End Frame)
extern int16_t PidD00A64; // $D00A64 Machine Roll
extern int16_t PidD00409; // $D00409 Machine Side Slope

// Display Screen Code
extern uint16_t PidD01396; // Display Module Screen Selection Code

// Payload Manual Add Configuration
extern AppPidU16_t PidD11919; // Payload Manual Add Configuration

// Payload Auto Store Pass Count Threshold
extern AppPidU16_t PidD0273B; // Payload Auto Store Pass Count Threshold

// Payload Memory Remaining Status
extern uint16_t PidD001A4; // $D001A4 Payload Memory Remaining Status

// Requested Payload Control System (PCS) Display Status
extern const uint16_t PidD106E7;

#ifdef __cplusplus
}
#endif
#endif /* __APP_CAN_INIT_H__ */
 
