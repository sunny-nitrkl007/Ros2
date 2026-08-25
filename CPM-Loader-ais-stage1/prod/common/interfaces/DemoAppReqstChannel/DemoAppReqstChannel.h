#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#ifndef _DemoAppReqstChannel_h_
#define _DemoAppReqstChannel_h_

#include <cstdint>
#include <type_traits>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <interfaces/DemoAppTxChannel/InterfaceTypes.h>


enum class DemoAppReqstCommand : uint8_t {
    WRITE_LIFT_POSITION,
    WRITE_LIFT_POSITION_RATE,
    WRITE_TILT_POSITION,
    WRITE_TILT_POSITION_RATE,
    WRITE_PAYLOAD,
    WRITE_PAYLOAD_RATE,
    DIG,
    WRW,
    DUMP,
    LLW,
    PDUMP,
    CARRY,
    NONE
};

#define DemoAppReqstCommand_Base_t(val) (static_cast<typename std::underlying_type<DemoAppReqstCommand>::type>(val))

class DemoAppReqst {
public:
    DemoAppReqst() :
        command(DemoAppReqstCommand::NONE),
        arg(0.f),
        arg1(false) {}

    inline float liftposition() const { return arg; }
    inline void liftposition(float f) { arg = f; }

    inline float liftposition_rate() const { return arg; }
    inline void liftposition_rate(float f) { arg = f; }

    inline float tiltposition() const { return arg; }
    inline void tiltposition(float f) { arg = f; }

    inline float tiltposition_rate() const { return arg; }
    inline void tiltposition_rate(float f) { arg = f; }

    inline float payload() const { return arg; }
    inline void payload(float f) { arg = f; }

    inline float payload_rate() const { return arg; }
    inline void payload_rate(float f) { arg = f; }

    inline bool dig() const { return arg1; }
    inline void dig(bool s) { arg1 = s; }

    inline bool wrw() const { return arg1; }
    inline void wrw(bool s) { arg1 = s; }

    inline bool llw() const { return arg1; }
    inline void llw(bool s) { arg1 = s; }

    inline bool carry() const { return arg1; }
    inline void carry(bool s) { arg1 = s; }

    inline bool pdump() const { return arg1; }
    inline void pdump(bool s) { arg1 = s; }

    inline bool dump() const { return arg1; }
    inline void dump(bool s) { arg1 = s; }

    inline bool rack() const { return arg1; }
    inline void rack(bool s) { arg1 = s; }

    inline float weight1() const { return arg;}
    inline void weight1(float f) { arg = f;}


    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & command;
        ar & arg;
        ar & arg1;
    }

public:
    DemoAppReqstCommand command;

private:
    float arg;
    bool arg1;
};


class DemoAppReqstChannelStorage: public csvable
{
public:

    struct {
        bool enabled;
    } data;


    std::vector<DemoAppReqst> requests;

    DemoAppReqstChannelStorage() :
        data {
        false
    },
        requests() {}

    void reInit() {
        *this = DemoAppReqstChannelStorage();
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version) {
        if (version < 1) {
            throw boost::archive::archive_exception(boost::archive::archive_exception::exception_code::unsupported_class_version);
        }

        ar & data.enabled;
        ar & requests;
    }

    void toCsv(CsvOutStream& out) const {
        out("Not", "Supported");
    }

private:

};  

typedef Datum<DemoAppReqstChannelStorage> DemoAppReqstChannel;

BOOST_CLASS_VERSION(DemoAppReqstChannelStorage, 1);
#endif

