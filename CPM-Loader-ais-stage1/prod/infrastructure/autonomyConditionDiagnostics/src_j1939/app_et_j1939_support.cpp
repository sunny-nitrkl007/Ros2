/*******************************************************************************
***     COPYRIGHT (C) 2011-2015 CATERPILLAR INC. ALL RIGHTS RESERVED.
*** ****************************************************************************
***
***  FILE:  app_et_j1939_support.c
***
***  DESCRIPTION: ET support over J1939
******************************************************************************/

/******************************************************************************
***
***    External function prototypes, data, & symbols
***    -- #Include's --
***
******************************************************************************/
#include <stdio.h>
#include <string.h>

#include <oel_pack.h>
#include <hal_boot_proto.h>
extern "C" {
#include <scl_j1939_pri.h> // This is needed because scl_j1939_mbuf.h needs it.
#include <scl_j1939_mbuf.h>
}

#include <ais/task/load_version.h>

#include "app_can_init.h"
#include "app_et_j1939_support.h"


/*******************************************************************************
***
***    Constants defined for this file
***    -- #Define's --
***
*******************************************************************************/

/******************************************************************************* 
*** 
***
***    Data types defined for this file
***    -- Struct's, Typedef's, Enum's --
***
*******************************************************************************/
char app_description_dl_tx_pie[APPLICATION_TEXT_MAX_SIZE];
char ecm_serial_number[ECM_SERIAL_NUMBER_MAX_SIZE]; /* ECM Serial Number */
char ecm_part_number[ECM_PART_NUMBER_MAX_SIZE]; /* ECM Part Number */
char software_group_part_number[SOFTWARE_GROUP_PN_LENGTH];
char software_group_date_code[SOFTWARE_GROUP_DATE_CODE_LENGTH];
char WeighRangeBottomRqstFlg;
char WeighRangeSizeRqstFlg;
char NewLpsSaWeighScsTxIn_ = 0;
int NewLpsSaJobMgrScsTxIn_ = 0;
char PendingLpsSaWeighScsReqOut_ = 0;
char PendingLpsSaJobMgrScsReqOut_ = 0;

/******************************************************************************
***
***   File scope symbols defined for this file
***   -- Symbols --
***
******************************************************************************/

/******************************************************************************
NAME:        app_scl_j1939_build_extid_f016   
DESCRIPTION: Builds the PGN EF00 for Cat Ext ID F016.  
NOTES:       It is the responsibility of this function to make sure, the PGN
             will be sent. Also make sure,the Destination address is extracted
             from the message.
61184 $F016 a b b c c d d
  a - Set to 0 for CDL Device ID Format
  b b - CDL Module ID for ECM Function (LSB First)
  c c - Service Tool Support Change Level (LSB First)
  d d - Application Code (LSB First)
GLOBALS:     
*****************************************************************************/    

void app_scl_j1939_build_extid_f016(scl_j1939_mbuf_hdr_t *mbuf_ptr) 
{

	unsigned_16 length = 9; //The length and data size are hardcoded.
	unsigned_8 data[9];
	memset( data, 0xFF, length );
    scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));
   
	/* Update device id data for Cat Ext ID $F016 */
	 data[ 0 ] = (unsigned_8)  0xF0;
	 data[ 1 ] = (unsigned_8)  0x16;

	 //Device ID version Supported. Set to 0 for CDL Device ID Format
	 data[ 2 ] = (unsigned_8)  SCL_J1939_TEST_DEVICE_ID_VER;

	 data[ 3 ] = (unsigned_8)(j1939_mid & 0x00FF);
	 data[ 4 ] = (unsigned_8)(j1939_mid>>8 & 0x00FF);

	 /*save device service tool change level(LSB first)*/
	 data[ 5 ] = (unsigned_8)(st_chg_lvl    & 0x00FF);
	 data[ 6 ] = (unsigned_8)(st_chg_lvl>>8 & 0x00FF);

	 /*save device service tool application number(LSB first)*/
	 data[ 7 ] = (unsigned_8)(st_app_num    & 0x00FF);
	 data[ 8 ] = (unsigned_8)(st_app_num>>8 & 0x00FF);

	(void)scl_j1939_fill_mbuf_data(mbuf_ptr,data,length);

    return;
}

