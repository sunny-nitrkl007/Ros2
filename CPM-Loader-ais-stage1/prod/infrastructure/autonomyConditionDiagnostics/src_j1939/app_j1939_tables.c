/*******************************************************************************
***
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%                                                                       %%
*** %%  COPYRIGHT (C) 2014 CATERPILLAR INC.   ALL RIGHTS RESERVED.           %%
*** %%     This work contains proprietary information which may              %%
*** %%     constitute a trade secret and/or be confidential.                 %%
*** %%     Copyright notice is precautionary only and does not               %%
*** %%     imply publication.                                                %%
*** %%                                                                       %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*
* FILE NAME : app_j1939_tables.c
*
* DESCRIPTION :
*    Template which helps the application developer to understand: (1) how to
*    configure the address table, the transmitting message table, and the
*    receiving message table; (2) how to code the parse function for receiving
*    messages and build function for transmitting messages. the structures,
*    proto types, and macros used in this file can be found in j1939_struct.h
*    and j1939_proto.h.
*
* LANGUAGE : ANSI C
*
* PORTABILITY ISSUES :
*
*/

/*******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
*******************************************************************************/
#ifndef SCL_J1939_H
#include <scl_j1939.h>
#endif
#ifndef STD_TYPES_H
#include <std_types.h>
#endif
#ifndef APP_PUBLIC_H_
#include <app_public.h>
#endif
#include <scl_j1939.h>

#ifndef APP_J1939_TABLES_H_
#include "app_j1939_tables.h"
#endif

/*******************************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
*******************************************************************************/

#define GET_NAME_FIELDS( name ) \
    &name.Mu8_industry_group, \
    &name.Mu8_vehicle_system, \
    &name.Mu8_vehicle_system_instance, \
    &name.Mu8_function, \
    &name.Mu8_function_instance, \
    &name.Mu8_ecu_instance, \
    &name.Mu16_manufacturer_code, \
    &name.Mu32_identity_number, \
    &name.Mu8_preferred_address

#define J1939_INDUSTRY_GROUP_CONSTRUCTION_EQUIPMENT (3)

#define J1939_MANUFACTURER_CODE_CATERPILLAR (8)

#define J1939_FUNCTION_INFORMATION_SYSTEM_CONTROLLER (47)

#define J1939_FUNCTION_SERVICE_TOOL (129)

/*******************************************************************************
***
***    Data types defined for this file
***    -- Struct's, Typedef's, Enum's --
***
*******************************************************************************/

/******************************************************************************
***
***    File scope functions defined for this file
***    -- Function Prototypes --
***
******************************************************************************/
boolean_t app_j1939_build_pgn_EA00_RQST_FDC5(
   scl_j1939_mbuf_hdr_t *Ppst_mbuf,
   unsigned_32 Pu32_pgn);

boolean_t app_j1939_build_pgn_EA00_RQST_FEDA(
   scl_j1939_mbuf_hdr_t *Ppst_mbuf,
   unsigned_32 Pu32_pgn);

/******************************************************************************
***
***    File scope symbols defined for this file
***    -- Symbols --
***
******************************************************************************/

/*********************************************************************************
 rx msg tbl. this table lists the information about all pgns which can be
 received by the ecm. for each pgn receivable to the ecm, there is an entry in
 this table. the table should be defined as "const". information about each pgn
 include:

    Mu32_pgn       J1939 PGN identifying this message.

    Mpfn_parser       Function that unpacks the data from the J1939
                component and injects it into the application.

    Mu8_priority      The priority of the mesage when requesting this PGN.

    Mb_address_by_name   Flag used to determine whether the Source is to
                be determined by NAME or address.  If the value of
                this field is TRUE, then the NAME (Mau8_source_name)
                is used to determine the device from which the message
                is to be received.  If the value is FALSE, then the
                address (Mu8_source_address) is used.

    Mu8_source_address   The source address (the network address of the ECM
                from where the PGN came) of this PGN.

    Mau8_source_name  The J1939 NAME of the device that originated this PGN.

    Mu8_init_cntrl    Initial value that indicates how the message is received,
                e.g., if the lost timer is applicable.
    Allowed values:
    00 - Inactive (SCL_J1939_RX_MSG_CBF)
    01 - Enabled (SCL_J1939_RX_MSG_CBF_ENABLED)
    02 - Use time out value (SCL_J1939_RX_MSG_CBF_LOST_TIMER_APPLICABLE)

    Mi16_lost_timeout The amount of time before a message is assumed lost.
----------------------------------------------------------------------------------*/

