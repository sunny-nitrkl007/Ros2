/*******************************************************************************
 ** @attention COPYRIGHT (C) 2013-2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
 **
 ** @file    AisJhm2CANProcess.h
 **
 ** @brief   This is the definition include file for the module.
 *******************************************************************************/
#ifndef AisJhm2CANProcess_H_
#define AisJhm2CANProcess_H_

/*******************************************************************************
 ** -- #Include's --
 *******************************************************************************/
#include <stdint.h>

#include <CANBase/CanPacket.h>
#include <CANBase/SocketCanUtil.h>

#include <interfaces/LpsSaJobMgrTxChannel/InterfaceTypes.h>
#include "AisJhm2DisplayState.h"

/******************************************************************************/

/*******************************************************************************
 ** -- #Define, Struct's, Typedef's, Enum's --
 *******************************************************************************/
/* defines */

/* class definition */
class AisJhm2CANProcess {
public:
    struct PayloadInfo_t {
        std::string truckWeight = "";
        LpsSaJobMgrOperationMode_t operationMode = LPS_SA_JOB_MGR_WEIGH_MODE;
        uint8_t accuracy = 0;
        std::string bucketWeight = "";
        std::string remainingWeight = "";
        bool showExclamationPoint = false;
        int decimalPrecision = 1;
        LpsCommonWeightUnits weighUnits = LpsCommonWeightUnits::TONNE;
    };

    AisJhm2CANProcess();
    ~AisJhm2CANProcess();
    bool init(float cycle_rate_hz);
    void update(PayloadInfo_t& payloadInfo);

    void sendTX(uint8_t data[8], uint8_t length, uint16_t mesg_id);

private:

    void create_can11bit700_message(PayloadInfo_t& payloadInfo, uint8_t* data);
    void create_can11bit701_message(PayloadInfo_t& payloadInfo, uint8_t* data);

    int socket_; // Socket for the CAN interface
    int counter_;
    uint8_t updatePeriod_;

};

#endif /* AisJhm2CANProcess_H_ */
