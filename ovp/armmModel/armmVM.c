/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * YOUR ACCESS TO THE INFORMATION IN THIS MODEL IS CONDITIONAL
 * UPON YOUR ACCEPTANCE THAT YOU WILL NOT USE OR PERMIT OTHERS
 * TO USE THE INFORMATION FOR THE PURPOSES OF DETERMINING WHETHER
 * IMPLEMENTATIONS OF THE ARM ARCHITECTURE INFRINGE ANY THIRD
 * PARTY PATENTS.
 *
 * THE LICENSE BELOW EXTENDS ONLY TO USE OF THE SOFTWARE FOR
 * MODELING PURPOSES AND SHALL NOT BE CONSTRUED AS GRANTING
 * A LICENSE TO CREATE A HARDWARE IMPLEMENTATION OF THE
 * FUNCTIONALITY OF THE SOFTWARE LICENSED HEREUNDER.
 * YOU MAY USE THE SOFTWARE SUBJECT TO THE LICENSE TERMS BELOW
 * PROVIDED THAT YOU ENSURE THAT THIS NOTICE IS REPLICATED UNMODIFIED
 * AND IN ITS ENTIRETY IN ALL DISTRIBUTIONS OF THE SOFTWARE,
 * MODIFIED OR UNMODIFIED, IN SOURCE CODE OR IN BINARY FORM.
 *
 * Licensed under an Imperas Modfied Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.ovpworld.org/licenses/OVP_MODIFIED_1.0_APACHE_OPEN_SOURCE_LICENSE_2.0.pdf
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>      // for sprintf

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiTypes.h"

// model header files
#include "armExceptions.h"
#include "armFunctions.h"
#include "armStructure.h"
#include "armMessage.h"
#include "armSys.h"
#include "armVM.h"
#include "armUtils.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_VM"

//
// This is the minimum region size usable with SRD bits and the number of
// subregions per region
//
#define SRD_MIN_SIZE 256
#define SRD_REGIONS    8


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Return instruction memory domain set for the passed processor
//
inline static armDomainSetP getDomainSetI(armP arm) {
    return &arm->ids;
}

//
// Return data memory domain set for the passed processor
//
inline static armDomainSetP getDomainSetD(armP arm) {
    return &arm->dds;
}

//
// Get the current code memDomain
//
inline static memDomainP getVirtualCodeDomain(armP arm) {
    return vmirtGetProcessorCodeDomain((vmiProcessorP)arm);
}

//
// Get the current data memDomain
//
inline static memDomainP getVirtualDataDomain(armP arm) {
    return vmirtGetProcessorDataDomain((vmiProcessorP)arm);
}

//
// Is code demain required for the passed privilege?
//
inline static Bool isFetch(memPriv priv) {
    return priv==MEM_PRIV_X;
}

//
// Return the memory domain set to use for the passed memory access type
//
static armDomainSetP getDomainSetPriv(armP arm, memPriv priv) {
    return isFetch(priv) ? getDomainSetI(arm) : getDomainSetD(arm);
}

//
// Return the virtual memory domain to use for the passed memory access type
//
static memDomainP getVirtualDomainPriv(armP arm, memPriv priv) {
    return isFetch(priv) ? getVirtualCodeDomain(arm) : getVirtualDataDomain(arm);
}

//
// Set the current data memDomain
//
inline static void setVirtualDataDomain(armP arm, memDomainP domain) {
    vmirtSetProcessorDataDomain((vmiProcessorP)arm, domain);
}

//
// Push to a new data memDomain, marking the processor to indicate restoration
// is required
//
inline static void pushVirtualDataDomain(armP arm, memDomainP domain) {
    setVirtualDataDomain(arm, domain);
    arm->restoreDomain = True;
}

//
// Return the virtual data domain for the passed mode
//
static memDomainP getVirtualDataDomainMode(armP arm) {
    armDomainSetP set = getDomainSetD(arm);
    return IN_USER_MODE(arm) ? set->vmUser : set->vmPriv;
}

//
// Restore correct virtual code domain for the processor, if required
//
static void restoreVirtualDataDomain(armP arm) {
    setVirtualDataDomain(arm, getVirtualDataDomainMode(arm));
    arm->restoreDomain = False;
}

//
// Given a raw privilege (AP format), return the effective privilege based on
// mode only for user mode MPU accesses
//
static memPriv getPrivMPUU(armP arm, Uns8 AP, Bool XN) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_NONE,
        [2] = MEM_PRIV_RX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_NONE,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_NONE
    };

    memPriv result = (AP>7) ? MEM_PRIV_NONE : privMap[AP];

    // apply XN bit if required
    if(XN) {result &= ~MEM_PRIV_X;}

    return result;
}

