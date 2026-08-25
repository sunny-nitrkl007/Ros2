#include <ais/serialization/TypeRestorer.h>
#include <ais/interfaces/traits/csvable.h>

#include "../ResponseInterface.hpp"

extern "C"
{
    csvable *getCSVable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaTotalsResponseInterface, csvable*>(blob, size);
    }
}

