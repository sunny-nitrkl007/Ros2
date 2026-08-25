/******************************************************************************************************************
 Copyright 2009-2014 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: app_flags.h

Description: File contains the flags defined for different platforms
******************************************************************************************************************/
#ifndef APP_FLAGS_H_
#define APP_FLAGS_H_

#ifdef __cplusplus
extern "C"
{
#endif
/* Organised flags for all platforms for better readability and uniformity*/
/* Added HAL_INIT flag - common flag for platforms which provides initialisation
   of platform services modules using one init macro
   If initialisation support is not available on certain platform HAL_INIT is undefined
   
   NVM_TOTAL_DEVICES is set to 1 to indicate the number of nvm/s being tested is 1
   RTC initialisation flags RTC_fl,HAL_RTC_INIT_fl are added on platforms where RTC support
   is available
   RTC_fl is undefined if RTC support is not available
   
   BBRAM support is provided for E4 PI2 using BBRAM_DEVICE_fl
   Currently EEPROM test support is not available for E4 PI2
   
   Define SMALL_MEM flag for platforms that have RAM memory constraints
   
   Define OBD_CRIT_CONFIG_fl flag for including the test configuration related to critical faults
   Disabling the flag will result in failure of tests dependent on critical faults
   #define CDL_FRONT_PANEL **** Flag for CDL front panel support 
   */

#ifdef M1_PI2
   #define CDL_fl
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #define AT25256_fl   
   #define NVM_SIZE_CORRECTED_F1
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif


#ifdef E4_PI2
   #define CDL_fl
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl    
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif   
   #define AT25640_fl   
   #define NVM_TOTAL_DEVICES    1
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif


#ifdef A5L0I1
   #ifdef CDL_fl 
      #undef CDL_f1
   #endif
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5L0I1
   #define NVM_TOTAL_DEVICES    1
   #define AT25640_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5M2I1
   #ifdef CDL_fl 
      #undef CDL_f1
   #endif
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1  
   #define X25LC256_f1
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A4ISM1I1
   #define CDL_fl
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #define AT25640_fl  
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5E2
   #define CDL_fl
   #define SCL_J1939_f1
   #define SCL_railcan_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5E2
   #define NVM_TOTAL_DEVICES    1
   #define AT25640_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef B5M1I1
   #define CDL_fl
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif

   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #define HAL_INIT_fl
   #define SCL_J1939_f1
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #define X24LC512_f1
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5L3I1
   #ifdef CDL_fl 
      #undef CDL_f1
   #endif
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif   
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5L3I1
   #define NVM_TOTAL_DEVICES    1
   #define MRAM_DEVICE_fl
   #define MRAM_ADDRESS   0x30205000
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
   #ifdef SCL_railcan_f1
      #undef SCL_railcan_f1
   #endif
#endif

#ifdef PVD
   #ifdef CDL_fl 
      #undef CDL_f1
   #endif
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl 
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif   
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_PVD
   #define NVM_TOTAL_DEVICES    1
   #define M95512_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef N2_PI1
   #define CDL_fl
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #define AT25256_fl   
   #define BBRAM_DEVICE_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A4G4I1
   #define CDL_fl
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #define AT25256_fl   
   #define BBRAM_DEVICE_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

/* For J1939 and PJ1939 testing, SCL_J1939_f1 to be enabled and CDL_fl to be disabled */
/* For CDL testing, CDL_fl to be enabled and SCL_J1939_f1 to be disabled */
#ifdef D5G1I1
   #define SCL_J1939_f1
   #undef  CDL_fl
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1  
   #define X25LC256_f1
   #define SMALL_MEM
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif



#ifdef A5M4I1
   #define CDL_fl 
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5M4I1
   #define NVM_TOTAL_DEVICES    1  
   #define AT25256_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5E2V2I1
   #define CDL_fl
   #define SCL_J1939_f1
   #define SCL_railcan_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5E2V2I1
   #define NVM_TOTAL_DEVICES    1
   #define AT25256_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5E4V2I1
   #define CDL_fl
   #define SCL_J1939_f1
   #define SCL_railcan_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5E4V2I1
   #define NVM_TOTAL_DEVICES    1
   #define AT25256_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

/* For J1939 and PJ1939 testing, SCL_J1939_f1 to be enabled and CDL_fl to be disabled */
/* For CDL testing, CDL_fl to be enabled and SCL_J1939_f1 to be disabled */

#ifdef WIN32
   #define SCL_J1939_f1
   #define CDL_fl
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define NVM_TOTAL_DEVICES    1
   #undef AT25640_fl
   #define NVM_FILESYS_fl /* Flag for NVM file system */
   #undef J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
#endif

#ifdef A5N2
   #define SCL_J1939_f1
   #define CDL_fl
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #undef AT25640_fl
   #define NVM_FILESYS_fl /* Flag for NVM file system */
   #undef J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
#endif

#ifdef A5M12I1
   #define CDL_fl 
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #undef RTC_fl
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5M12I1
   #define NVM_TOTAL_DEVICES    1  
   #define M95256_fl
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef A5E12I1
   #define CDL_fl 
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #undef RTC_fl
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5E12I1
   #define NVM_TOTAL_DEVICES    1  
   #define M95512_V2_fl	   /* Separate flag used for A5E12*/
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING 
   #define CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif 

#ifdef A5M15I1
   #define SCL_J1939_f1
   #undef  CDL_fl
   #undef OBD_CRIT_CONFIG_fl
   #undef RTC_fl
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #define HAL_INIT HAL_INIT_A5M15I1
   #define NVM_TOTAL_DEVICES    1  
   #define AT25080B_f1
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   /*For EEPROM size less than or equal to 1k */
   #define ULTRA_SMALL_MEM_f1
#endif

#ifdef A5N6
   #define SCL_J1939_f1
   #undef CDL_fl
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #ifdef HAL_INIT_fl
      #undef HAL_INIT_fl
   #endif
   #undef HAL_INIT
   #define NVM_TOTAL_DEVICES    1
   #undef AT25640_fl
   #define NVM_FILESYS_fl /* Flag for NVM file system */
   #undef J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
#endif

#ifdef A5M8I1
   #ifdef CDL_fl 
      #undef CDL_f1
   #endif
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #ifdef RTC_fl
      #undef RTC_fl
   #endif
   #ifdef HAL_RTC_INIT_fl
      #undef HAL_RTC_INIT_fl
   #endif
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   
   /* A5M8I1 doesnot have NVM & RTC support .Part of Internal RAM treated as BBRAM */
   #define INT_MRAM_DEVICE_fl
   #define INT_MRAM_ADDRESS   0x2001DEF0 
   #define INT_MRAM_SIZE      0x2000 
   
   #ifdef SMALL_MEM
      #undef SMALL_MEM
    #endif
   #define J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #ifdef ULTRA_SMALL_MEM_f1
      #undef ULTRA_SMALL_MEM_f1
   #endif
#endif

#ifdef T5R2
   #define SCL_J1939_f1
   #define OBD_CRIT_CONFIG_fl
   #define RTC_fl
   #define HAL_RTC_INIT_fl
   #define HAL_INIT_fl
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #undef AT25640_fl
   #define NVM_FILESYS_fl /* Flag for NVM file system */
   #undef J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #undef CDL_fl
#endif

#ifdef T5R1
   #undef SCL_J1939_f1
   #ifdef HAL_INIT
      #undef HAL_INIT
   #endif
   #define NVM_TOTAL_DEVICES    1
   #undef AT25640_fl
   #define NVM_FILESYS_fl /* Flag for NVM file system */
   #undef J1939_FLASHING  /* Flag for flash initialisation for scl_j1939 */
   #undef CDL_FRONT_PANEL
   #define CDL_fl
#endif

#ifdef __cplusplus
}
#endif
#endif /* #ifndef APP_FLAGS_H_ */

