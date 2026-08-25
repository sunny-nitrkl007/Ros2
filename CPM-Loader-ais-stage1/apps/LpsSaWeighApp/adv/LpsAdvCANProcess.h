/*******************************************************************************
 ** @attention COPYRIGHT (C) 2013-2016 CATERPILLAR INC. ALL RIGHTS RESERVED.
 **
 ** @file    LpsAdvCANProcess.h
 **
 ** @brief   This is the definition include file for the module.
 *******************************************************************************/
#ifndef LpsAdvCANProcess_H_
#define LpsAdvCANProcess_H_

/*******************************************************************************
 ** -- #Include's --
 *******************************************************************************/
#include <stdint.h>

#include <CANBase/CanPacket.h>

/******************************************************************************/

/*******************************************************************************
 ** -- #Define, Struct's, Typedef's, Enum's --
 *******************************************************************************/
/* defines */

/* class definition */
class LpsAdvCANProcess
{
public:
    struct LiftLeverStatus_t {
        bool lift_lever_centered = false;
        bool lift_lever_lower_detent = false;
        bool lift_lever_raise_detent = false;
        bool lift_lever_faulted = false;
    };

    LpsAdvCANProcess( );
	~LpsAdvCANProcess( );
	bool init( );
	bool update(bool& allRxd);
	bool get_lift_he_pressure(float &he_pressure_val);
	bool get_lift_re_pressure(float &re_pressure_val);
	bool get_tilt_linkage_dc(float &tilt_linkage_dc);
	bool get_lift_linkage_dc(float &lift_linkage_dc);
	bool get_lift_valve_cmd(float &lift_valve_cmd_);
	bool get_tilt_valve_cmd(float &tilt_valve_cmd_);
	bool isCalOverrideActive();
	void get_lift_lever_status(LiftLeverStatus_t& lift_lever_status);

	void sendTX(uint8_t data[8], uint8_t length, uint16_t mesg_id);
    int purgeRxBuffer();

    int CAN734_count;
    int CAN733_count;
    int dropped;

protected:
    bool getPacket(CanPacket& m, int timeout_ms);

private:
    bool unpack734Data(const CanPacketStorage& packet);
    bool unpack733Data(const CanPacketStorage& packet);

    int socket_; // Socket for the CAN interface
    bool init_flag_;
    unsigned int lastMsgId_;
    bool CAN734Received_;
    bool CAN733Received_;

    uint16_t lift_he_pressure_;
    uint16_t lift_re_pressure_;
    int16_t lift_valve_cmd_;
    int16_t tilt_valve_cmd_;
    uint8_t cal_override_acknowledge_;
    uint8_t lift_lever_status_;
    uint16_t tilt_linkage_dc_;
    uint16_t lift_linkage_dc_;
};

#endif /* LpsAdvCANProcess_H_ */
