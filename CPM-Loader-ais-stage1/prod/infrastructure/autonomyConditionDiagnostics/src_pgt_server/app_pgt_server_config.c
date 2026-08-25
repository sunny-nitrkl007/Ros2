/******************************************************************************************************************
 Copyright 2018 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: csf_pgt_server_config.c

Description:  PGT server  static allocations and static constant configuration. 
              Applications has to build this file 
******************************************************************************************************************/

#include <csf_pgt_server_alloc.h>
#include "app_pgt_server_tune.h"

/*Maximu number of sessions */
uint_least8_t  csf_pgt_server_max_num_sessions = CSF_PGT_SERVER_MAX_SESSION_COUNT;

uint_least16_t csf_pgt_server_max_grp_count = CSF_PGT_SERVER_MAX_GROUP_COUNT;

uint_least16_t csf_pgt_server_max_param_count = CSF_PGT_SERVER_MAX_PARAM_COUNT;

/* UDP data buffer length - Need to be assigned in tune.h based on application requiremnt*/
uint_least16_t csf_pgt_server_max_grp_udp_tx_bytes = CSF_PGT_SERVER_MAX_GRP_UDP_TX_BYTES;

/*Server PID Configuration Array */
csf_pgt_server_param_config_t   csf_pgt_server_param_config_array[CSF_PGT_SERVER_MAX_PARAM_COUNT];

/*This allocates maximum number of session supported.*/
csf_pgt_server_grp_t   csf_pgt_server_grp_array[CSF_PGT_SERVER_MAX_GROUP_COUNT];

/*This allocates maximum number of session supported*/
csf_pgt_server_session_t csf_pgt_server_session_array[CSF_PGT_SERVER_MAX_SESSION_COUNT];

/* Buffer to send UDP data - User configurable length */
uint_least8_t csf_pgt_server_grp_tx_buf[CSF_PGT_SERVER_MAX_GRP_UDP_TX_BYTES] ;
