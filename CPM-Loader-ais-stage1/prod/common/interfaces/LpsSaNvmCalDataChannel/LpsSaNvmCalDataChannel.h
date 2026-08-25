#include <ais/serialization/Datum.h> // For Boost serialization
#include <cat_std_types.h>
#include <stdio.h>
#include <LpsCalPublic.h>

#ifndef _LpsSaNvmCalDataChannel_h_
#define _LpsSaNvmCalDataChannel_h_

#define LPSSA_GROUP_ELEM_COUNT      11 // Number of elements for groups: raise_slow_empty_lift_heights.*, raise_slow_empty_pressures.*, etc.

// The following macro expands into a for-loop that iterates through the passed array and outputs each element to CSV
#define LPSSA_CSVDUMPMAIN_LOOP(field_name)      for (unsigned short iter=0; iter < LPSSA_GROUP_ELEM_COUNT; iter++)                  \
                                                {                                                                                   \
                                                   std::stringstream field_header;                                                  \
                                                   field_header << #field_name << "." << iter;                                      \
                                                   out(field_header.str(), lps_sa_nvm_calibration_data_main.field_name[iter]);      \
                                                }

class LpsSaNvmCalDataChannelStorage : public csvable
{
public:
   typedef enum LpsHydOilType_e
   {
      LPS_OIL_TYPE_SAE_10W = 0,
      LPS_OIL_TYPE_SAE_0W20
   } LpsHydOilType_t; // Duplicated from LpsPublic.h

   // Structure with the main calibration required data
   typedef struct LpsSaNvmCalDataChannel_DataMain_s
   {
      float          raise_slow_empty_lift_heights[LPSSA_GROUP_ELEM_COUNT];   // Corresponds to LpsSaCalNvmTbl_t field SlowRaiseEmptyBktLiftHt
      float          raise_slow_empty_pressures[LPSSA_GROUP_ELEM_COUNT];      // Corresponds to LpsSaCalNvmTbl_t field SlowRaiseEmptyBktLiftPres
      float          lower_slow_empty_lift_heights[LPSSA_GROUP_ELEM_COUNT];   // Corresponds to LpsSaCalNvmTbl_t field SlowLowerEmptyBktLiftHt
      float          lower_slow_empty_pressures[LPSSA_GROUP_ELEM_COUNT];      // Corresponds to LpsSaCalNvmTbl_t field SlowLowerEmptyBktLiftPres
      float          raise_slow_full_lift_heights[LPSSA_GROUP_ELEM_COUNT];    // Corresponds to LpsSaCalNvmTbl_t field SlowRaiseFullBktLiftHt
      float          raise_slow_full_pressures[LPSSA_GROUP_ELEM_COUNT];       // Corresponds to LpsSaCalNvmTbl_t field SlowRaiseFullBktLiftPres
      float          lower_slow_full_lift_heights[LPSSA_GROUP_ELEM_COUNT];    // Corresponds to LpsSaCalNvmTbl_t field SlowLowerFullBktLiftHt
      float          lower_slow_full_pressures[LPSSA_GROUP_ELEM_COUNT];       // Corresponds to LpsSaCalNvmTbl_t field SlowLowerFullBktLiftPres
      float          raise_speed_empty_slow;                                  // Corresponds to LpsSaCalNvmTbl_t field EmptyBktSlowRaiseSpd
      float          raise_speed_empty_fast;                                  // Corresponds to LpsSaCalNvmTbl_t field EmptyBktFastRaiseSpd
      float          raise_fast_delta_p_empty;                                // Corresponds to LpsSaCalNvmTbl_t field FastRaiseEmptyBktDeltaPres
      float          raise_speed_full_slow;                                   // Corresponds to LpsSaCalNvmTbl_t field FullBktSlowRaiseSpd
      float          raise_speed_full_fast;                                   // Corresponds to LpsSaCalNvmTbl_t field FullBktFastRaiseSpd
      float          raise_fast_delta_p_full;                                 // Corresponds to LpsSaCalNvmTbl_t field FastRaiseFullBktDeltaPres
   } LpsSaNvmCalDataChannel_DataMain_t;

