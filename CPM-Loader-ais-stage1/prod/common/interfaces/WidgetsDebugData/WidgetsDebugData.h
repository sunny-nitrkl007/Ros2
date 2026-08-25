#include <ais/serialization/Datum.h> // For Boost serialization
#include "tire_rev.h"
#include "dig_timer.h"

#ifndef _WidgetsDebugData_h_
#define _WidgetsDebugData_h_

class WidgetsDebugDataStorage: public csvable
{
public:
    /* contains the distances traveled in each direction in mm */
    struct TireRevDistances{
        uint32_t ForwardMm = 0;
        uint32_t ReverseMm = 0;
        bool Status = false;

        template <class Archive>
        void serialize(Archive &ar, unsigned int version)
        {
            /* Archive Fields Here */
            ar & ForwardMm;
            ar & ReverseMm;
            ar & Status;
        }
    };

    /* contains the number of tire rotations in each direction */
    struct TireRevRotations{
        float Forward = 0.f;
        float Reverse = 0.f;
        bool Status = false;

        template <class Archive>
        void serialize(Archive &ar, unsigned int version)
        {
            /* Archive Fields Here */
            ar & Forward;
            ar & Reverse;
            ar & Status;
        }
    };

    WidgetsDebugDataStorage() :
        Distance(),
        Rotation() {}

    const TireRevDistances& getDistance() const
    {
        return Distance;
    }

    void setDistance( const TireRevDistances& distance )
    {
        Distance = distance;
    }

    const TireRevRotations& getRotation() const
    {
        return Rotation;
    }

    void setRotation( const TireRevRotations& rotation )
    {
        Rotation = rotation;
    }

    pcs_dig_state_t getDigState() const
    {
        return dig_state;
    }

    void setDigState(
            pcs_dig_state_t digState )
    {
        dig_state = digState;
    }

    uint16_t getDigTimeCurrent() const
    {
        return digTimeCurrent;
    }

    void setDigTimeCurrent(
            uint16_t digTimeCurrent )
    {
        this->digTimeCurrent = digTimeCurrent;
    }

    GEAR_INFO getGearInfo() const
    {
        return gear_info;
    }

    void setGearInfo(
            GEAR_INFO gearInfo )
    {
        gear_info = gearInfo;
    }

    GROUNDSPEED_INFO getGroundSpeedInfo() const
    {
        return ground_speed_info;
    }

    void setGroundSpeedInfo(
            GROUNDSPEED_INFO groundSpeedInfo )
    {
        ground_speed_info = groundSpeedInfo;
    }

    uint16_t getTravelingLoadedRevs() const
    {
        return travelingLoadedRevs;
    }

    void setTravelingLoadedRevs(
            uint16_t travelingLoadedRevs )
    {
        this->travelingLoadedRevs = travelingLoadedRevs;
    }

    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
        /* Archive Fields Here */
        ar & Distance;
        ar & Rotation;
        ar & digTimeCurrent;
        ar & travelingLoadedRevs;
        ar & dig_state;
        ar & ground_speed_info.speed;
        ar & ground_speed_info.data_status;
        ar & digTimeCurrent;
        ar & gear_info.gear;
        ar & gear_info.gear_status;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:
    /* Add Fields Here */
    TireRevDistances Distance;
    TireRevRotations Rotation;
    uint16_t digTimeCurrent = 0; // Resolution: 0.01 sec/bit
    uint16_t travelingLoadedRevs = 0;
    pcs_dig_state_t dig_state = PCS_DIG_UNKNOWN;
    GROUNDSPEED_INFO ground_speed_info = { 0.f, 0 };
    GEAR_INFO gear_info = { 0, 0 };
};

typedef Datum<WidgetsDebugDataStorage> WidgetsDebugData;

BOOST_CLASS_VERSION(WidgetsDebugDataStorage, 1);
#endif

