#ifndef AISJHM2FLOATARRAYPARAM_H_
#define AISJHM2FLOATARRAYPARAM_H_

#include <vector>
#include <initializer_list>

#include <jhm2/Jhm2Param/Jhm2Param.h>
#include <jhm2/jsonCommon/JsonOutStream.h>
#include <ais/serialization/JsonSerializer.h>

class AisJhm2FloatArrayParam: public Jhm2Param
{
public:
    AisJhm2FloatArrayParam(): Jhm2Param(), value_() {}
    AisJhm2FloatArrayParam(const std::string& name, std::initializer_list<float> L): Jhm2Param(name), value_(L) {}
    AisJhm2FloatArrayParam(const std::string& name, const std::vector<float>& v): Jhm2Param(name), value_(v) {}

    virtual ~AisJhm2FloatArrayParam() override {}

    constexpr static auto TYPE = "Param";

    virtual std::string getType() const override { return TYPE; }

    virtual void getXml(std::ostream& xml) const override {
        bool firstValue = true;
        Jhm2Param::getXmlStart( xml );
        xml << "<Value>";
        for (auto v : value_) {
            if (firstValue) {
                firstValue = false;
                xml << v;
            }
            else {
                xml << "," << v;
            }
        }
        xml << "</Value>";
        Jhm2Param::getXmlEnd( xml );
    }

    virtual void jsonSerialize(JsonIosAbstractBase& json) override {
        // We need the JsonOutStream in order to do anything useful here
        JsonOutStream* jostream = dynamic_cast<JsonOutStream*>(&json);
        if (nullptr != jostream) {
            jostream->String(m_name); // Can't call Key with std::string&
            jostream->StartArray();
            for (auto v : value_) {
                jostream->Double(v);
            }
            jostream->EndArray();
        }
        jsonSerializeDsi(json);
    }

    virtual void jsonSerialize2(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override {
        writer.String(m_name);
        writer.StartObject();
        writer.Key("value");
        { // Serialize the value
            auto maxDecimalPlaces = writer.GetMaxDecimalPlaces();
            writer.SetMaxDecimalPlaces(3);
            writer.StartArray();
            for (auto v : value_) {
                if (std::isfinite(v)) {
                    writer.Double(v); // This truncates to number decimal places.
                }
            }
            writer.EndArray();
            writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }
        if (0 != m_dsi) {
            writer.Key("dsi");
            writer.Uint(m_dsi);
        }
        writer.EndObject();
    }

private:
    std::vector<float> value_;
};


#endif /* AISJHM2FLOATARRAYPARAM_H_ */
