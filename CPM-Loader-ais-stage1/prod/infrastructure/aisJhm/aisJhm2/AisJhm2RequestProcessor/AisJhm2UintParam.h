#ifndef AISJHM2UINTPARAM_H_
#define AISJHM2UINTPARAM_H_

#include <cstdint>

#include <jhm2/Jhm2Param/Jhm2Param.h>
#include <jhm2/jsonCommon/JsonOutStream.h>
#include <ais/serialization/JsonSerializer.h>

class AisJhm2UintParam: public Jhm2Param
{
public:
    AisJhm2UintParam(): Jhm2Param(), value_(0) {}
    AisJhm2UintParam(const std::string& name, uint_least32_t value): Jhm2Param(name), value_(value) {}
    AisJhm2UintParam(const std::string& name, const unsigned char& dsi, uint_least32_t value): Jhm2Param(name, dsi), value_(value) {}

    virtual ~AisJhm2UintParam() override {}

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
            jostream->Uint(value_);
        }
        jsonSerializeDsi(json);
    }

    virtual void jsonSerialize2(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override {
        writer.String(m_name);
        writer.StartObject();
        writer.Key("value");
        writer.Uint(value_);
        if (0 != m_dsi) {
            writer.Key("dsi");
            writer.Uint(m_dsi);
        }
        writer.EndObject();
}

private:
    uint_least32_t value_;
};


#endif /* AISJHM2UINTPARAM_H_ */
