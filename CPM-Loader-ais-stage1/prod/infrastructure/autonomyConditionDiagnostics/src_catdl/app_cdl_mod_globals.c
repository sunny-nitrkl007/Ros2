/******************************************************************************
**
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*** %%
*** %% COPYRIGHT (C) 2001-2001, 2008,2015 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** %%     This work contains proprietary information which may
*** %%     constitute a trade secret and/or be confidential.
*** %%     Copyright notice is precautionary only and does not
*** %%     imply publication.
*** %%
*** %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
**
*** FILE:
***     app_cdl_mod_globals.c
***
*** DESCRIPTION:
***     This file declares the application's CAT data global storage 
***     variables.  
***
***     The define variables should be customized to suit the
***     applications desires.
***
*** HISTORY:
*** Change 00  05JAN2001 (DFS)
***     Created.
******************************************************************************/

/******************************************************************************
***
*** $Source: $
***
*** $Author: $
***
*** $Locker: $
***
*** $Date: $
***
*** $Revision: $
***
*** $View: $
***
******************************************************************************/

/******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
******************************************************************************/

#include <catdl_public.h>
#include <cdl2_proto.h>
/******************************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
******************************************************************************/

#define MAX_RECEIVE_PARAMETERS 100

/******************************************************************************
***
***    Data types defined for this file
***    -- Struct's, Typedef's, Enum's --
***
******************************************************************************/

/******************************************************************************
***
***    File scope functions defined for this file
***    -- Function Prototypes --
***
******************************************************************************/

/******************************************************************************
***
***    File scope symbols defined for this file
***    -- Symbols --
***
******************************************************************************/

/******************************************************************************
***
***    Global data symbols defined in this file
***    -- Global Symbols --
***
******************************************************************************/


