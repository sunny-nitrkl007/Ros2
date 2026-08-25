#ifndef LPSSALISTMGR_HPP
#define LPSSALISTMGR_HPP

#include <sstream>
#include <fstream>
#include <iterator>

#include <boost/filesystem.hpp>

#include <ais/log/Logger.h>

#include <fileio/sha1_fstream.hpp>
#include <fileio/iflocker.hpp>
#include <fileio/oflocker.hpp>
#include <hub/filesystem/extensions/nvm.h>

#include <interfaces/LpsSaListMgr/InterfaceCommon.h>
#include <interfaces/LpsSaListMgr/TxInterface.hpp>
#include <interfaces/LpsSaUI/DisplayStateInterface.hpp>

#include "CPMList.h"
#include "CPMListSync/CPMList.pb.h"

/*
 * LpsSaListRevisionTimestamp - stores the last revision timestamp of the list
 */
class LpsSaListRevisionTimestamp {

public:
    LpsSaListRevisionTimestamp() : 
        revision_(),
        storageRoot_(),
        fileName_() {}        

    void setStoragePath(boost::filesystem::path storageRoot, std::string fileName) {
        storageRoot_ = storageRoot;
        fileName_ = fileName;
    }

    // Load data from storage file(s)
    bool load() {
        bool success = ais::filesystem::file::read(storageRoot_, fileName_, *this);
     
        if (success) {
            //do nothing
        }
        else if (storageRoot_.empty() || fileName_.empty()) {
            AIS_LOG_ERROR("Storage root or filename not set for list revision");            
        }
        else {
            AIS_LOG_ERROR("Error reading list revision from file storage");
        }

        return success;
    }
    
    void setRevision(const std::chrono::system_clock::time_point& timepoint) {
        revision_ = timepoint;

        save();
    }

    const std::chrono::system_clock::time_point& getRevision() const {
        return revision_;     
    }
                    

private:
    // Save data to storage file(s)
    bool save() const {
        bool success = ais::filesystem::file::store(storageRoot_, fileName_, *this);

        if (success) {
            //do nothing
        }
        else {
            AIS_LOG_ERROR("Error saving list revision to file");
        }

        return success;
    }


    // revision timestamp
    std::chrono::system_clock::time_point revision_;
    boost::filesystem::path storageRoot_;
    std::string fileName_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & revision_;
    }
};


class LpsSaListMgr {
public:
    LpsSaListMgr(LpsSaListMgrInterfaceListType listType) :
        storagePath_(),
        listType_(listType),
        listRevision_(),
        logPrefix_(std::string(LpsSaListMgrInterfaceListType_string(listType)) + " :") {}

    virtual ~LpsSaListMgr() = default;

    void setStoragePath(const boost::filesystem::path& storageRoot) {
        storagePath_ = (storageRoot / getJsonFileName()).string();
    }

    LpsSaListMgrInterfaceListType getListType() const {
        return listType_;
    }    

    static std::string getDefaultListName(LpsSaListMgrInterfaceListType type) {
        switch (type) {
        case (LpsSaListMgrInterfaceListType::TRUCK): { return "Truck"; }
        case (LpsSaListMgrInterfaceListType::MATERIAL): { return "Material"; }
        case (LpsSaListMgrInterfaceListType::OPERATOR_ID): { return "Operator"; }
        case (LpsSaListMgrInterfaceListType::CUSTOM1): { return "Custom 1"; }
        case (LpsSaListMgrInterfaceListType::CUSTOM2): { return "Custom 2"; }
        case (LpsSaListMgrInterfaceListType::CUSTOM3): { return "Custom 3"; }
        case (LpsSaListMgrInterfaceListType::CUSTOM4): { return "Custom 4"; }
        case (LpsSaListMgrInterfaceListType::NONE): { return "None"; }
        default: { return "Unknown"; }
        }
    }

