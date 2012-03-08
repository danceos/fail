// Modifies values read from or written to memory

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#include <time.h>
extern "C" {
// VMI module includes
#include "vmi/vmiMessage.h"
#include "vmi/vmiMmcAttrs.h"
#include "vmi/vmiMmc.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiTypes.h"
#include "flipBits.h"
}

//#include "../core/SAL/ovp/OVPController.hpp"
#include "SAL/SALInst.hpp"

// Model prefix (used by vmiMessage interface)
#define CPU_PREFIX "flakyMemory"

// ...
// Cache object
typedef struct flakyObjectS {
    // MODELLING ARTIFACTS
    memDomainP      nextDomain;    // next domain (FULL model)
    memRegionP      lastRegion;    // last accessed (FULL model)

    Uns64           count;         // intercepted reads and writes

} flakyObject, *flakyObjectP;

// ...
#define BUF_SIZE 30
static char tmpCharBuffer[BUF_SIZE];


// Return a static temporary string that has the digits in the passed string
// separated into groups by a comma, e.g. "1234567" -> "1,234,567". The result
// is in a static buffer so it will be overwritten by the next call.
static const char *getCommaString(const char *string) {

    static char buffer2[BUF_SIZE];

    Uns32       length = strlen(string);
    Uns32       count  = 0;
    char       *dst    = buffer2+BUF_SIZE;
    const char *src    = string+length;

    // copy null terminator for comma string
    *--dst = *src--;

    // copy remaining characters in groups
    do {

        *--dst = *src--;

        // insert comma every three characters
        if((++count==3) && (src>=string)) {
            count = 0;
            *--dst = ',';
        }

    } while((src>=string) && (dst>=buffer2));

    return dst;
}


// Return a static temporary string that has the passed Uns64 value shown
// with digits separated by commas
static const char *uns64String(Uns64 value) {
    sprintf(tmpCharBuffer, FMT_64u, value);
    return getCommaString(tmpCharBuffer);
}


// Utility routine for statistics reporting
static void printStats(flakyObjectP info) {
    vmiMessage("I", CPU_PREFIX, "TOTAL ALTERED READS/WRITES: %15s\n", uns64String(info->count));
}


// Cache object constructor
static VMIMMC_CONSTRUCTOR_FN(flakyConstructor) {
    //srand(time(NULL));
}

// Cache object link
static VMIMMC_LINK_FN(flakyLink)
{
    flakyObjectP flaky        = (flakyObjectP)component;
    VMI_ASSERT(
        vmimmcGetNextPort(component, "mp1") == NULL,
        "%sCannot accept a transparent connection",
        vmimmcGetHierarchicalName(component)
    );

    memDomainP nextDomain = vmimmcGetNextDomain(component, "mp1");

    // sanity check that we are in full mode
    VMI_ASSERT(
        nextDomain,
        "%s: expected an opaque connection",
        vmimmcGetHierarchicalName(component)
    );
    flaky->nextDomain = nextDomain;
}

// Cache object destructor
static VMIMMC_DESTRUCTOR_FN(flakyDestructor) {
    flakyObjectP flaky = (flakyObjectP)component;
    printStats(flaky);
}

// N-byte read function
static VMI_MEM_READ_FN(readNFull)
{
    vmimmcPortP port         = reinterpret_cast<vmimmcPortP>(userData);
    flakyObjectP flaky       = (flakyObjectP)port->component;
    Uns32       shortAddress = (Uns32)address;
    
    // read from real memory connected to mmc:
    vmirtReadNByteDomain(
        flaky->nextDomain,
        shortAddress,
        value,
        bytes,
        &flaky->lastRegion,
        MEM_AA_TRUE
    );
    
//    flipBits(value, bytes, processor, address);
	//sal::simulator.onMemoryAccessEvent(address, bytes, false, processor.getPC());
       
    // increment counter of reads and writes:
    flaky->count += 1;
}

// N-byte write function
static VMI_MEM_WRITE_FN(writeNFull)
{
    vmimmcPortP port         = reinterpret_cast<vmimmcPortP>(userData);
    flakyObjectP flaky       = (flakyObjectP)port->component;
    Uns32       shortAddress = (Uns32)address;

    //flipBits(value, bytes, processor, address);
    
    // write to real memory connected to this mmc:
    vmirtWriteNByteDomain(
        flaky->nextDomain,
        shortAddress,
        value,
        bytes,
        &flaky->lastRegion,
        MEM_AA_TRUE
    );
    
    // increment counter of reads and writes:
    flaky->count += 1;
}


vmimmcAttr modelAttrs =
{
    // VERSION
    VMI_VERSION,            // version string (THIS MUST BE FIRST)
    VMI_MMC_MODEL,          // type
    sizeof(flakyObject),     // size in bytes of MMC object

    // CONSTRUCTOR/DESTRUCTOR ROUTINES
    flakyConstructor,        // constructor
    flakyLink,               // link component
    flakyDestructor,         // destructor

    // MODEL REFRESH (AT START OF TIME SLICE)
    0,                      // refresh
    
    // FULL MODEL CALLBACKS
    readNFull,              // N-byte read callback
    writeNFull,             // N-byte write callback
    
    // TRANSPARENT MODEL CALLBACKS
    0,                      // N-byte read callback
    0                       // N-byte write callback
};

