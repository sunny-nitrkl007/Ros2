#include <chrono>
#include <chrono/convert.hpp>

#include <oel_pack.h>
#include <catdllib_fid_def.h>

#include <ais/task/InterfaceDb.h>
#include <ais/task/Task.h>
#include <ais/log/Logger.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <toa_wl00_model_if.h>
#include <toa_wl00_po.h>
#include <toa_wl00.h>


#ifdef __cplusplus
}
#endif

#include "TipoffAssist.h"

#define TIPOFF_ASSIST_RUN_UNIT_TEST 0 /* set to 1 to run unit test, 0 for production */

static void initTipoffAssistModelData(LpsSaMachineProperties_t const& machine_properties);

static void readCSV(TipoffAssistInputs& tipoff_inputs);

extern toa_wl00_catParameters_si_RAM_t toa_wl00_catParameters_si_RAM;

TipoffAssist::TipoffAssist():
        TipoffAssistOut(),
        TipoffModelTestPointsOut(nullptr) {}

bool TipoffAssist::initialize(LpsSaMachineProperties_t const& machine_properties) {
    uint32_t appNumber = machine_properties.internalMsn;
    AIS_LOG_INFO("TipoffAssist::initialize");

    // If we don't have an application number, we won't run.
    if (0 == appNumber) {
        AIS_LOG_WARN("Tip-off Assist not supported for this machine configuration.");
    }

    // Initialize Test Output SCS Channel
    if (!task::InterfaceDb::bind("TipoffModelTestPointsOutput", TipoffModelTestPointsOut)) {
        AIS_LOG_WARN("Tip-off Assist Testpoints objects, TipoffModelTestPointsOut failed to initialize.");
    }

    initTipoffAssistModelData(machine_properties);

    /* Initialize config parameters pointer */
    toa_wl00_work.toa_wl00_M.toa_wl00_catParameters_si_RAM_p = &toa_wl00_catParameters_si_RAM;

    /* Initialize tipoff assist model */
    toa_wl00_po_init();

    return true;
}

