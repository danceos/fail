#ifndef FLIP_BITS_H
#define FLIP_BITS_H
extern "C" {
#include "vmi/vmiTypes.h"


void flipBits(const void *value, Uns32 bytes, vmiProcessorP processor, Addr address);
}

#endif