/*******************************************************************************/
/******************************************************************************
NAME:        app_scl_j1939_build_extid_f003
61184 $F003 a b c…c [a b c…c] … [a b c…c]
  Where: a – Data Identifier   b – Data Length (in bytes)   c – Data
  Note: [a b c…c] comprises one flash information record, of which there may be multiples
GLOBALS:
*****************************************************************************/
void app_scl_j1939_build_extid_f003(scl_j1939_mbuf_hdr_t *mbuf_ptr)
{
    unsigned_8 data[128];
    memset( data, 0xFF, sizeof(data) );
    scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));
    unsigned_16 ii = 0;

    data[ii++] = (unsigned_8)  0xF0;
    data[ii++] = (unsigned_8)  0x03;

    // flash level (0x01)
    data[ii++] = (unsigned_8)0x01;   // Data Identifier
    data[ii++] = (unsigned_8)0x01;   // Data length
    data[ii++] = (unsigned_8)0x02;   // Data

    // ECM flash method (0x03)
    data[ii++] = (unsigned_8)0x03;
    data[ii++] = (unsigned_8)0x02;
    data[ii++] = (unsigned_8)0x00;
    data[ii++] = (unsigned_8)0x04;

    // ECM location code (0x40)
    data[ii++] = (unsigned_8)0x40;
    data[ii++] = (unsigned_8)0x02;
    data[ii++] = (unsigned_8)0x00;
    data[ii++] = hal_boot_read_location_code();

    // ECM component ID (0x41)
    const int_least16_t FlashComponentIdentifier = get_flashComponentIdentifier();
    data[ii++] = (unsigned_8)0x41;
    data[ii++] = (unsigned_8)0x02;
    data[ii++] = (unsigned_8)((FlashComponentIdentifier >> 8) & 0xFF);
    data[ii++] = (unsigned_8)(FlashComponentIdentifier & 0xFF);

    // ECM application ID (0x42)
    const int_least16_t FlashApplicationIdentifier = get_flashApplicationIdentifier();
    data[ii++] = (unsigned_8)0x42;
    data[ii++] = (unsigned_8)0x02;
    data[ii++] = (unsigned_8)((FlashApplicationIdentifier >> 8) & 0xFF);
    data[ii++] = (unsigned_8)(FlashApplicationIdentifier & 0xFF);

    // ECM Aftermarket Id Status
    data[ii++] = (unsigned_8)0x45;
    data[ii++] = (unsigned_8)0x03; // Length
    data[ii++] = (unsigned_8)0xFF; // No Limit to Number of Aftermarket Id Changes
    { // Get Aftermarket ID
        hal_aftermarket_id_t aftermarket_id = hal_boot_get_aftermarket_id();
        data[ii++] = (unsigned_8)((aftermarket_id >> 8) & 0xFF);
        data[ii++] = (unsigned_8)(aftermarket_id & 0xFF);
    }

    // ECM Serial Number (0x80)
    data[ii++] = (unsigned_8)0x80;
    data[ii++] = (unsigned_8)0x10;
    for (int idx = 0; idx < ECM_SERIAL_NUMBER_MAX_SIZE; ++idx) {
        data[ii++] = ecm_serial_number[idx];
    }

    // Last service tool to flash ECM (0x81)
    data[ii++] = (unsigned_8)0x81;
    data[ii++] = (unsigned_8)0x08;
    for (int j = 0; j < 8; ++j) {
        data[ii++] = (unsigned_8)0x2A;
    }

    // ECM part number (0x82)
    data[ii++] = (unsigned_8)0x82;
    data[ii++] = (unsigned_8)0x0C;
    for (int idx = 0; idx < ECM_PART_NUMBER_MAX_SIZE; ++idx) {
        data[ii++] = ecm_part_number[idx];
    }

    // Customer ECM Part Number (0x83)
    data[ii++] = (unsigned_8)0x83;
    data[ii++] = (unsigned_8)0x10;
    { // Get customer ecm part number
        // If the customer part number is less than 16 characters long,
        // the field should be end-filled with ASCII spaces ($20).
        hal_customer_partno_t customer_partno = hal_boot_get_customer_pn();
        int idx = 0;
        while (idx < 16) {
            if ((idx >= customer_partno.len) ||
                    (0 == customer_partno.str[idx])) {
                break;
            }

            data[ii++] = customer_partno.str[idx++];
        }

        while (idx < 16) {
            data[ii++] = 0x20;
            ++idx;
        }
    }

    // Software Type and Part Number
    data[ii++] = (unsigned_8)0x84;
    data[ii++] = (unsigned_8)0x0B;
    data[ii++] = (unsigned_8)0x02;
    for (int idx = 0; idx < SOFTWARE_GROUP_PN_LENGTH; ++idx) {
        data[ii++] = software_group_part_number[idx];
    }

    (void)scl_j1939_fill_mbuf_data(mbuf_ptr,data,ii);

    return;
}