/* RX Msg Table for Link A */
static const scl_j1939_rx_msg_t ecm_rx_table_eddt_1[] =
{
   {
      0x00EF00,                                                   /*     PGN  */
      (scl_j1939_rx_msg_parser)scl_j1939_cat_ppgn_hdlr,     /* Message Parser */
      0x06,                                                       /* Priority */
      FALSE,                     /* Source Name/Address Flag - Source Address */
      SCL_J1939_GLOBAL_ADDRESS,         /* Source Address - ANY ECM           */
      " ",
      ( SCL_J1939_RX_MSG_CBF_ENABLED ), /* Enable to call message parser      */
      0                                                      /* Time-out (ms) */
   },
};

/*********************************************************************************
 tx msg tbl. this table lists the information about all pgns which can be
 transmitted by the ecm. for each pgn transmittable to the ecm, there is an entry in
 this table. this table should be defined as "const". information about each pgn
 include:
** Mu32_pgn          The PGN number which will be sent by PGN 59904 to
**                   request the message.
**
** Mu8_priority         Priority to be assigned to the message when it is
**                   transmitted.
**
** Mb_address_by_name      Flag used to determine whether the Destination is to
**                   be determined by NAME or address.  If the value of this
**                   field is TRUE, then the NAME (Mau8_dest_name) is
**                   used to determine the device to which the message is to
**                   be sent.  If the value is FALSE, then the address
**                   (Mu8_dest_address) is used.
**
** Mu8_dest_address     Network address of the ECM to send the messages to.
**
** Mau8_dest_name       J1939 NAME of the ECM to send the message to.
**
** Mpfn_builder         Pointer to the functions used to build and queue the
**                   message for transmittal.
**
** Mu8_init_cntrl       Initial value that determines how message is send out,
**                   either "diasbled", "ON Request" (via PGN 59904), "periodic"
**                   or periodic but stops at DM13 stop broadcast request:
**    Allowed values:
**       00 - Disabled (SCL_J1939_TX_MSG_CBF)
**       01 - On Request (SCL_J1939_TX_MSG_CBF_ON_REQUEST)
**       02 - Periodic (SCL_J1939_TX_MSG_CBF_PERIODIC)
**       04 - Stop broacast on DM13 stop broadcast request
**            (SCL_J1939_TX_MSG_CBF_STOP_BCAST)
**
**  Mb_on_request_accept_all
**                          Flag indicating that all messages by request will be
**                          accepted for this entry on request.  It will send
**                          back the response to the address requesting this
**                          pgn.
**
** Mi16_init_period     Initial value for period. If the Mu8_init_cntrl is
**                   set up for Periodic Mode, then this value determines
**                   the number of milliseconds between transmittals of
**                   the associated message.
**
**  Mb_msg_slip             If set to true, the message can slip to the next
**                          time period if the message cannot queue and transmit.
**                          Otherwise, it will be marked to transmit the next
**                          time the periodic task is called.  This will change
**                          load balancing if message slipping is not allowed.
**
**  Mi16_offset             Offset of initial period for first broadcast
**                          from startup.
----------------------------------------------------------------------------------*/

/* TX Msg Table for Link A */
//static const scl_j1939_tx_msg_t ecm_tx_table_eddt_1[] =
//{
//  {
//  	0xEF00,                             /* PGN                            */
//  	0x03,                               /* Priority                       */
//  	TRUE,                               /* Name/Address Flag FALSE=Address*/
//      SCL_J1939_GLOBAL_ADDRESS,           /* Source Address - ANY ECM       */
//      SCL_J1939_NAME(
//  		 0,                             /* Industry Group                 */
//  		 0,                             /* Vehicle System instance        */
//  		 0,                             /* Vehicle System                 */
//  		 145,                           /* Function                       */
//  		 0,                             /* Function Instance              */
//  		 0),                            /* ECU Instance                   */
//  	app_j1939_build_pgn_EF00_imu,       /* Build Function                 */
//  	SCL_J1939_TX_MSG_CBF_PERIODIC,
//  	TRUE,                               /* Mb_on_request_accept_all       */
//  	10,                                 /* Periodicity in ms              */
//  	FALSE,                              /* Mb_msg_slip                    */
//  	100,                                /* Mi16_offset                    */
//  },
//};

