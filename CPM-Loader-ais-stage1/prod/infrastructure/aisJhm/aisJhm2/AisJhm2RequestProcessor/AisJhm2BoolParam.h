#ifndef AISJHM2BOOLPARAM_H_
#define AISJHM2BOOLPARAM_H_

#include <jhm2/Jhm2Param/Jhm2Param.h>
#include <jhm2/jsonCommon/JsonOutStream.h>
#include <ais/serialization/JsonSerializer.h>

class AisJhm2BoolParam: public Jhm2Param
{
public:
    AisJhm2BoolParam(): Jhm2Param(), value_(false) {}
    AisJhm2BoolParam(const std::string& name, bool value): Jhm2Param(name), value_(value) {}
    AisJhm2BoolParam(const std::string& name, const unsigned char& dsi, bool value): Jhm2Param(name, dsi), value_(value) {}

    virtual ~AisJhm2BoolParam() override {}

    constexpr static auto TYPE = "Param";

    virtual std::string getType() const override { return TYPE; }

    virtual void getXml(std::ostream& xml) const override {
        Jhm2Param::getXmlStart( xml );
        xml << "<Value>" << value_ << "</Value>";
        Jhm2Param::getXmlEnd( xml );
    }

    virtual void jsonSerialize(JsonIosAbstractBase& json) override {
        // We need the JsonOutStream in order to do anything useful here
        JsonOutStream* jostream = dynamic_cast<JsonOutStream*>(&json);
        if (nullptr != jostream) {
            jostream->String(m_name); // Can't call Key with std::string&
            jostream->wBool(value_);
        }
        jsonSerializeDsi(json);
    }

    virtual void jsonSerialize2(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override {
        writer.String(m_name);
        writer.StartObject();
        writer.Key("value");
        writer.tBool(value_);
        if (0 != m_dsi) {
            writer.Key("dsi");
            writer.Uint(m_dsi);
        }
        writer.EndObject();
}

private:
    bool value_;
};


#endif /* AISJHM2BOOLPARAM_H_ */
