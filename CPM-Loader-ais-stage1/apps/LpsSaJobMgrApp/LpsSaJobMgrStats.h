#ifndef LPSSAJOBMGRSTATS_H
#define LPSSAJOBMGRSTATS_H

#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>

/*
 * LpsSaJobMgrStats Statistics Storage
 */
class LpsSaJobMgrStats {
public:
    LpsSaJobMgrStats(const std::string& filePath) :
        tipoffAssistActivationCount(0),
        filePath_(filePath),
        saveNeeded_(false) {};

    LpsSaJobMgrStats() : LpsSaJobMgrStats("") {}

    uint32_t tipoffAssistActivationCount;

    inline void setFilePath(const std::string& filePath) {
        filePath_ = filePath;
    }

    inline void setFilePath(const boost::filesystem::path& filePath) {
        setFilePath(filePath.string());
    }

    inline void notifyTipoffAssistActivation() {
        ++tipoffAssistActivationCount;
        setSaveNeeded();
    }

    bool load();
    bool reset();
    bool save(bool force = false) const;

    void setSaveNeeded() { saveNeeded_ = true; }

private:
    std::string filePath_;
    mutable bool saveNeeded_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & tipoffAssistActivationCount;
    }

};

BOOST_CLASS_VERSION(LpsSaJobMgrStats, 1);

#endif
