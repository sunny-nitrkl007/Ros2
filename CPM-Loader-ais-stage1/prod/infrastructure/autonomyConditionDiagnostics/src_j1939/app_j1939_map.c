#include <ctype.h>
#include <oel_pack.h>
#include <string.h>
#include <stdio.h>

#include "../src_catdl/app_catdl_init.h"
#include "../src_j1939/app_can_init.h"

#include "app_j1939_map.h"
#include "catdllib_fid_def.h"

#define TASK_1X_LPTIM (0.010)

#define POWER_UP_DELAY (2.0/TASK_1X_LPTIM)
#define RESEND_TIMER  (1.0/TASK_1X_LPTIM)

char pidf9b4_pdata[PIDF9B4_LENGTH] =  " ";

unsigned_16 pidd10049_pdata = UNKNOWN2U + FMIDNI;
int_least16_t pidd02bb6_pdata = UNKNOWN2S + FMIDNR;
int_least16_t pidd02bb7_pdata = UNKNOWN2S + FMIDNR;
unsigned_16 pidd02bb8_pdata = UNKNOWN2U + FMIDNR;
unsigned_8 pidf9d1_pdata[PIDF9D1_LENGTH] = {0};
char pidf9d0_pdata[PIDF9D0_LENGTH] = "" ;



/*
 * This must be an invalid product id in order to
 * force the user to enter the correct product id
 * prior to installing the premium software.
 * Also known as SEA (Software Enabled Attachment).



 * Billing and cost for SEAs is dependent upon
 * serial number.
 */
char pidF82D_pdata[PIDF82D_LENGTH] = "--------";

char pidF810_pdata[PIDF810_LENGTH] = "ABC12345";   /* needed for SEA, sent as component sn */
char pidF81A_pdata[PIDF81A_LENGTH] = "                 ";
char pidF958_pdata[PIDF958_LENGTH] = "";

const unsigned_8 app_scl_j1939_max_ma_sessions = 2;

boolean_t is_valid_product_id(const char* c_str, uint_least8_t len) {
    // Check ptr
    if (NULL == c_str) {
        return FALSE;
    }

    // Check length
    if (len != PIDF82D_LENGTH) {
        return FALSE;
    }

    // Check for alpha-numeric characters
    for (unsigned ii = 0; ii < 3; ++ii) {
        if (0 == isalnum(c_str[ii])) {
            return FALSE;
        }
    }

    // Check for numeric characters
    for (unsigned ii=3; ii<8; ++ii) {
        if (0 == isdigit(c_str[ii])) {
            return FALSE;
        }
    }

    return TRUE;
}

boolean_t save_product_id_to_nvm(const char* c_str, uint_least8_t len) {
    if (is_valid_product_id(c_str, len)) {
        memcpy(pidF82D_pdata, c_str, len);
        nvm_block_write(
                app_nvm_file_block_table[6], /* block handle */
                PIDF82D_LENGTH,              /* size to be written */
                0,                           /* offset */
                (unsigned_8*)pidF82D_pdata,  /* source buffer */
                0);                          /* priority */
        return TRUE;
    }
    return FALSE;
}

boolean_t save_equipment_id_to_nvm(const char* c_str, uint_least8_t len) {
    // Check length
    if (len == PIDF81A_LENGTH) {
        memcpy(pidF81A_pdata, c_str, len);
        PidWrF81AFlag = 1;
        nvm_block_write(
                app_nvm_file_block_table[12], /* block handle */
                PIDF81A_LENGTH,                          /* size to be written */
                0,                           /* offset */
                (unsigned_8*)pidF81A_pdata,  /* source buffer */
                0);                          /* priority */
        return TRUE;
    }
    return FALSE;
}

int_least16_t app_j1939_f82d_wr_hndl(void *wr_pie, unsigned_16 len, unsigned_8 *wr_data)
{
    int_least16_t ret = 0;

    if ((NULL != wr_data) && (len > 1)) {
        const char* c_str = (char*)&wr_data[1];
        if (save_product_id_to_nvm(c_str, len-1)) {
            ret = len;
        }
    }

    return ret; /* total bytes added */
}

int_least16_t app_j1939_f81a_wr_hndl(void *wr_pie, unsigned_16 len, unsigned_8 *wr_data)
{
    int_least16_t ret = 0;

    if (lft_disable_write_pids) {
        return 0;
    }

    if ((NULL != wr_data) && (len > 1)) {
        const char* c_str = (char*)&wr_data[1];
        if (save_equipment_id_to_nvm(c_str, len-1)) {
            ret = len;
        }
    }

    return ret; /* total bytes added */
}

int_least16_t app_j1939_d10049_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 2;
    (void) tx_pie;
    
    OEL_PACK_BE_16_NO_INCR(txData, (unsigned_16) pidd10049_pdata);
    return( len ); /* total bytes added */
}

