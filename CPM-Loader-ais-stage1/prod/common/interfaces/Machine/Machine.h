#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>

#ifndef _Machine_h_
#define _Machine_h_
class MachineStorage: public csvable
{  
public:  
     MachineStorage()
		:  appNumber(8200),
		   /* Payload */
		   payload_ratio(0),
		   payload_ratio_status(0),
		   payload_ratio_raw(0),
		   payload_ratio_raw_status(0),
		   payload_weight(0),
		   payload_weight_status(0),
		   payload_calc_method(0),
		   /* impl */
		   lift_valve_cmd(0),
		   tilt_valve_cmd(0),
		   bucket_angle(0),
		   lift_angle(0),
		   tilt_lever_cmd(0),
		   tilt_cyl_pos(0),
		   impl_operation_status(0),
		   machine_idle_status(0),
		   dump_solenoid_percent(0),
		   rack_solenoid_percent(0),
		   lower_solenoid_percent(0),
		   raise_solenoid_percent(0),
		   brake_pedal_position(0),
		   lift_cylinder_he_pressure(0),
		   lift_cylinder_re_pressure(0),
		   hydarulic_oil_temp(0),
		   bucket_payload(0),

		   /* trans */
		   desired_gear(0),
		   ground_speed(0),
		   engine_speed(0),
		   torque_converter_speed(0),

		   keyswitch_state(0),
		   real_time(0),
		   real_time_status(0),
		   timezone_offset(0),
		   total_fuel_high_res(0),
		   total_fuel_low_res(0),
		   fuel_rate(0),
		   ard_fuel_rate(0),
		   service_hour_meter(0),
		   external_force_normalized(0),
		   /* gps */
		   gps_latitude(0),
		   gps_longitude(0),
		   gps_status(0)
		{
		};
     ///////////////////////////////////////////////////////////////////////////////
     /// @brief Output data in CSV format
     /// @param out - out stream to dump data
     ///////////////////////////////////////////////////////////////////////////////
     void toCsv(CsvOutStream& out) const
     {
    	 out("appNumber" , (appNumber));
     }

     template <class Archive>
     void serialize(Archive &ar, unsigned int version)
     {
    	 ar & appNumber;
    	 ar & payload_ratio;
    	 ar & payload_ratio_status;
    	 ar & payload_ratio_raw;
    	 ar & payload_ratio_raw_status;
    	 ar & payload_weight;
    	 ar & payload_weight_status;
    	 ar & payload_calc_method;
    	 ar & lift_valve_cmd;
    	 ar & tilt_valve_cmd;
    	 ar & bucket_angle;
    	 ar & lift_angle;
    	 ar & tilt_lever_cmd;
    	 ar & tilt_cyl_pos;
    	 ar & impl_operation_status;
    	 ar & machine_idle_status;
    	 ar & dump_solenoid_percent;
    	 ar & rack_solenoid_percent;
    	 ar & lower_solenoid_percent;
    	 ar & raise_solenoid_percent;
    	 ar & brake_pedal_position;
    	 ar & lift_cylinder_he_pressure;
    	 ar & lift_cylinder_re_pressure;
    	 ar & hydarulic_oil_temp;
    	 ar & bucket_payload;
    	 ar & desired_gear;
    	 ar & ground_speed;
    	 ar & engine_speed;
    	 ar & torque_converter_speed;
    	 ar & keyswitch_state;
    	 ar & real_time;
    	 ar & real_time_status;
    	 ar & timezone_offset;
    	 ar & total_fuel_high_res;
    	 ar & total_fuel_low_res;
    	 ar & fuel_rate;
    	 ar & ard_fuel_rate;
    	 ar & service_hour_meter;
    	 ar & external_force_normalized;
    	 ar & gps_latitude;
    	 ar & gps_longitude;
    	 ar & gps_status;
     }

     /*****************************************************************************/
     /***                          SET/GET FUNCTIONS                            ***/
     /*****************************************************************************/
     void setAppNumber(uint16_t newValue) { appNumber = newValue; };
     uint16_t getAppNumber() const { return appNumber;	 };

     void setPayloadRatio(float newValue) { payload_ratio = newValue; };
     float getPayloadRatio() const { return payload_ratio; };

     void setPayloadRatioStatus(uint16_t newValue) { payload_ratio_status = newValue; };
     uint16_t getPayloadRatioStatus() const { return payload_ratio_status; };

     void setPayloadRatioRaw(float newValue) { payload_ratio_raw = newValue; };
     float getPayloadRatioRaw() const { return payload_ratio_raw; };

     void setPayloadRatioRawStatus(uint16_t newValue) { payload_ratio_raw_status = newValue; };
     uint16_t getPayloadRatioRawStatus() const { return payload_ratio_raw_status; };

