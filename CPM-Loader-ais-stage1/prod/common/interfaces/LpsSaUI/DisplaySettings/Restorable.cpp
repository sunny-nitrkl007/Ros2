#include <ais/serialization/TypeRestorer.h>
#include <ais/serialization/Restorable.h>

#include "../DisplaySettingsInterface.hpp"

extern "C"
{
    Restorable *getRestorable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaUIDisplaySettingsInterface, Restorable*>(blob, size);
    }
}

