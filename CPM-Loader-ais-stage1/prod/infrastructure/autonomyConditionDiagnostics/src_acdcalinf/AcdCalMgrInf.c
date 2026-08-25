/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: AcdCalInf.c
DESCRIPTION:
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <stdio.h>
#include <unistd.h>

#ifndef  ACDCALMGRINF_H_
#include <AcdCalMgrInf.h>
#endif

/* calibration work space data starts here*/
CAL_MGR_Work_t cal_mgr_workspace;

/* Workspace and dummy configuration for cal mechanism, used by all
   calibrations */
CAL_SAWork_t cal_sa_work;

const CAL_SACnfg_t cal_sa_cnfg =
{
    0
};

// scs values
unsigned_16 Cal_id; /* Calibration ID */
CAL_MGR_MC_E Calcmd;

/* CAL resp data */
CAL_MGR_MR_E CalResp;
unsigned_8 stepNo;
unsigned_16 CalError;
unsigned_16 warning;
unsigned_8 EnableQualRead;


/*****************************************************************************/
/*****     Application Defined Calibration Module Global Definitions     *****/

static CAL_MGR_Cal_t      cal_mgr_cals[] = {
    CAL_MGR_APP_CAL_LIST,
};


static cal_step_tbl_entry_t cal_registration_space[ CCM_CALS + CAL_MGR_CALS + 1 ];


/*Calibration interface configuration. */
const cal_interface_init_t ccm_interface = {
    ((unsigned_32)(CAL_LPTIM/0.0001)),  /* application periodic task period  100ms  */
    cal_registration_space,
    EH_ARRAY_ENTRIES(cal_registration_space),
    0,
    0
};

//static const CAL_MGR_Cnfg_t     cal_mgr_cnfg = {
	const CAL_MGR_Cnfg_t     cal_mgr_cnfg = {
    CAL_MGR_CALS,   /* Number of cals using CAL_MGR */
    cal_mgr_cals,   /* pointer to cal entries using CAL_MGR */

    0,   /* Number of app srvmodes using CAL_MGR */
    0,   /* pointer to srvmode entries using CAL_MGR */
   (cal_interface_init_t*)&ccm_interface  /* pointer to Core's CCM interface configuration */

};

///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
FUNCTION init_calibration
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void init_calibration()
{
    cal_mgr_j1939_init(&cal_mgr_workspace, &cal_mgr_cnfg);
}
/******************************************************************************
FUNCTION enable_calibration
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void enable_calibration()
{
	unsigned_8 ret;
	ret=cal_mgr_enable_cal(SA_EMPTY_BUCKET_CAL_ID);
	printf("LpsSaCalInit -1  %d \n",ret);
	ret=cal_mgr_enable_cal(SA_FULL_BUCKET_CAL_ID);
	printf("LpsSaCalInit -2  %d  \n",ret);
	ret=cal_mgr_enable_cal(SA_LIFT_LINKAGE_CAL_ID);
	printf("LpsSaCalInit -3  %d  \n",ret);
	ret=cal_mgr_enable_cal(SA_TILT_LINKAGE_CAL_ID);
	printf("LpsSaCalInit -4  %d \n",ret);
	ret=cal_mgr_enable_cal(SA_TILT_INLINE_LINKAGE_CAL_ID);
	printf("LpsSaCalInit -5  %d \n",ret);

    ret=cal_mgr_enable_cal(ADV_EMPTY_BUCKET_CAL_ID);
    printf("LpsSaCalInit -1  %d \n",ret);
    ret=cal_mgr_enable_cal(ADV_FULL_BUCKET_CAL_ID);
    printf("LpsSaCalInit -2  %d  \n",ret);

}

/******************************************************************************
FUNCTION cal_mgr_update
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void acd_cal_mgr_update()
{
    static boolean gotQrLimitsOnce = FALSE;

    // Read any UI commands
    AcdCalReadUIScsCmd();

    // Run cal manager
    cal_mgr_main(&cal_mgr_workspace, &cal_mgr_cnfg);

    if (gotQrLimitsOnce == FALSE) {
        // Get the Qr Limits from LpsWeighApp TxChannel, only once
        gotQrLimitsOnce = AcdCalScsGetQRLimits();
    }
}

/******************************************************************************
FUNCTION LpsSaEmptyBucketWeightCalibration
DESCRIPTION:
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
CAL_MGR_MR_E cal_mgr_callback(CAL_MGR_MC_E cmd,
        CAL_MGR_Work_t* cmw,
        const CAL_MGR_Cnfg_t* cmc,
        CAL_MGR_Cal_t* cal)
{
    boolean received = FALSE;

    Calcmd=cmd;
    Cal_id=cal->id;
    printf("cal_mgr_callback-start calid %d Calcmd %02d ",Cal_id, Calcmd);

    (void)cal; /* touch variables to avoid compiler warnings */
    (void)cmc; /* touch variables to avoid compiler warnings */

    if (AcdCalScsCmd()) {
        uint_fast16_t counter = 0;
        while (counter < 50) {
            received = AcdCalScsCmdResp(); /* Read resp from weighing app */
            if (received) { break; }
            counter++;
            usleep(5000);
        }
    }

    if (received) {
        static boolean QREnabled = FALSE;

        if (FALSE == QREnabled && TRUE == EnableQualRead) {
            app_scl_qr_j1939_server_add_entry();
            QREnabled = TRUE;
        }
        else if (TRUE == QREnabled && FALSE == EnableQualRead) {
            app_scl_qr_j1939_server_remove_entry();
            QREnabled = FALSE;
        }

        if (CalError != 0) {
            cal_add_error(CalError);
        }

        if (stepNo != 0xFF) {
            CAL_MGR_STEPNUMBER(cmw) = stepNo;
        }

        return CalResp; // CalResp is global variable, set by AcdCalScsCmdResp()
    }

    cal_add_error(0x000A);
    return CAL_MGR_MR_FAIL;
}


