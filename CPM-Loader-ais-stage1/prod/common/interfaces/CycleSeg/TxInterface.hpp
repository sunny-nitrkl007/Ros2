#ifndef CYCLESEG_TXINTERFACE_HPP
#define CYCLESEG_TXINTERFACE_HPP

#include <cstdint>

#include <ais/serialization/Datum.h>
#include <ais/serialization/csvable.h>

struct CycleSegInputs_t {
    int16_t       ground_spd_kph;
    float        payload_ratio;
    uint16_t     payload_ratio_status;
    float        payload_ratio_raw;
    uint16_t     payload_ratio_raw_status;
    int16_t       lift_valve_cmd;
    int16_t       tilt_valve_cmd;
    int16_t       bucket_angle;
    int16_t       lift_angle;
    int16_t       desired_gear;
    int16_t       engine_spd_rpm;
    int16_t       tilt_lever_cmd;
    uint16_t     tilt_cyl_pos;
    uint16_t     machine_idle_status;
    uint16_t     keyswitch_state;
    uint32_t     real_time;
    uint8_t      realTime_status;
    int16_t       timezone_offset;
    uint32_t     total_fuel_high_res;
    uint32_t     total_fuel_low_res;
    uint16_t     fuel_rate;
    uint16_t     ARD_fuel_rate;
    uint32_t     GPS_latitude;
    uint32_t     GPS_longitude;
    uint8_t      GPS_status;
    uint32_t     service_hour_meter;
    float        payload_weight;
    uint16_t     payload_weight_status;
    uint16_t     payload_calc_method;
    int16_t       extForceNormalized;
    uint16_t     impl_operation_status;
    int16_t       Engine_Load_Factor;
};

struct CycleSegOutputs_t {
    uint8_t      commit_segment_record;
    uint8_t      segment_record_id;
    uint32_t     segment_rec_start_simTime;
    uint32_t     segment_rec_duration;
    float        segment_rec_distance;
    uint32_t     segment_rec_start_real_time;
    float        segment_rec_fuel_used;
    uint32_t     segment_rec_latitude;
    uint32_t     segment_rec_longitude;
    uint32_t     segment_rec_idle_time;
    uint32_t     segment_rec_hour_meter;
    float        segment_rec_payload;
    uint16_t     segment_rec_payload_calc_method;
    uint8_t      commit_rec_gps;
    uint8_t      segment_dig_state;
    uint8_t      segment_last_completed_id;
    uint32_t     segment_time_since_last_end;
    uint8_t      segment_sequence_number;
    uint8_t      keyswitch_override_flag;
    uint8_t      segment_active_segment_id;
};

/*
 * The interface storage class.
 */
class CycleSegTxInterfaceStorage : public csvable {
public:
    CycleSegTxInterfaceStorage() :
        activeSegmentId(0),
        digState(0),
        segmentLastCompletedId(0),
        segmentSecondsSinceLastEnd(0.f),
        segmentSequenceNumber(0),
        cycleSegIn(),
        cycleSegOut() {}

    uint8_t activeSegmentId;

    // This should be coming from pcs_public.h or some other common area, but that doens't exist yet.
    uint8_t digState; // 0 = Inactive, 1 = Tentative, 2 = Active

    uint8_t segmentLastCompletedId;
    float segmentSecondsSinceLastEnd;
    uint8_t segmentSequenceNumber;

    CycleSegInputs_t cycleSegIn;
    CycleSegOutputs_t cycleSegOut;

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & activeSegmentId;
        ar & digState;
        ar & segmentLastCompletedId;
        ar & segmentSecondsSinceLastEnd;
        ar & segmentSequenceNumber;

        ar & cycleSegIn.ground_spd_kph;
        ar & cycleSegIn.payload_ratio;
        ar & cycleSegIn.payload_ratio_status;
        ar & cycleSegIn.payload_ratio_raw;
        ar & cycleSegIn.payload_ratio_raw_status;
        ar & cycleSegIn.lift_valve_cmd;
        ar & cycleSegIn.tilt_valve_cmd;
        ar & cycleSegIn.bucket_angle;
        ar & cycleSegIn.lift_angle;
        ar & cycleSegIn.desired_gear;
        ar & cycleSegIn.engine_spd_rpm;
        ar & cycleSegIn.tilt_lever_cmd;
        ar & cycleSegIn.tilt_cyl_pos;
        ar & cycleSegIn.machine_idle_status;
        ar & cycleSegIn.keyswitch_state;
        ar & cycleSegIn.real_time;
        ar & cycleSegIn.realTime_status;
        ar & cycleSegIn.timezone_offset;
        ar & cycleSegIn.total_fuel_high_res;
        ar & cycleSegIn.total_fuel_low_res;
        ar & cycleSegIn.fuel_rate;
        ar & cycleSegIn.ARD_fuel_rate;
        ar & cycleSegIn.GPS_latitude;
        ar & cycleSegIn.GPS_longitude;
        ar & cycleSegIn.GPS_status;
        ar & cycleSegIn.service_hour_meter;
        ar & cycleSegIn.payload_weight;
        ar & cycleSegIn.payload_weight_status;
        ar & cycleSegIn.payload_calc_method;
        ar & cycleSegIn.extForceNormalized;
        ar & cycleSegIn.impl_operation_status;
        ar & cycleSegIn.Engine_Load_Factor;

        ar & cycleSegOut.commit_segment_record;
        ar & cycleSegOut.segment_record_id;
        ar & cycleSegOut.segment_rec_start_simTime;
        ar & cycleSegOut.segment_rec_duration;
        ar & cycleSegOut.segment_rec_distance;
        ar & cycleSegOut.segment_rec_start_real_time;
        ar & cycleSegOut.segment_rec_fuel_used;
        ar & cycleSegOut.segment_rec_latitude;
        ar & cycleSegOut.segment_rec_longitude;
        ar & cycleSegOut.segment_rec_idle_time;
        ar & cycleSegOut.segment_rec_hour_meter;
        ar & cycleSegOut.segment_rec_payload;
        ar & cycleSegOut.segment_rec_payload_calc_method;
        ar & cycleSegOut.commit_rec_gps;
        ar & cycleSegOut.segment_dig_state;
        ar & cycleSegOut.segment_last_completed_id;
        ar & cycleSegOut.segment_time_since_last_end;
        ar & cycleSegOut.segment_sequence_number;
        ar & cycleSegOut.keyswitch_override_flag;
        ar & cycleSegOut.segment_active_segment_id;
    }
};

/*
 * Datum template specialization.
 *   Defines a new specialization of the Datum class
 *   derived from the base 'storage' class.
 */
typedef Datum<CycleSegTxInterfaceStorage> CycleSegTxInterface;

BOOST_CLASS_VERSION(CycleSegTxInterfaceStorage, 0);

#endif /* CYCLESEG_TXINTERFACE_HPP */