//
// Given a raw privilege (AP format), return the effective privilege based on
// mode only for privileged mode MPU accesses
//
static memPriv getPrivMPUP(armP arm, Uns8 AP, Bool XN) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_RWX,
        [2] = MEM_PRIV_RWX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_RX,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_NONE
    };

    memPriv result = (AP>7) ? MEM_PRIV_NONE : privMap[AP];

    // apply XN bit if required
    if(XN) {result &= ~MEM_PRIV_X;}

    return result;
}

//
// Given a raw privilege (AP format), return the effective privilege considering
// mode only (MPU accesses). A special case is that accesses in kernel mode
// where the current data memDomain is actually the user mode memDomain should
// be treated as user mode accesses (LDRT, STRT).
//
static memPriv getPrivMPU(armP arm, Uns8 AP, Bool XN) {

    if(IN_USER_MODE(arm)) {
        // normal user-mode case
        return getPrivMPUU(arm, AP, XN);
    } else if(getVirtualDataDomain(arm)==getDomainSetD(arm)->vmUser) {
        // special case for LDRT and STRT accesses
        return getPrivMPUU(arm, AP, XN);
    } else {
        // normal privileged-mode case
        return getPrivMPUP(arm, AP, XN);
    }
}

//
// Return a domian mask (the largest valid address in a domain)
//
static Uns64 getDomainMask(memDomainP domain) {

    Uns8 bits = vmirtGetDomainAddressBits(domain);

    return (bits==64) ? -1ULL : ((1ULL<<bits)-1);
}

//
// Create an alias of the passed memDomain
//
static memDomainP makeDomainAlias(memDomainP master, const char *name) {

    Uns8       bits  = vmirtGetDomainAddressBits(master);
    memDomainP slave = vmirtNewDomain(name, bits);

    vmirtAliasMemory(master, slave, 0, getDomainMask(master), 0, 0);

    return slave;
}

//
// Map memory virtual addresses starting at 'lowVA' to the physical address
// range 'lowPA':'highPA' with privilege 'priv'. Return a code indicating
// whether the mapping was successful (it may fail if there is no physical
// memory at the mapped address).
//
static void mapDomainPairRange(
    armP    arm,
    memPriv requiredPriv,
    memPriv priv,
    Uns64   lowPA,
    Uns64   highPA,
    Uns32   lowVA,
    Bool    G,
    Uns8    ASID
) {
    armDomainSetP domainSet = getDomainSetPriv(arm, requiredPriv);
    memDomainP    domainV   = getVirtualDomainPriv(arm, requiredPriv);

    // create the required mapping (with ASID, if required)
    vmirtAliasMemoryVM(
        domainSet->external, domainV, lowPA, highPA, lowVA, 0, priv, G, ASID
    );
}

//
// Do required actions when an invalid memory access occurs, either a processor
// access or a DMA unit access
//
static void handleInvalidAccess(
    armP           arm,
    Uns32          MVA,
    memPriv        requiredPriv,
    memAccessAttrs attrs
) {
    if(!attrs) {
        // no action for artifact access
    } else {
        // processor-generated exception
        armMemoryAbort(arm, MVA, requiredPriv);
    }
}


////////////////////////////////////////////////////////////////////////////////
// MPU FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

typedef enum protRegionCodeE {
    PEC_ENABLED,        // enabled area description
    PEC_DISABLED,       // disabled area description
    PEC_INVALID_SIZE,   // invalid size (less than minumum)
    PEC_INVALID_BASE,   // invalid base (not a multiple of size)
} protRegionCode;

// this type represents a single protection area entry
typedef struct protRegionS {
    Uns32 Base;         // entry base address
    Uns32 SRD  : 8;     // subrange disable
    Uns32 Size : 5;     // area size
    Uns32 TEX  : 3;     // TEX field (ARMv6 and later)
    Uns32 AP   : 3;     // area permissions
    Bool  E    : 1;     // enabled?
    Bool  XN   : 1;     // XN bit (ARMv6 and later)
    Bool  S    : 1;     // S bit (ARMv6 and later)
    Bool  C    : 1;     // C bit (ARMv6 and later)
    Bool  B    : 1;     // B bit (ARMv6 and later)
} protRegion;

// pointer to a constant protRegion
typedef const struct protRegionS *protRegionCP;

//
// Return description string identifying MPU
//
static const char *getMPUName(armP arm, Bool isData) {
    return MPU_UNIFIED(arm) ? "" : isData ? " (D)" : " (I)";
}

