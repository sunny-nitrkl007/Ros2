/*
 * AisJhm2Keypad.h
 *
 *  Created on: Dec 15, 2022
 *      Author: pf
 */

#ifndef AISJHM2KEYPAD_H_
#define AISJHM2KEYPAD_H_

#define BUTTON_PRESS_CONFIG_TIME_IN_SECS    2
#define BUTTON_1_PRESS_PID                  0xD11ECB
#define BUTTON_2_PRESS_PID                  0xD11ECC
#define BUTTON_3_PRESS_PID                  0xD11ECD
#define BUTTON_1_MODE_PID                   0xD11ECE
#define BUTTON_2_MODE_PID                   0xD11ECF
#define BUTTON_3_MODE_PID                   0xD11ED0

typedef enum {
    PAYLOAD_NONE = 0x003A,    
    PAYLOAD_CYCLE_TRUCK_SHORT_LIST = 0x0A03,
    PAYLOAD_CYCLE_MATERIAL_SHORT_LIST = 0x0A04,
    PAYLOAD_CYCLE_CUSTOM1_SHORT_LIST = 0x0A05,
    PAYLOAD_CYCLE_CUSTOM2_SHORT_LIST = 0x0A06,
    PAYLOAD_CYCLE_CUSTOM3_SHORT_LIST = 0x0A07,
    PAYLOAD_CYCLE_CUSTOM4_SHORT_LIST = 0x0A08,
    PAYLOAD_SPLIT_LOAD = 0x0A09,
    PAYLOAD_REMOVE_LAST_PASS = 0x0A0A,
    PAYLOAD_ZERO = 0x0A0B,
    PAYLOAD_REWEIGH = 0X0A0C,
    PAYLOAD_CYCLE_TARGET_WEIGHT = 0x0A0D,
    PAYLOAD_STANDBY = 0x0A0E,
    PAYLOAD_CYCLE_MULTI_TASK = 0x0A34,
} KeypadButtonMode_t;

typedef struct {
    typedef struct {
        KeypadButtonMode_t mode = KeypadButtonMode_t::PAYLOAD_NONE;
        bool previouslyDepressed = false;
        std::chrono::steady_clock::time_point timePressed = decltype(timePressed)::min();
        int numberOfReleases = 0;
    } Button_t;

    Button_t button1;
    Button_t button2;
    Button_t button3;
} Keypad_t;


#endif /* AISJHM2KEYPAD_H_ */
