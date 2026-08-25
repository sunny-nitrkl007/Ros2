#ifndef ECMSUMMARYWRITER_HPP
#define ECMSUMMARYWRITER_HPP

#include <ais/log/Logger.h>
#include "fileio/oflocker.hpp"
#include <ext/rapidjson/RapidJsonGuarded.h>
#include <boost/asio.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include <thread>         // std::thread


struct ECMSummary {
    std::string ecm_name;
    int CDL_MID;
    std::string product_id;
    std::string ecm_partnumber;
    std::string ecm_serialnumber;
    std::string sw_partnumber;
    std::string sw_releasedate;
    std::string sw_configpartnumber;
    std::string sw_description;
    std::string machine_make;
    std::string machine_model;
    std::string machine_linkage;
};

class ECMSummaryWriter {
public:
    ECMSummaryWriter() :
        outputDir_(),
        filename_(),
        ip_address_(),
        thread_running_{false},
        connection_bad_{false} {}

    ECMSummaryWriter(boost::filesystem::path tempRoot, std::string filename, std::string ip_address="") :
        outputDir_(tempRoot),
        filename_(filename),
        ip_address_(ip_address),
        thread_running_{false},
        connection_bad_{false} {
        try {
            boost::filesystem::create_directories(outputDir_);
        }
        catch (const boost::filesystem::filesystem_error& e) {
            AIS_LOG_ERROR(e.what());
        }
    }

    virtual ~ECMSummaryWriter() = default;

    bool isConnectionBad() {
        return connection_bad_;
    }

    bool getServiceDashboard() {
        if (!ip_address_.empty()) {
            if (thread_running_) {
                /* a thread is already out there trying to connect */
                //AIS_LOG_INFO("Already attempting to retrieve ECM info, file creation in process");
            }
            else {
                /* We will spawn a thread to go get the service dashboard using http API
                 * It is important that the class instance is not destroyed
                 * */
                thread_running_ = true;
                std::thread(&ECMSummaryWriter::ecmServiceDashboardThread, this).detach();
            }
        }
        else {
            //AIS_LOG_ERROR("No IP Address provided, cannot get service dashboard \n");
            return false;
        }


        return true;
    }