//
// Return the count of the number of protection regions
//
static Uns32 mpuRegionNum(armP arm, Bool isData) {
    if(isData || MPU_UNIFIED(arm)) {
        return SCS_FIELD(arm, MPU_TYPE, DREGION);
    } else {
        return SCS_FIELD(arm, MPU_TYPE, DREGION);
    }
}

//
// Validate that the passed index is a valid MPU region index
//
static Bool validateRegionNum(armP arm, Bool isData, Uns32 index) {

    Uns32 regionNum = mpuRegionNum(arm, isData);

    if(index >= regionNum) {

        vmiMessage("a", CPU_PREFIX"_ROOR",
            SRCREF_FMT "protection area index %u exceeds maximum (%u)",
            SRCREF_ARGS(arm, getPC(arm)),
            index, regionNum-1
        );

        return False;

    } else {

        return True;
    }
}

//
// Return a pointer to the Protection Area Control registers
//
inline static protRegionP getPACRegs(armP arm, Bool isData) {
    return (isData && arm->dmpu) ? arm->dmpu : arm->impu;
}

//
// If the passed protection entry describes a valid region, set the bounds of
// that region and return PEC_ENABLED; otherwise return an enum member giving
// the reason why it is not valid
//
static protRegionCode getProtectionEntryRange(
    armP         arm,
    protRegionCP pa,
    Uns32       *lowP,
    Uns32       *highP
) {
    if(!pa->E) {

        // disabled region
        return PEC_DISABLED;

    } else {

        // minimum region size allowed depends on MPU revision
        Uns32 minSize  = 4;
        Uns32 minDelta = (2 << minSize) - 1;

        // get the region size (minus 1)
        Uns32 size  = pa->Size;
        Uns32 delta = (2 << size) - 1;

        // are some subranges enabled?
        Uns32 SRE = ~pa->SRD;

        if(!SRE && (delta>=(SRD_MIN_SIZE-1))) {

            // all subranges disabled
            return PEC_DISABLED;

        } else {

            // get the region base address
            Uns32 low = pa->Base;

            // return implied bounds
            *lowP  = low;
            *highP = low + delta;

            if(delta<minDelta) {
                // invalid size
                return PEC_INVALID_SIZE;
            } else if(low & delta) {
                // base not a multiple of size
                return PEC_INVALID_BASE;
            } else {
                // bounds valid
                return PEC_ENABLED;
            }
        }
    }
}

//
// Remove all access privileges from the instruction and/or data domain pair,
// depending on whether this is an instruction or data access and whether the
// MPU is unified
//
static void removePrivMPU(
    armP       arm,
    memDomainP iDomain,
    memDomainP dDomain,
    Uns32      lowAddr,
    Uns32      highAddr,
    Bool       isData
) {
    Bool    unified = MPU_UNIFIED(arm);
    Bool    removeI = unified || !isData;
    Bool    removeD = unified ||  isData;
    memPriv priv    = MEM_PRIV_NONE;

    if(removeI) {
        vmirtProtectMemory(iDomain, lowAddr, highAddr, priv, MEM_PRIV_SET);
    }
    if(removeD) {
        vmirtProtectMemory(dDomain, lowAddr, highAddr, priv, MEM_PRIV_SET);
    }
}

//
// If the indexed protection entry is valid, unmap it, internal routine
//
static void unmapProtectionEntryInt(
    armP  arm,
    Uns32 low,
    Uns32 high,
    Bool  isData
) {
    // if debug mode is enabled, report the range being unmapped
    if(ARM_DEBUG_MMU(arm)) {
        vmiPrintf(
            "MPU%s UNMAP VA 0x%08x:0x%08x\n",
            getMPUName(arm, isData), low, high
        );
    }

    // remove privileged domain access permissions
    removePrivMPU(arm, arm->ids.vmPriv, arm->dds.vmPriv, low, high, isData);

    // remove user domain access permissions
    removePrivMPU(arm, arm->ids.vmUser, arm->dds.vmUser, low, high, isData);
}

//
// If the indexed protection entry is valid, unmap it
//
static void unmapProtectionEntry(armP arm, Uns32 low, Uns32 high, Bool isData) {

    // remove main mappings
    unmapProtectionEntryInt(arm, low, high, isData);
}

