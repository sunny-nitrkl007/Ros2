/*******************************************************************************
** COPYRIGHT (C) 2017 CATERPILLAR INC. ALL RIGHTS RESERVED.
--------------------------------------------------------------------------------
FILE NAME: acd_cal_connector.cpp
DESCRIPTION:This file call ACD c++ API from cal C functions.
*******************************************************************************/
/*******************************************************************************
** -- #Include's --
*******************************************************************************/
#include <stdio.h>
#include <ais/log/Logger.h>

#include <AcdCalScsInf.h>
#include <AcdCalParamHndlr.h>

extern "C" {
#include <cal_mgr.h>
#include <cal_mod_proto.h>
}

///////////////////////////////////////////////////////////////////////////////
// -- #Define, Struct's, Typedef's, Enum's --

extern unsigned_16 Cal_id; /* Calibration cmd data */
extern CAL_MGR_MC_E Calcmd;

extern CAL_MGR_MR_E CalResp;
extern unsigned_8	stepNo;
extern unsigned_16 CalError;
extern unsigned_16 warning;
extern unsigned_8 EnableQualRead;

///////////////////////////////////////////////////////////////////////////////
// -- Start of code for this file --
///////////////////////////////////////////////////////////////////////////////
using namespace task;

/* Null, because instance will be initialized on demand. */
AcdCalScsInf *AcdCalScsInf::AcdCalScsInf_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////
/// @brief AcdCalScsInf Constructor
///////////////////////////////////////////////////////////////////////////////
AcdCalScsInf::AcdCalScsInf() :
    LpsCalCmdScsRespIn(nullptr),
    LpsCalCmdScsReqstOut(nullptr),
    LpsTxChannelForCalsInput(nullptr),
    calRequestUIInput_(nullptr),
    calRequestUIInputConnection_(),
    calResponseUIOutput_(nullptr),
    m_handshakeNumberRequest(-1),
    m_handshakeNumberCallback(-1),
    requestQueueMtx_(),
    requestQueue_() {}


