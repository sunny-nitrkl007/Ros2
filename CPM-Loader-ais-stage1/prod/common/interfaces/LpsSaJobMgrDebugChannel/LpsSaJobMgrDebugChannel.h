#include <cstdint>

#include <ais/serialization/Datum.h> // For Boost serialization
#include <ais/interfaces/traits/csvable.h>

#ifndef _LpsSaJobMgrDebugChannel_h_
#define _LpsSaJobMgrDebugChannel_h_

class LpsSaJobMgrDebugChannelStorage: public csvable
{  
public:  
    LpsSaJobMgrDebugChannelStorage() :
        currentState(0),
        tipoffAssistActivationCount(0) {}

    template <class Archive>
    void serialize(Archive &ar, unsigned int version)
    {
    	ar & currentState;
    	ar & tipoffAssistActivationCount;
    }

    void toCsv(CsvOutStream& out) const
    {
    }

    uint32_t currentState;

    /*
     * Putting this statistic here for now.  If we start to accumulate more statistics,
     * we should move them to another channel that isn't published as frequently.
     */
    uint32_t tipoffAssistActivationCount;

 private:  
     /* Add Fields Here */
};  


typedef Datum<LpsSaJobMgrDebugChannelStorage> LpsSaJobMgrDebugChannel;

BOOST_CLASS_VERSION(LpsSaJobMgrDebugChannelStorage, 1);
#endif

