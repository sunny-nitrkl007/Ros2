#ifndef AISJHM2FLOATPARAM_H_
#define AISJHM2FLOATPARAM_H_

#include <jhm2/Jhm2Param/Jhm2Param.h>
#include <jhm2/jsonCommon/JsonOutStream.h>
#include <ais/serialization/JsonSerializer.h>

class AisJhm2FloatParam: public Jhm2Param
{
public:
    AisJhm2FloatParam(): Jhm2Param(), value_(0.f) {}
    AisJhm2FloatParam(const std::string& name, float value): Jhm2Param(name), value_(value) {}
    AisJhm2FloatParam(const std::string& name, const unsigned char& dsi, float value): Jhm2Param(name, dsi), value_(value) {}

    virtual ~AisJhm2FloatParam() override {}

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
            jostream->Double(value_);
        }
        jsonSerializeDsi(json);
    }

    virtual void jsonSerialize2(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override {
        writer.String(m_name);
        writer.StartObject();
        writer.Key("value");
        if (std::isfinite(value_)) { 
            // Serialize the value
            auto maxDecimalPlaces = writer.GetMaxDecimalPlaces();
            writer.SetMaxDecimalPlaces(3);
            writer.Double(value_); // This truncates to number decimal places.
            writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }
        else {
            writer.Null();
        }  

        if (0 != m_dsi) {
            writer.Key("dsi");
            writer.Uint(m_dsi);
        }
        writer.EndObject();
    }

private:
    float value_;
};


#endif /* AISJHM2FLOATPARAM_H_ */