   // Structure with the debug calibration data
   typedef struct LpsSaNvmCalDataChannel_DataDebug_s
   {
      int_32               lift_full_lower_DC_inf_value;                      // Corresponds to LpsSaLiftCalNvmTbl_t field lift_full_lower_dc
      int_32               lift_full_raise_DC_inf_value;                      // Corresponds to LpsSaLiftCalNvmTbl_t field lift_full_raise_dc
      int_32               tilt_full_dump_DC_inf_value;                       // Corresponds to LpsSaTiltCalNvmTbl_t field tilt_full_dump_dc
      int_32               tilt_full_rack_DC_inf_value;                       // Corresponds to LpsSaTiltCalNvmTbl_t field tilt_full_rack_dc
      int_32               tilt_dump_stop_angle_inf_value;                    // Corresponds to LpsSaTiltCalNvmTbl_t field tilt_dump_stop_angle
      int_32               tilt_rack_stop_angle_inf_value;                    // Corresponds to LpsSaTiltCalNvmTbl_t field tilt_rack_stop_angle
      float                pcs_vel_slope;                                     // TBD where this comes from
      float                pcs_cal_weight;                                    // Corresponds to LpsSaCalNvmTbl_t field Calwt
      float                zero_weight;                                       // Corresponds to LpsSaCalNvmTbl_t field ZeroWeight
      float                cal_adjust;                                        // Corresponds to LpsSaCalNvmTbl_t field CalAdjust
      float                temp_slope;                                        // TBD where this comes from
      float                empty_temp;                                        // Corresponds to LpsSaCalNvmTbl_t field EmptyTemp
      float                full_temp;                                         // Corresponds to LpsSaCalNvmTbl_t field FullTemp
      float                empty_bucket_weight_est;                           // Corresponds to LpsSaCalNvmTbl_t field EmptyBktWtEst
      float                cal_adjust_temp;                                   // TBD where this comes from
      int_32               hyd_oil_type_index;                                // Corresponds to LpsMachSpecificCfg_t field HydOilType
   } LpsSaNvmCalDataChannel_DataDebug_t;

   LpsSaNvmCalDataChannel_DataMain_t lps_sa_nvm_calibration_data_main;
   LpsSaNvmCalDataChannel_DataDebug_t lps_sa_nvm_calibration_data_debug;
   LpsCalIMUResults_t imu_cal_results;

   LpsSaNvmCalDataChannelStorage() :
       lps_sa_nvm_calibration_data_main(),
       lps_sa_nvm_calibration_data_debug(),
       imu_cal_results() { }

   template <class Archive>
   void serialize(Archive& ar, unsigned int version)
   {
      // Serialize lps_sa_nvm_calibration_data_main
      for (unsigned short iter=0; iter < LPSSA_GROUP_ELEM_COUNT; iter++)
      {
         ar & lps_sa_nvm_calibration_data_main.raise_slow_empty_lift_heights[iter];
         ar & lps_sa_nvm_calibration_data_main.raise_slow_empty_pressures[iter];
         ar & lps_sa_nvm_calibration_data_main.lower_slow_empty_lift_heights[iter];
         ar & lps_sa_nvm_calibration_data_main.lower_slow_empty_pressures[iter];
         ar & lps_sa_nvm_calibration_data_main.raise_slow_full_lift_heights[iter];
         ar & lps_sa_nvm_calibration_data_main.raise_slow_full_pressures[iter];
         ar & lps_sa_nvm_calibration_data_main.lower_slow_full_lift_heights[iter];
         ar & lps_sa_nvm_calibration_data_main.lower_slow_full_pressures[iter];
      }
      ar & lps_sa_nvm_calibration_data_main.raise_speed_empty_slow;
      ar & lps_sa_nvm_calibration_data_main.raise_speed_empty_fast;
      ar & lps_sa_nvm_calibration_data_main.raise_fast_delta_p_empty;
      ar & lps_sa_nvm_calibration_data_main.raise_speed_full_slow;
      ar & lps_sa_nvm_calibration_data_main.raise_speed_full_fast;
      ar & lps_sa_nvm_calibration_data_main.raise_fast_delta_p_full;

      // Serialize lps_sa_nvm_calibration_data_debug
      ar & lps_sa_nvm_calibration_data_debug.lift_full_lower_DC_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.lift_full_raise_DC_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.tilt_full_dump_DC_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.tilt_full_rack_DC_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.tilt_dump_stop_angle_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.tilt_rack_stop_angle_inf_value;
      ar & lps_sa_nvm_calibration_data_debug.pcs_vel_slope;
      ar & lps_sa_nvm_calibration_data_debug.pcs_cal_weight;
      ar & lps_sa_nvm_calibration_data_debug.zero_weight;
      ar & lps_sa_nvm_calibration_data_debug.cal_adjust;
      ar & lps_sa_nvm_calibration_data_debug.temp_slope;
      ar & lps_sa_nvm_calibration_data_debug.empty_temp;
      ar & lps_sa_nvm_calibration_data_debug.full_temp;
      ar & lps_sa_nvm_calibration_data_debug.empty_bucket_weight_est;
      ar & lps_sa_nvm_calibration_data_debug.cal_adjust_temp;
      ar & lps_sa_nvm_calibration_data_debug.hyd_oil_type_index;

      if (version == 2) {
          ar & imu_cal_results.FullSlowImuOffsetFinal;
          ar & imu_cal_results.EmptySlowImuOffsetFinal;
          ar & imu_cal_results.FullSlowLumpedWeightFinal;
          ar & imu_cal_results.EmptySlowLumpedWeightFinal;
          ar & imu_cal_results.EmptySlowImuOffsetTemp1;
          ar & imu_cal_results.EmptySlowImuOffsetTemp2;
          ar & imu_cal_results.EmptySlowImuOffsetTemp3;
          ar & imu_cal_results.FullSlowImuOffsetTemp1;
          ar & imu_cal_results.FullSlowImuOffsetTemp2;
          ar & imu_cal_results.FullSlowImuOffsetTemp3;
          ar & imu_cal_results.EmptySlowLumpedWeightTemp1;
          ar & imu_cal_results.EmptySlowLumpedWeightTemp2;
          ar & imu_cal_results.EmptySlowLumpedWeightTemp3;
          ar & imu_cal_results.FullSlowLumpedWeightTemp1;
          ar & imu_cal_results.FullSlowLumpedWeightTemp2;
          ar & imu_cal_results.FullSlowLumpedWeightTemp3;
      }
   }

