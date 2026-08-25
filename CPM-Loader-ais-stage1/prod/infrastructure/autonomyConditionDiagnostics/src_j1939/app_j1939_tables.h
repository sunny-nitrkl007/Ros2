/*******************************************************************************
***
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%                                                                        %%
*** %%  COPYRIGHT (C) 2005 CATERPILLAR INC.   ALL RIGHTS RESERVED.            %%
*** %%      This work contains proprietary information which may              %%
*** %%      constitute a trade secret and/or be confidential.                 %%
*** %%                                                                        %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
***
***
*** DESCRIPTION :
***   Header file for application module global data.
***
*** LANGUAGE :                  
***    ANSI C
***
*** NOTES:
***
*** MACRO LIST:
***
*******************************************************************************/
#ifndef __APP_J1939_TABLES_H__
#define __APP_J1939_TABLES_H__

#ifndef __APP_PUBLIC_H__
#include <app_public.h>
#endif
#include <scl_j1939.h>

#include "app_et_j1939_support.h"


typedef struct
{
    /*-------------
    ** NAME fields
    **-------------*/
    unsigned_8  Mu8_industry_group;
    unsigned_8  Mu8_vehicle_system;
    unsigned_8  Mu8_vehicle_system_instance;
    unsigned_8  Mu8_function;
    unsigned_8  Mu8_function_instance;
    unsigned_8  Mu8_ecu_instance;
    unsigned_16 Mu16_manufacturer_code;
    unsigned_32 Mu32_identity_number;

    /*-------------------
    ** Preferred Address
    **-------------------*/
    unsigned_8  Mu8_preferred_address;


} j1939_name_address_t;

/*J1939Table_t
//     Contains the lists of addresses that will be used for each can link.
//     Associated with each address are the PGNs that will be received and
//     transmitted.
*/
typedef struct
{
     /*CAN A Packet Object Maximum
     //     This entry is the maximum number of packets that may be
     //     allocated for packet objects.  These many objects will be
     //     allocated on startup, and no more memory for packets
     //     may be allocated after this time.
     */
     size_t                         can_a_packet_object_max;

     /*CAN A Packet Reader Object Maximum
     //     This entry is the maximum number of packet readers that may be
     //     allocated for packet reader objects.  These many objects will be
     //     allocated on startup, and no more memory for packet readers
     //     may be allocated after this time.
     */
     size_t                         can_a_preader_object_max;

     /*CAN B Packet Object Maximum
     //     This entry is the maximum number of packets that may be
     //     allocated for packet objects.  These many objects will be
     //     allocated on startup, and no more memory for packets
     //     may be allocated after this time.
     */
     size_t                         can_b_packet_object_max;

     /*CAN B Packet Reader Object Maximum
     //     This entry is the maximum number of packet readers that may be
     //     allocated for packet reader objects.  These many objects will be
     //     allocated on startup, and no more memory for packet readers
     //     may be allocated after this time.
     */
     size_t                         can_b_preader_object_max;

     /* Table of addresses for CAN A */
     scl_j1939_addr_tbl_t          *can_a_address_table;
     unsigned_8                     can_a_address_table_num;

     /* Proprietary PGN acceptable addresses for CAN A */
     scl_j1939_ppgn_enable_t       *can_a_ppgn_enable;
     int_8                          can_a_ppgn_enable_num;

     /* Proprietary PGN acceptable messages for CAN A */
     SCL_J1939_CAT_PPGN_ID_HDLR_t  *can_a_ppgn_msg;
     unsigned_16                    can_a_ppgn_msg_num;

     /* Proprietary PGN acceptable Commands + ExtID for CAN A */
     SCL_J1939_CAT_PPGN_ID_HDLR_t  *can_a_ppgn_ext_msg;
     unsigned_16                    can_a_ppgn_ext_msg_num;

     /* Table of addresses for CAN B */
     scl_j1939_addr_tbl_t          *can_b_address_table;
     unsigned_8                     can_b_address_table_num;

     /* Proprietary PGN acceptable addresses for CAN B */
     scl_j1939_ppgn_enable_t       *can_b_ppgn_enable;
     int_8                          can_b_ppgn_enable_num;

     /* Proprietary PGN acceptable messages for CAN B */
     SCL_J1939_CAT_PPGN_ID_HDLR_t  *can_b_ppgn_msg;
     unsigned_16                    can_b_ppgn_msg_num;

     /* Proprietary PGN acceptable Commands + ExtID for CAN B */
     SCL_J1939_CAT_PPGN_ID_HDLR_t  *can_b_ppgn_ext_msg;
     unsigned_16                    can_b_ppgn_ext_msg_num;
     
} J1939Table_t;

extern const ApplicationEntry_t j1939_directory[];
extern const unsigned_8 j1939_max_applications;
extern const int_16 app_scl_j1939_tsk_period;

/* Address Management Address Claim Status callback */
void app_scl_j1939_ac_status_callback(
      scl_j1939_link_t  Ph_link,
      unsigned_8     Pu8_ecm_idx, 
      unsigned_8     Pu8_status,
      unsigned_8     Pu8_address );

#endif /* __SCL_J1939_H__ */