     void setPayloadWeight(float newValue) { payload_weight = newValue; };
     float getPayloadWeight() const { return payload_weight; };

     void setPayloadWeightStatus(uint16_t newValue) { payload_weight_status = newValue; };
     uint16_t getPayloadWeightStatus() const { return payload_weight_status; };

     void setPayloadCalcMethod(uint16_t newValue) { payload_calc_method = newValue; };
     uint16_t getPayloadCalcMethod() const { return payload_calc_method; };

     void setLiftValveCmd(float newValue)	 {
    	 /* Lift Valve Command (1/16)0.0625%/bit */
    	 lift_valve_cmd = newValue * 16;
     };
     int16_t getLiftValveCmd() const { return lift_valve_cmd; };

     void setTiltValveCmd(float newValue)	 {
    	 /* Tilt Valve Command (1/16)0.0625%/bit */
    	 tilt_valve_cmd = newValue * 16;
     };
     int16_t getTiltValveCmd() const { return tilt_valve_cmd; };

     void setBucketAngle(float newValue)	 {
    	 /* Bucket Angle (1/64 deg/bit or 0.015625deg/bit*/
    	 bucket_angle = newValue * 64;
     };
     int16_t getBucketAngle() const { return bucket_angle; };

     void setLiftAngle(float newValue)	 {
    	 /* Lift Angle (1/64 deg/bit or 0.015625deg/bit*/
    	 lift_angle = newValue * 64;
     };
     float getLiftAngle() const { return lift_angle; };

     void setTiltLeverCmd(float newValue) {
    	 /* Tilt Lever Command ( 1/256 %/bit or 0.00390625 %/bit ) */
    	 tilt_lever_cmd = newValue * 256;
     };
     int16_t getTiltLeverCmd() const { return tilt_lever_cmd; };

     /* TODO: Needs work, hard-coding tilt cylinder stroke for 982 */
     void setTiltCylPos(float newValue) {
    	 /* Tilt cyclinder extension (absolute mm) converted to 0.00390625 %/bit */
    	 //tilt_cyl_pos = newValue/742*100 * 256;
    	 tilt_cyl_pos = newValue * 256;
     };
     uint16_t getTiltCylPos() const { return tilt_cyl_pos; };

     void setImplOperationStatus(uint16_t newValue) { impl_operation_status = newValue; };
     uint16_t getImplOperationStatus() const { return impl_operation_status; };

     void setMachineIdleStatus(uint16_t newValue) { machine_idle_status = newValue; };
     uint16_t getMachineIdleStatus() const { return machine_idle_status; };

     void setHydraulicOilTemp(int16_t newValue) { hydarulic_oil_temp = newValue; };
     int16_t getHydraulicOilTemp() const { return hydarulic_oil_temp; };

     void setBucketPayload(float newValue) { bucket_payload = newValue; };
     float getBucketPayload() const { return bucket_payload; };

     void setDumpSolenoidPercent(uint8_t newValue) { dump_solenoid_percent = newValue; };
     uint8_t getDumpSolenoidPercent() const { return dump_solenoid_percent; };

     void setRackSolenoidPercent(uint8_t newValue) { rack_solenoid_percent = newValue; };
     uint8_t getRackSolenoidPercent() const { return rack_solenoid_percent; };

     void setLowerSolenoidPercent(uint8_t newValue) { lower_solenoid_percent = newValue; };
     uint8_t getLowerSolenoidPercent() const { return lower_solenoid_percent; };

     void setRaiseSolenoidPercent(uint8_t newValue) { raise_solenoid_percent = newValue; };
     uint8_t getRaiseSolenoidPercent() const { return raise_solenoid_percent; };

     void setBrakePedalPosition(float newValue) { brake_pedal_position = newValue; };
     float getBrakePedalPosition() const { return brake_pedal_position; };

     void setLiftCylinderHEPressure(uint16_t newValue) { lift_cylinder_he_pressure = newValue; };
     float getLiftCylinderHEPressure() const { return lift_cylinder_he_pressure; };

     void setLiftCylinderREPressure(uint16_t newValue) { lift_cylinder_re_pressure = newValue; };
     float getLiftCylinderREPressure() const { return lift_cylinder_re_pressure; };

     void setDesiredGear(uint16_t desired_gear_dl)	 {
    	 if (desired_gear_dl >> 8 == 64) //forward
    	 {
    		 desired_gear = desired_gear_dl & 0x001F;
    	 }
    	 else if (desired_gear_dl >> 8 == 16) //reverse
    	 {
    		 desired_gear = -(desired_gear_dl & 0x001F);
    	 }
    	 else
    	 {
    		 desired_gear = 0;
    	 }
     };
     void setDesiredGearsc2(int16_t desired_gear_dl)	 {
         	 desired_gear = desired_gear_dl;
          };
     int16_t getDesiredGear() const { return desired_gear; };

