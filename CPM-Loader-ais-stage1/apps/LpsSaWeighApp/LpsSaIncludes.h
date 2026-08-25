/*******************************************************************************
** COPYRIGHT (C) 2016-2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: LpsSaIncludes.h
DESCRIPTION:
*******************************************************************************/
#ifndef __LPS_SA_INCLUDES_H__
#define __LPS_SA_INCLUDES_H__
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <LpsPublic.h>

using namespace std;
/*******************************************************************************
** -- #Define, Struct's, Typedef's, Enum's --
*******************************************************************************/

//#define WEIGH_LIB_DUMP_BUFF_SIZE (unsigned_8) (1.0/WEIGH_LIB_EXEC_RATE)
#define WEIGH_LIB_DUMP_BUFF_SIZE 50

#define LIFT_CYL_DC_LP_FLT_CF        5.0  /* Low pass filter corner frequency in Hz for
                                            * Lift Cylinder Dutycycle */

#define TILT_CYL_DC_LP_FLT_CF        5.0  /* Low pass filter corner frequency in Hz for
                                            * Tilt Cylinder Dutycycle */

#define LIFT_CYL_VEL_LP_FLT_KFF		0.26773053165931 /* based on a 2.5 Hz filter frequency */
#define TILT_CYL_VEL_LP_FLT_KFF		0.26773053165931 /* based on a 2.5 Hz filter frequency */

/* 2.5 Hz @ 20ms sample rate */
#define LIFT_ANG_VEL_LP_FLT_KFF (0.267730532)

/* Sensors min and max limits for diagnostics */
#define CYL_POS_SENSOR_MAX_FREQ   (float) 600.0
#define CYL_POS_SENSOR_MIN_FREQ   (float) 200.0//400.0

#define HYD_PRES_SENSOR_MAX_DC 	(float)97
#define HYD_PRES_SENSOR_MIN_DC 	(float)3
#define HYD_PRES_SENSOR_MAX_FREQ   (float) 600.0
#define HYD_PRES_SENSOR_MIN_FREQ   (float) 400.0
#define HYD_PRES_SENSOR_SLOPE      0.0018
#define HYD_PRES_SENSOR_INTERCEPT  5.0
#define HYD_PRES_SENSOR_MAX_VALUE_KPA	(float)50000
#define HYD_PRES_SENSOR_MIN_VALUE_KPA	(float)0

#define DEFAULT_MIN_WEIGH_RANGE_START (35.0) // Default value for minimum weigh range start
#define DEFAULT_MAX_WEIGH_RANGE_END (80.0)   //Default value for maximum weigh range end
#define DEFAULT_WEIGH_RANGE_START (50.0)
#define DEFAULT_WEIGH_RANGE_END (65.0)
#define DEFAULT_MIN_WEIGH_RANGE_SIZE (15.0)

#define    UNKNOWN1S    -128        /*Unknown 1 Byte Signed Parameter   */
#define    UNKNOWN2S    -32768        /*Unknown 2 Byte Signed Parameter   */
#define    UNKNOWN4S    (-2147483647-1)    /*Unknown 4 Byte Signed Parameter   */

#define    UNKNOWN1U    0xE0        /*Unknown 1 Byte Unsigned Parameter */
#define    UNKNOWN2U    0xFFE0        /*Unknown 2 Byte Unsigned Parameter */
#define    UNKNOWN4U    0xFFFFFFE0    /*Unknown 4 Byte Unsigned Parameter */

/* Sensors min and max limits for diagnostics */
#define LIFT_TILT_CYL_MAX_DC  		(float)96.5
#define LIFT_TILT_CYL_MIN_DC  		(float)3.5

/* PWM channel index for Tilt HE and RE Pressures */
#define TILT_CYL_HE_PRES_SENS_CH_NUM    1   /* Pin 28 */
#define TILT_CYL_RE_PRES_SENS_CH_NUM    0   /* Pin 29 */

