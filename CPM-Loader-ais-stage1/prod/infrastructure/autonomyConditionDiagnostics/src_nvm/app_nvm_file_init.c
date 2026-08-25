/******************************************************************************************************************
 Copyright 2011 - 2015 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_nvm_file_init.c

Description:This file contains startup and initialization code for nvm_file
******************************************************************************************************************/

#ifndef __APP_NVM_FILE_INIT_H__
#include "app_nvm_file_init.h"
#endif

#ifndef __APP_NVM_FILE_CFG_H__
#include "app_nvm_file_cfg.h"
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../src_j1939/app_j1939_map.h"

static void app_nvm_init_product_id(void);
static void app_nvm_init_equipment_id(void);

/******************************************************************************************************************
Function name: app_nvm_file_init

Description:  
Intialzie the nvm_file. 

Parameter Description: 
    
Return Description:     
    None
*****************************************************************************************************************/
void app_nvm_file_init(void)
{
    if(NVM_SUCCESS == nvm_file_init(nvm_file_block_size)) 
    {
        int indx;
        for (indx = 0; indx < nvm_file_block_size; indx++) 
        {
            if (NVM_SUCCESS == nvm_file_block_cfg(&app_nvm_file_cfg[indx])) 
            { 
                if ( (NVM_KEY_SEA1 == (nvm_file_key_e) indx) || (NVM_KEY_SEA2 == (nvm_file_key_e) indx) || (NVM_KEY_SEA3 == (nvm_file_key_e) indx) ) 
                {
                   printf("nvm_block_init %d OMIT for Premium Features\n", indx); 
                }       
                else            
                {
                   if(NVM_SUCCESS == nvm_block_init(
                                               NVM_BLOCK_UNCACHED,              /* block_type, not used for nvm_file */
                                               NULL,                            /* block_cache_ptr, not used for nvm_file */
                                               app_nvm_file_cfg[indx].data_id,  /* unique data id */
                                               0,                               /* priority, not used for nvm_file */
                                               &app_nvm_file_block_table[indx])) 
                   {
                       printf("nvm_block_init %d success\n", indx);
                   }
                   else 
                   {
                       printf("nvm_block_init %d failed\n", indx);
                   }
                }  
            }
            else 
            {
                printf("nvm_file_block_cfg  %d failed\n", indx);
            }
        }
    }
    else 
    {
        printf("nvm_file_init failed\n");
    }

    app_nvm_init_product_id();
    app_nvm_init_equipment_id();
}

static void app_nvm_init_product_id(void) {
	char read_buffer[PIDF82D_LENGTH] = {0};

	if (NVM_SUCCESS == nvm_block_read(
		    app_nvm_file_block_table[6], /* reference handle to access block*/
		    PIDF82D_LENGTH,	             /* size of data block to read      */
			0,	                         /* offset into block to be read    */
			(unsigned_8*)read_buffer,    /* pointer to destination buffer   */
			0)) {                        /* priority */
		if (is_valid_product_id(read_buffer, PIDF82D_LENGTH)) {
			memcpy(pidF82D_pdata, read_buffer, PIDF82D_LENGTH);
		}
	}
}

static void app_nvm_init_equipment_id(void) {
    char read_buffer[PIDF81A_LENGTH] = {0};

    if (NVM_SUCCESS == nvm_block_read(
            app_nvm_file_block_table[12], /* reference handle to access block*/
            PIDF81A_LENGTH,              /* size of data block to read      */
            0,                           /* offset into block to be read    */
            (unsigned_8*)read_buffer,    /* pointer to destination buffer   */
            0)) {                        /* priority */
        if (isalnum(read_buffer[0])) {
            memcpy(pidF81A_pdata, read_buffer, PIDF81A_LENGTH);
        }
    }
}
