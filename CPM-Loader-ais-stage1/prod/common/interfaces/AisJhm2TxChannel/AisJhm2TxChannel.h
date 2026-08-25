
#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>
#ifndef _AisJhm2TxChannel_h_
#define _AisJhm2TxChannel_h_

class AisJhm2TxChannelStorage :public csvable
{  
public:  
    AisJhm2TxChannelStorage() :
        simplecal_data(),
        tipoff_weight_adjust_data() {}

    struct simplecal_data_t {
        std::string timeStamp;
        float adjtruckweight = 0.f;
        float zeroedTruckWt = 0.f;
        bool newDataFlag = false;;
    };

    struct tipoff_weight_adjust_data_t {
        float tipOffWeight1 = NAN;
        float weighRangeWeight1 = NAN;
        float tipOffWeight2 = NAN;
        float weighRangeWeight2 = NAN;
        bool newDataFlag = false;
        bool reset = false;
    };

    simplecal_data_t simplecal_data;
    tipoff_weight_adjust_data_t tipoff_weight_adjust_data;

     template <class Archive>
     void serialize(Archive &ar, unsigned int version)
     {
       /* Archive Fields Here */
       ar & simplecal_data.timeStamp;
       ar & simplecal_data.adjtruckweight;
       ar & simplecal_data.zeroedTruckWt;
	   ar & simplecal_data.newDataFlag;

	   ar & tipoff_weight_adjust_data.tipOffWeight1;
	   ar & tipoff_weight_adjust_data.weighRangeWeight1;
	   ar & tipoff_weight_adjust_data.tipOffWeight2;
	   ar & tipoff_weight_adjust_data.weighRangeWeight2;
	   ar & tipoff_weight_adjust_data.newDataFlag;
	   ar & tipoff_weight_adjust_data.reset;
     }

     void toCsv(CsvOutStream& out) const
     {	
		out("simplecaldata timestamp",simplecal_data.timeStamp);
		out("simplecaldata truck weight",simplecal_data.adjtruckweight);
		out("simplecaldata adjusted truck weight",simplecal_data.zeroedTruckWt);
		out("simplecaldata adjusted truck weight",simplecal_data.newDataFlag);
     }

 private:  
     /* Add Fields Here */
};  

typedef Datum<AisJhm2TxChannelStorage> AisJhm2TxChannel;

BOOST_CLASS_VERSION(AisJhm2TxChannelStorage, 1);
#endif