int_least16_t app_j1939_d02bb6_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 2;
    (void) tx_pie;
    
    OEL_PACK_LE_16_NO_INCR(txData, (int_least16_t) pidd02bb6_pdata);
    return( len ); /* total bytes added */
}

int_least16_t app_j1939_d02bb7_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 2;
    (void) tx_pie;
    
    OEL_PACK_LE_16_NO_INCR(txData, (int_least16_t) pidd02bb7_pdata);
    return( len ); /* total bytes added */
}

int_least16_t app_j1939_d02bb8_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 2;
    (void) tx_pie;
    
    OEL_PACK_LE_16_NO_INCR(txData, (unsigned_16) pidd02bb8_pdata);
    return( len ); /* total bytes added */
}



int_least16_t app_j1939_f9b4_tx_hndl( void *tx_pie, unsigned_8 *txData)
{
    unsigned_8 len = 0;
    (void) tx_pie;
    
    /* pidf9b4_pdata can only consists upto 26 character.
       If pidf9b4_pdata contains less than 26 characters, 
       for loop will be will terminated by null 
       terminating character since machineModelDetail.c_str()
       has terminating null-character ('\0') at the end. 
    */
    for (int i = 0; 
        ((i < PIDF9B4_LENGTH) && (pidf9b4_pdata[i] != '\0')); 
         i++)
     {
        txData[i] = pidf9b4_pdata[i];
        len++; // length starts with 1
     }    
    
    return( len ); /* total bytes added */
}

int_least16_t app_j1939_f82d_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = PIDF82D_LENGTH;
    (void) tx_pie;

    memcpy( txData, pidF82D_pdata, len);

    return( len ); /* total bytes added */
}

int_least16_t app_j1939_f810_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = PIDF810_LENGTH;
    (void) tx_pie;

    memcpy( txData, pidF810_pdata, len);

    return( len ); /* total bytes added */
}


int_least16_t app_j1939_f81a_tx_hndl( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = PIDF81A_LENGTH;
    (void) tx_pie;

    memcpy( txData, pidF81A_pdata, len);

    return( len ); /* total bytes added */
}

int_least16_t app_j1939_f958_tx_hndl(void* tx_pie, unsigned_8* txData)
{
    unsigned_8 len = 0;
    (void)tx_pie;

    /* pidf958_pdata can only consists up to 26 characters.
       If pidf958_pdata contains less than 26 characters,
       the for loop will be will terminated by a null
       terminating character.
    */
    for (int i = 0; ((i < PIDF958_LENGTH) && (pidF958_pdata[i] != '\0')); ++i) {
        txData[i] = pidF958_pdata[i];
        ++len;
    }

    return len; /* total bytes added */

}

int_least16_t app_j1939_f012_tx_hdlr( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 1;
    (void) tx_pie;

    txData[0] =   (uint_least8_t)0x1;

    return( len );
}


int_least16_t app_j1939_59_tx_hdlr( void *tx_pie, unsigned_8 *txData )
{
    unsigned_8 len = 2;
    (void) tx_pie;

    txData[0] =   (uint_least8_t)passwd_pkg_work.ttt & 0xFF;
    txData[1] =  (uint_least8_t)(passwd_pkg_work.ttt & 0xFF00);

    return( len );
}


int_least16_t app_j1939_f9d0_tx_hndl( void *tx_pie, unsigned_8 *txData)
{
    unsigned_8 len = 0;
    (void) tx_pie;
    
    /* pidf9d0_pdata can only consists upto 26 character.
       If pidf9d0_pdata contains less than 26 characters, 
       for loop will be will terminated by null 
       terminating character since pidf9d0_pdata.c_str()
       has terminating null-character ('\0') at the end. 
    */
    for (int i = 0; 
        ((i < PIDF9D0_LENGTH) && (pidf9d0_pdata[i] != '\0')); 
         i++)
     {
        txData[i] = pidf9d0_pdata[i];
        len++; // length starts with 1
     }    
    
    return( len ); /* total bytes added */
}


int_least16_t app_j1939_f9d1_tx_hndl( void *tx_pie, unsigned_8 *txData)
{
    // First data for this PID is number of bytes that follow. 
    unsigned_8 len = PIDF9D1_LENGTH;
    (void) tx_pie;
    
    /* pidf9d1_pdata can only consists 6 character */
     memcpy( txData, pidf9d1_pdata, PIDF9D1_LENGTH);
    
    return( len ); /* total bytes added */
}


/*********************************************************************************
 Memory Access Parameter and Security Definitions are used to associate
 an MA parameter (SPN or PID) with a security level.
 This data is used in the MA Parameter table entries in the lists of parameters
 that are supported by a virtual ECM.  Members:

   Mu32_param  The SPN or PID value associated with this parameter.
   Mu8_read_security The security level that is required to read this parameter.
   Mu8_write_security   The security level that is required to write this parameter.

 */