/* TX Msg Table for Link B */
static const scl_j1939_tx_msg_t ecm_tx_table_eddt_2[] =
{
  {
      0xEA00,                             /* PGN                            */
      0x03,                               /* Priority                       */
      FALSE,                              /* Name/Address Flag FALSE=Address*/
      0xFF,                               /* Destination Address - IMU      */
      SCL_J1939_NAME(
           0,                             /* Industry Group                 */
           0,                             /* Vehicle System instance        */
           0,                             /* Vehicle System                 */
           145,                           /* Function                       */
           0,                             /* Function Instance              */
           0),                            /* ECU Instance                   */
      (scl_j1939_tx_msg_builder)app_j1939_build_pgn_EA00_RQST_FDC5, /* Build Function                 */
      SCL_J1939_TX_MSG_CBF_PERIODIC,
      TRUE,                               /* Mb_on_request_accept_all       */
      30000,                              /* Periodicity in ms              */
      FALSE,                              /* Mb_msg_slip                    */
      100,                                /* Mi16_offset                    */
  },
  {
      0xEA00,                             /* PGN                            */
      0x03,                               /* Priority                       */
      FALSE,                              /* Name/Address Flag FALSE=Address*/
      0xFF,                               /* Destination Address - IMU      */
      SCL_J1939_NAME(
           0,                             /* Industry Group                 */
           0,                             /* Vehicle System instance        */
           0,                             /* Vehicle System                 */
           145,                           /* Function                       */
           0,                             /* Function Instance              */
           0),                            /* ECU Instance                   */
      (scl_j1939_tx_msg_builder)app_j1939_build_pgn_EA00_RQST_FEDA, /* Build Function                 */
      SCL_J1939_TX_MSG_CBF_PERIODIC,
      TRUE,                               /* Mb_on_request_accept_all       */
      30000,                              /* Periodicity in ms              */
      FALSE,                              /* Mb_msg_slip                    */
      15000,                               /* Mi16_offset                    */
    },
};

/* Build function for PGN EA00(59904) (RQST for 0xFDC5) */
boolean_t app_j1939_build_pgn_EA00_RQST_FDC5(
   scl_j1939_mbuf_hdr_t *Ppst_mbuf,
   unsigned_32 Pu32_pgn)
{
    unsigned_8 data[] = {0xC5, 0xFD, 0x00};

    unsigned_16 length = sizeof(data);

   return (scl_j1939_fill_mbuf_data(Ppst_mbuf,data,length));
}

/* Build function for PGN EA00(59904) (RQST for 0xFEDA) */
boolean_t app_j1939_build_pgn_EA00_RQST_FEDA(
   scl_j1939_mbuf_hdr_t *Ppst_mbuf,
   unsigned_32 Pu32_pgn)
{
    unsigned_8 data[] = {0xDA, 0xFE, 0x00};

    unsigned_16 length = sizeof(data);

   return (scl_j1939_fill_mbuf_data(Ppst_mbuf,data,length));
}


/********************************************************************************
Name and Address Table

This structure is intended to be an element of a table used
to determine all the "virtual" ECMs connected to the network via one particular
CAN port. information about an address includes:

   Industry Group NAME field.  Only the 3 least significant bits are used.

   Vehicle System NAME field.Only the 7 least significant bits are used.

   Vehicle System Instance NAME field.Only the 4 least significant bits are used.

   Function NAME field.

   Function Instance NAME field.Only the 5 least significant bits are used.

   ECU Instance NAME field.Only the 3 least significant bits are used.

   Manufacturer Code NAME field.Only the 11 least significant bits are used.

   Identity Number NAME field.Only the 21 least significant bits are used.

   The address that should be attempted to be claimed first.
   Only after a claim for this address fails will a claim attempt be made
   for another addresses in the Mpau8_AddressList.

   The number of valid entries in the Mpau8_address_list array.

   A pointer to a list of addresses that may be claimed for the ECM using
   the NAME constructed from the above fields.
   The application must allocate and initialize this list to at least
   Mpau8_address_list_size  entries before passing the table to the stack.

   Mb_self_config flag indicates that the ECM is self-configurable.
   If this value is TRUE, the Arbitrary Address bit will be set in the NAME,
   and the stack will attempt to claim addresses from the Mpau8_address_list
   until a claim is successful.
   If this value is FALSE, the Arbitrary Address bit will be cleared in the NAME,
   and the stack will only attempt to claim the Mu8_preferred_address.

   Mb_enable_ext_addresses flag indicates that the ECM should attempt to
   claim addresses in the range 128-247 if no address in Mpau8_address_list
   could be claimed.
   If this value is TRUE, after all addresses in Mpau8_address_list have failed
   claims, addresses in the extended range will be used.
   If this value is FALSE, after all addresses in Mpau8_address_list have failed
   claims, the ECM will send a Cannot Claim Source Address message and remain off-line.
   If Mb_self_config is FALSE, the value of this field is ignored.

---------------------------------------------------------------------------------*/

