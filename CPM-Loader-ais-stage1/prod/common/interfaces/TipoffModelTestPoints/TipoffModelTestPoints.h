#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#include <boost/serialization/vector.hpp>

#ifndef _TipoffModelTestPoints_h_
#define _TipoffModelTestPoints_h_

class TipoffModelTestPointsStorage: public csvable
{
public:
    TipoffModelTestPointsStorage():
        toa_pfwSecLatch(false),
        toa_pfwSecPayload(0.f),
        toa_pfwSecStatus(0),
        toa_pfwSecStDev(0.f),
        toa_targetPayloadFinal(0.f),
        toa_targetPayloadStatus(0.f),
        toa_pfwNlNotchMeanEst(0.f),
        toa_pfwNlNotchStDevEst(0.f),
        toa_lwLpfPost(0.f),
        toa_payloadAdjusted(0.f),
        toa_sendPayloadArb(0.f),
        toa_sfuncInGravX(0.f),
        toa_sfuncInGravY(0.f),
        toa_sfuncInLiftForce(0.f),
        toa_sfuncInTiltForce(0.f),
        toa_sfuncOutRawMassTonne(0.f),
        toa_appNumber(0),
        toa_liftBoreDia(0.f),
        toa_liftRodDia(0.f),
        toa_payloadAncAdjusted(0.f),
        toa_payloadAncZeroed(0.f),
        toa_ratedPayload(0.f),
        toa_tiltBoreDia(0.f),
        toa_tiltNumCyl(0),
        toa_tiltRodDia(0.f),
        toa_toolBcLength(0.f),
        toa_toolBcAngle(0.f),
        toa_imu_cal_pitch_angle(0.f),
        toa_pfwUnsecStatus(0),
        toa_pfwUnsecBoundLower(0.f),
        toa_pfwUnsecBoundUpper(0.f),
        toa_lwSpillRate(0.f),
        toa_lwStatus(0),
        toa_rawDist(0.f),
        toa_rawPayload(0.f),
        toa_payloadZeroed(0.f),
        toa_pfwIsWarm(false),
        toa_pfwSecIsSecure(false),
        toa_pfwSecIsNoSliding(false),
        toa_pfwSecIsNoCollapsing(false),
        toa_pfwSecIsRacked(false),
        toa_pfwSecMinBucketAng(0.f),
        toa_pfwMcOk(false),
        toa_pfwMcMaybeStalled(false),
        toa_pfwMcMaybeRackStall(false),
        toa_pfwMcMaybeGrounded(false),
        toa_statPayloadNlNotchOneUp(0.f),
        toa_statPayloadNlNotchOneLow(0.f),
        toa_statPayloadNlNotchOneSize(0.f),
        toa_statPayloadNlNotchTwoUp(0.f),
        toa_statPayloadNlNotchTwoLow(0.f),
        toa_statPayloadNlNotchTwoSize(0.f),
        toa_statPayloadNlOneUp(0.f),
        toa_statPayloadNlOneLow(0.f),
        toa_statPayloadNlOneSize(0.f),
        toa_statPayloadNlTwoUp(0.f),
        toa_statPayloadNlTwoLow(0.f),
        toa_statPayloadNlTwoSize(0.f),
        toa_pfwNlStDevEst(0.f),
        toa_pfwNlMeanEst(0.f),
        toa_pfwPayloadFiltFinal(0.f),
        toa_pfwPayloadNotchFiltFinal(0.f),
        toa_pfwPayloadNotchPost(0.f),
        toa_pfwLatchInvalidate(false),
        toa_imuRawEefAcclX(0.f),
        toa_imuRawEefAcclY(0.f),
        toa_imuRawEefAcclZ(0.f),
        toa_imuNeefGravX(0.f),
        toa_imuNeefGravY(0.f),
        toa_imuNeefGravZ(0.f),
        toa_imuNeefPitch(0.f),
        toa_targetCompMargin(0.f),
        toa_targetPayloadPostComp(0.f),
        toa_targetPayloadPreComp(0.f),
        toa_statPayloadSecureTwoSize(0.f),
        toa_statPayloadSecureOneUpper(0.f),
        toa_statPayloadSecureOneLower(0.f),
        toa_statPayloadSecureOneSize(0.f),
        toa_statPayloadSecureTwoUpper(0.f),
        toa_statPayloadSecureTwoLower(0.f),
        toa_sendCpmLivePayload(0.f)
    { }

