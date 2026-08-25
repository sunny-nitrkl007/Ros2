#ifndef AISJHM2MULTIPARAM_H_
#define AISJHM2MULTIPARAM_H_

#include <utility>
#include <memory>

#include <jhm2/Jhm2Param/Jhm2Param.h>
#include <jhm2/Jhm2Param/Jhm2SimpleParam.h>
#include <jhm2/jsonCommon/JsonOutStream.h>
#include <ais/serialization/JsonSerializer.h>

#include "AisJhm2FloatArrayParam.h"

class AisJhm2MultiParam: public Jhm2Param
{
public:
    AisJhm2MultiParam(): Jhm2Param(), params_() {}
    AisJhm2MultiParam(const std::string& name): Jhm2Param(name), params_() {}

    virtual ~AisJhm2MultiParam() override {}

    constexpr static auto TYPE = "Param";

    virtual std::string getType() const override { return TYPE; }

    virtual void getXml( std::ostream& xml ) const override {
        Jhm2Param::getXmlStart(xml);
        for (const auto& p : params_) {
            if (p) {
                p->getXml(xml);
            }
        }
        Jhm2Param::getXmlEnd(xml);
    }

    virtual void jsonSerialize(JsonIosAbstractBase& json) override {
        // We need the JsonOutStream in order to do anything useful here
        JsonOutStream* jostream = dynamic_cast<JsonOutStream*>(&json);
        if (nullptr != jostream) {
            jostream->String(m_name); // Can't call Key with std::string&
            jostream->StartArray();
            for (auto& p : params_) {
                if (p) {
                    jostream->StartObject();
                    p->jsonSerializeImpl(json);
                    jostream->EndObject();
                }
            }
            jostream->EndArray();
        }
    }

    virtual void jsonSerialize2(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override {
        writer.String(m_name);
        writer.StartObject();
        for (const auto& p : params_) {
            if (p) {
                p->jsonSerialize2(writer);
            }
        }
        writer.EndObject();
    }

    template<class T>
    typename std::enable_if<std::is_base_of<Jhm2Param, T>::value>::type
    addParameter(T&& param) {
        params_.push_back(std::unique_ptr<Jhm2Param>(new T(std::forward<T>(param))));
    }

private:
    std::vector<std::unique_ptr<Jhm2Param>> params_;
};


#endif /* AISJHM2MULTIPARAM_H_ */
