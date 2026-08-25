/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: CalInfConnect..h
DESCRIPTION:
*******************************************************************************/
#ifndef CALINFCONNECT_H
#define CALINFCONNECT_H

#include <cat_std_types.h>

#ifdef __cplusplus
extern "C" {
#endif

boolean AcdCalScsCmd(); // send calibration request to weighing app
boolean AcdCalScsCmdResp(); // read resp from weighing app
void AcdCalReadUIScsCmd(); // read UI scs Command
boolean AcdCalScsGetQRLimits(); /* Read LpsWeighTxChannel to get QR limits */

#ifdef __cplusplus
}
#endif


#endif //CALINFCONNECT_H
