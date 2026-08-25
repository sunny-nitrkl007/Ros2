/*******************************************************************************
 ** @attention COPYRIGHT (C) 2013-2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
 **
 ** @file    AisJhm2CANProcess.cpp
 **
 ** @brief   This reads the Machineinput Interface for poseTask
 *******************************************************************************/

/*******************************************************************************
 ** -- #Include's --
 *******************************************************************************/
#include <cstring>
#include <errno.h>
#include <stdexcept>
#include <sys/socket.h>
#include <chrono>
#include <map>
#include <oel_pack.h>
#include <ais/log/Logger.h>
#include <catdllib_fid_def.h>

#include "AisJhm2CANProcess.h"

/*******************************************************************************
 ** -- #Define, Struct's, Typedef's, Enum's --
 *******************************************************************************/
static const std::map<char, uint8_t> charValMap = {
        { '0', 0x0 }, { '1', 0x1 }, { '2', 0x2 }, { '3', 0x3 }, { '4', 0x4 },
        { '5', 0x5 }, { '6', 0x6 }, { '7', 0x7 }, { '8', 0x8 }, { '9', 0x9 },
        { '.', 0xA }, { '*', 0xB }, { ' ', 0xC }, { '-', 0xD }, { ',', 0xE }
};

/*******************************************************************************
 ** -- Function Prototypes --
 *******************************************************************************/

/*******************************************************************************
 ** -- Symbols --
 *******************************************************************************/

/*******************************************************************************
 ** -- Start of code for this file --
 *******************************************************************************/

/******************************************************************************
 FUNCTION NAME:AisJhm2CANProcess constructor
 DESCRIPTION: AisJhm2CANProcess constructor for initializes an instance of its class
 PARAMETER DESCRIPTION:  task name is passed
 RETURN VALUE:  No return Value form constructor
 *******************************************************************************/
AisJhm2CANProcess::AisJhm2CANProcess() :
        socket_(-1), counter_(0), updatePeriod_(2) {
}

/******************************************************************************
 FUNCTION NAME:AisJhm2CANProcess destructor
 DESCRIPTION: AisJhm2CANProcess destructor for deallocating an instance of its class
 PARAMETER DESCRIPTION: No parameter
 RETURN VALUE:   No return Value form destructor
 *******************************************************************************/
AisJhm2CANProcess::~AisJhm2CANProcess() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

/******************************************************************************
 FUNCTION NAME:initialize
 DESCRIPTION: SCS channel is initialized
 PARAMETER DESCRIPTION:  No parameter
 RETURN VALUE:  Boolean
 *******************************************************************************/
bool AisJhm2CANProcess::init(
        float cycle_rate_hz) {
    bool success = true;
    AIS_LOG_INFO( "AisJhm2CANProcess::initialize");

    // calculate update factor based on cycle rate of calling app. Process needs to run at 100ms.
    // Rounding to nearest positive integer number of updates.
    updatePeriod_ = (uint8_t)((100 / (1000 / cycle_rate_hz)) + 0.5f);

    // Close if already open.
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }

    // Not using SocketCanUtil due to bug in error handling.
    socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_ >= 0) {
        if (!SocketCanUtil::setBlockingMode(socket_, false)) {
            success = false;
        }

        { // Apply filter (empty filter, block all messages)
            std::vector<unsigned int> rxFilter = { };
            if (!SocketCanUtil::applyCanFilter(socket_, rxFilter)) {
                success = false;
            }
        }

        if (!SocketCanUtil::setReceiveLocalMsgs(socket_, false)) {
            success = false;
        }

        if (!SocketCanUtil::setReceiveOwnMsgs(socket_, false)) {
            success = false;
        }

        if (!SocketCanUtil::bindInterface(socket_, "can0")) {
            success = false;
        }
    } else {
        AIS_LOG_FATAL(
                "Failed to open socket, error %d (%s)", errno, strerror(errno));
        success = false;
    }

    return success;
}

/******************************************************************************
 FUNCTION NAME: sendTX
 DESCRIPTION:   Sends a CAN-11bit message for mesg_id with 8 data bytes
 PARAMETER DESCRIPTION: data[8] - 8 data bytes to send
 mesg_id - is the CAN11-bit message id
 RETURN VALUE: None -
 *******************************************************************************/