/*******************************************************************************
**            EDDT PGN NAME configuration for J1939
*******************************************************************************/
static j1939_name_address_t app_can_a_name =
{
   /*
   // NAME
   */
   J1939_INDUSTRY_GROUP_CONSTRUCTION_EQUIPMENT,  /* Industry Group                            */
   0,                                            /* Vehicle System                            */
   0,                                            /* Vehicle System Instance                   */
   J1939_FUNCTION_INFORMATION_SYSTEM_CONTROLLER, /* Function                                  */
   0,                                            /* Function Instance                         */
   0,                                            /* ECU Instance                              */
   J1939_MANUFACTURER_CODE_CATERPILLAR,          /* Manufacturer Code                         */
   0x230813,   		                             /* Identity Number                           */

   /*
   // Preferred Address
   */
   GATEWAY_CAN_A_J1939_ADD, /* Preferred Address                  */
};


/* Address List for Port #1 (CAN A) */
static unsigned_8  app_j1939_addr_a_list[] = {GATEWAY_CAN_A_J1939_ADD};

/* SCL J1939 Table for Port #1 (CAN A)*/
scl_j1939_addr_tbl_t app_scl_j1939_a_address[] =
{
   {
      /*ECM J1939 Name                                                   */
      GET_NAME_FIELDS(app_can_a_name),

      /*Address List To Try If ECM Cannot Claim Preferred Source Address */
      sizeof(app_j1939_addr_a_list),                     /*Size          */
      app_j1939_addr_a_list,                             /*Pointer       */

      TRUE, /*Support Self-Configuration Of Source Address         [Yes]*/
      TRUE, /*Enable Extended Addresses During Self-Configuration  [Yes]*/

      /*Receive Message Table                                            */
      sizeof(ecm_rx_table_eddt_1)/sizeof(ecm_rx_table_eddt_1[0]),  /*Size    */
      ecm_rx_table_eddt_1,                                    /*Pointer    */

      /*Transmit Message Table*/
      0,                                                 /*Size          */
      NULL,                                              /*Pointer       */
   },
};

static j1939_name_address_t app_can_b_name =
{
   /*
   // NAME
   */
   J1939_INDUSTRY_GROUP_CONSTRUCTION_EQUIPMENT,  /* Industry Group                            */
   0,                                            /* Vehicle System                            */
   0,                                            /* Vehicle System Instance                   */
   J1939_FUNCTION_INFORMATION_SYSTEM_CONTROLLER, /* Function                                  */
   0,                                            /* Function Instance                         */
   0,                                            /* ECU Instance                              */
   J1939_MANUFACTURER_CODE_CATERPILLAR,          /* Manufacturer Code                         */
   0x230813,                                     /* Identity Number                           */

   /*
   // Preferred Address
   */
   GATEWAY_CAN_B_J1939_ADD, /* Preferred Address                  */
};


/* Address List for Port #1 (CAN B) */
static unsigned_8  app_j1939_addr_b_list[] = {GATEWAY_CAN_B_J1939_ADD};

