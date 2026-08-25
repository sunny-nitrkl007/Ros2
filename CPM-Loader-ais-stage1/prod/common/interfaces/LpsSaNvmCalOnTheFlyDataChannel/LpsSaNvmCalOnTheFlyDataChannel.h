#include <ais/serialization/Datum.h> // For Boost serialization
#include "LpsCommonStructures.h"
#include "LpsCalPublic.h"

#ifndef _LpsSaNvmCalOnTheFlyDataChannel_h_
#define _LpsSaNvmCalOnTheFlyDataChannel_h_

class LpsSaNvmCalOnTheFlyDataChannelStorage : public csvable
{
public:
   typedef struct LpsSaNvmCalOnTheFlyDataChannel_StatusFlags_s
   {
      bool  CalInProgress;
      bool  EmptyBktCalDone;
      bool  FullBktCalDone;
   } LpsSaNvmCalOnTheFlyDataChannel_StatusFlags_t; // LpsCalStatus_t

   typedef struct LpsSaNvmCalOnTheFlyDataChannel_CurveFitInfo_s
   {
       bool HydOilTempMet;
       bool EnforceRaiseDetent;
       bool EnforceLowerDetent;
       bool RaiseFastVelDone;
       bool RaiseSlowFitDone;
       bool RaiseFastFitDone;
       bool LowerSlowFitDone;
       uint8_t InternalStep;
       uint16_t RaiseNumPoints;
       uint16_t LowerNumPoints;
       LpsCalIMUResults_t IMUCalResults;
   } LpsSaNvmCalOnTheFlyDataChannel_CurveFitInfo_t; // Part of LpsCalCurveFitInfo_t

   typedef struct {
       uint8_t  LpsCalTiltOvrState;
       float    TiltNoiseMax;
       float    TiltNoiseMin;
       float    DesiredTiltExtmm;
       uint8_t  CountOfSamples;
       float    TiltDumpCmdOvr;
       bool     LpsCalTiltOvrStartRequest;
       bool     CalTiltOverrideComplete;
       bool     CalTiltOverrideFail;
   } LpsSaNvmCalOnTheFlyTiltOverrides_t;

   typedef struct LpsSaNvmCalOnTheFlyDataChannel_MiscUpdates_s
   {
      LpsStat_t LiftHePres_Stat;
      LpsStat_t LiftRePres_Stat;
      LpsStat_t LiftCylLengthNorm_Stat;
      LpsHydOilTempSensorStat_t HydOilTemp_Stat;
      LpsStat_t LiftCylVel_Stat;
      LpsStat_t TiltCylLengthNorm_Stat;
      LpsStat_t BktAngle_Stat;
      bool LiftLeverInfo_LeverInfoAvailable;
      bool LiftLeverInfo_Faulted;
   } LpsSaNvmCalOnTheFlyDataChannel_MiscUpdates_t; // Part of LpsCalUpdate_t

   typedef struct LpsSaNvmCalOnTheFlyDataChannel_Overrides_s
   {
       uint8_t  LpsCalOvrState;
       int8_t   LpsCalAppInfRaiseCmdLmt;
       int8_t   LpsCalAppInfLowerCmdLmt;
       int8_t   LpsCalAppInfRackCmdLmt;
       int8_t   LpsCalAppInfDumpCmdLmt;
       int16_t   LpsCalAppInfEngSpdLmt;
       bool     LpsCalAppInfCalOvrActive;
       bool     LiftBottomFlag;
       bool     LiftTopFlag;
       bool CalOvrAcknowledge;
       float  LiftValveCmdPercent;
       bool   LiftInRaiseDetent;
       bool   LiftInLowerDetent;
       bool   LiftInCenter;
       bool   LiftFaulted;
       bool   LeverInfoAvailable;
       LpsSaNvmCalOnTheFlyTiltOverrides_t TiltOverrides;
   } LpsSaNvmCalOnTheFlyDataChannel_Overrides_t;

   typedef struct LpsSaNvmCalOnTheFlyDataChannel_DataOnTheFly_s
   {
      LpsSaNvmCalOnTheFlyDataChannel_StatusFlags_t       statusFlags;
      LpsSaNvmCalOnTheFlyDataChannel_CurveFitInfo_t      curveInfo;
      LpsSaNvmCalOnTheFlyDataChannel_MiscUpdates_t       calUpdates;
      LpsSaNvmCalOnTheFlyDataChannel_Overrides_t         calOverrides;
   } LpsSaNvmCalOnTheFlyDataChannel_DataOnTheFly_t;

   LpsSaNvmCalOnTheFlyDataChannel_DataOnTheFly_t lps_sa_nvm_calibration_data_on_the_fly;

   LpsSaNvmCalOnTheFlyDataChannelStorage() : lps_sa_nvm_calibration_data_on_the_fly() { }

