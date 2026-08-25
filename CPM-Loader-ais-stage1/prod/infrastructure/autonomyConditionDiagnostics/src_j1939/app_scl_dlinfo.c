/*****************************************************************************************************************
** COPYRIGHT (C) 2007 - 2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------------------------------------------
FILE NAME:    app_scl_dlinfo.c
DESCRIPTION:  
*****************************************************************************************************************/
/*****************************************************************************************************************
** -- #Include's --
*****************************************************************************************************************/
#include <oel_assert.h>
#include "app_scl_dlinfo.h"
#include <oel_malloc.h>
#include <stdio.h>
#include "../src_nvm/app_nvm_file_cfg.h"

/*****************************************************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*****************************************************************************************************************/

/****************************************************************************************************************/
/*NOTE: IF ANY OF THE FOLLOWING #define ARE CHANGED, IT IS REQUIRED TO CHANGE                                   
**      NVM SIZE IN AUTONOMYCONDITIONDIAGNOSTICS.rb                                                             
**                                                                                                              
**      NVM SIZE CALCULATION for SCL_DLINFO_J39_INIT_LINK_CONFIG_V1:                                                                            
**         Computes the number of bytes of NVM needed to store the files for one link.                          
**         The app must allocate an NVM block that is at least this slze.                                       
** 
**         SCL_DLINFO_NVM_LINK_HEADER_SIZE +                                  
**         (max_files * (SCL_DLINFO_NVM_FILE_HEADER_SIZE +                    
**         (SCL_DLINFO_NVM_PEER_SIZE * max_peers)))
** 
** 
**         #define SCL_DLINFO_NVM_LINK_HEADER_SIZE   15
**         #define SCL_DLINFO_NVM_FILE_HEADER_SIZE   20
**         #define SCL_DLINFO_NVM_PEER_SIZE          17
**         #define SCL_DLINFO_NVM_PARAM_SIZE         8
** 
** 
**         CAN A:
**          15 + (5 * (20 + (17 * 7)))  = 710 bytes
** 
**          CAN B:
**          15 + (5 * (20 + (17 * 3 ))) = 370 bytes
**                                                                                                              */
/****************************************************************************************************************/

/* file_replacement_criteria: Specifies the criteria to use for determining 
   which file to replace when a new health file is created 
*/
#define LINK1_FILE_REPLACEMENT_CRITERIA SCL_DLINFO_REPLACE_OLDEST_REAL_TIME
#define LINK2_FILE_REPLACEMENT_CRITERIA SCL_DLINFO_REPLACE_OLDEST_REAL_TIME


/* link_instance: Link instance number. This is the instance number that will 
   be transmitted in the BDT file. Its only purpose is to help identify the 
   affected data link to a service technician. The value is stored and 
   transmitted with the health file, but it is not used for any decisions 
   in data link health monitoring 
*/
#define LINK1_LINK_INSTANCE 1
#define LINK2_LINK_INSTANCE 3
#define LINK2SA_LINK_INSTANCE 2


/* max_files: The maximum number of health files that will be stored for this
   link. Old files will be retained until a new data link fault becomes active,
   at which time an inactive file will be replaced with the new data link 
   health file. Each file consumes 20 bytes of NVM storage in addition to the 
   storage for remote peers and missing parameters
*/
#define LINK1_MAX_FILES 5
#define LINK2_MAX_FILES 5


/* max_params: The maximum number of missing parameters that will be recorded 
   in a file. Each parameter will be associated with one of the peer ECUs in 
   the file. Once this number of missing parameters has been stored in the 
   file, no additional missing parameters will be recorded until the file is 
   cleared. Each missing parameter in a file consumes 8 bytes of NVM storage.
   Note: max_params represent
   nvm_data_id: The unique data id for the NVM block, to be used when the NVM 
   block is initialized. The NVM data id must match the data_id for a block in
   the block Init Table (BIT). The scl_dlinfo library will use this data_id to 
   initialize the block and obtain an NVM block handle.
   The block must be large enough to hold all of the files for the link. Use 
   the macro SCL_DLINFO_NVM_BLOCK_SIZE to determine the minimum acceptable 
   block size.s the total number of missing parameters that the
   file can hold. They may bell associated with a single remote peer, or they
   may be divided among multiple remote peers (depending on the sequence in 
   which the missing parameters were detected and reported). Once the file 
   contains this number of parameters, no additional missing parameters will be
   recorded for any remote peer until the file is cleared.
*/
#define LINK1_MAX_PARAMS 30
#define LINK2_MAX_PARAMS 30


