#ifndef CPMLIST_H
#define CPMLIST_H

#include <cstdint>

enum class CPMListWeightUnits : uint8_t {
    TONNES, // Metric tonnes
    TONS, // English Tons
    KLBS, // Thousands of pounds
    KGS, // Kilograms
    LBS // Pounds
};

/*
 * These are used to communicate to the back office the maximum
 * list length and name length so the back office UI can
 * enforce these limits.  These limits are not enforces onboard
 * at this time.
 */
#define CPMLIST_MAX_LIST_LENGTH (500)
#define CPMLIST_MAX_NAME_LENGTH (255)

#endif
