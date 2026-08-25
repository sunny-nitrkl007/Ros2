/*
 * AisJhm2Keypad.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: pf
 */


#include "AisJhm2RequestProcessor.h"

void AisJhm2RequestProcessor::processKeypadInputs() {
    processKeypadButton(keypad_.button1);
    processKeypadButton(keypad_.button2);
    processKeypadButton(keypad_.button3);
}

void AisJhm2RequestProcessor::processKeypadButton(Keypad_t::Button_t& button) {
    if (button.numberOfReleases <= 0) {
        return;
    }

    switch (button.mode) {
    case(KeypadButtonMode_t::PAYLOAD_REMOVE_LAST_PASS): {
        LpsSaJobMgrReqstChannel req;
        if (LPS_SA_JOB_MGR_CLEAR_BTN_ENABLED == jobMgrTxData_.ClearMinusOneEnableStat) {
            req.command = LpsSaJobMgrReqstChannel::Command::CLEAR;
        }
        else {
            req.command = LpsSaJobMgrReqstChannel::Command::MINUS_ONE;
        }
        jobMgrHelper_.sendRequest(req);
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_ZERO): {
        LpsSaWeighReqstChannel req;
        req.command = LpsSaWeighReqstChannel::Command::ZERO;
        weighAppInf_.sendRequest(req);
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_REWEIGH): {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::REWEIGH;
        jobMgrHelper_.sendRequest(req);
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_STANDBY): {
        LpsSaJobMgrReqstChannel req;
        if (jobMgrTxData_.OperationMode != LPS_SA_JOB_MGR_STANDBY_MODE ) {
            req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_ACTIVATE;
        }
        else {
            req.command = LpsSaJobMgrReqstChannel::Command::STANDBY_DEACTIVATE;
        }
        jobMgrHelper_.sendRequest(req);
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_SPLIT_LOAD): {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_PAYLOAD_NEXT_SUBTOTAL;
        jobMgrHelper_.sendRequest(req);
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_MULTI_TASK): {
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::SELECT_NEXT_TASK;
        jobMgrHelper_.sendRequestWaitForTxData(req, jobMgrTxData_);
        break;
    }    
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_TARGET_WEIGHT): {
        float targetWeight = 0.f;

        // if there are items in the list, then send the target weight to jobMgr
        if (targetWeights_.getNextTargetWeight(button.numberOfReleases, targetWeight)) {
            LpsSaJobMgrReqstChannel msg;
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_TARGET_WEIGHT;
            req.truckTargetWeight(targetWeight);
            msg.requests.push_back(std::move(req));
            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_TRUCK_SHORT_LIST): {
        std::string truckName;
        uint32_t truckId;
        float targetWt;

        // if there are items in the list, then send the list item to jobMgr
        if (truckFreqList_.getNextListItem(button.numberOfReleases, truckName, truckId, targetWt)) {
            LpsSaJobMgrReqstChannel msg;

            { // Truck Name
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_NAME;
                req.truckName(truckName);
                msg.requests.push_back(std::move(req));
            }

            { // Truck ID
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_ID;
                req.truckId(truckId);
                msg.requests.push_back(std::move(req));
            }

            { // Target Weight
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_TRUCK_TARGET_WEIGHT;
                req.truckTargetWeight(targetWt);
                msg.requests.push_back(std::move(req));
            }

            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_MATERIAL_SHORT_LIST): {
        std::string materialName;
        uint32_t materialId;
        float materialDensity;

        // if there are items in the list, then send the list item to jobMgr
        if (materialFreqList_.getNextListItem(button.numberOfReleases, materialName, materialId, materialDensity)) {
            LpsSaJobMgrReqstChannel msg;

            { // Material Name
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_NAME;
                req.materialName(materialName);
                msg.requests.push_back(std::move(req));
            }

            { // Material ID
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_ID;
                req.materialId(materialId);
                msg.requests.push_back(std::move(req));
            }

            { // Material Density
                LpsSaJobMgrReqst req;
                req.command = LpsSaJobMgrReqstCommand::WRITE_MATERIAL_DENSITY;
                req.materialDensity(materialDensity);
                msg.requests.push_back(std::move(req));
            }

            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_CUSTOM1_SHORT_LIST): {
        std::string tag1;

        // if there are items in the list, then send the list item to jobMgr
        if (custom1FreqList_.getNextListItem(button.numberOfReleases, tag1)) {
            LpsSaJobMgrReqstChannel msg;
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG1;
            req.tag(tag1, listMgrTxData_.customList1Name);
            msg.requests.push_back(std::move(req));
            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_CUSTOM2_SHORT_LIST): {
        std::string tag2;

        // if there are items in the list, then send the list item to jobMgr
        if (custom2FreqList_.getNextListItem(button.numberOfReleases, tag2)) {
            LpsSaJobMgrReqstChannel msg;
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG2;
            req.tag(tag2, listMgrTxData_.customList2Name);
            msg.requests.push_back(std::move(req));
            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_CUSTOM3_SHORT_LIST): {
        std::string tag3;

        // if there are items in the list, then send the list item to jobMgr
        if (custom3FreqList_.getNextListItem(button.numberOfReleases, tag3)) {
            LpsSaJobMgrReqstChannel msg;
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG3;
            req.tag(tag3, listMgrTxData_.customList3Name);
            msg.requests.push_back(std::move(req));
            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_CYCLE_CUSTOM4_SHORT_LIST): {
        std::string tag4;

        // if there are items in the list, then send the list item to jobMgr
        if (custom4FreqList_.getNextListItem(button.numberOfReleases, tag4)) {
            LpsSaJobMgrReqstChannel msg;
            LpsSaJobMgrReqst req;
            req.command = LpsSaJobMgrReqstCommand::WRITE_TAG4;
            req.tag(tag4, listMgrTxData_.customList4Name);
            msg.requests.push_back(std::move(req));
            jobMgrHelper_.sendRequestWaitForTxData(msg, jobMgrTxData_);
        }
        break;
    }
    case(KeypadButtonMode_t::PAYLOAD_NONE):
    default: {
        break;
    }
    }

    button.numberOfReleases = 0;
}
