#include <ais/serialization/TypeRestorer.h>
#include <ais/serialization/Restorable.h>

#include "../SettingsInterface.hpp"

extern "C"
{
    Restorable *getRestorable(const void *blob, const unsigned int size)
    {
        TypeRestorer rest;
        return rest.restoreAltType<WorkOrderAssistSettingsInterface, Restorable*>(blob, size);
    }
}

