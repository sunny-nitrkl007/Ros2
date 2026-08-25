/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: AcdCalDeviceidUpdate.c
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif

///******************************************************************************
//FUNCTION update_calibration_device_id
//DESCRIPTION:
//PARAMETER DESCRIPTION:
//RETURN VALUE:
//*******************************************************************************/
//void update_calibration_device_id(unsigned_8 *device_id)
//{
//	//0x96010000B001 ->Calibration Device ID STandAlone PCS
//	#define CHANGE_LEVEL      0x0000
//	#define APPLICATION_TYPE   0x01B0
//	#define APP_ECM_ADDRESS   0x0196
//
//			/* Load ECM Address : LSB first */
//	device_id[3] = (unsigned_8) APP_ECM_ADDRESS & 0xFF;
//	device_id[4] = (unsigned_8) (APP_ECM_ADDRESS >> 8);
//
//			/* Load J1939 Service Tool Support Version LSB first */
//	device_id[5] = (unsigned_8) CHANGE_LEVEL & 0xFF;
//	device_id[6] = (unsigned_8) (CHANGE_LEVEL >> 8);
//			/* Load Application ID - LSB first */
//	device_id[7] = (unsigned_8) APPLICATION_TYPE & 0xFF;
//	device_id[8] = (unsigned_8) (APPLICATION_TYPE >> 8);
//}