//
// Update MPU status when an MPU region entry has been updated
//
static void resetProtectionEntryRange(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Bool  verbose
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;
    Uns32       low;
    Uns32       high;

    // validate new protection area permissions
    switch(getProtectionEntryRange(arm, pacReg, &low, &high)) {

        case PEC_ENABLED:
            // remove any existing mapping for the new protection entry range
            unmapProtectionEntry(arm, low, high, isData);
            break;

        case PEC_INVALID_SIZE:
            // size error assertion
            if(verbose) {
                vmiMessage("a", CPU_PREFIX"_PEIS",
                    SRCREF_FMT "protection area %u has invalid size (%u bytes)",
                    SRCREF_ARGS(arm, getPC(arm)),
                    index, high - low + 1
                );
            }
            break;

        case PEC_INVALID_BASE:
            // base error assertion
            if(verbose) {
                vmiMessage("a", CPU_PREFIX"_PEIB",
                    SRCREF_FMT "protection area %u base 0x%08x is not a "
                    "multiple of size",
                    SRCREF_ARGS(arm, getPC(arm)),
                    index, low
                );
            }
            break;

        default:
            // disabled entry
            break;
    }
}

//
// Update the indexed region Base, SRD, Size or E fields
//
static void updateRegionBaseSRDSizeE(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Uns32 Base,
    Uns32 SRD,
    Uns32 Size,
    Uns32 E
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;

    // action is required only if the register changes value
    if(
        (pacReg->Base!=Base) ||
        (pacReg->SRD!=SRD)   ||
        (pacReg->Size!=Size) ||
        (pacReg->E!=E)
    ) {
        // remove any current mapping for the protection entry
        resetProtectionEntryRange(arm, index, isData, False);

        // update the entry
        pacReg->Base = Base;
        pacReg->SRD  = SRD;
        pacReg->Size = Size;
        pacReg->E    = E;

        // remove any mapping for the new region location
        resetProtectionEntryRange(arm, index, isData, True);
    }
}

//
// Modify AP or XN fields for the indexed MPU region
//
static void updateRegionAPXN(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Uns8  newAP,
    Bool  newXN
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;
    Uns8        oldAP   = pacReg->AP;
    Uns8        oldXN   = pacReg->XN;

    // update AP and XN fields
    pacReg->AP = newAP;
    pacReg->XN = newXN;

    // have region permissions on an enabled region changed?
    if(pacReg->E && ((oldAP!=newAP) || (oldXN!=newXN))) {

        // get privileges implied by old and new permissions in each mode
        memPriv oldPrivU = getPrivMPUU(arm, oldAP, oldXN);
        memPriv oldPrivP = getPrivMPUP(arm, oldAP, oldXN);
        memPriv newPrivU = getPrivMPUU(arm, newAP, newXN);
        memPriv newPrivP = getPrivMPUP(arm, newAP, newXN);

        // if privileges have been reduced, disable all access to the area of
        // memory assocaited with the region
        if((oldPrivU & ~newPrivU) || (oldPrivP & ~newPrivP)) {
            resetProtectionEntryRange(arm, index, isData, False);
        }
    }
}

//
// This specifies the regions in the default system address map
//
static const protRegion sysRegions[] = {
    {.Base=0x00000000, .Size=0x1c, .AP=3, .TEX=0, .C=1, .B=0, .E=1, .S=1, .XN=0},
    {.Base=0x20000000, .Size=0x1c, .AP=3, .TEX=1, .C=1, .B=1, .E=1, .S=1, .XN=0},
    {.Base=0x40000000, .Size=0x1c, .AP=3, .TEX=0, .C=0, .B=1, .E=1, .S=1, .XN=1},
    {.Base=0x60000000, .Size=0x1c, .AP=3, .TEX=1, .C=1, .B=1, .E=1, .S=1, .XN=0},
    {.Base=0x80000000, .Size=0x1c, .AP=3, .TEX=0, .C=1, .B=0, .E=1, .S=1, .XN=0},
    {.Base=0xa0000000, .Size=0x1c, .AP=3, .TEX=0, .C=0, .B=1, .E=1, .S=1, .XN=1},
    {.Base=0xc0000000, .Size=0x1c, .AP=3, .TEX=2, .C=0, .B=0, .E=1, .S=1, .XN=1},
    {.Base=0xe0000000, .Size=0x1c, .AP=3, .TEX=2, .C=0, .B=0, .E=1, .S=1, .XN=1}
};

//
// Return a protRegion structure representing the implicit Private Peripheral
// Bus region
//
static protRegionCP getPPBRegion(void) {

    static const protRegion ppbRegion = {
        .Base=PPB_LOW, .Size=0x13, .AP=3, .E=1, .XN=1
    };

    return &ppbRegion;
}

