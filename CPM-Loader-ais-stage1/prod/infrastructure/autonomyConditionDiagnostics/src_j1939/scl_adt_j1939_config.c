/******************************************************************************************************************
 Copyright 2012-2016 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: scl_adt_j1939_config.c

Description:  This file contains configuration data for the ADT J1939 library. This file has to be included by the
 application.
******************************************************************************************************************/
#include <scl_adt_j1939_alloc.h>
#include <scl_adt_j1939_tune.h>


const uint_least16_t scl_adt_j1939_max_link_count = SCL_ADT_J1939_MAX_LINK_COUNT;
 
#if( SCL_ADT_J1939_MAX_LINK_COUNT != 0 )
   scl_adt_j1939_link_t scl_adt_j1939_link_array[SCL_ADT_J1939_MAX_LINK_COUNT] = { 0 };
   SCL_J1939_CAT_PPGN_ID_HDLR_t scl_adt_j1939_request[SCL_ADT_J1939_MAX_LINK_COUNT]= { 0 };
   SCL_J1939_CAT_PPGN_ID_HDLR_t scl_adt_j1939_response[SCL_ADT_J1939_MAX_LINK_COUNT]= { 0 };
   SCL_J1939_CAT_PPGN_ID_HDLR_t scl_adt_j1939_data[SCL_ADT_J1939_MAX_LINK_COUNT]= { 0 };
   SCL_J1939_CAT_PPGN_ID_HDLR_t scl_adt_j1939_data_ack[SCL_ADT_J1939_MAX_LINK_COUNT]= { 0 };
   SCL_J1939_CAT_PPGN_ID_HDLR_t scl_adt_j1939_svr_ctrl[SCL_ADT_J1939_MAX_LINK_COUNT]= { 0 }; 
   
#endif

const uint_least16_t scl_adt_j1939_max_ecm_count = (SCL_ADT_J1939_MAX_LINK_COUNT * SCL_ADT_J1939_MAX_ECM_COUNT);

#if( SCL_ADT_J1939_MAX_ECM_COUNT != 0 ) 
   scl_adt_j1939_ecm_t scl_adt_j1939_ecm_array[SCL_ADT_J1939_MAX_LINK_COUNT * SCL_ADT_J1939_MAX_ECM_COUNT];
#endif

const uint_least16_t scl_adt_j1939_max_server_count = SCL_ADT_J1939_MAX_SERVER_COUNT;

#if( SCL_ADT_J1939_MAX_SERVER_COUNT != 0 )
   scl_adt_j1939_server_t scl_adt_j1939_server_array[SCL_ADT_J1939_MAX_LINK_COUNT*SCL_ADT_J1939_MAX_SERVER_COUNT];
   
#endif

const uint_least16_t scl_adt_j1939_max_server_grp_count = SCL_ADT_J1939_MAX_SERVER_GRP_COUNT;

#if( SCL_ADT_J1939_MAX_SERVER_GRP_COUNT != 0 )
   scl_adt_j1939_server_grp_t scl_adt_j1939_server_grp_array[SCL_ADT_J1939_MAX_LINK_COUNT*SCL_ADT_J1939_MAX_SERVER_GRP_COUNT];
   
#endif

const uint_least16_t scl_adt_j1939_max_client_count = SCL_ADT_J1939_MAX_CLIENT_COUNT;

#if( SCL_ADT_J1939_MAX_CLIENT_COUNT != 0 )
   scl_adt_j1939_client_t scl_adt_j1939_client_array[SCL_ADT_J1939_MAX_LINK_COUNT*SCL_ADT_J1939_MAX_CLIENT_COUNT];   
#endif

const uint_least16_t scl_adt_j1939_max_client_grp_count =  SCL_ADT_J1939_MAX_CLIENT_GRP_COUNT;

#if( SCL_ADT_J1939_MAX_CLIENT_GRP_COUNT != 0 )
   scl_adt_j1939_client_grp_t scl_adt_j1939_client_grp_array[SCL_ADT_J1939_MAX_LINK_COUNT*SCL_ADT_J1939_MAX_CLIENT_GRP_COUNT];   
#endif
#if( SCL_ADT_J1939_MAX_SRV_GRP_FIFO < SCL_ADT_J1939_DEFAULT_SRV_GRP_FIFO )
   const uint_least16_t scl_adt_j1939_max_srv_grp_fifo = SCL_ADT_J1939_DEFAULT_SRV_GRP_FIFO;
#else
   const uint_least16_t scl_adt_j1939_max_srv_grp_fifo = SCL_ADT_J1939_MAX_SRV_GRP_FIFO;
#endif 