    // Fields
    bool toa_pfwSecLatch;
    float toa_pfwSecPayload;
    uint8_t toa_pfwSecStatus;
    float toa_pfwSecStDev;
    float toa_targetPayloadFinal;
    bool toa_targetPayloadStatus;
    float toa_pfwNlNotchMeanEst;
    float toa_pfwNlNotchStDevEst;
    float toa_lwLpfPost;
    float toa_payloadAdjusted;
    float toa_sendPayloadArb;
    float toa_sfuncInGravX;
    float toa_sfuncInGravY;
    float toa_sfuncInLiftForce;
    float toa_sfuncInTiltForce;
    float toa_sfuncOutRawMassTonne;
    uint32_t toa_appNumber;
    float toa_liftBoreDia;
    float toa_liftRodDia;
    float toa_payloadAncAdjusted;
    float toa_payloadAncZeroed;
    float toa_ratedPayload;
    float toa_tiltBoreDia;
    uint8_t toa_tiltNumCyl;
    float toa_tiltRodDia;
    float toa_toolBcLength;
    float toa_toolBcAngle;
    float toa_imu_cal_pitch_angle;
    uint8_t toa_pfwUnsecStatus;
    float toa_pfwUnsecBoundLower;
    float toa_pfwUnsecBoundUpper;
    float toa_lwSpillRate;
    uint8_t toa_lwStatus;
    float toa_rawDist;
    float toa_rawPayload;
    float toa_payloadZeroed;
    bool toa_pfwIsWarm;
    bool toa_pfwSecIsSecure;
    bool toa_pfwSecIsNoSliding;
    bool toa_pfwSecIsNoCollapsing;
    bool toa_pfwSecIsRacked;
    float toa_pfwSecMinBucketAng;
    bool toa_pfwMcOk;
    bool toa_pfwMcMaybeStalled;
    bool toa_pfwMcMaybeRackStall;
    bool toa_pfwMcMaybeGrounded;
    float toa_statPayloadNlNotchOneUp;
    float toa_statPayloadNlNotchOneLow;
    float toa_statPayloadNlNotchOneSize;
    float toa_statPayloadNlNotchTwoUp;
    float toa_statPayloadNlNotchTwoLow;
    float toa_statPayloadNlNotchTwoSize;
    float toa_statPayloadNlOneUp;
    float toa_statPayloadNlOneLow;
    float toa_statPayloadNlOneSize;
    float toa_statPayloadNlTwoUp;
    float toa_statPayloadNlTwoLow;
    float toa_statPayloadNlTwoSize;
    float toa_pfwNlStDevEst;
    float toa_pfwNlMeanEst;
    float toa_pfwPayloadFiltFinal;
    float toa_pfwPayloadNotchFiltFinal;
    float toa_pfwPayloadNotchPost;
    bool toa_pfwLatchInvalidate;
    float toa_imuRawEefAcclX;
    float toa_imuRawEefAcclY;
    float toa_imuRawEefAcclZ;
    float toa_imuNeefGravX;
    float toa_imuNeefGravY;
    float toa_imuNeefGravZ;
    float toa_imuNeefPitch;
    float toa_targetCompMargin;
    float toa_targetPayloadPostComp;