TipoffAssistOutputs* TipoffAssist::update(TipoffAssistInputs& tipoff_inputs) {

    /* unit test with data from ppg */
    if (TIPOFF_ASSIST_RUN_UNIT_TEST) {
        readCSV(tipoff_inputs);
        AIS_LOG_INFO("Tipoff Assist in Unit Test Mode");
    }

    /* Update inputs */
    TOA_invalidate_outputs = tipoff_inputs.invalidate_outputs;
    TOA_lift_valve_cmd = tipoff_inputs.lift_valve_cmd;
    TOA_tilt_valve_cmd = tipoff_inputs.tilt_valve_cmd;
    TOA_lift_angle = tipoff_inputs.lift_angle;
    TOA_tilt_extension = tipoff_inputs.tilt_extension;
    TOA_lift_he_pressure = tipoff_inputs.lift_he_pressure;
    TOA_lift_re_pressure = tipoff_inputs.lift_re_pressure;
    TOA_tilt_he_pressure = tipoff_inputs.tilt_he_pressure;
    TOA_tilt_re_pressure = tipoff_inputs.tilt_re_pressure;
    TOA_eef_imu_accelX = tipoff_inputs.eef_imu_accelX;
    TOA_eef_imu_accelY = tipoff_inputs.eef_imu_accelY;
    TOA_eef_imu_accelZ = tipoff_inputs.eef_imu_accelZ;
    TOA_steering_angle = tipoff_inputs.steering_angle;
    TOA_tool_mass = tipoff_inputs.tool_mass;
    TOA_truck_target_wt = tipoff_inputs.truck_target_wt;
    TOA_truck_start_weight = tipoff_inputs.truck_start_weight;
    TOA_bucket_current_weight_accuracy = tipoff_inputs.bucket_current_weight_accuracy;
    TOA_bucket_current_weight  = tipoff_inputs.bucket_current_weight;
    TOA_tipoff_mode = tipoff_inputs.tipoff_mode;
    TOA_zero_offset = tipoff_inputs.zero_offset;
    TOA_simple_cal_factor = tipoff_inputs.simple_cal_factor;
    TOA_imu_pitch_cal = tipoff_inputs.mach_pitch_cal_offset;
    TOA_unlatch_trigger = tipoff_inputs.unlatch_trigger;
    TOA_bucket_angle = tipoff_inputs.bucket_angle;
    TOA_anchor_zero_offset = tipoff_inputs.anchor_zero_offset;
    TOA_anchor_factor = tipoff_inputs.anchor_factor;
    TOA_Pass_Count = tipoff_inputs.pass_count;
    TOA_Lift_Norm_Angle = tipoff_inputs.lift_norm_angle;
    TOA_Lift_Norm_Length = tipoff_inputs.lift_norm_length;
    TOA_Tilt_Norm_Angle = tipoff_inputs.tilt_norm_angle;
    TOA_Tilt_Norm_Length = tipoff_inputs.tilt_norm_length;
    TOA_Friction_Mu = tipoff_inputs.friction_mu;
    TOA_Friction_Offset = tipoff_inputs.friction_offset;

    /* Update tipoff assist model */
    toa_wl00_po_0();

    /* Update outputs */
    TipoffAssistOut.tilt_sensitivity_out = TOA_tilt_sensitivity_out;
    TipoffAssistOut.arbitrated_payload_norm_error_out = TOA_arbitrated_payload_norm_error_out;
    TipoffAssistOut.weigh_status_out = TOA_weigh_status_out;
    TipoffAssistOut.payload_norm_stdev_out = TOA_payload_norm_stdev_out;
    TipoffAssistOut.pcs_weight_accuracy_out = TOA_pcs_weight_accuracy_out;
    TipoffAssistOut.current_weight_norm_error_out = TOA_current_weight_norm_error_out;
    TipoffAssistOut.error_code_out = TOA_error_code_out;
    TipoffAssistOut.spill_rate_out = TOA_spill_rate_out;
    TipoffAssistOut.payload_send_to_CPM = TOA_payload_send_to_CPM;
    TipoffAssistOut.payload_status_send_to_CPM = TOA_payload_status_send_to_CPM;
    TipoffAssistOut.bucket_payload_target = TOA_bucket_payload_target;
    TipoffAssistOut.unsecured_payload_lower_bound_norm = TOA_unsecured_payload_lower_bound_norm;
    TipoffAssistOut.unsecured_payload_upper_bound_norm = TOA_unsecured_payload_upper_bound_norm;
    TipoffAssistOut.min_secure_bucket_angle = TOA_min_secure_bucket_angle;
    TipoffAssistOut.unsecured_PFW_status = TOA_unsecured_PFW_status;

    /* Update Model TestPoints Outputs */
    TipoffModelTestPointsData.toa_pfwSecLatch = toa_wl00_work.BlockIO.toa_pfwSecLatch;
    TipoffModelTestPointsData.toa_pfwSecPayload = toa_wl00_work.BlockIO.toa_pfwSecPayload;
    TipoffModelTestPointsData.toa_pfwSecStatus = toa_wl00_work.BlockIO.toa_pfwSecStatus;
    TipoffModelTestPointsData.toa_pfwSecStDev = toa_wl00_work.BlockIO.toa_pfwSecStDev;
    TipoffModelTestPointsData.toa_targetPayloadFinal = toa_wl00_work.BlockIO.toa_targetPayloadFinal;
    TipoffModelTestPointsData.toa_targetPayloadStatus = toa_wl00_work.BlockIO.toa_targetPayloadStatus;
    TipoffModelTestPointsData.toa_pfwNlNotchMeanEst = toa_wl00_work.BlockIO.toa_pfwNlNotchMeanEst;
    TipoffModelTestPointsData.toa_pfwNlNotchStDevEst = toa_wl00_work.BlockIO.toa_pfwNlNotchStDevEst;
    TipoffModelTestPointsData.toa_lwLpfPost = toa_wl00_work.BlockIO.toa_lwLpfPost;
    TipoffModelTestPointsData.toa_payloadAdjusted = toa_wl00_work.BlockIO.toa_payloadAdjusted;
    TipoffModelTestPointsData.toa_sendPayloadArb = toa_wl00_work.BlockIO.toa_sendPayloadArb;
    TipoffModelTestPointsData.toa_sfuncInGravX = toa_wl00_work.BlockIO.toa_sfuncInGravX;
    TipoffModelTestPointsData.toa_sfuncInGravY = toa_wl00_work.BlockIO.toa_sfuncInGravY;
    TipoffModelTestPointsData.toa_sfuncInLiftForce = toa_wl00_work.BlockIO.toa_sfuncInLiftForce;
    TipoffModelTestPointsData.toa_sfuncInTiltForce = toa_wl00_work.BlockIO.toa_sfuncInTiltForce;
    TipoffModelTestPointsData.toa_sfuncOutRawMassTonne = toa_wl00_work.BlockIO.toa_sfuncOutRawMassTonne;

    TipoffModelTestPointsData.toa_appNumber = toa_wl00_catParameters_si_RAM.toa_appNumber;
    TipoffModelTestPointsData.toa_liftBoreDia = toa_wl00_catParameters_si_RAM.toa_liftBoreDia;
    TipoffModelTestPointsData.toa_liftRodDia = toa_wl00_catParameters_si_RAM.toa_liftRodDia;
    TipoffModelTestPointsData.toa_ratedPayload = toa_wl00_catParameters_si_RAM.toa_ratedPayload;
    TipoffModelTestPointsData.toa_tiltBoreDia = toa_wl00_catParameters_si_RAM.toa_tiltBoreDia;
    TipoffModelTestPointsData.toa_tiltNumCyl = toa_wl00_catParameters_si_RAM.toa_tiltNumCyl;
    TipoffModelTestPointsData.toa_tiltRodDia = toa_wl00_catParameters_si_RAM.toa_tiltRodDia;
    TipoffModelTestPointsData.toa_toolBcLength = toa_wl00_catParameters_si_RAM.toa_toolBcLength;
    TipoffModelTestPointsData.toa_toolBcAngle = toa_wl00_catParameters_si_RAM.toa_toolBcAngle;

    TipoffModelTestPointsData.toa_payloadAncAdjusted = toa_wl00_work.BlockIO.toa_payloadAncAdjusted;
    TipoffModelTestPointsData.toa_payloadAncZeroed = toa_wl00_work.BlockIO.toa_payloadAncZeroed;
    TipoffModelTestPointsData.toa_imu_cal_pitch_angle = TOA_imu_pitch_cal;
    TipoffModelTestPointsData.toa_pfwUnsecStatus = toa_wl00_work.BlockIO.toa_pfwUnsecStatus;
    TipoffModelTestPointsData.toa_pfwUnsecBoundLower = toa_wl00_work.BlockIO.toa_pfwUnsecBoundLower;
    TipoffModelTestPointsData.toa_pfwUnsecBoundUpper = toa_wl00_work.BlockIO.toa_pfwUnsecBoundUpper;
    TipoffModelTestPointsData.toa_lwSpillRate = toa_wl00_work.BlockIO.toa_lwSpillRate;
    TipoffModelTestPointsData.toa_lwStatus = toa_wl00_work.BlockIO.toa_lwStatus;
    TipoffModelTestPointsData.toa_rawDist = toa_wl00_work.BlockIO.toa_rawDist;
    TipoffModelTestPointsData.toa_rawPayload = toa_wl00_work.BlockIO.toa_rawPayload;
    TipoffModelTestPointsData.toa_payloadZeroed = toa_wl00_work.BlockIO.toa_payloadZeroed;
    TipoffModelTestPointsData.toa_pfwIsWarm = toa_wl00_work.BlockIO.toa_pfwIsWarm;
    TipoffModelTestPointsData.toa_pfwSecIsSecure = toa_wl00_work.BlockIO.toa_pfwSecIsSecure;
    TipoffModelTestPointsData.toa_pfwSecIsNoSliding = toa_wl00_work.BlockIO.toa_pfwSecIsNoSliding;
    TipoffModelTestPointsData.toa_pfwSecIsNoCollapsing = toa_wl00_work.BlockIO.toa_pfwSecIsNoCollapsing;
    TipoffModelTestPointsData.toa_pfwSecIsRacked = toa_wl00_work.BlockIO.toa_pfwSecIsRacked;
    TipoffModelTestPointsData.toa_pfwSecMinBucketAng = toa_wl00_work.BlockIO.toa_pfwSecMinBucketAng;
    TipoffModelTestPointsData.toa_pfwMcOk = toa_wl00_work.BlockIO.toa_pfwMcOk;
    TipoffModelTestPointsData.toa_pfwMcMaybeStalled = toa_wl00_work.BlockIO.toa_pfwMcMaybeStalled;
    TipoffModelTestPointsData.toa_pfwMcMaybeRackStall = toa_wl00_work.BlockIO.toa_pfwMcMaybeRackStall;
    TipoffModelTestPointsData.toa_pfwMcMaybeGrounded = toa_wl00_work.BlockIO.toa_pfwMcMaybeGrounded;
    TipoffModelTestPointsData.toa_statPayloadNlNotchOneUp = toa_wl00_work.BlockIO.toa_statPayloadNlNotchOneUp;
    TipoffModelTestPointsData.toa_statPayloadNlNotchOneLow = toa_wl00_work.BlockIO.toa_statPayloadNlNotchOneLow;
    TipoffModelTestPointsData.toa_statPayloadNlNotchOneSize = toa_wl00_work.BlockIO.toa_statPayloadNlNotchOneSize;
    TipoffModelTestPointsData.toa_statPayloadNlNotchTwoUp = toa_wl00_work.BlockIO.toa_statPayloadNlNotchTwoUp;
    TipoffModelTestPointsData.toa_statPayloadNlNotchTwoLow = toa_wl00_work.BlockIO.toa_statPayloadNlNotchTwoLow;
    TipoffModelTestPointsData.toa_statPayloadNlNotchTwoSize = toa_wl00_work.BlockIO.toa_statPayloadNlNotchTwoSize;
    TipoffModelTestPointsData.toa_statPayloadNlOneUp = toa_wl00_work.BlockIO.toa_statPayloadNlOneUp;
    TipoffModelTestPointsData.toa_statPayloadNlOneLow = toa_wl00_work.BlockIO.toa_statPayloadNlOneLow;
    TipoffModelTestPointsData.toa_statPayloadNlOneSize = toa_wl00_work.BlockIO.toa_statPayloadNlOneSize;
    TipoffModelTestPointsData.toa_statPayloadNlTwoUp = toa_wl00_work.BlockIO.toa_statPayloadNlTwoUp;
    TipoffModelTestPointsData.toa_statPayloadNlTwoLow = toa_wl00_work.BlockIO.toa_statPayloadNlTwoLow;
    TipoffModelTestPointsData.toa_statPayloadNlTwoSize = toa_wl00_work.BlockIO.toa_statPayloadNlTwoSize;
    TipoffModelTestPointsData.toa_pfwNlStDevEst = toa_wl00_work.BlockIO.toa_pfwNlStDevEst;
    TipoffModelTestPointsData.toa_pfwNlMeanEst = toa_wl00_work.BlockIO.toa_pfwNlMeanEst;
    TipoffModelTestPointsData.toa_pfwPayloadFiltFinal = toa_wl00_work.BlockIO.toa_pfwPayloadFiltFinal;
    TipoffModelTestPointsData.toa_pfwPayloadNotchFiltFinal = toa_wl00_work.BlockIO.toa_pfwPayloadNotchFiltFinal;
    TipoffModelTestPointsData.toa_pfwPayloadNotchPost = toa_wl00_work.BlockIO.toa_pfwPayloadNotchPost;
    TipoffModelTestPointsData.toa_pfwLatchInvalidate = toa_wl00_work.BlockIO.toa_pfwLatchInvalidate;
    TipoffModelTestPointsData.toa_imuRawEefAcclX = toa_wl00_work.BlockIO.toa_imuRawEefAcclX;
    TipoffModelTestPointsData.toa_imuRawEefAcclY = toa_wl00_work.BlockIO.toa_imuRawEefAcclY;
    TipoffModelTestPointsData.toa_imuRawEefAcclZ = toa_wl00_work.BlockIO.toa_imuRawEefAcclZ;
    TipoffModelTestPointsData.toa_imuNeefGravX = toa_wl00_work.BlockIO.toa_imuNeefGravX;
    TipoffModelTestPointsData.toa_imuNeefGravY = toa_wl00_work.BlockIO.toa_imuNeefGravY;
    TipoffModelTestPointsData.toa_imuNeefGravZ = toa_wl00_work.BlockIO.toa_imuNeefGravZ;
    TipoffModelTestPointsData.toa_imuNeefPitch = toa_wl00_work.BlockIO.toa_imuNeefPitch;
    TipoffModelTestPointsData.toa_targetCompMargin = toa_wl00_work.BlockIO.toa_targetCompMargin;
    TipoffModelTestPointsData.toa_targetPayloadPostComp = toa_wl00_work.BlockIO.toa_targetPayloadPostComp;
    TipoffModelTestPointsData.toa_targetPayloadPreComp = toa_wl00_work.BlockIO.toa_targetPayloadPreComp;
    TipoffModelTestPointsData.toa_statPayloadSecureTwoSize = toa_wl00_work.BlockIO.toa_statPayloadSecureTwoSize;
    TipoffModelTestPointsData.toa_statPayloadSecureOneUpper = toa_wl00_work.BlockIO.toa_statPayloadSecureOneUpper;
    TipoffModelTestPointsData.toa_statPayloadSecureOneLower = toa_wl00_work.BlockIO.toa_statPayloadSecureOneLower;
    TipoffModelTestPointsData.toa_statPayloadSecureOneSize = toa_wl00_work.BlockIO.toa_statPayloadSecureOneSize;
    TipoffModelTestPointsData.toa_statPayloadSecureTwoUpper = toa_wl00_work.BlockIO.toa_statPayloadSecureTwoUpper;
    TipoffModelTestPointsData.toa_statPayloadSecureTwoLower = toa_wl00_work.BlockIO.toa_statPayloadSecureTwoLower;
    TipoffModelTestPointsData.toa_sendCpmLivePayload = toa_wl00_work.BlockIO.toa_sendCpmLivePayload;

    /* Publish Tipoff Model Test Points */
    if (TipoffModelTestPointsOut != nullptr)
    {
        TipoffModelTestPointsOut->publish(TipoffModelTestPointsData);
    }

    return (&TipoffAssistOut);
}