/*****************************************************************************/
/*CAT Data Link Number Of Interrupt Receive Message Buffers
//     These variables are used by the CDL/CDL2 receive routines.
//     Define the number of receive buffers that can be held by the interrupt
//     routines for main level processing.  To assure adequate storage for 
//     incoming traffic, allocate 1.25 buffers for each millisecond in the
//     maximum period between calls to the cdl_receive routine (including worst
//     latency), then add two to allow for unfilled buffers at the start and 
//     end of the loop time.
//     cdl_num_rxb >= (CDL loop time) * (task watch dog loops) * 1.25 + 2
//     cdl_num_rxb >=       10ms      *           2            * 1.25 + 2
//     cdl_num_rxb >= 25 + 2
//     cdl_num_rxb >= 27
*/
Cdl_mbuf_t         cdl_rxmb[(CATDL_MS_LPTIM * 2) + 3];
const unsigned_16  cdl_num_rxb = sizeof(cdl_rxmb)/sizeof(Cdl_mbuf_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Number Of Allocatable Transmit Buffers
//     These variables are used by the CDL/CDL2 transmit routines.
//     This does not affect the number of buffers that can be queued for
//     transmit at one time.
*/
Cdl_mbuf_t         cdl_txmb[55];
const unsigned_16  cdl_num_txb = sizeof(cdl_txmb)/sizeof(Cdl_mbuf_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Receive Parameters
//     These variables are used by the CDL receive routines.
//     Define the largest number of receive parameters that may be 
//     supported by the application.
//     NOTE:  Should be set to 1 when using CDL2 package.
*/
Cdl_rx_pit_t       cdl_rx_pit[1];
const unsigned_16  cdl_num_rx_pit = sizeof(cdl_rx_pit)/sizeof(Cdl_rx_pit_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Transmit Parameters
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     NOTE:  Should be set to 1 when using CDL2 package.
*/
Cdl_tx_pit_t       cdl_tx_pit[1];
const unsigned_16  cdl_num_tx_pit = sizeof(cdl_tx_pit)/sizeof(Cdl_tx_pit_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Number Of Queued Receive Parameter Storage Blocks
//     These are used by the general queued receive parameter handler.
//     NOTE:  Should be set to 1 when using CDL2 package.
*/
Cdl_queued_parm_t  cdl_queued_rx_parm[1];
const unsigned_16  cdl_num_qrpsb = 
                         sizeof(cdl_queued_rx_parm)/sizeof(Cdl_queued_parm_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Receive Parameters
//     These variables are used by the CDL2 receive routines.
//     Define the largest number of receive parameters that may be 
//     supported by the application.
*/
cdl2_rx_pit_t       cdl2_rx_pit[MAX_RECEIVE_PARAMETERS];
const unsigned_16   cdl2_num_rx_pit =
                                     sizeof(cdl2_rx_pit)/sizeof(cdl2_rx_pit_t);

//////////////////////////////////////////////////////////////////////////////////////
/*
In order to support the new read/write protocol for CDL PIDs over J1939, 
we have made a change in the CDL libraries which affects all CDL users.
All CDL users using AFSW_2010_2.0_ALPHA_2 or newer label will have to define two 
more variables and one MACRO.

Please note that if an application does not use the new protocol, it may define
CDL2_NUM_CON_RX_PIT as 1.

For more information regarding the read/write of CDL PIDs over J1939 feature, please
refer to: "\lib_a4\doc\reference_manuals\components\scl\scl_rw_j1939_user_guide.doc"
*/
cdl2_con_rx_pit_t	cdl2_con_rx_pit[70];
const unsigned_16	cdl2_num_con_rx_pit = 
                        sizeof(cdl2_con_rx_pit)/sizeof(cdl2_con_rx_pit_t);
///////////////////////////////////////////////////////////////////////////////////////																			

/*CAT Data Link Maximum Number Of Requested Parameters From Other Modules
//     These variables are used by the CDL2 receive routines.
//     Define the largest number of receive data that may be 
//     supported by the application.
*/
cdl2_rx_data_pit_t  cdl2_rx_data_pit[100];
const unsigned_16   cdl2_num_rx_data_pit =
                           sizeof(cdl2_rx_data_pit)/sizeof(cdl2_rx_data_pit_t);

/*CAT Data Link Maximum Number Of MID Sources For Receive Parameters
//     These variables are used by the CDL2 receive routines.
//     Define the largest number of different ECMs that this ECM will
//     receive data from.
*/
cdl2_rx_data_pit_MID_table_t  cdl2_rx_data_pit_MID_table[6];
const unsigned_16             cdl2_num_rx_data_pit_MID_table =
       sizeof(cdl2_rx_data_pit_MID_table)/sizeof(cdl2_rx_data_pit_MID_table_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Number Of Queued Receive Parameter Storage Blocks
//     These are used by the general queued receive parameter handler.
*/
cdl2_queued_parm_t  cdl2_queued_rx_parm[30];
const unsigned_16   cdl2_num_qrpsb =
                       sizeof(cdl2_queued_rx_parm)/sizeof(cdl2_queued_parm_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Transmit Parameters For Base & Extended PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes base and extended PIDs.
*/
cdl2_tx_pit_t      cdl2_tx_pit[100];
const unsigned_16  cdl2_num_tx_pit =
                                    sizeof(cdl2_tx_pit)/sizeof(cdl2_tx_pit_t);

/*CAT Data Link Maximum Number Of Transmit Parameters For 0xD0 Sublisted PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes 0xD0 Sublisted PIDs.
*/
cdl2_tx_pit_t      cdl2_sublist_D0_tx_pit[100];
const unsigned_16  cdl2_sublist_D0_num_tx_pit =
                         sizeof(cdl2_sublist_D0_tx_pit)/sizeof(cdl2_tx_pit_t);

/*CAT Data Link Maximum Number Of Transmit Parameters For 0xD1 Sublisted PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes 0xD1 Sublisted PIDs.
*/
cdl2_tx_pit_t      cdl2_sublist_D1_tx_pit[100];
const unsigned_16  cdl2_sublist_D1_num_tx_pit =
                         sizeof(cdl2_sublist_D1_tx_pit)/sizeof(cdl2_tx_pit_t);

/*CAT Data Link Maximum Number Of Transmit Parameters For 0xD2 Sublisted PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes 0xD2 Sublisted PIDs.
*/
cdl2_tx_pit_t      cdl2_sublist_D2_tx_pit[1];
const unsigned_16  cdl2_sublist_D2_num_tx_pit =
                         sizeof(cdl2_sublist_D2_tx_pit)/sizeof(cdl2_tx_pit_t);

/*CAT Data Link Maximum Number Of Transmit Parameters For 0xD3 Sublisted PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes 0xD3 Sublisted PIDs.
*/
cdl2_tx_pit_t      cdl2_sublist_D3_tx_pit[1];
const unsigned_16  cdl2_sublist_D3_num_tx_pit =
                         sizeof(cdl2_sublist_D3_tx_pit)/sizeof(cdl2_tx_pit_t);

/*CAT Data Link Maximum Number Of Transmit Parameters For 0xF916 Sublisted PIDs
//     These variables are used by the CDL transmit routines.
//     Define the largest number of transmit parameters that may be 
//     supported by the application.
//     This number includes 0xF916 Sublisted PIDs.
*/
cdl2_tx_pit_t      cdl2_sublist_F916_tx_pit[1];
const unsigned_16  cdl2_sublist_F916_num_tx_pit =
                       sizeof(cdl2_sublist_F916_tx_pit)/sizeof(cdl2_tx_pit_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Write Parameters For Base & Extended PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes base and extended PIDs.
*/
cdl2_wr_pit_t      cdl2_wr_pit[50];
const unsigned_16  cdl2_num_wr_pit = sizeof(cdl2_wr_pit)/sizeof(cdl2_wr_pit_t);

/*CAT Data Link Maximum Number Of Write Parameters For 0xD0 Sublisted PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes 0xD0 Sublisted PIDs.
*/
cdl2_wr_pit_t      cdl2_sublist_D0_wr_pit[50];
const unsigned_16  cdl2_sublist_D0_num_wr_pit =
                          sizeof(cdl2_sublist_D0_wr_pit)/sizeof(cdl2_wr_pit_t);

/*CAT Data Link Maximum Number Of Write Parameters For 0xD1 Sublisted PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes 0xD1 Sublisted PIDs.
*/
cdl2_wr_pit_t      cdl2_sublist_D1_wr_pit[50];
const unsigned_16  cdl2_sublist_D1_num_wr_pit =
                          sizeof(cdl2_sublist_D1_wr_pit)/sizeof(cdl2_wr_pit_t);

/*CAT Data Link Maximum Number Of Write Parameters For 0xD2 Sublisted PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes 0xD2 Sublisted PIDs.
*/
cdl2_wr_pit_t      cdl2_sublist_D2_wr_pit[1];
const unsigned_16  cdl2_sublist_D2_num_wr_pit =
                          sizeof(cdl2_sublist_D2_wr_pit)/sizeof(cdl2_wr_pit_t);

/*CAT Data Link Maximum Number Of Write Parameters For 0xD3 Sublisted PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes 0xD3 Sublisted PIDs.
*/
cdl2_wr_pit_t      cdl2_sublist_D3_wr_pit[1];
const unsigned_16  cdl2_sublist_D3_num_wr_pit =
                          sizeof(cdl2_sublist_D3_wr_pit)/sizeof(cdl2_wr_pit_t);

/*CAT Data Link Maximum Number Of Write Parameters For 0xF916 Sublisted PIDs
//     These variables are used by the CDL write routines.
//     Define the largest number of write parameters that may be 
//     supported by the application.
//     This number includes 0xF916 Sublisted PIDs.
*/
cdl2_wr_pit_t      cdl2_sublist_F916_wr_pit[1];
const unsigned_16  cdl2_sublist_F916_num_wr_pit =
                        sizeof(cdl2_sublist_F916_wr_pit)/sizeof(cdl2_wr_pit_t);
/*****************************************************************************/


/*****************************************************************************/
/*CAT Data Link Maximum Number Of Standard Broadcast Messages To This ECM
//     These variables are used by the CDL2 standard broadcast routines.
//     Define the largest number of standard broadcasts that may be
//     active at the same time.
*/
cdl2_spb_tx_msg_t cdl2_spb_tx_msg[20];
const unsigned_16 cdl2_spb_msg_max =
                             sizeof(cdl2_spb_tx_msg)/sizeof(cdl2_spb_tx_msg_t);

/*CAT Data Link Maximum Number Of Group Broadcast Messages To This ECM
//     These variables are used by the CDL2 group broadcast routines.
//     Define the largest number of group broadcasts that may be
//     active at the same time.
*/
cdl2_pgb_tx_msg_t cdl2_pgb_tx_msg[20];
const unsigned_16 cdl2_pgb_msg_max =
                             sizeof(cdl2_pgb_tx_msg)/sizeof(cdl2_pgb_tx_msg_t);

/*CAT Data Link Memory Allocations For cdl2_transmit Function FIFO
//     These variables are used by the cdl2_transmit function.
//     Defines the largest number of transmit messages that the cdl2_transmit
//     function will have available to it.
//     This must be at least 10 if you are using the CDL2 event interface
//     software and the CBS system (using cdl2_cbs_ginit related).  This must
//     be at least 5 if you are only using the CBS system.
//     Make larger than 10 (or 5) if your application uses the cdl2_transmit
//     function.  cdl2_transmit is a shared resource between the cdl2 datalink
//     software and the application.
*/
cdl2_transmit_fifo_t cdl2_transmit_fifo_buffer[25];
const unsigned_16    cdl2_transmit_buf_size =
                sizeof(cdl2_transmit_fifo_buffer)/sizeof(cdl2_transmit_fifo_t);
/*****************************************************************************/



/*****************************************************************************/
/*CBS ECM Control Table
//     Defines the largest number of ECMs you will able to make parameter
//     requests to.  If you set this large enough, you can add ECMs later
//     by simply updating you config.  If this number is too small, you will
//     have to allocate more memory when you expand your config to include
//     more ECMs.  There is only one ECM entry required to support all
//     requests related to that ECM.
//     
//     NOTE:  If the number is too small for your config, CBS will fail during
//            the initialization.
*/
cdl2_cbs_mid_entry_t  cdl2_cbs_control_mid_tbl[5];
const unsigned_16     cdl2_cbs_max_num_ecm =
                 sizeof(cdl2_cbs_control_mid_tbl)/sizeof(cdl2_cbs_mid_entry_t);
/*****************************************************************************/


/*****************************************************************************/
/*CBS Parameter Group Broadcast Reference Tags
//     These variables allocate the memory used to support PGB requests.
//     Each PGB request requires at least one reference tag(good for multiple
//     parameters) and one parameter entry per parameter.  If you are using a
//     large number of rates, you will use up more reference tags.  If you use
//     a standard rate for most parameters, you will require fewer reference
//     tags(since parameters are grouped).  
//     
//     NOTE:  A size error is not detectable during initialization.  A failure
//            will occur during run time and CBS will not complete the
//            connection with the ECM.
*/
cdl2_pgb_process_reftag_entry_t  cdl2_pgb_process_reftag_table[15];
const unsigned_16                cdl2_pgb_max_num_reftag =
 sizeof(cdl2_pgb_process_reftag_table)/sizeof(cdl2_pgb_process_reftag_entry_t);


/*CBS Parameter Group Broadcast Parameter Request Entries
//     Defines the largest number of parameters you will able to request
//     using the PGB protocol.  If the number is large enough, you can
//     add new requests to your config without any errors.  If the number is
//     too small, adding more config entries will require you to allocate 
//     more memory.  
//     
//     NOTE:  If the number is too small for your config, CBS will fail during
//            the initialization.
*/
cdl2_pgb_process_parm_entry_t    cdl2_pgb_process_parm_table[100];
const unsigned_16                cdl2_pgb_max_num_req_parm =
     sizeof(cdl2_pgb_process_parm_table)/sizeof(cdl2_pgb_process_parm_entry_t);
/*****************************************************************************/


/*****************************************************************************/
/*CBS Asynchronous Data Transfer Maximum Received Parameters
//     Defines the largest number of parameters you will able to request
//     using the ADT protocol.  If the number is large enough, you can
//     add new requests to your config without any errors.  If the number is
//     too small, adding more config entries will require you to allocate 
//     more memory.  
//     
//     NOTE:  If the number is too small for your config, CBS will fail during
//            the initialization.
*/
cdl2_adt_process_reftag_entry_t cdl2_adt_process_reftag_table[25];
const unsigned_16               cdl2_adt_max_num_req_parm =
 sizeof(cdl2_adt_process_reftag_table)/sizeof(cdl2_adt_process_reftag_entry_t);
/*****************************************************************************/


/*****************************************************************************/
/*CBS Asynchronous Data Transfer Parameter Request Entries
//     Defines the largest number of ECMs that will be able to make ADT
//     requests of this ECM.  Set this number large enough to support all
//     ECM's and service tools that may simultaneously request asynchronous
//     updates from this ECM.  One ECM entry is required for one or more
//     requests.  If the number is too small, a request will be rejected
//     with a "no buffers" error to the requesting ECM.
*/
cdl2_adt_status_mid_entry_t cdl2_adt_status_ecm_request_table[3];
const unsigned_16           cdl2_adt_max_num_requestor =
 sizeof(cdl2_adt_status_ecm_request_table)/sizeof(cdl2_adt_status_mid_entry_t);


/*CBS Asynchronous Data Transfer Maximum Transmited Parameters
//     Defines the largest number of parameters that may be requested from
//     this ECM using the ADT protocol.
//     Each requestable data element will require one data status entry
//     to guarantee the data is available to any ECM or service tool.  This
//     constant is independent of the number of ECMs which request the same
//     data since all requests share the same data status entry.  If this
//     number is less than the parameters you could make available to ECMs
//     and service tools then some requestor will get a "no buffers" error
//     when trying to access the data.  
*/
cdl2_adt_status_data_entry_t  cdl2_adt_status_data_table[55];
const unsigned_16             cdl2_adt_max_num_data_status =
       sizeof(cdl2_adt_status_data_table)/sizeof(cdl2_adt_status_data_entry_t);


/*CBS Asynchronous Data Transfer Requestor Data Reference Management
//     Each ADT requested parameter requires three lists to support the
//     transaction.  This symbol is used to set all three lists which
//     must be maintained with correct size proportional to each other.
//     The first list is a reference entry to relate the data status to
//     the appropriate report que.  The second list provides each requested
//     parameter with a buffer for pending reports.  The third list is
//     maintained to allow each requestor ECM to access references owned by
//     it to clear all or clear a specific request entry.
//     
//     This number should be large enough to support all the ADT transactions
//     this ECM will maintain at one time.  If the number is too small some
//     requestor will get a "no buffers" error when trying to access data.
*/
#define CDL2_ADT_MAX_NUM_REF_ENTRIES     55

/*memory allocation for ADT requestor data reference managment*/
cdl2_adt_status_ref_entry_t
                  cdl2_adt_status_ref_table[CDL2_ADT_MAX_NUM_REF_ENTRIES];

cdl2_adt_status_used_ref_t
                  cdl2_adt_status_used_ref_table[CDL2_ADT_MAX_NUM_REF_ENTRIES];

const unsigned_16 cdl2_adt_max_num_ref_entries = CDL2_ADT_MAX_NUM_REF_ENTRIES;


/*required number of report queue entries to support each reference entry
//made by a requestor ECM for ADT status reports
*/
#define CDL2_ADT_MAX_NUM_REPORT_ENTRIES   \
                (CDL2_ADT_MAX_NUM_REF_ENTRIES*CDL2_ADT_MAX_NUM_PENDING_REPORTS)


/*memory allocation for ADT requestor data report queue*/
cdl2_adt_status_report_entry_t
                cdl2_adt_status_report_buffer[CDL2_ADT_MAX_NUM_REPORT_ENTRIES];
const unsigned_16              cdl2_adt_max_num_report_entries =
  sizeof(cdl2_adt_status_report_buffer)/sizeof(cdl2_adt_status_report_entry_t);
/*****************************************************************************/



/*****************************************************************************/
/*CBS Read Poll Request Task Request Queue
//     Defines the largest number of active read requests that can be 
//     enabled at one time.  If the application attempts to activate too many
//     read polling requests, the return from the call will indicate failure.
//     This number should be large enough to support all read polling data
//     updates you will maintain at one time.  If the number is too small some
//     active requests will be rejeceted when trying to enable read polling.
*/
cdl2_cbs_read_poll_entry_t cdl2_cbs_read_poll_buffer[20];
const unsigned_16          cdl2_cbs_max_read_poll_entries =
          sizeof(cdl2_cbs_read_poll_buffer)/sizeof(cdl2_cbs_read_poll_entry_t);
/*****************************************************************************/


/*****************************************************************************/
/*CBS Information Task Request Queue
//     Defines the largest number of requests that the core can post from
//     the data link handlers to the application for data access.
//     If the number is too small, requests will not be passed to the core.
//     This would cause the service tool (or monitor) to re-request the
//     information.
*/
/*
com_inf_input_request_t com_inf_input_fifo_buffer[INF_QUEUE_SIZE];
const int               com_inf_input_que_size =
             sizeof(com_inf_input_fifo_buffer)/sizeof(com_inf_input_request_t);
*/

/*****************************************************************************/