/******************************************************************************
NAME:        app_scl_j1939_build_extid_f01a   
DESCRIPTION: Builds the PGN EF00 for Cat Ext ID F01A.  
NOTES:       It is the responsibility of this function to make sure, the PGN
             will be sent. Also make sure,the Destination address is extracted
             from the message.
ECM Service Support Identification
61184 $F016 a b b c c d d
  a - Set to 0 for CDL Device ID Format
  b b - CDL Module ID for ECM Function (LSB First)
  c c - Service Tool Support Change Level (LSB First)
  d d - Application Code (LSB First)

GLOBALS:     
*****************************************************************************/    
void app_scl_j1939_build_extid_f01a(scl_j1939_mbuf_hdr_t *mbuf_ptr) 
{
    unsigned_16 i = 0;
    unsigned_8 data[50]; //Max size for this ext id (data)

    //pad the entire data buffer with 0xFF
    memset( data, 0xFF, 50 );
    scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));

    data[ i++ ] = (unsigned_8)  0xF0;
    data[ i++ ] = (unsigned_8)  0x1A;

    { // Fill in ECM Part Number
        int idx;
        for (idx = 0; idx < ECM_PART_NUMBER_MAX_SIZE; ++idx) {
            if ('\0' == ecm_part_number[idx]) {
                break;
            }
            data[i++] = ecm_part_number[idx];
        }
    }

    /* delimit the data with ASCII "*" */
    data[ i++ ] = 0x2A;

    { // Fill in ECM Serial Number
        int idx;
        for (idx = 0; idx < ECM_SERIAL_NUMBER_MAX_SIZE; ++idx) {
            if ('\0' == ecm_serial_number[idx]) {
                break;
            }
            data[i++] = ecm_serial_number[idx];
        }
    }

    /* delimit the data with ASCII "*" */
    data[ i++ ] = 0x2A;
    
    scl_j1939_fill_mbuf_data(mbuf_ptr,data,i);

    return;
};

/******************************************************************************
NAME:        app_scl_j1939_build_extid_f019   
DESCRIPTION: Builds the PGN EF00 for Cat Ext ID F01F.  
NOTES:       It is the responsibility of this function to make sure, the PGN
             will be sent. Also make sure,the Destination address is extracted
             from the message.

$F019 Software Group Information
Message Format: $F019 a [a � a] $2A b [b � b] $2A c [c � c] $2A
   a � Software Group Part Number (ASCII) (see below for defn)
   Delimiter (ASCII "*" ($2A))
   b � Software Group Description (ASCII) (see below for defn)
   Delimiter (ASCII "*" ($2A))
   c � Software Group Release Date (ASCII) (see below for defn)
   Delimiter (ASCII "*" ($2A))
GLOBALS:     
*****************************************************************************/   
void app_scl_j1939_build_extid_f019(scl_j1939_mbuf_hdr_t *mbuf_ptr) 
{
    //Get the buffer. 2 Bytes for Cat Ext ID, 3 *s equals 5 more bytes
    unsigned_8 data[SOFTWARE_GROUP_PN_LENGTH+APPLICATION_TEXT_MAX_SIZE+
                                             SOFTWARE_GROUP_DATE_CODE_LENGTH+5];
    unsigned_8 i = 0, length = sizeof(data);

    //pad the entire data buffer with 0xFF
    memset( data, 0xFF, length );
    scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));

    //Set the first two fields with the Ext ID
    data[ i++ ] = (unsigned_8)0xF0;
    data[ i++ ] = (unsigned_8)0x19;

    //Update the software group part number - Get from getversion
    memcpy(&data[i], software_group_part_number, SOFTWARE_GROUP_PN_LENGTH);
    i += SOFTWARE_GROUP_PN_LENGTH;
    data[ i++ ] = 0x2A;   //Append the delimiter character

    //Update with description - Get from getversion
    memcpy(&data[i], app_description_dl_tx_pie, APPLICATION_TEXT_MAX_SIZE );
    i += APPLICATION_TEXT_MAX_SIZE;
    data[ i++ ] = 0x2A;
    
    //Update with software group code - Get from getversion
    memcpy(&data[i], software_group_date_code, SOFTWARE_GROUP_DATE_CODE_LENGTH);
    i += SOFTWARE_GROUP_DATE_CODE_LENGTH;
    data[ i++ ] = 0x2A;

    scl_j1939_fill_mbuf_data(mbuf_ptr,data,i);
    
    return;
};