void AisJhm2CANProcess::sendTX(uint8_t data[8], uint8_t length,
        uint16_t mesg_id) {
    if (socket_ >= 0) {
        CanPacket TxCANPacket;
        TxCANPacket.msgId = mesg_id;
        TxCANPacket.extendedId = false;
        TxCANPacket.remoteFrame = false;
        TxCANPacket.errorFrame = false;
        TxCANPacket.length = length;
        TxCANPacket.payload_[0] = data[0];
        TxCANPacket.payload_[1] = data[1];
        TxCANPacket.payload_[2] = data[2];
        TxCANPacket.payload_[3] = data[3];
        TxCANPacket.payload_[4] = data[4];
        TxCANPacket.payload_[5] = data[5];
        TxCANPacket.payload_[6] = data[6];
        TxCANPacket.payload_[7] = data[7];

        // Convert and send the message
        struct can_frame frame;
        SocketCanUtil::convertToSocketFormat(frame, TxCANPacket);

        int bytesSent = write(socket_, &frame, sizeof(frame));
        if (bytesSent < 0) {
            AIS_LOG_ERROR(
                    "Failed to write CAN message to socket, error %d (%s)", errno, strerror(errno));
        }
    } else {
        AIS_LOG_ERROR("Socket not open for publishing");
    }
}

/******************************************************************************
 DESCRIPTION: Update function to send data on CAN-11bit
 PARAMETERS:
 None.
 RETURN:
 None
 *******************************************************************************/
void AisJhm2CANProcess::update(PayloadInfo_t& payloadInfo) {
    // run at desired period
    if (++counter_ < updatePeriod_) {
        return;
    }

    counter_ = 0;

    uint8_t data700[8];
    uint8_t data701[8];

    create_can11bit700_message(payloadInfo, data700);
    sendTX(data700, 8, 0x8700);

    create_can11bit701_message(payloadInfo, data701);
    sendTX(data701, 8, 0x8701);

    return;
}

/******************************************************************************
 DESCRIPTION: create_can11bit700_message
 PARAMETERS:
 None.
 RETURN:
 None
 *******************************************************************************/