    uint32_t getListFileType() const {
           switch (getListType()) {
           case (LpsSaListMgrInterfaceListType::TRUCK): { return 10642; }
           case (LpsSaListMgrInterfaceListType::MATERIAL): { return 10643; }
           case (LpsSaListMgrInterfaceListType::CUSTOM1): { return 10638; }
           case (LpsSaListMgrInterfaceListType::CUSTOM2): { return 10639; }
           case (LpsSaListMgrInterfaceListType::CUSTOM3): { return 10640; }
           case (LpsSaListMgrInterfaceListType::CUSTOM4): { return 10641; }
           default: { return 0; }
           }
       }

    virtual bool initialize(const boost::filesystem::path& storageRoot) {
        setStoragePath(storageRoot);

        // initialize and load list revision
        std::string revisionFilename = LpsSaListMgrInterfaceListType_string(listType_);
        revisionFilename += "_revision";
        listRevision_.setStoragePath(storageRoot, revisionFilename);
        listRevision_.load();
    
        return loadFromStorage();
    }

    virtual bool loadFromStorage() {
        bool success = false;
        tes_common_ais::sha1_ifstream ifs{ true }; // two_copy
        ifs.open(storagePath_);
        if (ifs) {
            // Read the file in
            if (loadFromJson(ifs)) {
                AIS_LOG_NOTICE(logPrefix_ + "loaded from storage.");
                success = true;
            }
            else {
                AIS_LOG_ERROR(logPrefix_ + "could not be parsed from file.");
            }

            ifs.close();

            if (ifs.is_corrupt()) {
                if (ifs.fix_it()) {
                    AIS_LOG_WARN(logPrefix_ + "file was corrupt... fixed it.");
                }
                else {
                    AIS_LOG_ERROR(logPrefix_ + "file was corrupt... could not fix it.");
                }
            }
        }
        else {
            AIS_LOG_ERROR(logPrefix_ + "file could not be opened from storage.");
        }
        return success;
    }

    virtual bool saveToStorage(std::chrono::system_clock::time_point revision = std::chrono::system_clock::now()) {
        bool success = false;
        tes_common_ais::sha1_ofstream ofs{ true }; // two_copy
        ofs.open(storagePath_);
        if (ofs) {
            if (serializeToJson(ofs)) {
                // set the list revision timestamp
                listRevision_.setRevision(revision);
                AIS_LOG_NOTICE(logPrefix_ + "saved to storage.");
                success = true;
            }
            else {
                AIS_LOG_ERROR(logPrefix_ + "could not serialize file.");
            }
            ofs.close();
        }
        else {
            AIS_LOG_ERROR(logPrefix_ + "file could not be opened for storage.");
        }

        return success;
    }