/******************************************************************************* 
NAME:        app_scl_j1939_build_extid_f0a6
DESCRIPTION: Build the PGN EF00 CAT EXT id 0xF0A6
NOTES:      $F0A6 ECM Communication Preference
            Specifies the preferred communication mechanism used by the ECM 
            for communicating with other devices on the link for instance 
            Service Tool. For example the Service Tools read this message 
            to find out which protocol to use for writing the Data Link 
            configurations Parameters to the ECM.
            Data Length: 6-1783 Bytes
            Priority: 6
            DP: 0
            PF: 239
            PS: Destination Address
            Transmission Rate: As needed
            Data Format:
            Byte 1 Preferred CDL PID Read/Write Method
            2 Reserved for Future Assignment (Set to $FF) (1 byte)
            3 Reserved for Future Assignment (Set to $FF) (1 byte)
            4 Reserved for Future Assignment (Set to $FF) (1 byte)
            5 Reserved for Future Assignment (Set to $FF) (1 byte)
            6 Reserved for Future Assignment (Set to $FF) (1 byte)
GLOBALS:
*******************************************************************************/
void app_scl_j1939_build_extid_f0a6(scl_j1939_mbuf_hdr_t *mbuf_ptr)
{
    unsigned_8 data[8] = { 0xF0, 0xA6, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    unsigned_8 dst_add = scl_j1939_get_mbuf_da(mbuf_ptr);
    scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));
    scl_j1939_set_mbuf_sa(mbuf_ptr, dst_add);

    /*  Preferred CDL PID Write Method ($F0A6, Byte 1)
        Specifies the preferred method by the ECM for writing all the CDL 
        configuration parameters. For example the Service Tools read this 
        message to find out which protocol to use for writing the CDL PIDs 
        to the ECM.
        Data Length: 1 byte
        Type: Status
        $0 = Memory Access Protocol (MAP)
        $1 = CDL PID Read/Write over J1939
        $FE = Error
        $FF = Not Available 
     */
    /* Fill the entire bytes of the data message that needs to be transmitted */
    scl_j1939_fill_mbuf_data((scl_j1939_mbuf_hdr_t*)mbuf_ptr,data,8);
    return; 
}

/*
 * $F0BD Software Name Code Message
 * Data Length: 6-25 Bytes
 * Data Format:
 *   Byte 1    - Software Name Code Byte Length
 *   Byte 2-25 - Software Name Code (MSB First)
 */
void app_scl_j1939_build_extid_f0bd(scl_j1939_mbuf_hdr_t *mbuf_ptr) {
    // Extension ID (2) + Software Name Code Byte Length (1) + Longest Software Name Code (24)
    unsigned_8 data[2 + 1 + 24];
    unsigned_16 ii = 0;

    // Put in the extension id
    data[ii++] = (unsigned_8)0xF0;
    data[ii++] = (unsigned_8)0xBD;

    // The size, excluding the implicit null terminating character
    const std::string softwareNameCode = get_softwareNameCode();
    unsigned_8 softwareNameCodeLength = softwareNameCode.size();
    if (softwareNameCodeLength > 24) {
        softwareNameCodeLength = 24;
    }

    // Put in the size
    data[ii++] = softwareNameCodeLength;

    // Put in the name
    for (int idx = 0; idx < softwareNameCodeLength; ++idx) {
        data[ii++] = softwareNameCode[idx];
    }

    { // Swap source and destination addresses.
        unsigned_8 dst_add = scl_j1939_get_mbuf_da(mbuf_ptr);
        scl_j1939_set_mbuf_da(mbuf_ptr, scl_j1939_get_mbuf_sa(mbuf_ptr));
        scl_j1939_set_mbuf_sa(mbuf_ptr, dst_add);
    }

    // Send the response.
    (void)scl_j1939_fill_mbuf_data(mbuf_ptr, data, ii);

    return;
}

