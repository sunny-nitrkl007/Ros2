#include <ais/serialization/TypeRestorer.h>
#include <ais/interfaces/traits/csvable.h>

#include "../LpsSaLoadRecordChannel.h"

extern "C"
{
    csvable* getCSVable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaLoadRecordChannel, csvable*>(blob, size);
    }
}