   template <class Archive>
   void serialize(Archive& ar, unsigned int version)
   {
      // Serialize lps_sa_nvm_calibration_data_on_the_fly
      ar & lps_sa_nvm_calibration_data_on_the_fly.statusFlags.CalInProgress;
      ar & lps_sa_nvm_calibration_data_on_the_fly.statusFlags.EmptyBktCalDone;
      ar & lps_sa_nvm_calibration_data_on_the_fly.statusFlags.FullBktCalDone;

      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.HydOilTempMet;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.EnforceRaiseDetent;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.EnforceLowerDetent;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseFastVelDone;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseSlowFitDone;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseFastFitDone;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.LowerSlowFitDone;
      ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.InternalStep;
      if (version >= 2) {
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseNumPoints;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.LowerNumPoints;
      }

      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftHePres_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftRePres_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftCylLengthNorm_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.HydOilTemp_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftCylVel_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.TiltCylLengthNorm_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.BktAngle_Stat;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftLeverInfo_LeverInfoAvailable;
      ar & lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftLeverInfo_Faulted;

      if (version >= 3) {
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowImuOffsetTemp1;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowImuOffsetTemp2;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowImuOffsetTemp3;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowImuOffsetTemp1;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowImuOffsetTemp2;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowImuOffsetTemp3;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowImuOffsetFinal;
      }

      if (version >= 4) {
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LpsCalOvrState;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftBottomFlag;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftTopFlag;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LpsCalAppInfCalOvrActive;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LpsCalAppInfLowerCmdLmt;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LpsCalAppInfRaiseCmdLmt;


          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftFaulted;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftInCenter;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftInLowerDetent;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftInRaiseDetent;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.LiftValveCmdPercent;

          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.CalOvrAcknowledge;

      }

      if (version >= 5) {
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowLumpedWeightTemp1;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowLumpedWeightTemp2;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowLumpedWeightTemp3;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowLumpedWeightTemp1;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowLumpedWeightTemp2;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowLumpedWeightTemp3;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowImuOffsetFinal;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.FullSlowLumpedWeightFinal;
          ar & lps_sa_nvm_calibration_data_on_the_fly.curveInfo.IMUCalResults.EmptySlowLumpedWeightFinal;
      }

      if (version >= 6) {
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.CalTiltOverrideComplete;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.CalTiltOverrideFail;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.CountOfSamples;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.DesiredTiltExtmm;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.LpsCalTiltOvrStartRequest;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.LpsCalTiltOvrState;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.TiltDumpCmdOvr;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.TiltNoiseMax;
          ar & lps_sa_nvm_calibration_data_on_the_fly.calOverrides.TiltOverrides.TiltNoiseMin;
      }

   }

   void toCsv(CsvOutStream& out) const
   {
      // Dump lps_sa_nvm_calibration_data_on_the_fly
      out ("CalInProgress", lps_sa_nvm_calibration_data_on_the_fly.statusFlags.CalInProgress);
      out ("EmptyBktCalDone", lps_sa_nvm_calibration_data_on_the_fly.statusFlags.EmptyBktCalDone);
      out ("FullBktCalDone", lps_sa_nvm_calibration_data_on_the_fly.statusFlags.FullBktCalDone);

      out ("HydOilTempMet", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.HydOilTempMet);
      out ("EnforceRaiseDetent", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.EnforceRaiseDetent);
      out ("EnforceLowerDetent", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.EnforceLowerDetent);
      out ("RaiseFastVelDone", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseFastVelDone);
      out ("RaiseSlowFitDone", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseSlowFitDone);
      out ("RaiseFastFitDone", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.RaiseFastFitDone);
      out ("LowerSlowFitDone", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.LowerSlowFitDone);
      out ("InternalStep", lps_sa_nvm_calibration_data_on_the_fly.curveInfo.InternalStep);

      out ("LiftHePres_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftHePres_Stat);
      out ("LiftRePres_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftRePres_Stat);
      out ("LiftCylExt_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftCylLengthNorm_Stat);
      out ("HydOilTemp_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.HydOilTemp_Stat);
      out ("LiftCylVel_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftCylVel_Stat);
      out ("TiltCylExt_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.TiltCylLengthNorm_Stat);
      out ("BktAngle_Stat", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.BktAngle_Stat);
      out ("LiftLeverInfo_LeverInfoAvailable", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftLeverInfo_LeverInfoAvailable);
      out ("LiftLeverInfo_Faulted", lps_sa_nvm_calibration_data_on_the_fly.calUpdates.LiftLeverInfo_Faulted);
   }

private:
   // Nothing here yet
};

typedef Datum<LpsSaNvmCalOnTheFlyDataChannelStorage> LpsSaNvmCalOnTheFlyDataChannel;

BOOST_CLASS_VERSION(LpsSaNvmCalOnTheFlyDataChannelStorage, 6);
#endif

