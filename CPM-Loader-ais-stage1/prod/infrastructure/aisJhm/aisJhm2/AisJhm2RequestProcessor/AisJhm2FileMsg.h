#ifndef AISJHM2FILEMSG_H_
#define AISJHM2FILEMSG_H_

#include <ostream>

#include <jhm2/Jhm2Param/Jhm2Msg.h>
#include <jhm2/Jhm2Param/Jhm2ParamMsgRequest.h>

class AisJhm2FileMsg: public Jhm2Msg {
public:
    AisJhm2FileMsg(Jhm2ParamMsgRequest::eFormat format = Jhm2ParamMsgRequest::JSON_FORMAT) :
        format_(format),
        contents_() {}

    const std::string& contents(const std::string& s) { contents_ = s; return contents_; }
    const std::string& contents(void) const { return contents_; }

protected:
    /*virtual bool getJson(std::ostream& out) {
        if (Jhm2ParamMsgRequest::eFormat::JSON_FORMAT == format_) {
            out << contents_;
            return true;
        }
        return false;
    }*/

    virtual bool getJson(std::ostream& out) const {
        if (Jhm2ParamMsgRequest::eFormat::JSON_FORMAT == format_) {
            out << contents_;
            return true;
        }
        return false;
    }

    // No other formats are implemented.

private:
    Jhm2ParamMsgRequest::eFormat format_;
    std::string contents_;
};

#endif // AISJHM2FILEMSG_H_