int_16 CdlWrPidD00C9FWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD00C9F)) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, PidWrD00C9F);
    PidD00C9F = PidWrD00C9F;

    PidWrD0102D = PidD0102D;

    /* Set flag to send request to Weigh App */
    WeighRangeBottomRqstFlg = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidD0102DWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD0102D)) {
        return 1;
    }

    if (lft_disable_write_pids) {
        return 1;
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, PidWrD0102D);
    PidD0102D = PidWrD0102D;

    PidWrD00C9F = PidD00C9F;

    /* Set flag to send request to Weigh App */
    WeighRangeSizeRqstFlg = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrFC06WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    (void) pie;
    if (len != 4) {
        return 1;
    }
    OEL_UNPACK_LE_32_NO_INCR(src_data, PidWrFC06);
    PidFC06 = PidWrFC06;
    /* Set flag to send request to Weigh App */
    PidWrFC06Flag = 1;
    return 0;
}

int_16 CdlWrPidWrFCF7WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    (void) pie;
    if (len != 4) {
        return 1;
    }
    OEL_UNPACK_LE_32_NO_INCR(src_data, PidWrFCF7);
    PidFCF7 = PidWrFCF7;
    /* Set flag to send request to Weigh App */
    PidWrFCF7Flag = 1;
    return 0;
}

int_16 CdlWrPidWrFCF8WrHdlr(void* pie, unsigned_16 len, unsigned_8* src_data) {
    (void) pie;
    if (len != 4) {
        return 1;
    }
    OEL_UNPACK_LE_32_NO_INCR(src_data, PidWrFCF8);
    PidFCF8 = PidWrFCF8;
    /* Set flag to send request to Weigh App */
    PidWrFCF8Flag = 1;
    return 0;
}

int_16 CdlWrPidWrD009F5WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;
    if (len != sizeof(PidWrD009F5)) {
        return(1);
    }

    if (lft_disable_write_pids) {
        return(1);
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, PidWrD009F5);
    PidD009F5 = PidWrD009F5;

    PidWrD009F5Flag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD009A7WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD009A7)) {
        return(1);
    }

    if (lft_disable_write_pids)  {
        return(1);
    }

    OEL_UNPACK_LE_16_NO_INCR(src_data, PidWrD009A7);
    PidD009A7 = PidWrD009A7;

    PidWrD009A7Flag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD112B2WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD112B2)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD112B2);
    PidD112B2 = PidWrD112B2;

    PidWrD112B2Flag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return(0);
}


int_16 CdlWrPidWrD10ACCWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD10ACC)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD10ACC);
    PidD10ACC = PidWrD10ACC;

    PidWrD10ACCFlag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD10F96WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD10F96)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD10F96);
    PidD10F96 = PidWrD10F96;

    PidWrD10F96Flag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaWeighScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD10E4EWrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void) pie;

    if (len != sizeof(PidWrD10E4E)) {
        return(1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD10E4E);
    PidD10E4E = PidWrD10E4E;

    PidWrD10E4EFlag = 1;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return(0);
}

int_16 CdlWrPidWrD106C4WrHdlr(void *pie, unsigned_16 len, unsigned_8 *src_data)
{
    (void)pie;

    if (len != sizeof(PidWrD106C4)) {
        return (1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD106C4);
    PidD106C4 = PidWrD106C4;

    PidWrD106C4Flag = TRUE;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return (0);
}

int_16 CdlWrPidWrD10748WrHdlr(void *pie, unsigned_16 len, unsigned_8 *src_data)
{
    (void)pie;

    if (len != sizeof(PidWrD10748)) {
        return (1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD10748);
    PidD10748 = PidWrD10748;

    PidWrD10748Flag = TRUE;

    return (0);
}

int_16 CdlWrPidWrD10972WrHdlr(void *pie,unsigned_16  len,unsigned_8  *src_data)
{
    (void)pie;

    if (len != sizeof(PidWrD10972)) {
        return (1);
    }

    OEL_UNPACK_BE_16_NO_INCR(src_data, PidWrD10972);
    PidD10972 = PidWrD10972;

    PidWrD10972Flag = TRUE;

    /* Set flag to flush SCS buffer */
    PendingLpsSaJobMgrScsReqOut_ = 1;

    return (0);
}
