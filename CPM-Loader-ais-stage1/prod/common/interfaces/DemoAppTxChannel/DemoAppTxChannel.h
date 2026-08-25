#include <cstdint>
#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#ifndef _DemoAppTxChannel_h_
#define _DemoAppTxChannel_h_

class DemoAppTxChannelStorage: public csvable
{
public:
    DemoAppTxChannelStorage() :
        liftposition(0.f),
        liftposition_rate(0.f),
        tiltposition(0.f),
        tiltposition_rate(0.f),
        payload(0.f),
        payload_rate(0.f),
        dig(false),
        wrw(false),
        llw(false),
        dump(false),
        pdump(false),
        rack(false),
        carry(false),
        weight1(0.f),
        pres_lift_he_demo(0.f),
        pres_lift_re_demo(0.f),
        angle_lift_demo(0.f),
        angle_AFE_demo(0.f),
        angle_ABC_demo(0.f) {}

    float  liftposition;
    float  liftposition_rate;
    float  tiltposition;
    float  tiltposition_rate;
    float  payload;
    float  payload_rate;
    bool   dig;
    bool   wrw;
    bool   llw;
    bool   dump;
    bool   pdump;
    bool   rack;
    bool   carry;
    float  weight1;
    float  pres_lift_he_demo;  // Lift Cylinder He Pressure
    float  pres_lift_re_demo;  // Lift Cylinder Re Pressure
    float  angle_lift_demo;    // Lift Angle
    float  angle_AFE_demo;     // Angle AFE
    float  angle_ABC_demo;     // Angle ABC


    template <class Archive>

    void serialize(Archive &ar, unsigned int version)
    {
        ar & liftposition;
        ar & liftposition_rate;
        ar & tiltposition;
        ar & tiltposition_rate;
        ar & payload;
        ar & payload_rate;
        ar & dig;
        ar & wrw;
        ar & llw;
        ar & dump;
        ar & pdump;
        ar & rack;
        ar & carry;
        ar & weight1;
        ar & pres_lift_he_demo;
        ar & pres_lift_re_demo;
        ar & angle_lift_demo;
        ar & angle_AFE_demo;
        ar & angle_ABC_demo;
    }

    void toCsv(CsvOutStream& out) const
    {
        out("Lift position", liftposition);
        out("Lift position rate", liftposition_rate);
        out("Tilt position", tiltposition);
        out("Tilt position rate", tiltposition_rate);
        out("Payload", payload);
        out("Payload rate", payload_rate);
        out("Dig", dig);
        out("Wrw", wrw);
        out("llw", llw);
        out("dump", dump);
        out("pdump", pdump);
        out("rack", rack);
        out("carry", carry);
        out("He Pressure",pres_lift_he_demo);
        out("Re Pressure",pres_lift_re_demo);
        out("Angle lift",angle_lift_demo);
        out("Angle AFE",angle_AFE_demo);
        out("Angle ABC",angle_ABC_demo);
    }

 private:
};

typedef Datum<DemoAppTxChannelStorage> DemoAppTxChannel;

BOOST_CLASS_VERSION(DemoAppTxChannelStorage, 0);
#endif

