#ifndef APP_J1939_MAP_H_
#define APP_J1939_MAP_H_

#include <oel_pack.h>
#include <clock_proto.h>
#include <cdl2_proto.h>
#include <scl_j1939_struct.h>
#include <scl_j1939_proto.h>
#include <stdlib.h>
#include <cat_std_types.h>
#include "../src_prmsw/passwd.h"
#include "../src_nvm/app_nvm_file_cfg.h"


#ifdef __cplusplus
extern "C" {
#endif

// Product Identification Number
#define PIDF82D_LENGTH (8)

// Engine Serial Number
#define PIDF810_LENGTH (8)

// Equipment ID
#define PIDF81A_LENGTH (17)

// Machine Model
// Max Length is 26 not 27 because the first byte is "n"
#define PIDF9B4_LENGTH (26)

// Calibration Identification Number
// Max Length is 26 not 27 because the first byte is "n"
#define PIDF958_LENGTH (26)

// Payload Legal For Trade Measurement Seal Identifier
// Max Length is 26 not 27 because the first byte is "n"
#define PIDF9D0_LENGTH (26)

// Payload Legal For Trade Measurement Seal Status Change Time/Date
// Max Length is 6 not 7 because the first byte is "n"
#define PIDF9D1_LENGTH (6)

extern int_least16_t pidd02bb6_pdata;
extern int_least16_t pidd02bb7_pdata;
extern unsigned_16 pidd02bb8_pdata;
extern char pidf9b4_pdata[];
extern char pidf9d0_pdata[];
extern unsigned_8 pidf9d1_pdata[];
extern unsigned_16 pidd10049_pdata;
extern const unsigned_8 app_scl_j1939_max_ma_sessions;
extern char pidF82D_pdata[];
extern char pidF810_pdata[];
extern char pidF81A_pdata[];
extern char pidF958_pdata[];
extern int_least16_t app_j1939_59_tx_hdlr( void *tx_pie, unsigned_8 *txData );
extern scl_j1939_ma_param_def_t app_pid_list_ecm_0[];
extern const scl_j1939_ma_param_list_t app_scl_j1939_ma_param_tbl[];
extern const scl_j1939_ma_wr_param_config_t app_map_wr_config[];
extern const scl_j1939_ma_rd_param_config_t app_map_rd_config[];

extern int_least16_t app_j1939_f82d_wr_hndl( void *wr_pie, unsigned_16 len, unsigned_8 *wr_data );
extern int_least16_t app_j1939_f81a_wr_hndl( void *wr_pie, unsigned_16 len, unsigned_8 *wr_data );
extern boolean_t is_valid_product_id(const char* c_str, uint_least8_t len);
extern boolean_t save_product_id_to_nvm(const char* c_str, uint_least8_t len);
extern boolean_t save_equipment_id_to_nvm(const char* c_str, uint_least8_t len);

extern int_least16_t app_j1939_f82d_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f810_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f81a_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f958_tx_hndl(void* tx_pie, unsigned_8* txData);
extern int_least16_t app_j1939_f012_tx_hdlr( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_59_tx_hdlr( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_d10049_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f9b4_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_d02bb6_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_d02bb7_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_d02bb8_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f9d0_tx_hndl( void *tx_pie, unsigned_8 *txData );
extern int_least16_t app_j1939_f9d1_tx_hndl( void *tx_pie, unsigned_8 *txData );

extern unsigned_8 app_scl_j1939_get_ma_num( void );
/* Map Init function */
extern void app_j1939_map_init(scl_j1939_link_t link,int_least8_t ecm_indx);

extern unsigned_32 app_scl_j1939_ma_param_cb_process(
    scl_j1939_link_t  Ph_link,
    unsigned_8     Pu8_session,
    unsigned_8     Pu8_param_type,
    unsigned_32    Pu32_param,
    unsigned_8     Pu8_service,
    unsigned_16    Pu16_length,
    unsigned_16*   Ppu16_data_size,
    unsigned_8*    Pau8_data );
    
#ifdef __cplusplus
}
#endif
#endif

