/*******************************************************************************
 *** Copyright 2017 Caterpillar Inc.  All rights reserved.
 ***
 *** FILE NAME: app_disp_brightness.c
 ***
 ***
 *** DESCRIPTION:
 ***
 *******************************************************************************/

/******************************************************************************
 ***
 ***    External function prototypes, data, & symbols
 ***    -- #Include's --
 ***
 ******************************************************************************/
#ifndef __APP_DISP_BRIGHTNESS_H__
#include "app_disp_brightness.h"
#endif

#ifndef __APP_CATDL_INIT_H__
#include "../src_catdl/app_catdl_init.h"
#endif

#ifndef __APP_CAN_INIT_H__
#include "../src_j1939/app_can_init.h"
#endif

#include <taskPlugins/interfaces/PlatformDefinesC.h>

/******************************************************************************
 ***
 ***    Constants defined for this file
 ***    -- #Define's --
 ***
 ******************************************************************************/

/******************************************************************************
 ***
 ***    Data types defined for this file
 ***    -- Struct's, Typedef's, Enum's --
 ***
 ******************************************************************************/

/* counter to track requested sending message */
unsigned_16 dispbrightness_request_timer;
bool_t disp_brightness_received = FALSE;

/******************************************************************************
 ***
 ***    File scope functions defined for this file
 ***    -- Function Prototypes --
 ***
 ******************************************************************************/

static
int_16 j1939_dispbrightness_rx_ovr_m_hdlr ( cdl2_con_rx_data_link_t data_link_type,
        unsigned_32 con_index,
        cdl2_con_rx_event_t ack_event,
        void *rx_pie,
        unsigned_8 sid,
        unsigned_8 did,
        unsigned_32 data_ID,
        unsigned_8 pdl,
        unsigned_8 *data );

/******************************************************************************
 ***
 ***    File scope symbols defined for this file
 ***    -- Symbols --
 ***
 ******************************************************************************/


unsigned_8 pidD0022B_pdata[6] = "EMPTY!";
Cdl_gen_tx_pie_t pidD0022B_pie = { 6, pidD0022B_pdata };


/* Application Get Disp Brightness Request Receive Parameter   */
Cdl_gen_ovr_parm_06_t  appget_dispbrightness_request_parm;

/* Application Get Disp Brightness Request Receive Parameter Information Entry */
const Cdl_gen_ovr_pie_06_t   appget_dispbrightness_request_pie =
{
        /*Parameter To Receive Size  */
        sizeof(appget_dispbrightness_request_parm.data),

        /*Address Of Parameter Data  */
        &appget_dispbrightness_request_parm
};
/******************************************************************************
 ***
 ***    Global data symbols defined in this file
 ***    -- Global Symbols --
 ***
 ******************************************************************************/

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_disp_brightness_update_j1939
 ***
 *** DESCRIPTION:
 ***   Initialize routine for the reading display brightness.
 ***  - Send request reading data from UI
 ***  - received message register
 ***
------------------------------------------------------------------------------*/
void appget_disp_brightness_init_j1939(void)
{
    /* Register Message To Be Transmitted */
    //cdl2_add_to_tx_pit(0xD0022B,
    //        /*fixed byte non-reversed data handler*/
    //        cdl2_tx_f_hdlr,
    //        &pidD0022B_pie);

    /* Register Message To Be Received */
    cdl2_add_to_con_rx_pit( 0xD0022B,
            j1939_dispbrightness_rx_ovr_m_hdlr,
            &appget_dispbrightness_request_pie );

    dispbrightness_request_timer = (unsigned_16)(POWER_UP_DELAY);

    return;
}


/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_disp_brightness_update_j1939
 ***
 *** DESCRIPTION:
 ***   Periodic update routine for the Disp Brightness.
 ***  - Send request reading data from D6 ECM
 ***  - Compare if new value is
 ***   configured through (ET) and update
 ***
------------------------------------------------------------------------------*/
void appget_disp_brightness_update_j1939(void)
{   

   return;
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: j1939_dispbrightness_rx_ovr_m_hdlr
 ***
 *** DESCRIPTION:
 ***  - J1939 rx hander on display brightness request
 ***
------------------------------------------------------------------------------*/
static
int_16 j1939_dispbrightness_rx_ovr_m_hdlr ( cdl2_con_rx_data_link_t data_link_type,
        unsigned_32 con_index,
        cdl2_con_rx_event_t ack_event,
        void *rx_pie,
        unsigned_8 sid,
        unsigned_8 did,
        unsigned_32 data_ID,
        unsigned_8 pdl,
        unsigned_8 *data )
{
    Cdl_gen_ovr_parm_27_t *param;
    int_16 status;
    unsigned_16 l_index;

    /* unused parameters, hence use void to avoid compiler warnings */
    (void)data_link_type;
    (void)con_index;
    (void)ack_event;
    (void)data_ID;

    param = ((Cdl_gen_ovr_pie_27_t *)rx_pie)->parm;

    param->counter++;         /* increment the rception counter */
    param->sid = sid;        /* store the SID in the parameter area */
    param->did = did;        /* store the DID in the parameter area */
    param->pdl = pdl;         /* store the parameter data length */

    /* move the data */
    for (l_index=0; l_index<pdl; l_index++)
    {

        param->data[l_index] = data[l_index];
    }

    status = CDL2_OK;
    disp_brightness_received = TRUE;

    return(status);
}

