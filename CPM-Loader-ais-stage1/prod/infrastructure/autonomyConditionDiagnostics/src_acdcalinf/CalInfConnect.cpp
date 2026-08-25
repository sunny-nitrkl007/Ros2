/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: acd_cal_connector.cpp
DESCRIPTION:This file call ACD c++ API from cal C functions.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <CalInfConnect.h>
#include <AcdCalScsInf.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inside this "extern C" block, I can define C functions that are able to call C++ code

/* send calibration request to weighing app */
boolean AcdCalScsCmd()
{
    return AcdCalScsInf::AcdCalGetInstance()->AcdCalScsCmd();
}

/* Read resp from weighing app */
boolean AcdCalScsCmdResp()
{
    return AcdCalScsInf::AcdCalGetInstance()->AcdCalScsCmdResp();
}

/* send calibration request to weighing app */
void AcdCalReadUIScsCmd()
{
    AcdCalScsInf::AcdCalGetInstance()->AcdCalScsReadUiCalibrationRequest();
}

/* Read LpsWeighTxChannel to get QR limits */
boolean AcdCalScsGetQRLimits()
{
    return AcdCalScsInf::AcdCalGetInstance()->AcdCalScsGetQualifiedReadLimit();
}

#ifdef __cplusplus
}
#endif
