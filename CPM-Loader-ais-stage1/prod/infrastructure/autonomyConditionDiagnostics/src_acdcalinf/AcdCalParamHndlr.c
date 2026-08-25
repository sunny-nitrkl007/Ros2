/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: AcdCalParamHndlr.c
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <stdio.h>
#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif

#ifndef  ACDCALPARAMHNDLR_H_
#include <AcdCalParamHndlr.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
/* data needed to display calibration values on ET screen */

unsigned_16 HydOilTempMin = 40;
int_16 LiftCylVelMin = -25;
unsigned_16 LiftCylVelCalValue = 25;

scl_qr_j1939_server_t *app_scl_qr_j1939_server;

 /* J1939 qualified read class */
 scl_qr_j1939_t *app_scl_qr_j1939_1, *app_scl_qr_j1939_2;

 /* const array for holding read entries */
 const scl_qr_j1939_entry_t app_scl_qr_j1939_entries[] =
 {
    {                                   /*0*/
 	  0x0045,                          /* pid */
       SCL_QR_J1939_PID_TYPE_CDLPID, /* pid type */
       SCL_QR_J1939_MIN_POSSIBLE,       /* qualifier type */
       2,                               /* data length */
       SCL_QR_J1939_DL_ENDIAN_LSB       /* datalink endian */
    },
    {
                                         /* 1 */
 	  0x00D01961 ,                          /* pid */
       SCL_QR_J1939_PID_TYPE_CDLPID,    /* pid type */
       SCL_QR_J1939_MIN_POSSIBLE,       /* qualifier type */
       2,                               /* data length */
       SCL_QR_J1939_DL_ENDIAN_LSB       /* datalink endian */
    },
    {                                    /* 2 */
 	  0x00D01961 ,                          /* pid */
       SCL_QR_J1939_PID_TYPE_CDLPID, /* pid type */
       SCL_QR_J1939_CALIBRATION,       /* qualifier type */
       2,                               /* data length */
       SCL_QR_J1939_DL_ENDIAN_LSB       /* datalink endian */
    }
 };

 scl_qr_j1939_config_t app_scl_qr_j1939_config =
 {
    35,            /* max_requests */
 };
 int_least16_t  app_test_int16_min  = INT_LEAST16_MIN;
 uint_least32_t app_test_calibration_offset_default_4bytes = 0x1F1F1F1F;

/******************************************************************************************************************
Function name: app_scl_qr_j1939_server_init

Description:

Parameter Description: This parameter contains the J1939 link to initialize J1939 qualified reads on

Return Description: void
******************************************************************************************************************/

void app_scl_qr_j1939_server_init(scl_j1939_link_t link)
{
   app_scl_qr_j1939_1 = scl_qr_j1939_init(link, &app_scl_qr_j1939_config);
   oel_assert(app_scl_qr_j1939_1 != NULL);

   app_scl_qr_j1939_server =
      scl_qr_j1939_add_server
      (
         app_scl_qr_j1939_1,
         0,                 /* ecm idx */
         23                 /* max entries */
      );
   oel_assert(app_scl_qr_j1939_server != NULL);

}

void app_scl_qr_j1939_server_add_entry(void)
{
	printf("Qualifier Read enabled.\n");

	uint_least8_t i;

	for( i=0;
	        i< ( sizeof(app_scl_qr_j1939_entries) /
	             sizeof(app_scl_qr_j1939_entries[0]) );
	        i++ )
	   {
	      if ( scl_qr_j1939_server_add_entry
	           (
	              app_scl_qr_j1939_server,       /* server */
	              &app_scl_qr_j1939_entries[i],  /* entry to add */
	              app_qualified_data_clbk,       /* callback function to get data */
	              (void*)&app_scl_qr_j1939_entries[i]   /* context pointer */
	           ) == FALSE )
	      {
	         oel_assert(0);
	      }
	   }
}

void app_scl_qr_j1939_server_remove_entry(void)
{
	printf("Qualifier Read disabled.\n");
	uint_least8_t i;

	for( i=0;
	        i< ( sizeof(app_scl_qr_j1939_entries) /
	             sizeof(app_scl_qr_j1939_entries[0]) );
	        i++ )
	   {
	      if ( scl_qr_j1939_server_remove_entry
	           (
	              app_scl_qr_j1939_server,       /* server */
	              &app_scl_qr_j1939_entries[i]  /* entry to add */
	           ) == FALSE )
	      {
	         oel_assert(0);
	      }
	   }
}

/******************************************************************************************************************
Function name: app_qualified_data_clbk

Description: This operation will retrieve the qualified data requested.

Parameter Description: void *context: This parameter points to the context pointer supplied
               when adding a qualified data entry.

Return Description: int_least32_t: This function should return the qualified data
               buffer.
******************************************************************************************************************/

int_least32_t app_qualified_data_clbk(void *context)
{
   printf("app_qualified_data_clbk\n");
   scl_qr_j1939_entry_t *entry = context;

   if(entry == NULL)
   {
      return 0;
   }

   printf("entry->qualifier_type %d  entry->pid %x\n", entry->qualifier_type, entry->pid);

   /* checking condition based on qualifier type */
   if ( entry->qualifier_type == SCL_QR_J1939_MIN_POSSIBLE )
   {
      if(entry->pid == 0x0045 )
    	  {
    	    printf("QR HydOilTempMin %d\n", HydOilTempMin);
    	    return HydOilTempMin;
    	  }
      else if( entry->pid == 0x00D01961 ) {
    	  printf("QR LiftCylVelMin %d\n", LiftCylVelMin);
    	  return LiftCylVelMin;
      }
      else oel_assert(0);

   }
   else if ( entry->qualifier_type == SCL_QR_J1939_CALIBRATION)
   {
      if (entry->pid == 0x00D01961 )
      {
    	  printf("QR LiftCylVelCalValue %d\n", LiftCylVelCalValue);
         return LiftCylVelCalValue;
      }
      else
      {
         oel_assert(0);
      }
   }
   else oel_assert(0);

   return 0;
}

/******************************************************************************************************************
Function name: app_scl_qr_j1939_server_set_values

Description: This operation will set the Qualified Read values

Parameter Description: 	unsigned_16 HydOilTempMin, 
					int_16 LiftCylVelMin, 
					unsigned_16 LiftCylVelCalValue

Return Description: void
******************************************************************************************************************/
void app_scl_qr_j1939_server_set_values(int_16 set_HydOilTempMin, int_16 set_LiftCylVelMin, int_16 set_LiftCylVelCalValue)
{
	HydOilTempMin = set_HydOilTempMin;
	LiftCylVelMin = set_LiftCylVelMin;
	LiftCylVelCalValue = set_LiftCylVelCalValue;
}