/* SCL J1939 Table for Port #1 (CAN B)*/
scl_j1939_addr_tbl_t app_scl_j1939_b_address[] =
{
   {
      /*ECM J1939 Name                                                   */
      GET_NAME_FIELDS(app_can_b_name),

      /*Address List To Try If ECM Cannot Claim Preferred Source Address */
      sizeof(app_j1939_addr_b_list),                     /*Size          */
      app_j1939_addr_b_list,                             /*Pointer       */

      TRUE, /*Support Self-Configuration Of Source Address         [Yes]*/
      TRUE, /*Enable Extended Addresses During Self-Configuration  [Yes]*/

      /*Receive Message Table                                            */
      sizeof(ecm_rx_table_eddt_1)/sizeof(ecm_rx_table_eddt_1[0]),  /*Size    */
      ecm_rx_table_eddt_1,                                    /*Pointer    */

      /*Transmit Message Table*/
      sizeof(ecm_tx_table_eddt_2)/sizeof(ecm_tx_table_eddt_2[0]),  /*Size    */
      ecm_tx_table_eddt_2,                                    /*Pointer       */
   },
};

/********************************************************************************
    The Proprietary Enable Table contains information to identify the ECMs
that are able to use proprietary messaging with each virtual ECM.
The table is an array of j1939_prop_enable_t structures, and is indexed
by virtual ECM.  The table is initialized and maintained by the application.

prop_a_address  If the value is not 0xFF, it specifies the network address
    from which Proprietary A requests may be accepted.  Only Proprietary A
    requests with a source address that matches prop_a_address will be accepted.
    If the value is 0xFF, the NAME associated with the Proprietary A request's
    source address will be matched with the Proprietary A match fields below.

prop_a_manuf_code   The Manufacturer Code field of the NAME associated with
    a Proprietary A request's source address must match this value for the
    request to be accepted.Only the 11 least significant bits are used.
    This value is ignored if prop_a_address is not 0xFF.

prop_b_address  If the value is not 0xFF, it specifies the network address from
    which Proprietary B requests may be accepted.  Only Proprietary B requests
    with a source address that matches prop_b_address will be accepted.
    If the value is 0xFF, the NAME associated with the Proprietary B request's
    source address will be matched with the Proprietary B match fields below.

prop_b_industry_group   The Industry Group field of the NAME associated with a
    Proprietary B request's source address must match this value for the request
    to be accepted.  Only the 3 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_vehicle_system   The Vehicle System field of the NAME associated with a
    Proprietary B request's source address must match this value for the request
    to be accepted.  Only the 7 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_vehicle_system_inst  The Vehicle System Instance field of the NAME
    associated with a Proprietary B request's source address must match this value
    for the request to be accepted.  Only the 4 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_function The Function field of the NAME associated with a Proprietary B
    request's source address must match this value for the request to be accepted.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_function_inst    The Function Instance field of the NAME associated with a
    Proprietary B request's source address must match this value for the request
    to be accepted.  Only the 5 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_ecu_inst The ECU Instance field of the NAME associated with a Proprietary B
    request's source address must match this value for the request to be accepted.
    Only the 3 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.

prop_b_manuf_code   The Manufacturer Code field of the NAME associated with a
    Proprietary B request's source address must match this value for the request
    to be accepted.  Only the 11 least significant bits are used.
    This value is ignored if prop_b_address is not 0xFF.
---------------------------------------------------------------------------------*/
static scl_j1939_ppgn_enable_t app_scl_j1939_ppgn_enable_a[] =
{
    {
        /* Propriatary A (PDU1)*/
        SCL_J1939_GLOBAL_ADDRESS,            /* unsigned_8  Mu8_ppgn_a_address; */
        J1939_MANUFACTURER_CODE_CATERPILLAR, /* unsigned_16 Mu16_ppgn_a_manuf_code; */

        /* Propriatary B (PDU2)*/
        SCL_J1939_GLOBAL_ADDRESS,            /* unsigned_8 Mu8_ppgn_b_address; */
        0,                                   /* unsigned_8 Mu8_ppgn_b_industry_group; */
        0,                                   /* unsigned_8 Mu8_ppgn_b_vehicle_system; */
        0,                                   /* unsigned_8 Mu8_ppgn_b_vehicle_system_inst; */
        J1939_FUNCTION_SERVICE_TOOL,         /* unsigned_8 Mu8_ppgn_b_function; */
        0,                                   /* unsigned_8 Mu8_ppgn_b_function_inst; */
        0,                                   /* unsigned_8 Mu8_ppgn_b_ecu_inst; */
        J1939_MANUFACTURER_CODE_CATERPILLAR, /* prop_b_manuf_code */
    },
};