static void initTipoffAssistModelData(LpsSaMachineProperties_t const& machine_properties) {
    toa_wl00_catParameters_si_RAM.toa_appNumber = machine_properties.internalMsn;
    toa_wl00_catParameters_si_RAM.toa_liftBoreDia = machine_properties.liftBoreDiameter;
    toa_wl00_catParameters_si_RAM.toa_liftRodDia = machine_properties.liftRodDiameter;
    toa_wl00_catParameters_si_RAM.toa_ratedPayload = machine_properties.ratedPayload;
    toa_wl00_catParameters_si_RAM.toa_tiltBoreDia = machine_properties.tiltBoreDiameter;
    toa_wl00_catParameters_si_RAM.toa_tiltNumCyl = machine_properties.numOfTiltCylinders;
    toa_wl00_catParameters_si_RAM.toa_tiltRodDia = machine_properties.tiltRodDiameter;
    toa_wl00_catParameters_si_RAM.toa_toolBcLength = machine_properties.toolBcLength;
    toa_wl00_catParameters_si_RAM.toa_toolBcAngle = machine_properties.toolBcAngle;

    return;
}

#include "csvReader.h"

static void readCSV(TipoffAssistInputs& tipoff_inputs) {
    // Creating an object of CSVWriter
    static CSVReader reader("/opt/07312019_toa_weights1.csv");

    // Get the data from CSV File
    static std::vector<std::vector<std::string> > dataList = reader.getData();

    static int total_rows = dataList.size();
    static int index = 1;

    if (total_rows == 0)
    {
        AIS_LOG_ERROR("Attempt to run tipoff assist unit test with no data file records");
    }

    /* get row */
    if (index < total_rows) {
        std::vector<std::string> vec = dataList[index];
        index++;

        int i=0;
        TOA_weigh_status_out = std::stoi(vec[i++]);
        TOA_arbitrated_payload_norm_error_out = std::stof(vec[i++]);
        tipoff_inputs.bucket_current_weight = std::stof(vec[i++]);
        tipoff_inputs.bucket_current_weight_accuracy = std::stoi(vec[i++]);
        TOA_current_weight_norm_error_out = std::stof(vec[i++]);
        tipoff_inputs.eef_imu_accelX = std::stof(vec[i++]);
        tipoff_inputs.eef_imu_accelY = std::stof(vec[i++]);
        tipoff_inputs.eef_imu_accelZ = std::stof(vec[i++]);
        TOA_error_code_out = std::stof(vec[i++]);
        vec[i++];
        tipoff_inputs.invalidate_outputs = std::stoi(vec[i++]);
        tipoff_inputs.lift_angle = std::stof(vec[i++]);
        tipoff_inputs.lift_he_pressure = std::stof(vec[i++]);
        tipoff_inputs.lift_re_pressure = std::stof(vec[i++]);
        tipoff_inputs.lift_valve_cmd = std::stof(vec[i++]);
        TOA_payload_norm_stdev_out = std::stof(vec[i++]);
        TOA_payload_send_to_CPM = std::stof(vec[i++]);
        TOA_payload_status_send_to_CPM = std::stoi(vec[i++]);
        TOA_pcs_weight_accuracy_out = std::stoi(vec[i++]);
        TOA_spill_rate_out = std::stof(vec[i++]);
        tipoff_inputs.steering_angle = std::stof(vec[i++]);
        tipoff_inputs.tilt_extension = std::stof(vec[i++]);
        tipoff_inputs.tilt_he_pressure = std::stof(vec[i++]);
        TOA_tilt_pressure_out = std::stof(vec[i++]);
        tipoff_inputs.tilt_re_pressure = std::stof(vec[i++]);
        TOA_tilt_sensitivity_out = std::stof(vec[i++]);
        tipoff_inputs.tilt_valve_cmd = std::stof(vec[i++]);
        tipoff_inputs.tipoff_mode = std::stoi(vec[i++]);
        vec[i++];
        tipoff_inputs.tool_mass = std::stof(vec[i++]);
        tipoff_inputs.truck_start_weight = std::stof(vec[i++]);
        tipoff_inputs.truck_target_wt = std::stof(vec[i++]);
    }
    else
        tipoff_inputs.invalidate_outputs = 1;
}