//
// If the MVA lies in the region range rlow:rhigh, set by-ref arguments 'low'
// and 'high' with that range; otherwise, exclude the range rlow:rhigh from the
// currennt range in low:high
//
static Bool matchRegion(
    Uns32  MVA,
    Uns32  rlow,
    Uns32  rhigh,
    Uns32 *low,
    Uns32 *high
) {
    if((rlow<=MVA) && (rhigh>=MVA)) {

        // match in this region
        *low  = rlow;
        *high = rhigh;

        return True;

    } else if((rlow>MVA) && (rlow<*high)) {

        // remove part of region ABOVE matching address
        *high = rlow-1;

    } else if((rhigh<MVA) && (rhigh>*low)) {

        // remove part of region BELOW matching address
        *low = rhigh+1;
    }

    return False;
}

//
// Return a boolean indicating if the passed MVA lies in the protection entry
// and update by-ref arguments 'low' and 'high' with the largest bounds
// enclosing the address
//
static Bool selectProtectionEntry(
    armP         arm,
    protRegionCP try,
    Uns32        MVA,
    Uns32       *low,
    Uns32       *high
) {
    Uns32 rlow, rhigh;

    if(getProtectionEntryRange(arm, try, &rlow, &rhigh)!=PEC_ENABLED) {

        // entry is not enabled
        return False;

    } else if(try->SRD && ((rhigh-rlow)>=(SRD_MIN_SIZE-1))) {

        // possible match in a subregion
        Uns8  SRE   = ~try->SRD;
        Uns32 delta = ((Uns64)rhigh-rlow+1)/SRD_REGIONS;
        Uns32 i;

        // iterate over all enabled subregions
        for(i=0; SRE; i++, SRE>>=1) {

            // is this subregion enabled?
            if(SRE & 1) {

                // derive subregion bounds
                Uns32 srlow  = rlow+(delta*i);
                Uns32 srhigh = srlow+delta-1;

                // return if subregion matches
                if(matchRegion(MVA, srlow, srhigh, low, high)) {
                    return True;
                }
            }
        }

        // no subregion matches
        return False;

    } else {

        // possible match in an entire region
        return matchRegion(MVA, rlow, rhigh, low, high);
    }
}

//
// Try mapping memory at the passed address for the specified access type and
// return a Boolean indicating whether the mapping succeeded
//
static Bool mpuMiss(
    armP    arm,
    memPriv requiredPriv,
    Uns32   address,
    Uns32  *lowVAP,
    Uns32  *highVAP
) {
    Uns32        MVA       = address;
    Bool         isData    = !(requiredPriv & MEM_PRIV_X);
    protRegionP  pacRegs   = getPACRegs(arm, isData);
    protRegionCP ppb       = getPPBRegion();
    protRegionCP match     = 0;
    Uns32        regionNum = mpuRegionNum(arm, isData);
    Uns32        low       = 0;
    Uns32        high      = 0;
    Uns32        i;

    // use the system address map as a background region in privileged mode if
    // MPU_CONTROL.PRIVDEFENA is specified
    if(!IN_USER_MODE(arm) && SCS_FIELD(arm, MPU_CONTROL, PRIVDEFENA)) {
        match = sysRegions+(MVA/0x20000000);
        selectProtectionEntry(arm, match, MVA, &low, &high);
    }

    // scan regions in lowest-to-highest priority order
    for(i=0; i<regionNum; i++) {
        if(selectProtectionEntry(arm, pacRegs+i, MVA, &low, &high)) {
            match = pacRegs+i;
        }
    }

    // include the implicit Private Peripheral Bus region which is always
    // present at highest priority
    if(selectProtectionEntry(arm, ppb, MVA, &low, &high)) {
        match = ppb;
    }

    // map in region if a match was found
    if(match) {

        // get access permissions applicable to the range
        Uns8    AP     = match->AP;
        Bool    XN     = match->XN;
        memPriv priv   = getPrivMPU(arm, AP, XN);
        Uns32   lowVA  = low;
        Uns32   highVA = high;

        // if debug mode is enabled, report the range being mapped
        if(ARM_DEBUG_MMU(arm)) {
            vmiPrintf(
                "MPU%s MAP VA 0x%08x:0x%08x PA 0x%08x:0x%08x AP %u%s\n",
                getMPUName(arm, isData),
                lowVA, highVA,
                low, high,
                AP,
                XN ? "-x" : ""
            );
        }

        // does the entry have sufficient permissions?
        if((priv & requiredPriv) == requiredPriv) {

            // map MPU entry memory
            mapDomainPairRange(
                arm, requiredPriv, priv, low, high, lowVA, True, 0
            );

            // MPU entry permissions are ok, but access may still not be
            // possible if no physical memory exists at the physical address
            memDomainP domainV    = getVirtualDomainPriv(arm, requiredPriv);
            memPriv    actualPriv = vmirtGetDomainPrivileges(domainV, address);

            // indicate the address range that has been mapped
            *lowVAP  = lowVA;
            *highVAP = highVA;

            return (actualPriv & requiredPriv) && True;
        }
    }

    // invalid access, either because no matching entry found or existing entry
    // has insufficient permissions
    return False;
}

