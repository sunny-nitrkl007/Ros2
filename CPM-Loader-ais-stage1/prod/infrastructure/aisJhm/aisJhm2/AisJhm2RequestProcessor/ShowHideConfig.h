#ifndef SHOWHIDECONFIG_H
#define SHOWHIDECONFIG_H

#include <mutex>
#include <string>
#include <ostream>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>

class ShowHideConfig {
public:
    ShowHideConfig();

    bool load(const std::string& filePath);

    template <class T> T get(const std::string& path) const;

    void set(const std::string& path, bool newValue);
    void set(const std::string& path, const char* newValue);
    void set(const std::string& path, const std::string& newValue);

    std::string getSHA1() const {
        std::lock_guard<std::mutex> lck(mtx_);
        if (dirty_) { refreshSerializedContent(); }
        return sha1_;
    }

    std::string getSerializedContent() const {
        std::lock_guard<std::mutex> lck(mtx_);
        if (dirty_) { refreshSerializedContent(); }
        return serializedContent_;
    }

    std::ostream& getSerializedContent(std::ostream& os) const {
        std::lock_guard<std::mutex> lck(mtx_);
        if (dirty_) { refreshSerializedContent(); }
        os << serializedContent_;
        return os;
    }

private:
    friend std::ostream& operator<<(std::ostream& os, const ShowHideConfig& p);

    void refreshSerializedContent() const; // must lock externally

    // Data Model
    mutable std::mutex mtx_;

    boost::property_tree::ptree tree_;

    mutable std::unordered_map<std::string, bool> boolCache_;

    mutable bool dirty_;
    mutable std::string sha1_;
    mutable std::string serializedContent_;

};

// The only specialization defined is bool
template <> bool ShowHideConfig::get(const std::string& path) const;

extern std::ostream& operator<<(std::ostream& os, const ShowHideConfig& p);

#endif
