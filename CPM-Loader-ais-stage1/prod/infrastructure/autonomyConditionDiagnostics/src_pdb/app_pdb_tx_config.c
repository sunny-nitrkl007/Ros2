/******************************************************************************************************************
 Copyright 2016 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: scl_pdb_tx_config.c

Description:  SCL PDB  static allocations and static constant configuration. 
              Applications has to build this file 
******************************************************************************************************************/

#include <scl_pdb.h>
#include "app_pdb_tx_tune.h"

/* SPn Tx config */
const uint_least16_t scl_pdb_max_num_tx_spn = SCL_PDB_NUM_TX_SPN;
const scl_pdb_spn_tx_config_t* scl_pdb_spn_tx_config[SCL_PDB_NUM_TX_SPN];

/* PGN Tx config */
const uint_least16_t scl_pdb_max_num_tx_pgn = SCL_PDB_NUM_TX_PGN;
const scl_pdb_pgn_tx_config_t* scl_pdb_pgn_tx_config[SCL_PDB_NUM_TX_PGN];

/* CAT- Ext_id Config*/
const uint_least16_t scl_pdb_max_num_tx_cat_ext = SCL_PDB_NUM_TX_CAT_EXT;
const scl_pdb_cat_ext_id_tx_config_t* scl_pdb_cat_ext_tx_config[SCL_PDB_NUM_TX_CAT_EXT];


