#ifndef LPSSAJOBMGRSIMPLECAL_H_
#define LPSSAJOBMGRSIMPLECAL_H_

#include <cstdint>
#include <deque>
#include <vector>

#include <boost/serialization/deque.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem.hpp>

#include <interfaces/LpsSaJobMgrTxChannel/LpsSaJobMgrTxChannel.h>
#include <interfaces/LpsSaLoadRecordChannel/LpsSaLoadRecordChannel.h>


/*
 * LpsSaJobMgrSimpleCal Storage
 */
struct LpsSaJobMgrSimpleCal {
public:
    struct Entry {
        SimpleCalData_t simpleCalData;
        LpsSaLoadRecordChannelStorage loadRecord;

        template <class Archive>
        void serialize(Archive& ar, unsigned int version) {
            ar & simpleCalData;
            ar & loadRecord;
        }
    };

    LpsSaJobMgrSimpleCal(const std::string& filePath, uint_least32_t maxQueueSize) :
        filePath_(filePath),
        maxQueueSize_(maxQueueSize),
        saveNeeded_(false),
        queue_() {}

    LpsSaJobMgrSimpleCal() : LpsSaJobMgrSimpleCal("", 15) {}

    inline void setFilePath(const std::string& filePath) {
        filePath_ = filePath;
    }

    inline void setFilePath(const boost::filesystem::path& filePath) {
        setFilePath(filePath.string());
    }

    inline void setMaxQueueSize(uint_least32_t maxQueueSize) {
        maxQueueSize_ = maxQueueSize;
        if (queue_.size() > maxQueueSize) {
            queue_.resize(maxQueueSize);
        }
    }

    inline size_t queueSize() const {
        return queue_.size();
    }

    void addEntry(const LpsSaLoadRecordChannelStorage& loadRecord);
    void eraseEntry(const std::string& timestamp);

    void getSimpleCalData(std::deque<SimpleCalData_t>& simpleCalData) const;
    void getSimpleCalData(std::vector<SimpleCalData_t>& simpleCalData) const;

    bool load();
    bool reset();
    bool save(bool force = false) const;

    void setSaveNeeded() { saveNeeded_ = true; }

private:
    std::string filePath_;
    uint_least32_t maxQueueSize_;
    mutable bool saveNeeded_;
    std::deque<Entry> queue_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & queue_;
    }
};

BOOST_CLASS_VERSION(LpsSaJobMgrSimpleCal, 1);



#endif /* LPSSAJOBMGRSIMPLECAL_H_ */