//
// Free an MPU data structure
//
static void freeMPU(protRegionP *mpuHandle) {

    protRegionP mpu = *mpuHandle;

    STYPE_FREE(mpu);

    *mpuHandle = 0;
}

//
// Free an MPU data structure
//
static protRegionP newMPU(armP arm, Bool isData) {

    Uns32 regionNum = mpuRegionNum(arm, isData);

    return regionNum ? STYPE_CALLOC_N(protRegion, regionNum) : 0;
}

//
// Reset an MPU data structure
//
static void resetMPU(armP arm, Bool isData) {

    Uns32 regionNum = mpuRegionNum(arm, isData);
    Uns32 i;

    for(i=0; i<regionNum; i++) {
        updateRegionBaseSRDSizeE(arm, i, isData, 0, 0, 0, 0);
    }
}

//
// Free MPU structures for the passed processor
//
static void freeMPUs(armP arm) {

    // free instruction/unified MPU
    if(arm->impu) {
        freeMPU(&arm->impu);
    }

    // free data MPU if required
    if(arm->dmpu) {
        freeMPU(&arm->dmpu);
    }
}

//
// Allocate MPU structures for the passed processor
//
static void newMPUs(armP arm) {

    // create instruction/unified MPU
    arm->impu = newMPU(arm, False);

    // create data MPU if required
    if(!MPU_UNIFIED(arm)) {
        arm->dmpu = newMPU(arm, True);
    }
}

//
// Reset MPU contents
//
static void resetMPUs(armP arm) {

    // reset instruction/unified MPU
    if(arm->impu) {
        resetMPU(arm, False);
    }

    // reset data MPU if required
    if(arm->dmpu) {
        resetMPU(arm, True);
    }
}


////////////////////////////////////////////////////////////////////////////////
// BIT-BAND REGIONS
////////////////////////////////////////////////////////////////////////////////

//
// This is the size of a bit-band region
//
#define BIT_BAND_SIZE 0x100000

//
// This is the offset to the alias region from the bit-band base
//
#define ALIAS_OFFSET 0x2000000

//
// Callback function to read a system register
//
static VMI_MEM_READ_FN(readBitBand) {

    if(processor) {

        armP       arm        = (armP)processor;
        memDomainP domain     = arm->dds.external;
        UnsPS      regionLow  = (UnsPS)userData;
        Uns32      aliasLow   = regionLow + ALIAS_OFFSET;
        Uns32      regionAddr = regionLow + (address - aliasLow) / 32;
        Uns32      bitOffset  = ((address - aliasLow) % 32) / 4;

        // get byte alias value
        Uns8 result = vmirtRead1ByteDomain(domain, regionAddr, MEM_AA_TRUE);

        // extract required bit
        result = (result>>bitOffset) & 1;

        // update correct-size result value
        if(bytes==1) {
            *(Uns8*)value = result;
        } else if(bytes==2) {
            *(Uns16*)value = result;
        } else if(bytes==4) {
            *(Uns32*)value = result;
        } else {
            VMI_ABORT("unimplemented bit-band access size %u bytes", bytes);
        }
    }
}

//
// Callback function to read a system register
//
static VMI_MEM_WRITE_FN(writeBitBand) {

    if(processor) {

        armP       arm        = (armP)processor;
        memDomainP domain     = arm->dds.external;
        UnsPS      regionLow  = (UnsPS)userData;
        Uns32      aliasLow   = regionLow + ALIAS_OFFSET;
        Uns32      regionAddr = regionLow + (address - aliasLow) / 32;
        Uns32      bitOffset  = ((address - aliasLow) % 32) / 4;
        Uns8       mask       = 1<<bitOffset;

        // get initial value of the byte to modify
        Uns8 partial = vmirtRead1ByteDomain(domain, regionAddr, MEM_AA_TRUE);

        // set or clear the required bit
        if((*(Uns8*)value) & 1) {
            partial |= mask;
        } else {
            partial &= ~mask;
        }

        // write back modified value
        vmirtWrite1ByteDomain(domain, regionAddr, partial, MEM_AA_TRUE);
    }
}