    float toa_targetPayloadPreComp;
    float toa_statPayloadSecureTwoSize;
    float toa_statPayloadSecureOneUpper;
    float toa_statPayloadSecureOneLower;
    float toa_statPayloadSecureOneSize;
    float toa_statPayloadSecureTwoUpper;
    float toa_statPayloadSecureTwoLower;
    float toa_sendCpmLivePayload;

    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
        ar & toa_pfwSecLatch;
        ar & toa_pfwSecPayload;
        ar & toa_pfwSecStatus;
        ar & toa_pfwSecStDev;
        ar & toa_targetPayloadFinal;
        ar & toa_targetPayloadStatus;
        ar & toa_pfwNlNotchMeanEst;
        ar & toa_pfwNlNotchStDevEst;
        ar & toa_lwLpfPost;
        ar & toa_payloadAdjusted;
        ar & toa_sendPayloadArb;
        ar & toa_sfuncInGravX;
        ar & toa_sfuncInGravY;
        ar & toa_sfuncInLiftForce;
        ar & toa_sfuncInTiltForce;
        ar & toa_sfuncOutRawMassTonne;
        ar & toa_appNumber;
        ar & toa_liftBoreDia;
        ar & toa_liftRodDia;
        ar & toa_payloadAncAdjusted;
        ar & toa_payloadAncZeroed;
        ar & toa_ratedPayload;
        ar & toa_tiltBoreDia;
        ar & toa_tiltNumCyl;
        ar & toa_tiltRodDia;
        ar & toa_toolBcLength;
        ar & toa_imu_cal_pitch_angle;
        ar & toa_pfwUnsecStatus;
        ar & toa_pfwUnsecBoundLower;
        ar & toa_pfwUnsecBoundUpper;
        ar & toa_lwSpillRate;
        ar & toa_lwStatus;
        ar & toa_rawDist;
        ar & toa_rawPayload;
        ar & toa_payloadZeroed;
        ar & toa_pfwIsWarm;
        ar & toa_pfwSecIsSecure;
        ar & toa_pfwSecIsNoSliding;
        ar & toa_pfwSecIsNoCollapsing;
        ar & toa_pfwSecIsRacked;
        ar & toa_pfwSecMinBucketAng;
        ar & toa_pfwMcOk;
        ar & toa_pfwMcMaybeStalled;
        ar & toa_pfwMcMaybeRackStall;
        ar & toa_pfwMcMaybeGrounded;
        ar & toa_statPayloadNlNotchOneUp;
        ar & toa_statPayloadNlNotchOneLow;
        ar & toa_statPayloadNlNotchOneSize;
        ar & toa_statPayloadNlNotchTwoUp;
        ar & toa_statPayloadNlNotchTwoLow;
        ar & toa_statPayloadNlNotchTwoSize;
        ar & toa_statPayloadNlOneUp;
        ar & toa_statPayloadNlOneLow;
        ar & toa_statPayloadNlOneSize;
        ar & toa_statPayloadNlTwoUp;
        ar & toa_statPayloadNlTwoLow;
        ar & toa_statPayloadNlTwoSize;
        ar & toa_pfwNlStDevEst;
        ar & toa_pfwNlMeanEst;
        ar & toa_pfwPayloadFiltFinal;
        ar & toa_pfwPayloadNotchFiltFinal;
        ar & toa_pfwPayloadNotchPost;
        ar & toa_pfwLatchInvalidate;
        ar & toa_imuRawEefAcclX;
        ar & toa_imuRawEefAcclY;
        ar & toa_imuRawEefAcclZ;
        ar & toa_imuNeefGravX;
        ar & toa_imuNeefGravY;
        ar & toa_imuNeefGravZ;
        ar & toa_imuNeefPitch;
        ar & toa_targetCompMargin;
        ar & toa_targetPayloadPostComp;
        ar & toa_targetPayloadPreComp;
        ar & toa_statPayloadSecureTwoSize;
        ar & toa_statPayloadSecureOneUpper;
        ar & toa_statPayloadSecureOneLower;
        ar & toa_statPayloadSecureOneSize;
        ar & toa_statPayloadSecureTwoUpper;
        ar & toa_statPayloadSecureTwoLower;
        ar & toa_sendCpmLivePayload;
        ar & toa_toolBcAngle;
    }

    void toCsv(CsvOutStream& out) const
    {
        // Not currently supported
    }

private:
    /* Add Fields Here */
};

typedef Datum<TipoffModelTestPointsStorage> TipoffModelTestPoints;

BOOST_CLASS_VERSION(TipoffModelTestPointsStorage, 1);

#endif