   void toCsv(CsvOutStream& out) const
   {
      // Dump lps_sa_nvm_calibration_data_main. Note: the arrays elements are grouped together in the following macro
      LPSSA_CSVDUMPMAIN_LOOP(raise_slow_empty_lift_heights)
      LPSSA_CSVDUMPMAIN_LOOP(raise_slow_empty_pressures)
      LPSSA_CSVDUMPMAIN_LOOP(lower_slow_empty_lift_heights)
      LPSSA_CSVDUMPMAIN_LOOP(lower_slow_empty_pressures)
      LPSSA_CSVDUMPMAIN_LOOP(raise_slow_full_lift_heights)
      LPSSA_CSVDUMPMAIN_LOOP(raise_slow_full_pressures)
      LPSSA_CSVDUMPMAIN_LOOP(lower_slow_full_lift_heights)
      LPSSA_CSVDUMPMAIN_LOOP(lower_slow_full_pressures)
      out("raise_speed_empty_slow", lps_sa_nvm_calibration_data_main.raise_speed_empty_slow);
      out("raise_speed_empty_fast", lps_sa_nvm_calibration_data_main.raise_speed_empty_fast);
      out("raise_fast_delta_p_empty", lps_sa_nvm_calibration_data_main.raise_fast_delta_p_empty);
      out("raise_speed_full_slow", lps_sa_nvm_calibration_data_main.raise_speed_full_slow);
      out("raise_speed_full_fast", lps_sa_nvm_calibration_data_main.raise_speed_full_fast);
      out("raise_fast_delta_p_full", lps_sa_nvm_calibration_data_main.raise_fast_delta_p_full);

      // Dump lps_sa_nvm_calibration_data_debug
      out("lift_full_lower_DC_inf.value", lps_sa_nvm_calibration_data_debug.lift_full_lower_DC_inf_value);
      out("lift_full_raise_DC_inf.value", lps_sa_nvm_calibration_data_debug.lift_full_raise_DC_inf_value);
      out("tilt_full_dump_DC_inf.value", lps_sa_nvm_calibration_data_debug.tilt_full_dump_DC_inf_value);
      out("tilt_full_rack_DC_inf.value", lps_sa_nvm_calibration_data_debug.tilt_full_rack_DC_inf_value);
      out("tilt_dump_stop_angle_inf.value", lps_sa_nvm_calibration_data_debug.tilt_dump_stop_angle_inf_value);
      out("tilt_rack_stop_angle_inf.value", lps_sa_nvm_calibration_data_debug.tilt_rack_stop_angle_inf_value);
      out("PCS_Vel_Slope", lps_sa_nvm_calibration_data_debug.pcs_vel_slope);
      out("PCS_cal_weight", lps_sa_nvm_calibration_data_debug.pcs_cal_weight);
      out("zero_weight", lps_sa_nvm_calibration_data_debug.zero_weight);
      out("cal_adjust", lps_sa_nvm_calibration_data_debug.cal_adjust);
      out("temp_slope", lps_sa_nvm_calibration_data_debug.temp_slope);
      out("empty_temp", lps_sa_nvm_calibration_data_debug.empty_temp);
      out("full_temp", lps_sa_nvm_calibration_data_debug.full_temp);
      out("empty_bucket_weight_est", lps_sa_nvm_calibration_data_debug.empty_bucket_weight_est);
      out("cal_adjust_temp", lps_sa_nvm_calibration_data_debug.cal_adjust_temp);
      out("hyd_oil_type_index", lps_sa_nvm_calibration_data_debug.hyd_oil_type_index);
   }

private:
   // Nothing here yet
};

typedef Datum<LpsSaNvmCalDataChannelStorage> LpsSaNvmCalDataChannel;

BOOST_CLASS_VERSION(LpsSaNvmCalDataChannelStorage, 2);
#endif

