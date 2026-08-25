#ifndef DETAILEDDIAGEVENTSWRITER_HPP
#define DETAILEDDIAGEVENTSWRITER_HPP

#include <ais/log/Logger.h>
#include <fileio/oflocker.hpp>

using namespace tes_common_ais;
using namespace std;

struct DetailedDiagnostics {
    std::string name;
    std::string type;
    std::string MID_name;
    int MID_ID;
    bool active;
    int ID;
    int FMI;
    int WCI;
    int count;
    int first;
    int last;
};

class DetailedDiagEventsWriter {
public:
    DetailedDiagEventsWriter(boost::filesystem::path tempRoot) :
        outputFile_{ R"(DetailedDiagnostics.json)" },
        outputDir_{ tempRoot.string() } {
            try {
                fs::create_directories(outputDir_);
            }
            catch (const fs::filesystem_error& e) {
                AIS_LOG_ERROR(e.what());
            }
        }

    virtual ~DetailedDiagEventsWriter() = default;

    bool createFile(std::vector<DetailedDiagnostics>& diagList) const {
        OFlocker fout;

        string filepath = outputDir_ + "/" + outputFile_;
        try {
            fout.open(filepath);
        }
        catch(...) {
            AIS_LOG_DEBUG("Unable to open file for writing: %s", filepath.c_str() );
            return false;
        }

        fout.ofstream() << "{ " << endl;
        fout.ofstream() << "\"DetailedDiagnostics\": [" << endl;

        //create json file for DetailedDiagnostics
        for(auto diag = diagList.begin(); diag != diagList.end(); ++diag) {
            fout.ofstream() << "{ " << endl;
            fout.ofstream() << "\"name\":" << "\"" << diag->name << "\"" << "," << endl;
            fout.ofstream() << "\"type\":" << "\"" << diag->type << "\"" << "," << endl;
            fout.ofstream() << "\"MID_name\":" << "\"" << diag->MID_name << "\"" << "," << endl;
            fout.ofstream() << "\"active\":" << diag->active << "," << endl;
            fout.ofstream() << "\"MID_ID\":" << diag->MID_ID << "," << endl;
            fout.ofstream() << "\"ID\":" << diag->ID << "," << endl;
            fout.ofstream() << "\"FMI\":" << diag->FMI << "," << endl;
            fout.ofstream() << "\"WCI\":" << diag->WCI << "," << endl;
            fout.ofstream() << "\"count\":" << diag->count << "," << endl;
            fout.ofstream() << "\"first\":" << diag->first << "," << endl;
            fout.ofstream() << "\"last\":" << diag->last << endl;
            fout.ofstream() << "} ";
            if (next(diag) != diagList.end()) {
                fout.ofstream() << ", ";
            }
        }
        fout.ofstream() << "] " << endl;
        fout.ofstream() << "} " << endl;
        fout.close();

        return true;
    }

private:
    std::string outputFile_;
    std::string outputDir_;

};

#endif /* DETAILEDDIAGEVENTSWRITER_HPP */