#define LIFT_CYL_RE_PRES_SENS_CH_NUM    0   /* pin 29 */
#define LIFT_CYL_HE_PRES_SENS_CH_NUM    1   /* pin 28 */
#define LIFT_CYL_POS_SENS_CH_NUM        2   /* pin 27 */
#define TILT_CYL_POS_SENS_CH_NUM        3   /* pin 26 */


typedef struct {
    uint32_t internalMsn = 0;
    uint8_t numOfLiftCylinders = 0;
    uint8_t numOfTiltCylinders = 0;
    float toolBcLength = 0.f;
    float toolBcAngle = 0.f;
    float tiltBoreDiameter = 0.f;
    float tiltRodDiameter = 0.f;
    float liftBoreDiameter = 0.f;
    float liftRodDiameter = 0.f;
    float ratedPayload = 0.f;
    float bucketPayloadTargetWeightDefault = 0.f;

    float liftArmAbLength = 0.f;
    float kgPerLiftKpaAtMidExtension = 0.f;
    float liftKpaNoBucketMidExtension = 0.f;
    float imuCompLoadCGAngleOffset = 0.f;

    bool hydOilTempOptional = false;
    bool hydOilTempEnabledDefault = true;
    bool loadCheckValveInstalledDefault = false;

    bool accelCompSupported = false;
    bool imuCompSupported = false;
    bool legalForTradeSupported = false;
} LpsSaMachineProperties_t;

typedef enum
{
	LPS_SA_INIT_SUCCESS = 0,
	LPS_SA_INIT_FAIL,
	LPS_SA_INIT_CONFIG_ERROR,
	LPS_SA_INIT_DEPENDENT_LIBRARY_INIT_FAIL,
}LpsSaInitErrorType_t;

typedef enum
{
	LPS_SA_UPDT_SUCCESS = 0,
	LPS_SA_UPDT_FAIL,
	LPS_SA_UPDT_DEPENDENT_LIBRARY_UPDT_FAIL,
	LPS_SA_UPDT_INPUT_LIB_INCORRECT_RESPONSE,
}LpsSaUpdtErrorType_t;

typedef enum
{
    TILT_SENSOR_TYPE_ROTARY = 0,
    TILT_SENSOR_TYPE_INLINE = 1,
    TILT_SENSOR_TYPE_NA = 0xFF
} LpsSaTiltSensorType_t;

typedef struct
{
	LpsInitTbl_t   WeighInitTbl;
}LpsSaInitTbl_t;

typedef struct
{
    LpsSaTiltSensorType_t tiltSensorType;  // default TILT_SENSOR_TYPE_ROTARY 0
    float hydPresSensorSlope;		// default	HYD_PRES_SENSOR_SLOPE  0.0018
    float hydPresSensorIntercept; 	// default	HYD_PRES_SENSOR_INTERCEPT  5.0
    uint16_t invertLiftDc;	// 0= no invert, dc=sensorDc,   	1=invert, dc=(100-sensorDc)
    uint16_t invertTiltDc;	// 0= no invert, dc=sensorDc,   	1=invert, dc=(100-sensorDc)
    vector<float> inLiftDc;
    vector<float> outLiftAngle;
    vector<float> inTiltDc;
    vector<float> outTiltAngle;
    vector<float> inLiftAngle; // deg
    vector<float> outLiftCylLen; // mm
    vector<float> outLiftAngleGain; // deg/mm
    vector<float> outLiftPresNoBucket; // kPa
    vector<float> outLiftAngleAccel; // deg/s^2 @ reference velocity
    float liftRefVel; // Reference lift cylinder velocity in mm/s
    vector<float> xTiltAngle;  // Rotary sensor
    vector<float> yLiftAngle;  // Rotary sensor
    vector<float> xTiltCylLen; // Inline sensor
    vector<float> yLiftCylLen; // Inline sensor
    vector<float> outTiltCylLen;
    vector<float> inTiltAngle;
	vector<float> outBktAngle;
	vector<float> KnmaticsTiltCylExtMinMax;
	vector<float> KnmaticsTiltAngleMinMax;
    vector<float> KnmaticsLiftCylExtMinMax;
} LinkageMap_t;


#endif /* #ifndef __LPS_SA_INCLUDES_H__ */