void AisJhm2CANProcess::create_can11bit700_message(PayloadInfo_t& payloadInfo,
        uint8_t* data) {
    /*
     * Send out the CAN-11 bit messages - 0x700
     * Weigh Cycle Total Truck Payload String, 3 bytes
     * Bucket Payload Weight String, 3 bytes
     * Bucket Payload Green Box Icon, 1 byte
     * Unused - 1 byte (0xFF)
     * */

    // truck weight
    auto size = payloadInfo.truckWeight.size();
    try {
        if (size > 6 || payloadInfo.operationMode == LPS_SA_JOB_MGR_UNCALIBRATED
                || payloadInfo.operationMode == LPS_SA_JOB_MGR_STANDBY_MODE) {
            // send error
            data[0] = data[1] = data[2] = 0xCC;
        } else if (size <= 5 && payloadInfo.decimalPrecision == 1) {
            // If the length of the string is <= 5 and the decimal precision is equal to
            // 1, then append a ‘ ‘ (space) to the right of the string.  This is to keep
            // the truck payload from overhanging the truck icon on the primary display.
            payloadInfo.truckWeight.append(" ");

            size = payloadInfo.truckWeight.size();
            if (size < 6) {
                //Prepend spaces to the string until it is of length 6
                payloadInfo.truckWeight.insert(0, 6 - size, ' ');
            }

            // convert str to val using map
            data[0] = (charValMap.at(payloadInfo.truckWeight[0]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[1]);
            data[1] = (charValMap.at(payloadInfo.truckWeight[2]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[3]);
            data[2] = (charValMap.at(payloadInfo.truckWeight[4]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[5]);
        } else {
            //Prepend spaces to the string until it is of length 6, if not already
            payloadInfo.truckWeight.insert(0, 6 - size, ' ');

            // convert str to val using map
            data[0] = (charValMap.at(payloadInfo.truckWeight[0]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[1]);
            data[1] = (charValMap.at(payloadInfo.truckWeight[2]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[3]);
            data[2] = (charValMap.at(payloadInfo.truckWeight[4]) << 4)
                                    | charValMap.at(payloadInfo.truckWeight[5]);
        }
    } catch (const std::out_of_range& oor) {
        AIS_LOG_ERROR("Out of Range Exception");
        // send error
        data[0] = data[1] = data[2] = 0xCC;
    }

    // green box icon
    size = payloadInfo.bucketWeight.size();
    if (payloadInfo.accuracy == 3 && size <= 6) {
        // If the bucket weight accuracy is 3, then we will show a green box.
        //  The CAN message should contain the length of the bucket payload
        //  weight string prior to prepending spaces.
        data[6] = size;
    } else {
        // 0  no greenbox
        data[6] = 0x00;
    }

    // bucket weight
    size = payloadInfo.bucketWeight.size();
    try {
        if (size > 6 || payloadInfo.accuracy == 0) {
            // send error
            data[3] = 0xCC;
            data[4] = 0xCB;
            data[5] = 0xBB;
        } else {
            //Prepend spaces to the string until it is of length 6
            payloadInfo.bucketWeight.insert(0, 6 - size, ' ');

            // convert str to val using map
            data[3] = (charValMap.at(payloadInfo.bucketWeight[0]) << 4)
                                    | charValMap.at(payloadInfo.bucketWeight[1]);
            data[4] = (charValMap.at(payloadInfo.bucketWeight[2]) << 4)
                                    | charValMap.at(payloadInfo.bucketWeight[3]);
            data[5] = (charValMap.at(payloadInfo.bucketWeight[4]) << 4)
                                    | charValMap.at(payloadInfo.bucketWeight[5]);
        }
    } catch (const std::out_of_range& oor) {
        AIS_LOG_ERROR("Out of Range Exception");
        // send error
        data[3] = 0xCC;
        data[4] = 0xCB;
        data[5] = 0xBB;
    }

    switch (payloadInfo.weighUnits) {
    case (LpsCommonWeightUnits::KLB): {
        data[7] = 15;
        break; }

    case (LpsCommonWeightUnits::TONNE): {
        data[7] = 16;
        break; }

    case (LpsCommonWeightUnits::TON): {
        data[7] = 17;
        break; }

    case (LpsCommonWeightUnits::LB): {
        data[7] = 18;
        break; }

    case (LpsCommonWeightUnits::KG): {
        data[7] = 19;
        break; }

    default : {
        data[7] = 0xFF;
        break; }
    }
}

/******************************************************************************
 DESCRIPTION: create_can11bit700_message
 PARAMETERS:
 None.
 RETURN:
 None
 *******************************************************************************/
void AisJhm2CANProcess::create_can11bit701_message(PayloadInfo_t& payloadInfo,
        uint8_t* data) {
    /*
     * Send out the CAN-11 bit messages - 0x701
     * Remaining Payload to Load String, 3 bytes
     * Unused - 5 bytes (0xFF)
     * */
    // remaining weight
    auto size = payloadInfo.remainingWeight.size();
    try {
        if (size > 6 || payloadInfo.operationMode == LPS_SA_JOB_MGR_UNCALIBRATED
                || payloadInfo.operationMode == LPS_SA_JOB_MGR_STANDBY_MODE) {
            // send error
            data[0] = data[1] = data[2] = 0xCC;
        } else {
            //Prepend spaces to the string until it is of length 6
            payloadInfo.remainingWeight.insert(0, 6 - size, ' ');

            // convert str to val using map
            data[0] = (charValMap.at(payloadInfo.remainingWeight[0]) << 4)
                                    | charValMap.at(payloadInfo.remainingWeight[1]);
            data[1] = (charValMap.at(payloadInfo.remainingWeight[2]) << 4)
                                    | charValMap.at(payloadInfo.remainingWeight[3]);
            data[2] = (charValMap.at(payloadInfo.remainingWeight[4]) << 4)
                                    | charValMap.at(payloadInfo.remainingWeight[5]);
        }
    } catch (const std::out_of_range& oor) {
        AIS_LOG_ERROR("Out of Range Exception");
        // send error
        data[0] = data[1] = data[2] = 0xCC;
    }

    //unused bytes
    data[3] = data[4] = data[5] = data[6] = data[7] = 0xFF;
}
