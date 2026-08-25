#include <mutex>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>

// Using get_sha1 from here.
#include <fileio/sha1_fstream.hpp>

#include "ShowHideConfig.h"

static inline std::string get_sha1(const std::string& s) {
    std::istringstream iss(s);
    return tes_common_ais::detail::get_sha1(iss);
}

ShowHideConfig::ShowHideConfig() :
        mtx_(),
        tree_(),
        boolCache_(),
        dirty_(true),
        sha1_(),
        serializedContent_() {
}

bool ShowHideConfig::load(const std::string& filePath) {
    bool success = true;
    std::lock_guard<std::mutex> lck(mtx_);

    boolCache_.clear();

    try {
        boost::property_tree::read_json(filePath, tree_);
    }
    catch (const boost::property_tree::json_parser_error&) {
        tree_.clear();
        success = false;
    }

    refreshSerializedContent();

    return success;
}

template <> bool ShowHideConfig::get(const std::string& path) const {
    bool value;
    std::lock_guard<std::mutex> lck(mtx_);
    try {
        value = boolCache_.at(path);
    }
    catch (const std::out_of_range&) {
        // Not found in cache.
        value = tree_.get(path, false);
        boolCache_[path] = value;
    }
    return value;
}

void ShowHideConfig::set(const std::string& path, bool newValue) {
    bool updateNeeded = false;
    std::lock_guard<std::mutex> lck(mtx_);

    // Find the current value
    try {
        // Find in cache
        if (newValue != boolCache_.at(path)) {
            updateNeeded = true;
        }
    }
    catch (const std::out_of_range&) {
        // Not found in cache.
        try {
            if (newValue != tree_.get<bool>(path)) {
                updateNeeded = true;
            }
            else {
                // Add it to the cache.
                boolCache_[path] = newValue;
            }
        }
        catch (const boost::property_tree::ptree_error&) {
            // Not found in tree either
            updateNeeded = true;
        }
    }

    if (updateNeeded) {
        // The value is changing
        tree_.put(path, newValue);
        boolCache_[path] = newValue;

        dirty_ = true;
        //refreshSerializedContent();
    }
}

void ShowHideConfig::set(const std::string& path, const char* newValue) {
    set(path, std::string(newValue));
}

void ShowHideConfig::set(const std::string& path, const std::string& newValue) {
    bool updateNeeded = false;
    std::lock_guard<std::mutex> lck(mtx_);

    try {
        if (newValue != tree_.get<std::string>(path)) {
            updateNeeded = true;
        }
    }
    catch (const boost::property_tree::ptree_error&) {
        // Not found in tree either
        updateNeeded = true;
    }

    if (updateNeeded) {
        // The value is changing
        tree_.put(path, newValue);

        dirty_ = true;
        //refreshSerializedContent();
    }
}

void ShowHideConfig::refreshSerializedContent() const {
    // Not thread safe.  Need to lock externally!
    try {
        std::ostringstream buf;
        boost::property_tree::write_json(buf, tree_, false);
        serializedContent_ = buf.str();

        // Replace all "true" with true
        boost::replace_all<std::string>(serializedContent_, "\"true\"", "true");

        // Replace all "false" with false
        boost::replace_all<std::string>(serializedContent_, "\"false\"", "false");
    }
    catch (const boost::property_tree::json_parser_error&) {
        serializedContent_.clear();
    }

    // Update the SHA1
    sha1_ = get_sha1(serializedContent_);

    // Content no longer dirty
    dirty_ = false;
}

std::ostream& operator<<(std::ostream& os, const ShowHideConfig& p) {
    return p.getSerializedContent(os);
}