const scl_j1939_ma_param_list_t app_scl_j1939_ma_param_tbl[] =
{
    {
        /* unsigned_16 Mu16_num_spn; */
        0,
        /* scl_j1939_ma_param_def_t* Mpast_spn_list; */
        NULL,

        /* unsigned_16 Mu16_num_pid; */
        0,
        /* scl_j1939_ma_param_def_t* Mpast_pid_list; */
        NULL,

        /* scl_j1939_ma_param_process_cb_t  *process_cb_f; */
        (scl_j1939_ma_param_process_cb_t*)app_scl_j1939_ma_param_cb_process,

        /* size_t Msz_max_overrides */
        5,

        0,       /* Mu16_num_rd_spn_hdrl */
        0,       /* Mu16_num_wr_spn_hdrl */
        0,      /* Mu16_num_ovr_spn_hdrl */
        30,/* Mu16_num_rd_pid_hdrl */
        30,/* Mu16_num_wr_pid_hdrl */
        0    /* Mu16_num_ovr_pid_hdrl */

    }
};

const scl_j1939_ma_wr_param_config_t app_map_wr_config[] =
{
    {
        { 0xf82d,0,0 },
        app_j1939_f82d_wr_hndl,
        NULL
    },
};

const scl_j1939_ma_rd_param_config_t app_map_rd_config[] =
{
    {
        { 0x59,0,0 },
        app_j1939_59_tx_hdlr,
        NULL
    },
    {
        { 0xf82d, 0, 0 },
        app_j1939_f82d_tx_hndl,
        NULL
    },
    {
        { 0xf81a,0,0 },
        app_j1939_f81a_tx_hndl,
        NULL
    },
    {
        { 0xf810,0,0 },
        app_j1939_f810_tx_hndl,
        NULL
    },
    {
        { 0xf958,0,0 },
        app_j1939_f958_tx_hndl,
        NULL
    },
    {
        { 0xf9b4,0,0 },
        app_j1939_f9b4_tx_hndl,
        NULL
    },
    {
        { 0xd10049,0,0 },
        app_j1939_d10049_tx_hndl,
        NULL
    },
    {
        { 0xd02bb6,0,0 },
        app_j1939_d02bb6_tx_hndl,
        NULL
    },
    {
        { 0xd02bb7,0,0 },
        app_j1939_d02bb7_tx_hndl,
        NULL
    },
    {
        { 0xd02bb8,0,0 },
        app_j1939_d02bb8_tx_hndl,
        NULL
    },
    {
        { 0xf9d0,0,0 },
        app_j1939_f9d0_tx_hndl,
        NULL
    },
    {
        { 0xf9d1,0,0 },
        app_j1939_f9d1_tx_hndl,
        NULL
    }
};

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_scl_j1939_get_ma_num
------------------------------------------------------------------------------*/
unsigned_8 app_scl_j1939_get_ma_num( void )
{
    return ( sizeof(app_scl_j1939_ma_param_tbl) /
        sizeof(app_scl_j1939_ma_param_tbl[0]) );
}

void app_j1939_map_init(scl_j1939_link_t link,int_least8_t ecm_indx)
{
    unsigned_8 i;

    for (i = 0; i < sizeof(app_map_rd_config) / sizeof(app_map_rd_config[0]);  i++)
    {
        scl_j1939_add_ma_rd_cdlpid(link, ecm_indx, &app_map_rd_config[i]);
    }

    for (i = 0; i < sizeof(app_map_wr_config) / sizeof(app_map_wr_config[0]);  i++)
    {
        scl_j1939_add_ma_wr_cdlpid(link, ecm_indx, &app_map_wr_config[i]);
    }

    /*
     * Adds MAP read and write support for RTC and Time Zone Info
     */
    clock_rtc_j1939_init(link, ecm_indx);
    return;
}

/*------------------------------------------------------------------------------
 *** FUNCTION NAME: app_scl_j1939_ma_param_cb_process
------------------------------------------------------------------------------*/
unsigned_32 app_scl_j1939_ma_param_cb_process(
    scl_j1939_link_t  Ph_link,
    unsigned_8     Pu8_session,
    unsigned_8     Pu8_param_type,
    unsigned_32    Pu32_param,
    unsigned_8     Pu8_service,
    unsigned_16    Pu16_length,
    unsigned_16*   Ppu16_data_size,
    unsigned_8*    Pau8_data )
{
    /* touch unused arguments to avoid compiler warnings, remove these
      statements if arguments are used */
    (void)Ph_link;
    (void)Pu8_session;
    (void)Pu8_param_type;
    (void)Pu32_param;
    (void)Pu8_service;
    (void)Pu16_length;
    (void)Ppu16_data_size;
    (void)Pau8_data;

    return MA_TRANS_ERROR_SECURITY_GENERAL;
}