//
// Create bit-band region at 'regionLow' with alias region at 'aliasLow'
//
static void createBitBandRegion(memDomainP domain, Uns32 regionLow) {

    Uns32 aliasLow  = regionLow + ALIAS_OFFSET;
    Uns32 aliasSize = BIT_BAND_SIZE*32;
    Uns32 aliasHigh = aliasLow+aliasSize-1;

    // remove any existing mapping for the alias address
    vmirtUnaliasMemory(domain, aliasLow, aliasHigh);

    // install callbacks to implement the alias
    vmirtMapCallbacks(
        domain, aliasLow, aliasHigh, readBitBand, writeBitBand, (void *)(UnsPS)regionLow
    );
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Try mapping memory at the passed address for the specified access type and
// return a status code indicating whether the mapping succeeded
//
armVMAction armVMMiss(
    armP           arm,
    memPriv        requiredPriv,
    Uns32          address,
    Uns32          bytes,
    memAccessAttrs attrs
) {
    Bool ok = False;

    // no action unless the MPU is enabled
    if(arm->mode&ARM_MODE_MPU) {

        Uns32 lastVA = address+bytes-1;
        Uns32 highVA = address-1;
        Uns32 lowVA  = address;

        // iterate over all MPU ranges straddled by this access
        do {
            address = highVA+1;
            ok      = mpuMiss(arm, requiredPriv, address, &lowVA, &highVA);
        } while(ok && ((lastVA<lowVA) || (lastVA>highVA)));
    }

    // generate exception for the first faulting address
    if(ok) {
        return MA_OK;
    } else {
        handleInvalidAccess(arm, address, requiredPriv, attrs);
        return MA_EXCEPTION;
    }
}

//
// Set the privileged mode data domain to the user domain (for LDRT, STRT)
//
void armVMSetUserPrivilegedModeDataDomain(armP arm) {
    pushVirtualDataDomain(arm, getDomainSetD(arm)->vmUser);
}

//
// Restore the normal data domain for the current mode
//
void armVMRestoreNormalDataDomain(armP arm) {
    restoreVirtualDataDomain(arm);
}

//
// Write the indexed MPU RBAR register value
//
void armVMWriteRBAR(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // prepare Base, SRD, Size and E arguments
        Uns32 Base = newValue & SCS_MASK_MPU_RBAR_ADDR;
        Uns32 SRD  = pacReg->SRD;
        Uns32 Size = pacReg->Size;
        Uns32 E    = pacReg->E;

        // update the required register
        updateRegionBaseSRDSizeE(arm, index, isData, Base, SRD, Size, E);
    }
}

//
// Read the indexed MPU RBAR register value
//
Uns32 armVMReadRBAR(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // create mask to select Base field
        union {SCS_REG_DECL(MPU_RBAR); Uns32 u32;} uMask = {{ADDR:-1}};

        // return masked result
        return pacReg->Base & uMask.u32;

    } else {

        return 0;
    }
}

//
// Write the indexed MPU RASR register value
//
void armVMWriteRASR(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // create union to enable field extraction
        union {Uns32 u32; SCS_REG_DECL(MPU_RASR);} u = {newValue};

        // update fields with no simulation effect
        pacReg->B   = u.MPU_RASR.B;
        pacReg->C   = u.MPU_RASR.C;
        pacReg->S   = u.MPU_RASR.S;
        pacReg->TEX = u.MPU_RASR.TEX;

        // prepare Base, SRD, Size, E, AP and XN arguments
        Uns32 Base = pacReg->Base;
        Uns32 SRD  = u.MPU_RASR.SRD;
        Uns32 Size = u.MPU_RASR.SIZE;
        Uns32 E    = u.MPU_RASR.ENABLE;
        Uns8  AP   = u.MPU_RASR.AP;
        Bool  XN   = u.MPU_RASR.XN;

        // update the required register
        updateRegionBaseSRDSizeE(arm, index, isData, Base, SRD, Size, E);
        updateRegionAPXN(arm, index, isData, AP, XN);
    }
}

//
// Read the indexed MPU RASR register value
//
Uns32 armVMReadRASR(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // initialize result
        union {Uns32 u32; SCS_REG_DECL(MPU_RASR);} u = {0};

        // extract required fields
        u.MPU_RASR.SRD    = pacReg->SRD;
        u.MPU_RASR.SIZE   = pacReg->Size;
        u.MPU_RASR.ENABLE = pacReg->E;
        u.MPU_RASR.B      = pacReg->B;
        u.MPU_RASR.C      = pacReg->C;
        u.MPU_RASR.S      = pacReg->S;
        u.MPU_RASR.TEX    = pacReg->TEX;
        u.MPU_RASR.AP     = pacReg->AP;
        u.MPU_RASR.XN     = pacReg->XN;

        // return composed result
        return u.u32;

    } else {

        return 0;
    }
}

