///////////////////////////////////////////////////////////////////////////////
// @attention COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
//
// @file    AcdCalScsInf.h
//
// @brief   This is the definition include file for the module.
///////////////////////////////////////////////////////////////////////////////
#ifndef ACDCALSCSINF_H_
#define ACDCALSCSINF_H_

///////////////////////////////////////////////////////////////////////////////
// -- #Include's --
///////////////////////////////////////////////////////////////////////////////
#include <queue>
#include <mutex>

#include <boost/signals2.hpp>

#include <ais/task/Task.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>
#include <interfaces/CalMgrCmdResp/InterfaceTypes.h>
#include <interfaces/CalMgrCmdReqst/InterfaceTypes.h>
#include <interfaces/CalibrationRequestUI/InterfaceTypes.h> // UI cals
#include <interfaces/CalibrationResponseUI/InterfaceTypes.h> // UI cals

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// -- Class Definition
///////////////////////////////////////////////////////////////////////////////
class AcdCalScsInf
{
public:
    AcdCalScsInf();
    virtual ~AcdCalScsInf( );

    //calibration
    bool AcdCalScsCmd();
    bool AcdCalScsCmdResp(void);
    void AcdCalScsInitInterface();
    static AcdCalScsInf* AcdCalGetInstance();
    void AcdCalScsResetUICalibrationResponseToDefault();
    void AcdCalScsReadUiCalibrationRequest();
    bool AcdCalScsGetQualifiedReadLimit();

protected:
    void notifyUIRequestInput();

private:
    /* Here will be the instance stored. */
    static AcdCalScsInf* AcdCalScsInf_instance;

    CalMgrCmdRespInput    *LpsCalCmdScsRespIn;/*getRes from weighing App*/
    CalMgrCmdReqstOutput  *LpsCalCmdScsReqstOut;/*setCmd to weighing App*/
    LpsSaWeighTxChannelInput  *LpsTxChannelForCalsInput;/*setCmd to weighing App*/

    static void  processAisJhm2TxChannel_callback(unsigned_8 *msg);

    /* cal for UI side  */
    CalibrationRequestUIInput* calRequestUIInput_;
    boost::signals2::connection calRequestUIInputConnection_;
    CalibrationResponseUIOutput* calResponseUIOutput_;

    int16_t m_handshakeNumberRequest;
    int16_t m_handshakeNumberCallback;

    std::mutex requestQueueMtx_;
    std::queue<CalibrationRequestUIStorage> requestQueue_;
};


#endif //ACDCALSCSINF_H_
