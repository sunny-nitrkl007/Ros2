#include <ais/serialization/TypeRestorer.h>
#include <ais/interfaces/traits/csvable.h>

#include "../BEMSimStateInterface.hpp"

extern "C"
{
    csvable *getCSVable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaUIBEMSimStateInterface, csvable*>(blob, size);
    }
}