/*
// Defines CAT Proprietary PGN Handler for Cat Ext Id command. For example,
// whenever Prop PGN is received with a command to read or write data for a
// given Cat Ext Id, this corresponding handler will be called. For example,
// if a command to read request a Cat Ext Id comes from other ECMs, this table
// matches for the command and Cat Ext Id and corresponding handler will be
// called.
*/
static SCL_J1939_CAT_PPGN_ID_HDLR_t j1939_a_ppgn_ext_msg[] =
{
    {
      PROP_PGN_CAT_EXT_ID_READ,                // 0x80 Read CAT ext ID
      SCL_J1939_ECM_INFO_CAT_ID,               // 0xF01A ET support CAT ext ID
      app_scl_j1939_build_extid_f01a,
      NULL,
      NULL,
      NULL
    },
    {
      PROP_PGN_CAT_EXT_ID_READ,                // 0x80 Read CAT ext ID
      SCL_J1939_DEVICE_INFO_EXT_ID,            // 0xF016 ET support CAT ext ID
      app_scl_j1939_build_extid_f016,
      NULL,
      NULL,
      NULL
   },
   {
     PROP_PGN_CAT_EXT_ID_READ,
     SCL_J1939_SW_NAME_CODE_EXT_ID,
     app_scl_j1939_build_extid_f0bd,
     NULL,
     NULL,
     NULL
   },
   {
      PROP_PGN_CAT_EXT_ID_READ,
      SCL_J1939_FLASH_INFORMATION_MESSAGE,
      app_scl_j1939_build_extid_f003,
      NULL,
      NULL,
      NULL
   },
   {
     PROP_PGN_CAT_EXT_ID_READ,                  // 0x80 Read CAT ext ID
     SCL_J1939_SW_GRP_INFO_EXT_ID,              // 0xF019 ET support CAT ext ID
     app_scl_j1939_build_extid_f019,
     NULL,
     NULL,
     NULL
   },
   {
     PROP_PGN_CAT_EXT_ID_READ,                  // 0x80 Read CAT ext ID
     SCL_J1939_ECM_COMM_PREF_EXT_ID,            // 0xF0A6 ET support CAT ext ID
     app_scl_j1939_build_extid_f0a6,
     NULL,
     NULL,
     NULL
   },
};

/*J1939 Data Link Proprietary A Extended Id Receive Table 02*/
SCL_J1939_CAT_PPGN_ID_HDLR_t  j1939dl_propa_extid_rx_02[] =
{
     {                        /*Engine Cooling Fan Current                   */
          0,                       /*Extended Id Command                     */
          0,                       /*Extended Id That Command Acts On   [N/A]*/
          0,                       /*Handler                                 */
          0,                       /*Pointer To Next Proprietary PGN Cfg[N/A]*/
          0,                       /*Context Parse Function             [N/A]*/
          0                        /*Context Pointer                    [N/A]*/
     }
};

/*****************************************************************************
 the following constant represent values needed for the initialization of
 j1939 component, the application may not want to define those values as
 constants. refer to the file "app_can_init.c" for the detail steps to
 initialize j1939 component and the usage of these values.
-----------------------------------------------------------------------------*/
const int_16 app_scl_j1939_tsk_period = 10;

scl_j1939_link_t  sclj1939dl_ac_link;
unsigned_8        sclj1939dl_ac_idx;
unsigned_8        sclj1939dl_ac_status;
unsigned_8        sclj1939dl_ac_addr;
unsigned_8        sclj1939dl_ac_cnt;

#define SCL_J1939_HALCANV3_A_PKT_LIMIT  50
#define SCL_J1939_HALCANV3_A_PRD_LIMIT  250
#define SCL_J1939_HALCANV3_B_PKT_LIMIT  50
#define SCL_J1939_HALCANV3_B_PRD_LIMIT  250

