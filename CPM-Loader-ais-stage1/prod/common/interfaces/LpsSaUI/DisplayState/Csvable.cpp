#include <ais/serialization/TypeRestorer.h>
#include <ais/interfaces/traits/csvable.h>

#include "../DisplayStateInterface.hpp"

extern "C"
{
    csvable *getCSVable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaUIDisplayStateInterface, csvable*>(blob, size);
    }
}

