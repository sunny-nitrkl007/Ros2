#include <ais/serialization/TypeRestorer.h>
#include <ais/serialization/Restorable.h>

#include "../USBSaveProgressInterface.hpp"

extern "C"
{
    Restorable *getRestorable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<LpsSaTotalsUSBSaveProgressInterface, Restorable*>(blob, size);
    }
}