/*******************************************************************************/
/* J1939 Table 01 - P5 */
/*******************************************************************************/
J1939Table_t t_j193901 =
{
    SCL_J1939_HALCANV3_A_PKT_LIMIT,  /*CAN A Packet Object Maximum       */
    SCL_J1939_HALCANV3_A_PRD_LIMIT,  /*CAN A Packet Reader Object Maximum*/

    SCL_J1939_HALCANV3_B_PKT_LIMIT,  /*CAN B Packet Object Maximum       */
    SCL_J1939_HALCANV3_B_PRD_LIMIT,  /*CAN B Packet Reader Object Maximum*/

    /* Table of addresses for CAN A */
    app_scl_j1939_a_address,
    (unsigned_8) (sizeof(app_scl_j1939_a_address) / sizeof(scl_j1939_addr_tbl_t)),

    /* Proprietary PGN acceptable addresses for CAN A */
    app_scl_j1939_ppgn_enable_a,
    (int_8) (sizeof(app_scl_j1939_ppgn_enable_a) / sizeof(scl_j1939_ppgn_enable_t)),

    /*CAN A Proprietary A Extended Id Receive Table*/
    j1939dl_propa_extid_rx_02,
    (int_8) (sizeof(j1939dl_propa_extid_rx_02) /sizeof(SCL_J1939_CAT_PPGN_ID_HDLR_t)),

    /* Proprietary PGN + Ext ID acceptable messages for CAN A */
    j1939_a_ppgn_ext_msg,
    (unsigned_16)
    (sizeof(j1939_a_ppgn_ext_msg) / sizeof(SCL_J1939_CAT_PPGN_ID_HDLR_t)),
   
    /* Table of addresses for CAN B */
    app_scl_j1939_b_address,
    (unsigned_8) (sizeof(app_scl_j1939_b_address) / sizeof(scl_j1939_addr_tbl_t)),

    /* Proprietary PGN acceptable addresses for CAN B */
    app_scl_j1939_ppgn_enable_a,
    (int_8) (sizeof(app_scl_j1939_ppgn_enable_a) / sizeof(scl_j1939_ppgn_enable_t)),

    /*CAN B Proprietary A Extended Id Receive Table*/
    j1939dl_propa_extid_rx_02,
    (int_8) (sizeof(j1939dl_propa_extid_rx_02) /sizeof(SCL_J1939_CAT_PPGN_ID_HDLR_t)),

    /* Proprietary PGN + Ext ID acceptable messages for CAN B */
    //j1939_a_ppgn_ext_msg, /* check this*/
    NULL,
    (unsigned_16)
    //(sizeof(j1939_a_ppgn_ext_msg) / sizeof(SCL_J1939_CAT_PPGN_ID_HDLR_t))
    0
};

/*J1939 Directory
//     This contains a list of all of the applications that are supported.
//     For each supported application number, this array defines the
//     corresponding data table that should be used.
*/
const ApplicationEntry_t j1939_directory[] =
{
     { 0, &t_j193901 }
};

/*J1939 Maximum Number Of Possible Applications
//     This variable is the number of applications that are supported.
*/
const unsigned_8 j1939_max_applications =
    ( sizeof j1939_directory / sizeof(ApplicationEntry_t) );

/* Address Management Address Claim Status callback */
void app_scl_j1939_ac_status_callback(
        scl_j1939_link_t    Ph_link, /* Identifies the link being initialized */

        unsigned_8      Pu8_ecm_idx, /* The index into the address table associated
                                        with the ECM whose address status is being reported.
                                        (Logical ECM) */

        unsigned_8      Pu8_status, /*  J1939_AC_STAT_CLAIMED - Successfully claimed Pu8_address
                                        J1939_AC_STAT_LOST - Lost Pu8_ddress to a contending claim
                                        J1939_AC_STAT_FAILED - Unable to claim an address. */

        unsigned_8      Pu8_address /* The address value associated with the status.
                                        In the case of J1939_AC_STAT_FAILED, Pu8_address
                                        is set to the preferred address. */
)
{
   sclj1939dl_ac_link   = Ph_link;
   sclj1939dl_ac_idx    = Pu8_ecm_idx;
   sclj1939dl_ac_status = Pu8_status;
   sclj1939dl_ac_addr   = Pu8_address;
   sclj1939dl_ac_cnt++;

   switch (Pu8_status)
   {
      case SCL_J1939_AC_STAT_CLAIMED:
         break;
      case SCL_J1939_AC_STAT_LOST:
         break;
      case SCL_J1939_AC_STAT_FAILED:
         break;
      default:
         // Shouldn't get here!
         break;
   }
}
