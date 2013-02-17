#include <stdlib.h>

#include "flipBits.h"

void flipBits(const void *value, Uns32 bytes, vmiProcessorP processor, Addr address) {
    if (address == 0x019e) {
        *((Uns32 *)value) ^= 1;
    }
}


