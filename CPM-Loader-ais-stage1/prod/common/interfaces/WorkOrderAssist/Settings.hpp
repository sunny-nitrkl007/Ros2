#ifndef WORKORDERASSIST_SETTINGS_HPP
#define WORKORDERASSIST_SETTINGS_HPP

#include <string>

#include <boost/serialization/version.hpp>

struct WorkOrderAssistSettings {
public:
    WorkOrderAssistSettings() :
        installed(false),
        enabled(true),
        jobTimeWarningMinutes(20.0f),
        siteId(-1),
        siteName() {}

    bool installed;
    bool enabled;
    float jobTimeWarningMinutes;
    int32_t siteId;
    std::string siteName;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & installed;
        ar & enabled;
        ar & jobTimeWarningMinutes;
        ar & siteId;
        ar & siteName;

        // Enabled by default in version 0
        if (version < 1) {
            enabled = true;
        }
    }
};

BOOST_CLASS_VERSION(WorkOrderAssistSettings, 1);

#endif