    virtual inline std::string getJsonFileName() const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return "";
    }

    virtual inline std::string getCsvFileName() const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return "";
    }

    virtual inline std::string getBdtFileName() const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return "";
    }

    inline std::string getFileName(LpsSaListMgrInterfaceListFormat format) const {
        if (LpsSaListMgrInterfaceListFormat::JSON == format) {
            return getJsonFileName();
        }
        else if (LpsSaListMgrInterfaceListFormat::CSV == format) {
            return getCsvFileName();
        }
        else if (LpsSaListMgrInterfaceListFormat::BDT == format) {
            return getBdtFileName();
        }
        else {
            AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
            return "";
        }
    }

    virtual std::string getListName() const {
        return getDefaultListName(listType_);
    }

    virtual void setListName(const std::string& name) {
        // Do nothing
    }

    virtual bool loadFromJson(std::istream& json) {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool loadFromCsv(std::istream& csv) {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool loadFromProtobuf(const cat::cpm::list::List& list) {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool loadFromBdt(const std::vector<unsigned char>& bdtData) {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool loadFromBdt(std::istream& bdt) {
        bool success = true;
        try {
            std::istreambuf_iterator<std::istream::char_type> begin(bdt), end;
            std::vector<unsigned char> bdtData(begin, end);
            success = loadFromBdt(bdtData);
        }
        catch (...) {
            success = false;
        }
        return success;
    }

    virtual bool serializeToJson(std::ostream& json) const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool serializeToProtobuf(std::ostream& pb) const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }    

    virtual bool serializeToCsv(std::ostream& csv, LpsCommonWeightUnits displayUnits) const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool serializeToBdt(std::vector<unsigned char>& bdtData) const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return false;
    }

    virtual bool serializeToBdt(std::ostream& bdt) const {
        bool success = true;
        try {
            std::vector<unsigned char> bdtData;
            if (serializeToBdt(bdtData)) {
                bdt.write(reinterpret_cast<char*>(bdtData.data()), bdtData.size() * sizeof(decltype(bdtData)::value_type));
            }
            else {
                success = false;
            }
        }
        catch (...) {
            success = false;
        }
        return success;
    }

    virtual bool loadNoLock(LpsSaListMgrInterfaceListFormat format, const std::string& srcPath) {
        /*
         * This was added because unlocking a file lock and closing a file,
         *  even a "read" file lock on a file opened for "input" was used,
         *  still generates an IN_CLOSE_WRITE inotify event.  This can cause
         *  a never ending loop of IN_CLOSE_WRITE events followed by locking
         *  and reading the file, followed by another IN_CLOSE_WRITE event, ...
         *  For this reason, it is useful to provide a load function that does
         *  not lock the file.
         */
        bool success = false;
        std::ifstream ifs;

        if (LpsSaListMgrInterfaceListFormat::BDT == format) {
            ifs.open(srcPath, std::ios::binary | std::ios::in);
        }
        else {
            ifs.open(srcPath);
        }

        if (ifs) {
            success = loadFrom(format, ifs);
            ifs.close();
        }
        else {
            AIS_LOG_ERROR(logPrefix_ + "file could not be opened reading.");
        }

        return success;
    }

    virtual bool load(LpsSaListMgrInterfaceListFormat format, const std::string& srcPath) {
        bool success = false;
        tes_common_ais::IFlocker ifl;

        if (LpsSaListMgrInterfaceListFormat::BDT == format) {
            ifl.open(srcPath, std::ios::binary | std::ios::in);
        }
        else {
            ifl.open(srcPath);
        }

        if (ifl) {
            success = loadFrom(format, ifl.ifstream());
            ifl.close();
        }
        else {
            AIS_LOG_ERROR(logPrefix_ + "file could not be opened and locked for reading.");
        }

        return success;
    }

    virtual bool publish(LpsSaListMgrInterfaceListFormat format, const std::string& destPath,
            LpsCommonWeightUnits displayUnits = LpsCommonWeightUnits::TONNE) const {
        namespace fs = boost::filesystem;
        bool success = false;
        tes_common_ais::OFlocker ofl;

        if (LpsSaListMgrInterfaceListFormat::BDT == format) {
            ofl.open(destPath, std::ios::binary);
        }
        else {
            ofl.open(destPath);
        }

        if (ofl) {
            if (LpsSaListMgrInterfaceListFormat::JSON == format) {
                success = serializeToJson(ofl.ofstream());
                if (success) {
                    AIS_LOG_NOTICE(logPrefix_ + "published JSON file.");
                }
                else {
                    AIS_LOG_ERROR(logPrefix_ + "could not serialize JSON file for publishing.");
                }
            }
            else if (LpsSaListMgrInterfaceListFormat::CSV == format) {
                success = serializeToCsv(ofl.ofstream(), displayUnits);
                if (success) {
                    AIS_LOG_NOTICE(logPrefix_ + "published CSV file.");
                }
                else {
                    AIS_LOG_ERROR(logPrefix_ + "could not serialize CSV file for publishing.");
                }
            }
            else if (LpsSaListMgrInterfaceListFormat::BDT == format) {
                success = serializeToBdt(ofl.ofstream());
                if (success) {
                    AIS_LOG_NOTICE(logPrefix_ + "published BDT file.");
                }
                else {
                    AIS_LOG_ERROR(logPrefix_ + "could not serialize BDT file for publishing.");
                }
            }
            else {
                AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
            }

            ofl.close();
        }
        else {
            AIS_LOG_ERROR(logPrefix_ + "file could not be opened and locked for publishing.");
        }

        if (!success) {
            try {
                if (fs::is_regular_file(destPath)) {
                    fs::remove(destPath);
                    AIS_LOG_WARN(logPrefix_ + "file removed.");
                }
            }
            catch (const fs::filesystem_error& e) {
                AIS_LOG_WARN(logPrefix_ + "file could not be removed.");
            }
        }

        return success;
    }

    virtual void createListRevision (std::ostream& os ) const {
        AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        return;
    } 

    bool isNewer (const cat::cpm::list::List& list, std::chrono::system_clock::time_point& revision) const  {
        //true if revision is newer

        // compare revision against what we have
        if (list.has_revision()) {
            auto temp_revision = std::chrono::system_clock::from_time_t(list.revision().seconds());
            if (temp_revision > listRevision_.getRevision()) {
                revision = temp_revision;
                return true;
            }
        }
        else {
            AIS_LOG_WARN(logPrefix_ + "revision not provided");
        }

        return false;
    }

    bool isOlder (const cat::cpm::list::List& list, std::chrono::system_clock::time_point& revision) const  {
        //true if revision is older

        // compare revision against what we have
        if (list.has_revision()) {
            auto temp_revision = std::chrono::system_clock::from_time_t(list.revision().seconds());
            if (temp_revision < listRevision_.getRevision()) {
                revision = temp_revision;
                return true;
            }
        }
        else {
            AIS_LOG_WARN(logPrefix_ + "revision not provided");
        }

        return false;
    }

    bool hasContent (const cat::cpm::list::List& list) const {
        //true if it has list content

        if (list.has_content()){
            return true;
        }

        return false;
    }

protected:
    bool loadFrom(LpsSaListMgrInterfaceListFormat format, std::istream& is) {
        bool success = false;

        if (LpsSaListMgrInterfaceListFormat::JSON == format) {
            success = loadFromJson(is);
            if (success) {
                AIS_LOG_NOTICE(logPrefix_ + "loaded JSON file.");
            }
            else {
                AIS_LOG_ERROR(logPrefix_ + "could not load JSON file.");
            }
        }
        else if (LpsSaListMgrInterfaceListFormat::CSV == format) {
            success = loadFromCsv(is);
            if (success) {
                AIS_LOG_NOTICE(logPrefix_ + "loaded CSV file.");
            }
            else {
                AIS_LOG_ERROR(logPrefix_ + "could not load CSV file.");
            }
        }
        else if (LpsSaListMgrInterfaceListFormat::BDT == format) {
            success = loadFromBdt(is);
            if (success) {
                AIS_LOG_NOTICE(logPrefix_ + "loaded BDT file.");
            }
            else {
                AIS_LOG_ERROR(logPrefix_ + "could not load BDT file.");
            }
        }
        else {
            AIS_LOG_WARN(logPrefix_ + "unsupported file format.");
        }

        return success;
    }

    static inline CPMListWeightUnits convertWeightUnits(LpsCommonWeightUnits displayUnits) {
        CPMListWeightUnits units;
        switch (displayUnits) {
        case (LpsCommonWeightUnits::TON): {
            units = CPMListWeightUnits::TONS;
            break;
        }
        case (LpsCommonWeightUnits::KLB): {
            units = CPMListWeightUnits::KLBS;
            break;
        }
        case (LpsCommonWeightUnits::LB): {
            units = CPMListWeightUnits::LBS;
            break;
        }
        case (LpsCommonWeightUnits::KG): {
            units = CPMListWeightUnits::KGS;
            break;
        }
        default: {
            units = CPMListWeightUnits::TONNES;
            break;
        }
        }
        return units;
    }

    std::string storagePath_;
    LpsSaListMgrInterfaceListType listType_;
    LpsSaListRevisionTimestamp listRevision_;
    const std::string logPrefix_;    
};


#endif /* LPSSALISTMGR_HPP */