//
// Flush the privileged mode MPU
//
void armVMFlushMPUPriv(armP arm) {

    Bool isData = True;

    // if debug mode is enabled, report the range being unmapped
    if(ARM_DEBUG_MMU(arm)) {
        vmiPrintf("MPU%s FLUSH\n", getMPUName(arm, isData));
    }

    // remove privileged domain access permissions
    removePrivMPU(arm, arm->ids.vmPriv, arm->dds.vmPriv, 0, -1, isData);
}

//
// Virtual memory constructor
//
VMI_VMINIT_FN(armVMInit) {

    armP       arm           = (armP)processor;
    Uns32      bits          = ARM_GPR_BITS;
    memDomainP extCodeDomain = codeDomains[0];
    memDomainP extDataDomain = dataDomains[0];
    memDomainP sysCodeDomain;
    memDomainP sysDataDomain;

    // create system physical domains
    if(extCodeDomain==extDataDomain) {
        extCodeDomain = makeDomainAlias(extCodeDomain, "external");
        extDataDomain = extCodeDomain;
        sysCodeDomain = makeDomainAlias(extCodeDomain, "system");
        sysDataDomain = sysCodeDomain;
    } else {
        extCodeDomain = makeDomainAlias(extCodeDomain, "external Code");
        extDataDomain = makeDomainAlias(extDataDomain, "external Data");
        sysCodeDomain = makeDomainAlias(extCodeDomain, "system Code");
        sysDataDomain = makeDomainAlias(extDataDomain, "system Data");
    }

    // add System Control Space register callbacks
    armSysCreateSCSRegion(arm, extDataDomain);

    if (!arm->disableBitBand) {
        // create bit-band regions
        createBitBandRegion(extDataDomain, 0x20000000);
        createBitBandRegion(extDataDomain, 0x40000000);
    }

    // addresses in System Control Space are never executable
    vmirtProtectMemory(
        extCodeDomain, SYSTEM_LOW, SYSTEM_HIGH, MEM_PRIV_X, MEM_PRIV_SUB
    );

    // some addresses in the default system address map are also not executable
    vmirtProtectMemory(
        sysCodeDomain, PERIPH_LOW, PERIPH_HIGH, MEM_PRIV_X, MEM_PRIV_SUB
    );
    vmirtProtectMemory(
        sysCodeDomain, DEVICE_LOW, DEVICE_HIGH, MEM_PRIV_X, MEM_PRIV_SUB
    );

    // save physical memDomains on processor structure
    arm->ids.external = extCodeDomain;
    arm->ids.system   = sysCodeDomain;
    arm->dds.external = extDataDomain;
    arm->dds.system   = sysDataDomain;

    // set physical code memDomains for each mode
    codeDomains[ARM_MODE_PRIV] = sysCodeDomain;
    codeDomains[ARM_MODE_USER] = sysCodeDomain;

    // set physical data memDomains for each mode
    dataDomains[ARM_MODE_PRIV] = sysDataDomain;
    dataDomains[ARM_MODE_USER] = sysDataDomain;

    // initialize MPU data structures if required
    if(MPU_PRESENT(arm)) {

        // create MPU-managed memDomains
        arm->ids.vmPriv = vmirtNewDomain("priv MPU Code", bits);
        arm->ids.vmUser = vmirtNewDomain("user MPU Code", bits);
        arm->dds.vmPriv = vmirtNewDomain("priv MPU Data", bits);
        arm->dds.vmUser = vmirtNewDomain("user MPU Data", bits);

        // set MPU code memDomains for each mode
        codeDomains[ARM_MODE_PRIV_MPU] = arm->ids.vmPriv;
        codeDomains[ARM_MODE_USER_MPU] = arm->ids.vmUser;

        // set MPU data memDomains for each mode
        dataDomains[ARM_MODE_PRIV_MPU] = arm->dds.vmPriv;
        dataDomains[ARM_MODE_USER_MPU] = arm->dds.vmUser;

        // initialize MPU if required
        if(MPU_PRESENT(arm)) {
            newMPUs(arm);
        }
    }
}

//
// Reset VM structures
//
void armVMReset(armP arm) {
    if(MPU_PRESENT(arm)) {
        resetMPUs(arm);
    }
}

//
// Free structures used for virtual memory management
//
void armVMFree(armP arm) {
    if(MPU_PRESENT(arm)) {
        freeMPUs(arm);
    }
}