/*
 max_peers: The maximum number of remote peer ECUs that can be recorded in 
 the file. Once this number of remote peers has been stored in the file, no 
 additional remote peers will be recorded until the file is cleared. Each 
 remote peer in a file consumes 16 bytes of NVM storage

1) Implement         (GID 0x000000F3)
2) IMU               (GID ???)
3) Steering          (GID 0x000000F6)
4) Secondary Display (GID 0x000004EF)
5) Transmission      (GID 0x00000252)
6) Engine            (0x000000F1)
7 )Product Link      (GID ???)

*/
#define LINK1_MAX_PEERS 7
#define LINK2_MAX_PEERS 3

/*nvm_data_id: The unique data id for the NVM block, to be used when the NVM 
  block is initialized. The NVM data id must match the data_id for a block in
  the block Init Table (BIT). The scl_dlinfo library will use this data_id to 
  initialize the block and obtain an NVM block handle.
  The block must be large enough to hold all of the files for the link. Use 
  the macro SCL_DLINFO_NVM_BLOCK_SIZE to determine the minimum acceptable 
  block size
*/  
//NOTE: If this BLOCK_ID is changed in AutonomyConditionDiagnostics.rb, 
//        it needs to be changed here too
#define APP_NVM_DLINFO_BLK1_BID  (0x00010023)
#define APP_NVM_DLINFO_BLK2_BID  (0x00010024)


/*****************************************************************************************************************
** -- Data Declarations --
*****************************************************************************************************************/
scl_dlinfo_config_t app_dlinfo_config_wrtc = SCL_DLINFO_INIT_CONFIG_W_RTC_V0();


 /* CAN 1: Ph_Link_app */ 
scl_dlinfo_j39_link_config_t app_dlinfo_j39_link_config_CAN_A = SCL_DLINFO_J39_INIT_LINK_CONFIG_V1(LINK1_FILE_REPLACEMENT_CRITERIA,
                                                                                                   LINK1_LINK_INSTANCE,
                                                                                                   LINK1_MAX_FILES,
                                                                                                   LINK1_MAX_PARAMS,
                                                                                                   LINK1_MAX_PEERS,
                                                                                                   APP_NVM_DLINFO_BLK1_BID);

/* CAN 2: TES_Link_app */ 
scl_dlinfo_j39_link_config_t app_dlinfo_j39_link_config_CAN_B = SCL_DLINFO_J39_INIT_LINK_CONFIG_V1(LINK2_FILE_REPLACEMENT_CRITERIA,
                                                                                                   LINK2_LINK_INSTANCE,
                                                                                                   LINK2_MAX_FILES,
                                                                                                   LINK2_MAX_PARAMS,
                                                                                                   LINK2_MAX_PEERS,
                                                                                                   APP_NVM_DLINFO_BLK2_BID);

/* CAN 2: Standalone */
scl_dlinfo_j39_link_config_t app_dlinfo_j39_link_config_CAN_B_SA = SCL_DLINFO_J39_INIT_LINK_CONFIG_V1(LINK2_FILE_REPLACEMENT_CRITERIA,
                                                                                                   LINK2SA_LINK_INSTANCE,
                                                                                                   LINK2_MAX_FILES,
                                                                                                   LINK2_MAX_PARAMS,
                                                                                                   LINK2_MAX_PEERS,
                                                                                                   APP_NVM_DLINFO_BLK2_BID);

/*****************************************************************************************************************
FUNCTION NAME: app_scl_dlinfo_init

DESCRIPTION: Calls the function to Initialize the data link health file system.
METER DESCRIPTION: none

RETURN VALUE: None
*****************************************************************************************************************/
void app_scl_dlinfo_init(scl_health_j39_link_t* link, scl_dlinfo_j39_link_config_t* config)
{
    scl_dlinfo_error_t dlinfo_error;

   /* Register CAN links to dlinfo since dlinfo init was successful */
   dlinfo_error = scl_dlinfo_add_j39_link(link, config); 

   if (dlinfo_error == SCL_DLINFO_SUCCESS)
   {
       printf("scl_dlinfo_add_j39_link SUCCESS\n");
   }
   else
   {
      printf("scl_dlinfo_add_j39_link NOT %d \n", dlinfo_error);
   }
       
   return;
}
