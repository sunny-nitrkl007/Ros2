#ifndef _TIPOFFASSSIST_H_
#define _TIPOFFASSSIST_H_

#include <chrono>
#include <interfaces/TipoffModelTestPoints/InterfaceTypes.h>
#include "../LpsSaIncludes.h"
#include <rclcpp/rclcpp.hpp>
#include <ros_shim/RosOutputInterface.h>
#include <weigh_app_interfaces/msg/tipoff_model_test_points.hpp>

/* Tipoff Assist Inputs */
struct TipoffAssistInputs {
    typedef union
    {
        struct bitfield
        {
            uint16_t used:7;                     /* LSB. bit 0 */
            uint8_t lift_valve_cmd_bad:1;

            uint8_t tilt_valve_cmd_bad:1;
            uint8_t tilt_extension_bad:1;
            uint8_t lift_angle_bad:1;
            uint8_t lift_he_press_bad:1;

            uint8_t lift_re_press_bad:1;
            uint8_t tilt_he_press_bad:1;
            uint8_t tilt_re_press_bad:1;
            uint8_t imu_accel_bad:1;                /* MSB. bit 31 */
        } flag;
        uint16_t data;
    } InputStatus_t;

    InputStatus_t input_status = {};
    bool invalidate_outputs = true;
    float lift_valve_cmd = 0.f;
    float tilt_valve_cmd = 0.f;
    float tilt_extension = 0.f;
    float lift_angle = 0.f;
    float lift_he_pressure = 0.f;
    float lift_re_pressure = 0.f;
    float tilt_he_pressure = 0.f;
    float tilt_re_pressure = 0.f;
    float eef_imu_accelX = 0.f;
    float eef_imu_accelY = 0.f;
    float eef_imu_accelZ = 0.f;
    float steering_angle = 0.f;
    float tool_mass = 0.f;
    float truck_target_wt = 0.f;
    float truck_start_weight = 0.f;
    uint8_t bucket_current_weight_accuracy = 0;
    float  bucket_current_weight = 0.f;
    uint8_t tipoff_mode = 0;
    float zero_offset = 0.f;
    float simple_cal_factor = 1.0;
    float mach_pitch_cal_offset = 0.f;
    bool unlatch_trigger = false;
    float bucket_angle = 0.f;
    float anchor_zero_offset = 0.f;
    float anchor_factor = 1.f;
    uint16_t pass_count = 0;
    float lift_norm_angle = 0.f;
    float lift_norm_length = 0.f;
    float tilt_norm_angle = 0.f;
    float tilt_norm_length = 0.f;

    float friction_mu = 0.f;
    float friction_offset = 0.f;
};

/* Tipoff Assist Outputs */
struct TipoffAssistOutputs {
    float tilt_sensitivity_out = 0.f;
    float tilt_pressure_out = 0.f;
    float arbitrated_payload_norm_error_out = 0.f;
    uint8_t weigh_status_out = 0;
    float payload_norm_stdev_out = 0.f;
    uint8_t pcs_weight_accuracy_out = 0;
    float current_weight_norm_error_out = 0.f;
    uint8_t error_code_out = 0;
    float spill_rate_out = 0.f;
    float payload_send_to_CPM = 0.f;
    uint8_t payload_status_send_to_CPM = 0;
    float bucket_payload_target = 0.f;
    float unsecured_payload_upper_bound_norm = 0.f;
    float unsecured_payload_lower_bound_norm = 0.f;
    uint8_t unsecured_PFW_status = 0;
    float min_secure_bucket_angle = 0.f;
};

class TipoffAssist {

public:
    TipoffAssist();

    bool initialize(LpsSaMachineProperties_t const& machine_properties);
    TipoffAssistOutputs* update(TipoffAssistInputs& tipoff_inputs);

    TipoffAssistOutputs TipoffAssistOut;

    /* Tipoff Assist  Model Test Points */
    weigh_app_interfaces::msg::TipoffModelTestPoints    TipoffModelTestPointsData;
    ros_shim::RosOutputInterface<weigh_app_interfaces::msg::TipoffModelTestPoints> *TipoffModelTestPointsOut;

private:
    rclcpp::Node::SharedPtr node_;

};


#endif /* _TIPOFFASSSIST_H_ */