AcdCalScsInf* AcdCalScsInf::AcdCalGetInstance()
{
    if (nullptr == AcdCalScsInf_instance) {
        AcdCalScsInf_instance = new AcdCalScsInf;
    }
    return AcdCalScsInf_instance;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief AcdCalScsInf Destructor
///////////////////////////////////////////////////////////////////////////////
AcdCalScsInf::~AcdCalScsInf()
{
    // Disconnect the responseInput listener if connected.
    if (nullptr != calRequestUIInput_) {
        if (calRequestUIInputConnection_.connected()) {
            calRequestUIInput_->removeNewDataSlot(calRequestUIInputConnection_);
        }
    }
}

/******************************************************************************
FUNCTION AcdCalScsInitInterface( )
DESCRIPTION:It will  initialize the scs channels for calibration data send/receive
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AcdCalScsInf::AcdCalScsInitInterface(){

    if (!task::InterfaceDb::bind("CalMgrCmdReqstOutput", LpsCalCmdScsReqstOut)) {
        AIS_LOG_FATAL("LpsSaWeighScsCmdOut Interface not configured.");
    }

    if (!task::InterfaceDb::bind("CalMgrCmdRespInput", LpsCalCmdScsRespIn)) {
        AIS_LOG_FATAL("LpsSaWeighScsRespIn Interface not configured.");
    }

    // add Calibration scs objects between UI and ACD
    if (task::InterfaceDb::bind("CalibrationRequestUIInput", calRequestUIInput_)) {
        calRequestUIInputConnection_ = calRequestUIInput_->addNewDataSlot([this]{
            notifyUIRequestInput();
        });

        if (!calRequestUIInputConnection_.connected()) {
            AIS_LOG_ERROR("Failed to connect listener to CalibrationRequestUIInput interface.");
        }
    }
    else {
        AIS_LOG_FATAL("CalibrationRequestUIInput Interface not configured.");
    }

    if (!task::InterfaceDb::bind("CalibrationResponseUIOutput", calResponseUIOutput_)) {
        AIS_LOG_FATAL("CalibrationResponseUIOutput Interface not configured.");
    }

    if (!task::InterfaceDb::bind("LpsSaWeighTxChannelInput", LpsTxChannelForCalsInput)) {
        AIS_LOG_FATAL("LpsSaWeighTxChannelInput Interface not configured.");
    }
}

/*
 * Queue all incoming requests.
 */
void AcdCalScsInf::notifyUIRequestInput() {
    if (nullptr != calRequestUIInput_) {
        CalibrationRequestUI request;
        while (calRequestUIInput_->get(request)) {
            std::unique_lock<std::mutex> lck(requestQueueMtx_);
            requestQueue_.push(std::move(request));
        }
    }
}


/******************************************************************************
FUNCTION AcdCalScsCmd
DESCRIPTION: It will send the cmd to weighing App through SCS channel by polling method
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool AcdCalScsInf::AcdCalScsCmd()
{
    CalMgrCmdReqst calCmdReqCh;

    calCmdReqCh.CalibrationRequest.CalibrationReqstFlag=true;
    calCmdReqCh.CalibrationRequest.Calcmd=Calcmd;
    calCmdReqCh.CalibrationRequest.Cal_id=Cal_id;
    AIS_LOG_INFO("Cal ID: % d", Cal_id);
    AIS_LOG_DEBUG("Cal Cmd: % d", Calcmd);
    memcpy(calCmdReqCh.CalibrationRequest.CalIterm, cal_iterm, sizeof(calCmdReqCh.CalibrationRequest.CalIterm));

    return LpsCalCmdScsReqstOut->publish(calCmdReqCh);
}

/******************************************************************************
FUNCTION  AcdCalScsCmdResp
DESCRIPTION: It will get the resp from weighing App through SCS channel and send the same to
in info table and  ET.
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool AcdCalScsInf::AcdCalScsCmdResp(void)
{
    AIS_LOG_INFO("AcdCalScsCmdResp-1\n");

    CalMgrCmdResp calCmdRespCh;

    /* Retreive SCS channel data */
    if (LpsCalCmdScsRespIn->get(calCmdRespCh)) {
        /* set the response flag */
        AIS_LOG_DEBUG("CAlResp: %d",calCmdRespCh.CalibrationResp.RespCode);
        if (calCmdRespCh.CalibrationResp.RespCode == LPS_SA_CAL_SUCCESS) {
            /* Acknowlegement received for the issued command */
            CalResp   		= calCmdRespCh.CalibrationResp.CalResp;
            CalError  		= calCmdRespCh.CalibrationResp.error;
            warning   		= calCmdRespCh.CalibrationResp.warning;
            stepNo 	  		= calCmdRespCh.CalibrationResp.stepNo;
            EnableQualRead  = calCmdRespCh.CalibrationResp.EnableQualRead;

            AIS_LOG_INFO("AcdCalScsCmdResp-2-if-success %d \n", CalResp);
            if (CalResp == CAL_MGR_MR_SAMPLE_PT_COLLECTED) {
                AIS_LOG_INFO("AcdCalScsCmdResp-2-if-success CAL_MGR_MR_SAMPLE_PT_COLLECTED %d \n",CalResp);
            }

            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION  AcdCalScsGetQualifiedReadLimit

DESCRIPTION: It will get the resp from weighing App through SCS channel and send the same to
in info table and  ET.
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
bool AcdCalScsInf::AcdCalScsGetQualifiedReadLimit(void)
{
    AIS_LOG_INFO("AcdCalScsGetQualifiedReadLimit-1\n");

    if (nullptr != LpsTxChannelForCalsInput) {
        /* Retreive SCS channel data */
        LpsSaWeighTxChannel WeighParam;
        if (LpsTxChannelForCalsInput->get(WeighParam)) {
            app_scl_qr_j1939_server_set_values(WeighParam.PidData.QR_HydOilTempMin_C, WeighParam.PidData.QR_LiftCylVelMin_mm_sec, WeighParam.PidData.QR_LiftCylVelMax_mm_sec);
            AIS_LOG_INFO("QR_HydOilTemp:%d  QR_MinVel:%d  QR_Max_Vel:%d\n ",WeighParam.PidData.QR_HydOilTempMin_C, WeighParam.PidData.QR_LiftCylVelMin_mm_sec, WeighParam.PidData.QR_LiftCylVelMax_mm_sec);
            return true;
        }
    }

    return false;
}

/******************************************************************************
FUNCTION  AcdCalScsResetUICalibrationResponseToDefault
DESCRIPTION: It will set the UI response to default values
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AcdCalScsInf::AcdCalScsResetUICalibrationResponseToDefault()
{
    if (nullptr != calResponseUIOutput_) {
        CalibrationResponseUI response;
        response.statusParams.Status = 0x00; // Cal not Active, Not Executing,
        response.stepno = 0;
        response.CalId = 0;
        response.HandshakeNumber = m_handshakeNumberRequest;
        calResponseUIOutput_->publish(response);
    }
}

/******************************************************************************
FUNCTION  AcdCalScsReadUiCalibrationRequest
DESCRIPTION: It will get the resp from weighing App through SCS channel and send the same to
in info table and  ET.
PARAMETER DESCRIPTION:
RETURN VALUE:
*******************************************************************************/
void AcdCalScsInf::AcdCalScsReadUiCalibrationRequest()
{
	static int noResponseCounter = 0;
	static bool publishInitialResponse = false;

	if (publishInitialResponse == false) {
	    // only do this once at startup to UI has something valid, in case of App restart
        m_handshakeNumberRequest = -1;
        m_handshakeNumberCallback = -1;
		AcdCalScsResetUICalibrationResponseToDefault();
		publishInitialResponse = true;
	}

	// Only continue if cal manager has caught up with the previous requests.
	if ((m_handshakeNumberRequest == m_handshakeNumberCallback)) {
        cal_mgr_obj_t* cal_mgr_obj = cal_mc_get_curr_object();
        if (!cal_mgr_obj) {
            AIS_LOG_ERROR("Call to cal_mc_get_curr_object() failed");
            return;
        }

        unsigned_8 cal_rx_q_cnt = cal_mgr_obj->cal_sm_supt.cal_rx_q_cnt;
        if (cal_rx_q_cnt > 0) {
            /*
             * If cal_rx_q_cnt > 0, then cal_rx_msg_buf already has at least 1 message in the buffer
             * It doesn't make any sense to add more into the buffer because it can only handle 1
             * at a time.  In fact, it takes at least 2 updates (100-200ms) for cal_msg_serv to
             * response to a request.  1 loop to check the rx message, the next loop to create and send
             * the response.  Then it takes 1 additional loop to clear the rx message out of
             * the buffer making room for another.  A total of 3 loops per rx message (300ms).
             */
            //AIS_LOG_INFO("cal_rx_q_cnt already has a message (%d).  %d in app queue.", cal_rx_q_cnt, requestQueue_.size());
            if (cal_rx_q_cnt > 1) {
                AIS_LOG_ERROR("cal_rx_q_cnt(%u) > 1.  %u stuck in app queue.", cal_rx_q_cnt, requestQueue_.size());
            }
        }
        else {
            unsigned short numDiscarded = 0;
            bool newRequest = false;
            CalibrationRequestUIStorage request;

            { // Get the next "interesting" request
                std::lock_guard<std::mutex> lck(requestQueueMtx_);
                while (!requestQueue_.empty()) {
                    request = requestQueue_.front();
                    requestQueue_.pop();
                    newRequest = true;

                    // If request is interesting, break
                    if ((0x80 != request.CalibParam.ControlBits) /* CAL_CMD_CTRL_ENABLED */ ||
                            (0x0000 != request.CalibParam.InputAction) /* CAL_ACT_IDLE */ ||
                            (request.CalibParam.HandshakeNumber != m_handshakeNumberRequest)) {
                        break;
                    }

                    ++numDiscarded;
                }
            }

            if (numDiscarded > 2) {
                AIS_LOG_ERROR("Discarded %d uninteresting requests, more than expected.", numDiscarded);
            }

            if (newRequest) {
                // Get the request handshake number to compare it back to callback
                m_handshakeNumberRequest = request.CalibParam.HandshakeNumber;

                AIS_LOG_INFO("cal_any_rx_hndl() MID:FA CALID:%02X #:%02x CTL:%02x  ACT:%02x  CMD:%02x    STEP:%02x", request.CalibParam.CalId, m_handshakeNumberRequest, request.CalibParam.ControlBits, request.CalibParam.InputAction, request.CalibParam.InputActionCommand, request.CalibParam.CurrStep);

                noResponseCounter = 0;

                cal_any_rx_hdlr(
                        1, // 1 Is Calibration (FLAG)
                        0xFA, // 1 Module ID
                        request.CalibParam.CalId, // 2 Cal./Serv. ID
                        request.CalibParam.HandshakeNumber, // 1 Handshake Number
                        request.CalibParam.ControlBits, // 1 Control Byte
                        request.CalibParam.InputAction, // 2 Action
                        request.CalibParam.InputActionCommand, // 2 Action Command
                        AcdCalScsInf::processAisJhm2TxChannel_callback);
            }
	    }
	}
	else if (noResponseCounter < 20) {
	    // waiting for callback response triggered by request
        ++noResponseCounter;

        if (noResponseCounter >= 20)    // Fail-Safe, in case CalMgr doesn't respond
        {
            AIS_LOG_ERROR("TIMEOUT Request:%02x  Callback:%02x\n", m_handshakeNumberRequest, m_handshakeNumberCallback);

            cal_any_rx_hdlr (   // get us back to STEP 0000
                1, // 1 Is Calibration (FLAG)
                0xFA, // 1 Module ID
                0x00, // 2 Cal./Serv. ID
                m_handshakeNumberRequest, // 1 Handshake Number
                0x00, // 1 Control Byte
                0x00, // 2 Action
                0x0000, // 2 Action Command
                AcdCalScsInf::processAisJhm2TxChannel_callback);

            if (nullptr != calResponseUIOutput_) {
                CalibrationResponseUI response;
                response.statusParams.Status = 0x01;  // Cal not Active, Not Executing, Unsuccessful
                response.stepno = 0;
                response.CalId = 0;
                response.HandshakeNumber = m_handshakeNumberRequest;
                response.activeWarningErrorVector.push_back(0x0A);    // Calibration Failure
                calResponseUIOutput_->publish(response);
            }

            // reset back to default values
            m_handshakeNumberRequest = -1;
            m_handshakeNumberCallback = -1;
        }
	}
	else {
        // reset back to default values
        m_handshakeNumberRequest = -1;
        m_handshakeNumberCallback = -1;
	}
}

/*******************************************************************************
***
*** FUNCTION NAME: AcdCalSpecificTx_UI_callback
***
*** DESCRIPTION: this function takes a pointer to message data and sends it out
*** CDL.
***
*** METHOD: none
***
*** NOTES: none
***
*** INPUTS:
*** msg - Pointer to datalink independent data containing the following format:
***      msg[0] - message length
***      msg[1] - destination address
***      msg[2-3] - CDL PID
***      msg[4] - data length (message length -4)
***      msg[5-6] - calibration or service test ID
***      msg[7] - handshake number
***      msg[8] - status
***      msg[9-10] - step number
***      msg[10+] - messge dependent
***
*** RETURNS: none
***
*** GLOBALS USED: none
***
*** FUNCTIONS CALLED:
*** cdl2_transmit
***
*** MACROS USED: none
***
*******************************************************************************/

void  AcdCalScsInf::processAisJhm2TxChannel_callback(unsigned_8 *msg)
{
    AcdCalScsInf* AcdCalScsInfPtr = AcdCalScsInf::AcdCalGetInstance();

    uint16_t stepNumber = (uint16_t)(((uint16_t)msg[9]) << 8) | msg[10];
    uint8_t handshakeNumber = msg[7];

    // Publish CAL data for UI
    if (nullptr != AcdCalScsInfPtr->calResponseUIOutput_) {
        CalibrationResponseUI response;

        response.statusParams.Status = msg[8];
        response.stepno = stepNumber;
        response.CalId  = (unsigned short)(((unsigned short)msg[5]) << 8) | msg[6];
        response.HandshakeNumber = handshakeNumber;

		if (0 != msg[11]) {
		    // we have Warnings or Errors
		    for (int loop = 0; loop < (msg[11]*2); loop += 2) {
                uint16_t code = (uint16_t)(((uint16_t)msg[loop+12]) << 8) | msg[loop+13];
                response.activeWarningErrorVector.push_back(code);
		    }

		    AIS_LOG_ERROR("Calibration Errors (%d), RHS = %d, CHS = %d", msg[11], AcdCalScsInfPtr->m_handshakeNumberRequest, handshakeNumber);
		}

		AcdCalScsInfPtr->calResponseUIOutput_->publish(response);
	}

    // Only set the callback handshake if we are expecting a response
    if (AcdCalScsInfPtr->m_handshakeNumberRequest >= 0) {
        AcdCalScsInfPtr->m_handshakeNumberCallback = handshakeNumber;
    }

    AIS_LOG_INFO("TxChannel_callback()  LG:%02x ID:%02x%02x #:%02x ST:%02x STEP:%02x%02x m#:%02x m1:%02x%02x m2:%02x%02x", (char)msg[4], (char)msg[5], (char)msg[6], (char)msg[7], (char)msg[8], (char)msg[9], (char)msg[10], (char)msg[11], (char)msg[12], (char)msg[13], (char)msg[14], (char)msg[15]);
}