    bool createFile(ECMSummary &ecm_summary) const {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        writer.StartObject();
        writer.Key("ECMSummary");
        writer.StartObject();

        if (!ecm_summary.ecm_name.empty()) {
            writer.Key("ecm_name");
            writer.String(ecm_summary.ecm_name);
        }
        if (!ecm_summary.CDL_MID == 0) {
            writer.Key("CDL_MID");
            writer.Int(ecm_summary.CDL_MID);
        }
        if (!ecm_summary.ecm_partnumber.empty()) {
            writer.Key("ecm_partnumber");
            writer.String(ecm_summary.ecm_partnumber);
        }
        if (!ecm_summary.ecm_serialnumber.empty()) {
            writer.Key("ecm_serialnumber");
            writer.String(ecm_summary.ecm_serialnumber);
        }
        if (!ecm_summary.machine_linkage.empty()) {
            writer.Key("machine_linkage");
            writer.String(ecm_summary.machine_linkage);
        }
        if (!ecm_summary.machine_make.empty()) {
            writer.Key("machine_make");
            writer.String(ecm_summary.machine_make);
        }
        if (!ecm_summary.machine_model.empty()) {
            writer.Key("machine_model");
            writer.String(ecm_summary.machine_model);
        }
        if (!ecm_summary.product_id.empty()) {
            writer.Key("product_id");
            writer.String(ecm_summary.product_id);
        }
        if (!ecm_summary.sw_configpartnumber.empty()) {
            writer.Key("sw_configpartnumber");
            writer.String(ecm_summary.sw_configpartnumber);
        }
        if (!ecm_summary.sw_description.empty()) {
            writer.Key("sw_description");
            writer.String(ecm_summary.sw_description);
        }
        if (!ecm_summary.sw_partnumber.empty()) {
            writer.Key("sw_partnumber");
            writer.String(ecm_summary.sw_partnumber);
        }
        if (!ecm_summary.sw_releasedate.empty()) {
            writer.Key("sw_releasedate");
            writer.String(ecm_summary.sw_releasedate);
        }

        writer.EndObject();
        writer.EndObject();

        // Write the file
        boost::filesystem::path filepath = outputDir_ / filename_;
        try {
            tes_common_ais::OFlocker fout(filepath);
            fout.ofstream() << sb.GetString();
            fout.close();
        }
        catch(...) {
            AIS_LOG_ERROR("Unable to open file for writing: %s", filepath.c_str());
            return false;
        }

        AIS_LOG_INFO("ECM Summary File created: %s", filename_.c_str());
        return true;
    }

private:
    void ecmServiceDashboardThread() {
        thread_running_ = true;
        AIS_LOG_INFO("Thread Started...");

        std::string ipAddress = ip_address_;
        std::string portNum = "80";
        std::string queryStr = "/ServiceDashboard/ServiceDashboard.xml";

        std::string hostAddress;
        if (portNum.compare("80") != 0) { // add the ":" only if the port number is not 80 (proprietary port number).
            hostAddress = ipAddress + ":" + portNum;
        }
        else {
            hostAddress = ipAddress;
        }

        static constexpr int retry_interval_seconds = 10;

        int num_of_tries = 0;
        bool success = false;
        std::string resp_string;

        while (num_of_tries < 3) {
            // About to try the request
            ++num_of_tries;
            AIS_LOG_INFO("Trying: %d", num_of_tries);

            try {
                std::stringstream resp_stream;
                boost::asio::io_service io_service;

                // Get a list of endpoints corresponding to the server name.
                boost::asio::ip::tcp::resolver resolver(io_service);
                boost::asio::ip::tcp::resolver::query query(ipAddress, portNum);
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

                // Try each endpoint until we successfully establish a connection.
                boost::asio::ip::tcp::socket socket(io_service);
                boost::asio::connect(socket, endpoint_iterator);

                // Form the request. We specify the "Connection: close" header so that the
                // server will close the socket after transmitting the response. This will
                // allow us to treat all data up until the EOF as the content.
                boost::asio::streambuf request;
                std::ostream request_stream(&request);
                request_stream << "GET " << queryStr << " HTTP/1.1\r\n";  // note that you can change it if you wish to HTTP/1.0
                request_stream << "Host: " << hostAddress << "\r\n";
                request_stream << "Accept: */*\r\n";
                request_stream << "Connection: close\r\n\r\n";

                // Send the request.
                boost::asio::write(socket, request);

                // Read the response status line. The response streambuf will automatically
                // grow to accommodate the entire line. The growth may be limited by passing
                // a maximum size to the streambuf constructor.
                boost::asio::streambuf response;
                boost::asio::read_until(socket, response, "\r\n");

                // Check that response is OK.
                std::istream response_stream(&response);
                std::string http_version;
                response_stream >> http_version;
                unsigned int status_code;
                response_stream >> status_code;
                std::string status_message;
                std::getline(response_stream, status_message);

                if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                    AIS_LOG_ERROR("Invalid Response Received");
                }
                else if (status_code != 200) {
                    AIS_LOG_ERROR("Response returned with error status code: %d", status_code);
                }
                else {
                    // Read the response headers, which are terminated by a blank line.
                    boost::asio::read_until(socket, response, "\r\n\r\n");

                    // Process the response headers.
                    std::string header;
                    while (std::getline(response_stream, header) && header != "\r") {
                        AIS_LOG_INFO("header: %s", header.c_str());
                    }

                    // Write whatever content we already have to output.
                    if (response.size() > 0) {
                        resp_stream << &response;
                    }

                    // Read until EOF, writing data to output as we go.
                    boost::system::error_code error;
                    while (boost::asio::read(socket, response,boost::asio::transfer_at_least(1), error)) {
                        resp_stream << &response;
                    }

                    if (error != boost::asio::error::eof) {
                        AIS_LOG_ERROR("Request to get ECM summary from remote failed: No eof encountered");
                        throw boost::system::system_error(error);
                    }

                    resp_string = resp_stream.str();
                    success = true;
                    break; // Break out of while loop
                }
            }
            catch (std::exception& e) {
                AIS_LOG_ERROR("Request to get ECM summary from %s failed: %s", ip_address_.c_str(), e.what());
            }

            /* wait the retry interval */
            sleep(retry_interval_seconds);
        }

        if (success) {
            // If we got the response, connection is not bad
            connection_bad_ = false;

            /* Create service dashboard file */
            boost::filesystem::path filepath = outputDir_ / filename_;
            try {
                tes_common_ais::OFlocker fout(filepath);
                fout.ofstream() << resp_string;
                fout.close();
            }
            catch (...) {
                AIS_LOG_ERROR("Unable to open file for writing: %s", filepath.c_str());
            }
        }
        else {
            // We did not get a response after retrying, connection is not good.
            connection_bad_ = true;
        }

        AIS_LOG_DEBUG("Thread exiting");
        thread_running_ = false;
    }

    boost::filesystem::path outputDir_;
    std::string filename_;
    std::string ip_address_;
    bool thread_running_;
    bool connection_bad_;
};

#endif /* ECMSUMMARYWRITER_HPP */