     void setGroundSpeed(float ground_speed_kph)	 {
    	 /* Ground Speed ( 1/256 km/h per bit ) */
    	 ground_speed = ground_speed_kph * 256;
     };
     int16_t getGroundSpeed() const { return ground_speed; };

     void setEngineSpeed(float engine_speed_rpm) {
    	 /* engine speed 1/8 rpm/bit or 0.125 rpm/bit */
    	 engine_speed = engine_speed_rpm * 8;
     };
     int16_t getEngineSpeed() const { return engine_speed; };

     void setTorqueConverterSpeed(int16_t newValue) {
    	 torque_converter_speed = newValue;
     };
     int16_t getTorqueConverterSpeed() const { return torque_converter_speed; };

     void setKeyswitchState(uint16_t newValue) { keyswitch_state = newValue; };
     uint16_t getKeyswitchState() const { return keyswitch_state; };

     void setRealTime(uint32_t newValue) { real_time = newValue; };
     uint32_t getRealTime() const { return real_time; };

     void setRealTimeStatus(uint8_t newValue) { real_time_status = newValue; };
     uint8_t getRealTimeStatus() const { return real_time_status; };

     void setTimezoneOffset(int16_t newValue) { timezone_offset = newValue; };
     int16_t getTimezoneOffset() const { return timezone_offset; };

     void setTotalFuelHighRes(float newValue) {
    	 /* Total Fuel (high res) ( 0.001 Gal/bit ) */
    	 total_fuel_high_res = newValue * 1000;
     };
     uint32_t getTotalFuelHighRes() const { return total_fuel_high_res; };

     void setTotalFuelLowRes(float newValue) {
    	 /* Total Fuel (low res) ( 0.125 Gal/bit ) */
    	 total_fuel_low_res = newValue * 8;
     };
     uint32_t getTotalFuelLowRes() const { return total_fuel_low_res; };

     void setFuelRate(uint16_t newValue) { fuel_rate = newValue; };
     uint16_t getFuelRate() const { return fuel_rate; };

     void setArdFuelRate(uint16_t newValue) { ard_fuel_rate = newValue; };
     uint16_t getArdFuelRate() const { return ard_fuel_rate; };

     void setServiceHourMeter(uint32_t newValue) { service_hour_meter = newValue; };
     uint32_t getServiceHourMeter() const { return service_hour_meter; };

     void setExternalForceNormalized(float newValue) {
    	 /* Fore-Aft Normalized External Force ( 0.0005 g/bit ) */
    	 external_force_normalized = newValue * 2000;
     };
     int16_t getExternalForceNormalized() const { return external_force_normalized; };

     void setGpsLatitude(uint32_t newValue) { gps_latitude = newValue; };
     uint32_t getGpsLatitude() const { return gps_latitude; };

     void setGpsLongitude(uint32_t newValue) { gps_longitude = newValue; };
     uint32_t getGpsLongitude() const { return gps_longitude; };

     void setGpsStatus(uint8_t newValue) { gps_status = newValue; };
     uint8_t getGpsStatus() const { return gps_status; };

     uint16_t appNumber;
     float payload_ratio;
     uint16_t payload_ratio_status;
     float payload_ratio_raw;
     uint16_t payload_ratio_raw_status;
     float payload_weight;
     uint16_t payload_weight_status;
     uint16_t payload_calc_method;

     int16_t lift_valve_cmd;
     int16_t tilt_valve_cmd;
     int16_t bucket_angle;
     float lift_angle;
     int16_t tilt_lever_cmd;
     uint16_t tilt_cyl_pos;
     uint16_t impl_operation_status;
     uint16_t machine_idle_status;
     uint8_t dump_solenoid_percent;
     uint8_t rack_solenoid_percent;
     uint8_t lower_solenoid_percent;
     uint8_t raise_solenoid_percent;
     float brake_pedal_position;
     uint16_t lift_cylinder_he_pressure;
     uint16_t lift_cylinder_re_pressure;
     int16_t hydarulic_oil_temp;
     float bucket_payload;

     int16_t desired_gear;
     int16_t ground_speed;
     int16_t engine_speed;
     int16_t torque_converter_speed;

     uint16_t keyswitch_state;
     uint32_t real_time;
     uint8_t real_time_status;
     int16_t timezone_offset;
     uint32_t total_fuel_high_res;
     uint32_t total_fuel_low_res;
     uint16_t fuel_rate;
     uint16_t ard_fuel_rate;
     uint32_t service_hour_meter;

     int16_t external_force_normalized;

     uint32_t gps_latitude;
     uint32_t gps_longitude;
     uint8_t gps_status;

private:
     /* Add Fields Here */
};  

typedef Datum<MachineStorage> Machine;

BOOST_CLASS_VERSION(MachineStorage, 1);
#endif

