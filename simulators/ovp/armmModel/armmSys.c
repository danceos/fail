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

// standard header files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// VMI header files
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiView.h"

// model header files
#include "armExceptions.h"
#include "armExceptionTypes.h"
#include "armMessage.h"
#include "armStructure.h"
#include "armSys.h"
#include "armSysRegisters.h"
#include "armUtils.h"
#include "armVM.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_SCS"


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
// Is the numbered exception enabled?
//
inline static Bool isEnabled(armP arm, armExceptNum num) {
    return EX_MASK_GET(arm->xEnable, num);
}

//
// Is the numbered exception pending?
//
inline static Bool isPending(armP arm, armExceptNum num) {
    return EX_MASK_GET(arm->xPend, num);
}

//
// Is the numbered exception active?
//
inline static Bool isActive(armP arm, armExceptNum num) {
    return EX_MASK_GET(arm->xActive, num);
}

//
// Enable or disabled the numbered exception
//
inline static void setEnabled(armP arm, armExceptNum num, Bool set) {
    EX_MASK_SET_V(arm->xEnable, num, set);
}

//
// Set or clear the pending bit for the passed exception
//
inline static void setPending(armP arm, armExceptNum num, Bool set) {
    EX_MASK_SET_V(arm->xPend, num, set);
}

//
// Set or clear the active bit for the passed exception
//
inline static void setActive(armP arm, armExceptNum num, Bool set) {
    EX_MASK_SET_V(arm->xActive, num, set);
}

//
// Write a value to the passed net
//
inline static void writeNet(armP arm, Uns32 netHandle, Uns32 value) {
    if(netHandle) {
        vmirtWriteNetPort((vmiProcessorP)arm, netHandle, value);
    }
}

//
// Is the region number valid?
//
inline static Bool validRegion(armP arm, Uns32 region) {
    return (region < SCS_FIELD(arm, MPU_TYPE, DREGION));
}

////////////////////////////////////////////////////////////////////////////////
// READ AND WRITE CALLBACKS
////////////////////////////////////////////////////////////////////////////////

//
// System register read callback type
//
#define ARM_SCS_READFN(_NAME) Uns32 _NAME( \
    armP        arm,       \
    armSCSRegId id,        \
    Uns32       byteOffset \
)
typedef ARM_SCS_READFN((*armSCSReadFn));

//
// System register write callback type
//
#define ARM_SCS_WRITEFN(_NAME) void _NAME( \
    armP        arm,       \
    armSCSRegId id,        \
    Uns32       newValue,  \
    Uns32       byteOffset \
)
typedef ARM_SCS_WRITEFN((*armSCSWriteFn));

//
// Update a system register, preserving read-only bits
//
#define UPDATE_SCS_MASKED(_P, _R, _NEW) \
    SCS_REG_UNS32(_P, _R) = (                               \
        (SCS_REG_UNS32(_P, _R) & ~SCS_WRITE_MASK_##_R) |    \
        (_NEW                  &  SCS_WRITE_MASK_##_R)      \
    )

//
// Dummy function to ignore a system register read
//
static ARM_SCS_READFN(ignoreSysRead) {

    Uns32 thisPC = getPC(arm);

    vmiMessage("W", CPU_PREFIX"_ISR",
        SRCREF_FMT "SCS register read ignored (return 0)",
        SRCREF_ARGS(arm, thisPC)
    );

    return 0;
}

//
// Dummy function to ignore a system register write
//
static ARM_SCS_WRITEFN(ignoreSysWrite) {

    Uns32 thisPC = getPC(arm);

    vmiMessage("W", CPU_PREFIX"_ISW",
        SRCREF_FMT "SCS register write ignored",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Write MPU_CONTROL
//
static ARM_SCS_WRITEFN(writeMPU_CONTROL) {

    // update field, preserving read-only bits
    Bool oldENABLE     = SCS_FIELD(arm, MPU_CONTROL, ENABLE);
    Bool oldHFNMIENA   = SCS_FIELD(arm, MPU_CONTROL, HFNMIENA);
    Bool oldPRIVDEFENA = SCS_FIELD(arm, MPU_CONTROL, PRIVDEFENA);
    UPDATE_SCS_MASKED(arm, MPU_CONTROL, newValue);
    Bool newENABLE     = SCS_FIELD(arm, MPU_CONTROL, ENABLE);
    Bool newHFNMIENA   = SCS_FIELD(arm, MPU_CONTROL, HFNMIENA);
    Bool newPRIVDEFENA = SCS_FIELD(arm, MPU_CONTROL, PRIVDEFENA);

    // mode switch may be required
    if((oldENABLE!=newENABLE) || (oldHFNMIENA!=newHFNMIENA)) {
        armSwitchMode(arm);
    }

    // if PRIVDEFENA has been *cleared*, the privileged mode MPU must be
    // refilled
    if(oldPRIVDEFENA && !newPRIVDEFENA) {
        armVMFlushMPUPriv(arm);
    }
}

//
// Write MPU_RNR
//
static ARM_SCS_WRITEFN(writeMPU_RNR) {

    // out-of-range writes to this register are ignored
    if(validRegion(arm, newValue)) {
        SCS_REG_UNS32(arm, MPU_RNR) = newValue;
    }
}

//
// Read MPU_RBAR
//
static ARM_SCS_READFN(readMPU_RBAR) {

    Uns32 region = SCS_FIELD(arm, MPU_RNR, REGION);

    // use union to assemble composed value
    union {Uns32 u32; SCS_REG_DECL(MPU_RBAR);} u = {
        armVMReadRBAR(arm, region, True)
    };

    // MPU_RBAR.VALID is RAZ
    u.MPU_RBAR.VALID = 0;

    // MPU_RBAR.REGION is an alias of MPU_RNR.REGION
    u.MPU_RBAR.REGION = SCS_FIELD(arm, MPU_RNR, REGION);

    // return composed value
    return u.u32;
}

//
// Write MPU_RBAR
//
static ARM_SCS_WRITEFN(writeMPU_RBAR) {

    Uns32 region;

    // use union to extract value fields
    union {Uns32 u32; SCS_REG_DECL(MPU_RBAR);} u = {newValue};

    // update region if valid and in range, and get the region to update
    if(u.MPU_RBAR.VALID && validRegion(arm, u.MPU_RBAR.REGION)) {
        region = SCS_FIELD(arm, MPU_RNR, REGION) = u.MPU_RBAR.REGION;
    } else {
        region = SCS_FIELD(arm, MPU_RNR, REGION);
    }

    // update the region
    armVMWriteRBAR(arm, region, True, newValue);
}

//
// Read MPU_RASR
//
static ARM_SCS_READFN(readMPU_RASR) {

    // derive region number, allowing for alias offset
    Uns32 region = SCS_FIELD(arm, MPU_RNR, REGION);

    return armVMReadRASR(arm, region, True);
}

//
// Write MPU_RASR
//
static ARM_SCS_WRITEFN(writeMPU_RASR) {

    // derive region number, allowing for alias offset
    Uns32 region = SCS_FIELD(arm, MPU_RNR, REGION);

    armVMWriteRASR(arm, region, True, newValue);
}

//
// Read ICSR
//
static ARM_SCS_READFN(readICSR) {

    // use union to assemble composed value
    union {Uns32 u32; SCS_REG_DECL(ICSR);} u = {0};

    // seed exception number fields
    u.ICSR.VECTACTIVE  = PSR_FIELD(arm, exceptNum);
    u.ICSR.ISRPENDING  = arm->pendingInterrupt;
    u.ICSR.VECTPENDING = arm->pendingException;

    // seed pending fields
    u.ICSR.PENDSTSET  = isPending(arm, AEN_SysTick);
    u.ICSR.PENDSVSET  = isPending(arm, AEN_PendSV);
    u.ICSR.NMIPENDSET = isPending(arm, AEN_NMI);

    // seed RETTOBASE
    u.ICSR.RETTOBASE = armGetRetToBase(arm);

    // return composed value
    return u.u32;
}

//
// Write ICSR
//
static ARM_SCS_WRITEFN(writeICSR) {

    // use union to extract value fields
    union {Uns32 u32; SCS_REG_DECL(ICSR);} u = {newValue};

    // raise NMI
    if(u.ICSR.NMIPENDSET) {
        setPending(arm, AEN_NMI, 1);
    }

    // raise/clear PendSV
    if(u.ICSR.PENDSVSET) {
        setPending(arm, AEN_PendSV, 1);
    } else if(u.ICSR.PENDSVCLR) {
        setPending(arm, AEN_PendSV, 0);
    }

    // raise/clear SysTick
    if(u.ICSR.PENDSTSET) {
        setPending(arm, AEN_SysTick, 1);
    } else if(u.ICSR.PENDSTCLR) {
        setPending(arm, AEN_SysTick, 0);
    }

    // refresh any pending exception state
    armRefreshPendingException(arm);
}

//
// Read AIRCR
//
static ARM_SCS_READFN(readAIRCR) {

    // VECTKEY field has a fixed magic value
    SCS_FIELD(arm, AIRCR, VECTKEY) = 0xfa05;

    // return composed value
    return SCS_REG_UNS32(arm, AIRCR);
}

//
// Write AIRCR
//
static ARM_SCS_WRITEFN(writeAIRCR) {

    // use union to extract value fields
    union {Uns32 u32; SCS_REG_DECL(AIRCR);} u = {newValue};

    // update only on magic code match
    if(u.AIRCR.VECTKEY == 0x05fa) {

        // update PRIGROUP if required
        if(SCS_FIELD(arm, AIRCR, PRIGROUP) != u.AIRCR.PRIGROUP) {
            SCS_FIELD(arm, AIRCR, PRIGROUP) = u.AIRCR.PRIGROUP;
            armRefreshExecutionPriorityPendingException(arm);
        }

        // assert interrupt request if required
        SCS_FIELD(arm, AIRCR, SYSRESETREQ) = u.AIRCR.SYSRESETREQ;
        if(SCS_FIELD(arm, AIRCR, SYSRESETREQ)) {
            writeNet(arm, arm->sysResetReq, 1);
        }
    }
}

//
// Write CCR
//
static ARM_SCS_WRITEFN(writeCCR) {

    // update field, preserving read-only bits
    Bool oldUnalignedTrap = SCS_FIELD(arm, CCR, UNALIGN_TRP);
    UPDATE_SCS_MASKED(arm, CCR, newValue);
    Bool newUnalignedTrap = SCS_FIELD(arm, CCR, UNALIGN_TRP);

    // modify blockMask if anligned access trap has changed
    if(oldUnalignedTrap!=newUnalignedTrap) {

        // if this is the first time that unaligned accesses have been enabled
        // or disabled, discard the current code dictionaries (they need to be
        // regenerated with unaligned access checking enabled in the block mask)
        if(!arm->checkUnaligned) {
            vmirtFlushAllDicts((vmiProcessorP)arm);
            arm->checkUnaligned = True;
        }

        // refresh block mask
        armSetBlockMask(arm);
    }
}

//
// Write CFSR
// NOTE: register has W1C (write-1-to-clear) semantics
//
static ARM_SCS_WRITEFN(writeCFSR) {
    SCS_REG_UNS32(arm, CFSR) &= ~newValue;
}

//
// Write HFSR
// NOTE: register has W1C (write-1-to-clear) semantics
//
static ARM_SCS_WRITEFN(writeHFSR) {
    SCS_REG_UNS32(arm, HFSR) &= ~newValue;
}

//
// Write CPACR register value
//
static ARM_SCS_WRITEFN(writeCPACR) {

    // get original register value and writable mask
    Uns32 oldValue = SCS_REG_UNS32(arm, CPACR);
    Uns32 mask     = SCS_MASK_UNS32(arm, CPACR);

    // set the new register value, allowing for writable bits
    SCS_REG_UNS32(arm, CPACR) = ((oldValue&~mask) | (newValue&mask));

    // update blockMask to reflect enabled or disabled coprocessors
    armSetBlockMask(arm);
}

//
// Write FPCCR register value
//
static ARM_SCS_WRITEFN(writeFPCCR) {

    // update register, preserving read-only bits
    Uns32 oldBMValue = SCS_REG_UNS32(arm, FPCCR) & SCS_BLOCKMASK_FPCCR;
    UPDATE_SCS_MASKED(arm, FPCCR, newValue);
    Uns32 newBMValue = SCS_REG_UNS32(arm, FPCCR) & SCS_BLOCKMASK_FPCCR;

    // refresh blockMask if any bits included in block mask have changed
    if(oldBMValue!=newBMValue) {
        armSetBlockMask(arm);
    }
}

//
// Read SYST_CSR
//
static ARM_SCS_READFN(readSYST_CSR) {
    return armReadSYST_CSR(arm);
}

//
// Write SYST_CSR
//
static ARM_SCS_WRITEFN(writeSYST_CSR) {
    armWriteSYST_CSR(arm, newValue);
}

//
// Write SYST_RVR
//
static ARM_SCS_WRITEFN(writeSYST_RVR) {
    armWriteSYST_RVR(arm, newValue);
}

//
// Read SYST_CVR
//
static ARM_SCS_READFN(readSYST_CVR) {
    return armReadSYST_CVR(arm);
}

//
// Write SYST_CVR
// NOTE: value written is always zero (supplied value is ignored)
//
static ARM_SCS_WRITEFN(writeSYST_CVR) {
    armWriteSYST_CVR(arm, 0);
}

//
// Read SHCSR
//
static ARM_SCS_READFN(readSHCSR) {

    // use union to assemble composed value
    union {Uns32 u32; SCS_REG_DECL(SHCSR);} u = {0};

    // seed enable fields
    u.SHCSR.USGFAULTENA = isEnabled(arm, AEN_UsageFault);
    u.SHCSR.BUSFAULTENA = isEnabled(arm, AEN_BusFault);
    u.SHCSR.MEMFAULTENA = isEnabled(arm, AEN_MemManage);

    // seed pending fields
    u.SHCSR.SVCALLPENDED   = isPending(arm, AEN_SVCall);
    u.SHCSR.USGFAULTPENDED = isPending(arm, AEN_UsageFault);
    u.SHCSR.BUSFAULTPENDED = isPending(arm, AEN_BusFault);
    u.SHCSR.MEMFAULTPENDED = isPending(arm, AEN_MemManage);

    // seed active fields
    u.SHCSR.SYSTICKACT  = isActive(arm, AEN_SysTick);
    u.SHCSR.PENDSVACT   = isActive(arm, AEN_PendSV);
    u.SHCSR.MONITORACT  = isActive(arm, AEN_DebugMonitor);
    u.SHCSR.SVCALLACT   = isActive(arm, AEN_SVCall);
    u.SHCSR.USGFAULTACT = isActive(arm, AEN_UsageFault);
    u.SHCSR.BUSFAULTACT = isActive(arm, AEN_BusFault);
    u.SHCSR.MEMFAULTACT = isActive(arm, AEN_MemManage);

    // return composed value
    return u.u32;
}

//
// Write SHCSR
//
static ARM_SCS_WRITEFN(writeSHCSR) {

    // use union to extract value fields
    union {Uns32 u32; SCS_REG_DECL(SHCSR);} u = {newValue};

    // extract enable fields
    setEnabled(arm, AEN_UsageFault, u.SHCSR.USGFAULTENA);
    setEnabled(arm, AEN_BusFault,   u.SHCSR.BUSFAULTENA);
    setEnabled(arm, AEN_MemManage,  u.SHCSR.MEMFAULTENA);

    // extract pending fields
    setPending(arm, AEN_SVCall,     u.SHCSR.SVCALLPENDED);
    setPending(arm, AEN_UsageFault, u.SHCSR.USGFAULTPENDED);
    setPending(arm, AEN_BusFault,   u.SHCSR.BUSFAULTPENDED);
    setPending(arm, AEN_MemManage,  u.SHCSR.MEMFAULTPENDED);

    // extract active fields
    setActive(arm, AEN_SysTick,      u.SHCSR.SYSTICKACT);
    setActive(arm, AEN_PendSV,       u.SHCSR.PENDSVACT);
    setActive(arm, AEN_DebugMonitor, u.SHCSR.MONITORACT);
    setActive(arm, AEN_SVCall,       u.SHCSR.SVCALLACT);
    setActive(arm, AEN_UsageFault,   u.SHCSR.USGFAULTACT);
    setActive(arm, AEN_BusFault,     u.SHCSR.BUSFAULTACT);
    setActive(arm, AEN_MemManage,    u.SHCSR.MEMFAULTACT);

    // refresh any pending exception state
    armRefreshPendingException(arm);
}

//
// Common routine for SHPR/NVIC_IPR read
//
static Uns32 readPriority(armP arm, Uns32 byteOffset) {

    if(byteOffset<arm->exceptNum) {
        return *(Uns32 *)(&arm->xPriority[byteOffset]);
    } else {
        return 0;
    }
}

//
// Common routine for SHPR/NVIC_IPR write
//
static void writePriority(armP arm, Uns32 byteOffset, Uns32 newValue) {

    if(byteOffset<arm->exceptNum) {

        // priority value must be masked to the supported bits
        newValue &= arm->priorityMask;

        // get indexed value in priority array
        Uns32 *valueP = (Uns32 *)(&arm->xPriority[byteOffset]);

        // update priority entry if required
        if(newValue != *valueP) {
            *valueP = newValue;
            armRefreshExecutionPriorityPendingException(arm);
        }
    }
}

//
// Read SHPR1
//
static ARM_SCS_READFN(readSHPR1) {
    return readPriority(arm, 4);
}

//
// Write SHPR1
//
static ARM_SCS_WRITEFN(writeSHPR1) {
    writePriority(arm, 4, newValue & SCS_WRITE_MASK_SHPR1);
}

//
// Read SHPR2
//
static ARM_SCS_READFN(readSHPR2) {
    return readPriority(arm, 8);
}

//
// Write SHPR2
//
static ARM_SCS_WRITEFN(writeSHPR2) {
    writePriority(arm, 8, newValue & SCS_WRITE_MASK_SHPR2);
}

//
// Read SHPR3
//
static ARM_SCS_READFN(readSHPR3) {
    return readPriority(arm, 12);
}

//
// Write SHPR3
//
static ARM_SCS_WRITEFN(writeSHPR3) {
    writePriority(arm, 12, newValue & SCS_WRITE_MASK_SHPR3);
}

//
// Write STIR
// NOTE: this can be made accessible to user mode
//
static ARM_SCS_WRITEFN(writeSTIR) {

    // use union to extract value fields
    union {Uns32 u32; SCS_REG_DECL(STIR);} u = {newValue};
    Uns32 exceptionNum = u.STIR.INTID + AEN_ExternalInt0;

    // ensure interrupt is within bounds
    if(exceptionNum<arm->exceptNum) {

        // raise the exception
        setPending(arm, exceptionNum, 1);

        // refresh any pending exception state
        armRefreshPendingException(arm);
    }
}

//
// Return a composed value from an interrupt mask register
//
static Uns32 readIntReg(armP arm, Uns32 *reg, Uns32 byteOffset) {

    Uns32 wordOffset = byteOffset*4;
    Uns32 result     = 0;

    // validate interrupt register is in bounds
    if(wordOffset<arm->exceptMaskNum) {

        // get lower half of the result
        result = reg[wordOffset] >> AEN_ExternalInt0;

        // upper half is extracted only if this is not the last entry
        if(wordOffset!=(arm->exceptMaskNum-1)) {
            result |= reg[wordOffset+1] << (32-AEN_ExternalInt0);
        }
    }

    return result;
}

//
// Add one-bits from the passed value into the register
//
static void setIntReg(armP arm, Uns32 *reg, Uns32 byteOffset, Uns32 newValue) {

    Uns32 wordOffset = byteOffset*4;

    // validate interrupt register is in bounds
    if(wordOffset<arm->exceptMaskNum) {

        // set lower half of the result
        reg[wordOffset] |= (newValue << AEN_ExternalInt0);

        // upper half is set only if this is not the last entry
        if(wordOffset!=(arm->exceptMaskNum-1)) {
            reg[wordOffset+1] |= (newValue >> (32-AEN_ExternalInt0));
        }

        // refresh any pending exception state
        armRefreshPendingException(arm);
    }
}

//
// Remove one-bits from the passed value into the register
//
static void clearIntReg(armP arm, Uns32 *reg, Uns32 byteOffset, Uns32 newValue) {

    Uns32 wordOffset = byteOffset*4;

    // validate interrupt register is in bounds
    if(wordOffset<arm->exceptMaskNum) {

        // set lower half of the result
        reg[wordOffset] &= ~(newValue << AEN_ExternalInt0);

        // upper half is set only if this is not the last entry
        if(wordOffset!=(arm->exceptMaskNum-1)) {
            reg[wordOffset+1] &= ~(newValue >> (32-AEN_ExternalInt0));
        }

        // refresh any pending exception state
        armRefreshPendingException(arm);
    }
}

//
// Read NVIC_ISER
//
static ARM_SCS_READFN(readNVIC_ISER) {
    return readIntReg(arm, arm->xEnable, byteOffset);
}

//
// Write NVIC_ISER
//
static ARM_SCS_WRITEFN(writeNVIC_ISER) {
    setIntReg(arm, arm->xEnable, byteOffset, newValue);
}

//
// Read NVIC_ICER
//
static ARM_SCS_READFN(readNVIC_ICER) {
    return readIntReg(arm, arm->xEnable, byteOffset);
}

//
// Write NVIC_ICER
//
static ARM_SCS_WRITEFN(writeNVIC_ICER) {
    clearIntReg(arm, arm->xEnable, byteOffset, newValue);
}

//
// Read NVIC_ISPR
//
static ARM_SCS_READFN(readNVIC_ISPR) {
    return readIntReg(arm, arm->xPend, byteOffset);
}

//
// Write NVIC_ISPR
//
static ARM_SCS_WRITEFN(writeNVIC_ISPR) {
    setIntReg(arm, arm->xPend, byteOffset, newValue);
}

//
// Read NVIC_ICPR
//
static ARM_SCS_READFN(readNVIC_ICPR) {
    return readIntReg(arm, arm->xPend, byteOffset);
}

//
// Write NVIC_ICPR
//
static ARM_SCS_WRITEFN(writeNVIC_ICPR) {
    clearIntReg(arm, arm->xPend, byteOffset, newValue);
}

//
// Read NVIC_IABR
//
static ARM_SCS_READFN(readNVIC_IABR) {
    return readIntReg(arm, arm->xActive, byteOffset);
}

//
// Read NVIC_IPR
//
static ARM_SCS_READFN(readNVIC_IPR) {
    return readPriority(arm, byteOffset+AEN_ExternalInt0);
}

//
// Write NVIC_IPR
//
static ARM_SCS_WRITEFN(writeNVIC_IPR) {
    writePriority(arm, byteOffset+AEN_ExternalInt0, newValue);
}


////////////////////////////////////////////////////////////////////////////////
// RESET AND INITIALIZATION
////////////////////////////////////////////////////////////////////////////////

//
// Macro used to set initial value of read-only system register from configuration
//
#define INITIALIZE_SCS_REG(_P, _N) \
    SCS_REG_STRUCT(_P, _N) = SCS_REG_STRUCT_DEFAULT(_P, _N)

//
// Macro used to set initial value of read-only system field from configuration
//
#define INITIALIZE_SCS_FIELD(_P, _N, _F) \
    SCS_FIELD(_P, _N, _F) = SCS_FIELD_DEFAULT(_P, _N, _F)

//
// Macro use to force reset of a system register to a fixed initial value
//
#define RESET_SCS_REG_VALUE(_P, _R, _V) \
    armWriteSysRegPriv(SCS_ID(_R), _P, _V)

//
// Macro use to force reset of a system register to its initial configured state
//
#define RESET_SCS_REG_CONFIG(_P, _R) { \
    union {SCS_REG_DECL(_R); Uns32 u32;} u = {SCS_REG_STRUCT_DEFAULT(_P, _R)}; \
    RESET_SCS_REG_VALUE(_P, _R, u.u32); \
}

//
// Call on initialization
//
void armSysInitialize(armP arm) {

    // initialize SCS registers that are directly represented
    INITIALIZE_SCS_REG(arm, ICTR);
    INITIALIZE_SCS_REG(arm, CPUID);
    INITIALIZE_SCS_REG(arm, SYST_CALIB);
    INITIALIZE_SCS_REG(arm, ID_PFR0);
    INITIALIZE_SCS_REG(arm, ID_PFR1);
    INITIALIZE_SCS_REG(arm, ID_DFR0);
    INITIALIZE_SCS_REG(arm, ID_AFR0);
    INITIALIZE_SCS_REG(arm, ID_MMFR0);
    INITIALIZE_SCS_REG(arm, ID_MMFR1);
    INITIALIZE_SCS_REG(arm, ID_MMFR2);
    INITIALIZE_SCS_REG(arm, ID_MMFR3);
    INITIALIZE_SCS_REG(arm, ID_ISAR0);
    INITIALIZE_SCS_REG(arm, ID_ISAR1);
    INITIALIZE_SCS_REG(arm, ID_ISAR2);
    INITIALIZE_SCS_REG(arm, ID_ISAR3);
    INITIALIZE_SCS_REG(arm, ID_ISAR4);
    INITIALIZE_SCS_REG(arm, ID_ISAR5);
    INITIALIZE_SCS_REG(arm, MVFR0);
    INITIALIZE_SCS_REG(arm, MVFR1);
    INITIALIZE_SCS_REG(arm, MPU_TYPE);

    // configure permanently-enabled exceptions
    arm->xEnable[0] = ARM_EXCEPT_PERMANENT_ENABLE;

    // ICTR.INTLINESNUM depends on the number of configured interrupts
    SCS_FIELD(arm, ICTR, INTLINESNUM) = NUM_INTERRUPTS(arm)/32;
}

//
// Call on reset
//
void armSysReset(armP arm) {

    Uns32 i;

    // reset SCS registers that are directly represented
    RESET_SCS_REG_CONFIG(arm, ACTLR);
    RESET_SCS_REG_CONFIG(arm, CPACR);
    RESET_SCS_REG_VALUE (arm, ICSR,     0x00000000);
    RESET_SCS_REG_VALUE (arm, VTOR,     0x00000000);
    RESET_SCS_REG_VALUE (arm, AIRCR,    0x05fa0000);
    RESET_SCS_REG_VALUE (arm, SCR,      0x00000000);
    RESET_SCS_REG_VALUE (arm, CCR,      0x00000200);
    RESET_SCS_REG_VALUE (arm, SHCSR,    0x00000000);
    RESET_SCS_REG_VALUE (arm, CFSR,     0xffffffff);
    RESET_SCS_REG_VALUE (arm, HFSR,     0x00000000);
    RESET_SCS_REG_VALUE (arm, SYST_CSR, 0x00000000);
    RESET_SCS_REG_VALUE (arm, FPCCR,    0xc0000000);
    RESET_SCS_REG_VALUE (arm, FPCAR,    0x00000000);
    RESET_SCS_REG_VALUE (arm, FPDSCR,   0x00000000);

    arm->FPCARdomain = NULL;

    // reset exception pending and enable masks
    for(i=0; i<arm->exceptMaskNum; i++) {
        arm->xPend  [i] = 0;
        arm->xEnable[i] = 0;
    }

    // reset exception priority numbers
    for(i=0; i<arm->exceptNum; i++) {
        arm->xPriority[i] = 0;
    }

    // configure permanently-enabled exceptions
    arm->xEnable[0] = ARM_EXCEPT_PERMANENT_ENABLE;
}


////////////////////////////////////////////////////////////////////////////////
// SYSTEM REGISTER ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// PPB address range
//
#define PPB_LOW  0xe0000000
#define PPB_HIGH 0xe00fffff

//
// System space base address
//
#define SCS_BASE 0xe000e000

//
// Type used to return information enabling a system register to be read
//
typedef struct armSCSReadInfoS {
    vmiReg        rs;       // if the register can be read by a simple access
    armSCSReadFn  cb;       // if the read requires a callback
    armSysRegDesc regDesc;  // regDesc for read callback
} armSCSReadInfo, *armSCSReadInfoP;

//
// Type used to return information enabling a system register to be written
//
typedef struct armSCSWriteInfoS {
    vmiReg        rd;       // if the register can be written by a simple access
    armSCSWriteFn cb;       // if the write requires a callback
    armSysRegDesc regDesc;  // regDesc for write callback
    Uns32         writeMask;// mask of writable bits in this register
} armSCSWriteInfo, *armSCSWriteInfoP;

//
// Type enumerating the units for which system registers are present
//
typedef enum armUnitE {
    AU_ALL, // always present
    AU_MPU, // present for cores with MPU only
    AU_USM, // access possible in user mode if CCR.USERSETMPEND
    AU_FPU, // access possible if FPU implemented
} armUnit;

//
// Type enumerating classes of access to register
//
typedef enum accessActionE {

    AA_READ  = 0x0,
    AA_WRITE = 0x1,
    AA_PRIV  = 0x0,
    AA_USER  = 0x2,

    AA_PRIV_READ  = AA_READ  | AA_PRIV, // privileged mode read
    AA_PRIV_WRITE = AA_WRITE | AA_PRIV, // privileged mode write
    AA_USER_READ  = AA_READ  | AA_USER, // user mode read
    AA_USER_WRITE = AA_WRITE | AA_USER, // user mode write

    AA_LAST

} accessAction;

//
// Descr strings for accessAction values
//
static const char *accessActionRW[2][2] = {
        {"--", "-w"},
        {"r-", "rw"},
};

//
// This structure records information about each system register
//
typedef struct scsRegAttrsS {
    const char   *name;
    Bool          access[AA_LAST];
    armUnit       unit;
    armSCSReadFn  readCB;
    armSCSWriteFn writeCB;
    Uns32         writeMask;
    Uns32         address;
    Uns32         numWords;
} scsRegAttrs;

//
// Macro to initialize one-word entry in scsRegInfo
//
#define SCS_ATTR(_ID, _PR,_PW,_UR,_UW, _UNIT, _READCB, _WRITECB) \
    [SCS_ID(_ID)] = {                               \
        name      : #_ID,                           \
        access    : {_PR,_PW,_UR,_UW},              \
        unit      : _UNIT,                          \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : SCS_WRITE_MASK_##_ID,           \
        address   : SCS_BASE | SCS_ADDRESS_##_ID,   \
        numWords  : 1                               \
    }

//
// Macro to initialize multi-word entry in scsRegInfo
//
#define SCS_ATTR_N(_ID, _N, _PR,_PW,_UR,_UW, _UNIT, _READCB, _WRITECB) \
    [SCS_ID(_ID##x##_N)] = {                        \
        name      : #_ID"x"#_N,                     \
        access    : {_PR,_PW,_UR,_UW},              \
        unit      : _UNIT,                          \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : SCS_WRITE_MASK_##_ID,           \
        address   : SCS_BASE | SCS_ADDRESS_##_ID,   \
        numWords  : _N                              \
    }

//
// Table of system register attributes
//
const static scsRegAttrs scsRegInfo[SCS_ID(Size)] = {

    ////////////////////////////////////////////////////////////////////////////
    // true registers (represented in processor structure)
    ////////////////////////////////////////////////////////////////////////////

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  ICTR,           1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ACTLR,          1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  CPUID,          1,0,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  VTOR,           1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  AIRCR,          1,1,0,0, AU_ALL, readAIRCR,     writeAIRCR      ),
    SCS_ATTR(  SCR,            1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  CCR,            1,1,0,0, AU_ALL, 0,             writeCCR        ),
    SCS_ATTR(  CFSR,           1,1,0,0, AU_ALL, 0,             writeCFSR       ),
    SCS_ATTR(  HFSR,           1,1,0,0, AU_ALL, 0,             writeHFSR       ),
    SCS_ATTR(  AFSR,           1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  MMAR,           1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  BFAR,           1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  CPACR,          1,1,0,0, AU_ALL, 0,             writeCPACR      ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  SYST_CSR,       1,1,0,0, AU_ALL, readSYST_CSR,  writeSYST_CSR   ),
    SCS_ATTR(  SYST_RVR,       1,1,0,0, AU_ALL, 0,             writeSYST_RVR   ),
    SCS_ATTR(  SYST_CVR,       1,1,0,0, AU_ALL, readSYST_CVR,  writeSYST_CVR   ),
    SCS_ATTR(  SYST_CALIB,     1,1,0,0, AU_ALL, 0,             0               ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  ID_PFR0,        1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_PFR1,        1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_DFR0,        1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_AFR0,        1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_MMFR0,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_MMFR1,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_MMFR2,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_MMFR3,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR0,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR1,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR2,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR3,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR4,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  ID_ISAR5,       1,1,0,0, AU_ALL, 0,             0               ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  FPCCR,          1,1,0,0, AU_FPU, 0,             writeFPCCR      ),
    SCS_ATTR(  FPCAR,          1,1,0,0, AU_FPU, 0,             0               ),
    SCS_ATTR(  FPDSCR,         1,1,0,0, AU_FPU, 0,             0               ),
    SCS_ATTR(  MVFR0,          1,0,0,0, AU_FPU, 0,             0               ),
    SCS_ATTR(  MVFR1,          1,0,0,0, AU_FPU, 0,             0               ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  MPU_TYPE,       1,1,0,0, AU_ALL, 0,             0               ),
    SCS_ATTR(  MPU_CONTROL,    1,1,0,0, AU_ALL, 0,             writeMPU_CONTROL),
    SCS_ATTR(  MPU_RNR,        1,1,0,0, AU_ALL, 0,             writeMPU_RNR    ),

    ////////////////////////////////////////////////////////////////////////////
    // pseudo-registers (not represented in processor structure)
    ////////////////////////////////////////////////////////////////////////////

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  ICSR,           1,1,0,0, AU_ALL, readICSR,      writeICSR       ),
    SCS_ATTR(  SHCSR,          1,1,0,0, AU_ALL, readSHCSR,     writeSHCSR      ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  SHPR1,          1,1,0,0, AU_ALL, readSHPR1,     writeSHPR1      ),
    SCS_ATTR(  SHPR2,          1,1,0,0, AU_ALL, readSHPR2,     writeSHPR2      ),
    SCS_ATTR(  SHPR3,          1,1,0,0, AU_ALL, readSHPR3,     writeSHPR3      ),
    SCS_ATTR(  STIR,           1,1,1,1, AU_USM, ignoreSysRead, writeSTIR       ),

    //         id          num access   unit    readCB         writeCB
    SCS_ATTR_N(NVIC_ISER,  16, 1,1,0,0, AU_ALL, readNVIC_ISER, writeNVIC_ISER  ),
    SCS_ATTR_N(NVIC_ICER,  16, 1,1,0,0, AU_ALL, readNVIC_ICER, writeNVIC_ICER  ),
    SCS_ATTR_N(NVIC_ISPR,  16, 1,1,0,0, AU_ALL, readNVIC_ISPR, writeNVIC_ISPR  ),
    SCS_ATTR_N(NVIC_ICPR,  16, 1,1,0,0, AU_ALL, readNVIC_ICPR, writeNVIC_ICPR  ),
    SCS_ATTR_N(NVIC_IABR,  16, 1,1,0,0, AU_ALL, readNVIC_IABR, ignoreSysWrite  ),
    SCS_ATTR_N(NVIC_IPR,  255, 1,1,0,0, AU_ALL, readNVIC_IPR,  writeNVIC_IPR   ),

    //         id              access   unit    readCB         writeCB
    SCS_ATTR(  MPU_RBAR,       1,1,0,0, AU_MPU, readMPU_RBAR,  writeMPU_RBAR   ),
    SCS_ATTR(  MPU_RASR,       1,1,0,0, AU_MPU, readMPU_RASR,  writeMPU_RASR   ),
    SCS_ATTR(  MPU_RBAR_A1,    1,1,0,0, AU_MPU, readMPU_RBAR,  writeMPU_RBAR   ),
    SCS_ATTR(  MPU_RASR_A1,    1,1,0,0, AU_MPU, readMPU_RASR,  writeMPU_RASR   ),
    SCS_ATTR(  MPU_RBAR_A2,    1,1,0,0, AU_MPU, readMPU_RBAR,  writeMPU_RBAR   ),
    SCS_ATTR(  MPU_RASR_A2,    1,1,0,0, AU_MPU, readMPU_RASR,  writeMPU_RASR   ),
    SCS_ATTR(  MPU_RBAR_A3,    1,1,0,0, AU_MPU, readMPU_RBAR,  writeMPU_RBAR   ),
    SCS_ATTR(  MPU_RASR_A3,    1,1,0,0, AU_MPU, readMPU_RASR,  writeMPU_RASR   )
};

//
// Iterator filling 'desc' with the next system register description -
// 'desc.name' should be initialized to NULL prior to the first call
//
Bool armGetSysRegisterDesc(armSysRegDescP desc) {

    armSCSRegId id = desc->name ? desc->id+1 : 0;

    if(id>=SCS_ID(FirstPseudoReg)) {

        return False;

    } else {

        const scsRegAttrs *attrs = &scsRegInfo[id];

        desc->name    = attrs->name;
        desc->id      = id;
        desc->address = attrs->address;
        desc->privRW  = accessActionRW[attrs->access[AA_PRIV_READ]][attrs->access[AA_PRIV_WRITE]];
        desc->userRW  = accessActionRW[attrs->access[AA_USER_READ]][attrs->access[AA_USER_WRITE]];

        return True;
    }
}

//
// Can the indicated system register be accessed using the passed action on
// this ARM processor variant?
//
static Bool canAccessSysReg(
    armP               arm,
    const scsRegAttrs *attrs,
    accessAction       action
) {
    if(!attrs->access[action]) {
        return False;
    } else switch(attrs->unit) {
        case AU_MPU: return MPU_PRESENT(arm);
        case AU_USM: return !(action&AA_USER) || SCS_FIELD(arm, CCR, USERSETMPEND);
        case AU_FPU: return FPU_PRESENT(arm);
        default:     return True;
    }
}

//
// Fill armSCSReadInfo structure with information about how to perform a system
// register read
//
static Bool getSysReadInfo(
    armP            arm,
    armSCSReadInfoP info,
    armSCSRegId     id,
    accessAction    action
) {
    info->rs = VMI_NOREG;

    if(id==SCS_ID(INVALID)) {

        // invalid register access
        info->cb = ignoreSysRead;

    } else {

        // read of plain register or by callback
        if(scsRegInfo[id].readCB) {
            info->cb = scsRegInfo[id].readCB;
        } else if(id<SCS_ID(FirstPseudoReg)) {
            info->rs = ARM_SCS_REG(id);
        }
    }

    return True;
}

//
// Fill armSCSWriteInfo structure with information about how to perform a
// system register write
//
static Bool getSysWriteInfo(
    armP             arm,
    armSCSWriteInfoP info,
    Uns32            id,
    accessAction     action
) {
    info->rd = VMI_NOREG;

    if(id==SCS_ID(INVALID)) {

        // invalid register access
        info->cb = ignoreSysWrite;

    } else {

        // write of plain register or by callback
        if(scsRegInfo[id].writeCB) {
            info->cb = scsRegInfo[id].writeCB;
        } else if(id<SCS_ID(FirstPseudoReg)) {
            info->rd = ARM_SCS_REG(id);
        }

        // get mask of writable bits
        info->writeMask = scsRegInfo[id].writeMask;
    }

    return True;
}

//
// Return a value read from a fixed offset in the processor structure
//
static ARM_SCS_READFN(readSCS) {
    return arm->scs.regs[id];
}

//
// Write a value to a fixed offset in the processor structure
//
static ARM_SCS_WRITEFN(writeSCS) {

    Uns32 writeMask = scsRegInfo[id].writeMask;
    Uns32 oldValue  = arm->scs.regs[id];

    arm->scs.regs[id] = ((oldValue&~writeMask) | (newValue&writeMask));
}

//
// Is the indicated system register supported on this processor?
//
Bool armGetSysRegSupported(armSCSRegId id, armP arm) {

    const scsRegAttrs *attrs = &scsRegInfo[id];

    return (
        canAccessSysReg(arm, attrs, AA_PRIV_READ) ||
        canAccessSysReg(arm, attrs, AA_PRIV_WRITE)
    );
}

//
// Get system register read callback (privileged mode)
//
static armSCSReadFn getSysPrivRegReadCallBack(armSCSRegId id, armP arm) {

    accessAction       action = AA_PRIV_READ;
    const scsRegAttrs *attrs  = &scsRegInfo[id];
    armSCSReadInfo     info   = {regDesc : {id:id}};

    if(!canAccessSysReg(arm, attrs, action)) {
        return 0;
    } else if(!getSysReadInfo(arm, &info, id, action)) {
        return 0;
    } else if(info.cb) {
        return info.cb;
    } else if(!VMI_ISNOREG(info.rs)) {
        return readSCS;
    } else {
        return 0;
    }
}

//
// Get system register write callback (privileged mode)
//
static armSCSWriteFn getSysPrivRegWriteCallBack(armSCSRegId id, armP arm) {

    accessAction       action = AA_PRIV_WRITE;
    const scsRegAttrs *attrs  = &scsRegInfo[id];
    armSCSWriteInfo    info   = {regDesc : {id:id}};

    if(!canAccessSysReg(arm, attrs, action)) {
        return 0;
    } else if(!getSysWriteInfo(arm, &info, id, action)) {
        return 0;
    } else if(info.cb) {
        return info.cb;
    } else if(!VMI_ISNOREG(info.rd)) {
        return writeSCS;
    } else {
        return 0;
    }
}


////////////////////////////////////////////////////////////////////////////////
// INSERT SYSTEM REGISTERS INTO DOMAIN
////////////////////////////////////////////////////////////////////////////////

//
// Callback function to write a system register
//
static VMI_MEM_WRITE_FN(writeSys) {

    if(processor) {

        armP               arm       = (armP)processor;
        const scsRegAttrs *attrs     = userData;
        armSCSRegId        id        = attrs-scsRegInfo;
        Uns32              offset    = address - attrs->address;
        Uns32              writeMask = attrs->writeMask;
        Bool               bigEndian = SCS_FIELD(arm, AIRCR, ENDIANNESS);
        Uns32              newValue;

        // get the value to write (may be partial register)
        if(bytes==1) {
            newValue = *(Uns8*)value;
        } else if(bytes==2) {
            newValue = SWAP_2_BYTE_COND(*(Uns16*)value, bigEndian);
        } else if(bytes==4) {
            newValue = SWAP_4_BYTE_COND(*(Uns32*)value, bigEndian);
        } else {
            VMI_ABORT("unimplemented system register size %u bytes", bytes);
        }

        if(IN_USER_MODE(arm) && !canAccessSysReg(arm, attrs, AA_USER_WRITE)) {

            // no access to this register in user mode
            armBusFault(arm, address, MEM_PRIV_W);

        } else if(attrs->writeCB) {

            // detect partial register write
            if((offset&3) || (bytes!=4)) {

                // access within register - do read-modify-write of entire 4-byte
                // register
                Uns32 offset4  = offset&~3;
                Uns32 oldValue;
                Uns32 i;

                // get 4-byte value, either directly or by callback
                if(attrs->readCB) {
                    oldValue = attrs->readCB(arm, id, offset4);
                } else {
                    oldValue = arm->scs.regs[id];
                }

                // get 4-byte value with its mask
                union {Uns32 u32; Uns8 u8[4];} uValue = {oldValue};
                union {Uns32 u32; Uns8 u8[4];} uMask  = {writeMask};

                // update writable bits in the subrange
                for(i=offset; i<offset+bytes; i++) {

                    // update one byte, preserving read-only bits
                    uValue.u8[i] = (
                        (uValue.u8[i] & ~uMask.u8[i]) |
                        (newValue     &  uMask.u8[i])
                    );

                    // shift newValue for next iteration
                    newValue >>= 8;
                }

                // put back the modified value
                attrs->writeCB(arm, id, uValue.u32, offset4);

            } else {

                // write full register
                attrs->writeCB(arm, id, newValue, offset);
            }

        } else {

            Uns32 *regValue = &arm->scs.regs[id];

            // detect partial register write
            if((offset&3) || (bytes!=4)) {

                // access within register
                Uns8 *regValue8 = (Uns8 *)regValue;
                Uns32 i;

                // align writeMask
                writeMask >>= (offset*8);

                // update writable bits in the subrange
                for(i=offset; i<offset+bytes; i++) {

                    // update one byte, preserving read-only bits
                    regValue8[i] = (
                        (regValue8[i] & ~writeMask) |
                        (newValue     &  writeMask)
                    );

                    // shift newValue for next iteration
                    newValue  >>= 8;
                    writeMask >>= 8;
                }

            } else {

                // write full register
                *regValue = (
                    (*regValue & ~writeMask) |
                    (newValue  &  writeMask)
                );
            }
        }
    }
}

//
// Callback function to read a system register
//
static VMI_MEM_READ_FN(readSys) {

    if(processor) {

        armP               arm       = (armP)processor;
        const scsRegAttrs *attrs     = userData;
        armSCSRegId        id        = attrs-scsRegInfo;
        Uns32              offset    = address - attrs->address;
        Uns32              result    = 0;
        Bool               bigEndian = SCS_FIELD(arm, AIRCR, ENDIANNESS);

        if(IN_USER_MODE(arm) && !canAccessSysReg(arm, attrs, AA_USER_READ)) {

            // no access to this register in user mode
            armBusFault(arm, address, MEM_PRIV_R);

        } else if(attrs->readCB) {

            // read using callback
            Uns32 offset4 = offset&~3;
            Uns32 shift   = (offset&3) * 8;

            result = attrs->readCB(arm, id, offset4) >> shift;

        } else {

            // read from processor structure
            Uns32 *regValue = &arm->scs.regs[id];
            Uns32  offset4  = offset/4;
            Uns32  shift    = (offset&3) * 8;

            result = regValue[offset4] >> shift;
        }

        // get the value read (may be partial register)
        if(bytes==1) {
            *(Uns8*)value = result;
        } else if(bytes==2) {
            *(Uns16*)value = SWAP_2_BYTE_COND(result, bigEndian);
        } else if(bytes==4) {
            *(Uns32*)value = SWAP_4_BYTE_COND(result, bigEndian);
        } else {
            VMI_ABORT("unimplemented system register size %u bytes", bytes);
        }
    }
}

//
// Callback function to write PPB region not mapped to a system register
//
static VMI_MEM_WRITE_FN(writePPB) {

    armP arm = (armP)processor;

    if(arm && IN_USER_MODE(arm)) {
        armBusFault(arm, address, MEM_PRIV_W);
    }
}

//
// Callback function to read PPB region not mapped to a system register
//
static VMI_MEM_READ_FN(readPPB) {

    armP arm = (armP)processor;

    if(arm && IN_USER_MODE(arm)) {
        armBusFault(arm, address, MEM_PRIV_W);
    } else if(bytes==1) {
        *(Uns8*)value = 0;
    } else if(bytes==2) {
        *(Uns16*)value = 0;
    } else if(bytes==4) {
        *(Uns32*)value = 0;
    } else {
        VMI_ABORT("unimplemented system register size %u bytes", bytes);
    }
}

//
// Type used to record SCS register bounds
//
typedef struct SCSRegAddrS {
    Uns32 lowAddr;      // low address
    Uns32 highAddr;     // high address
} SCSRegAddr, *SCSRegAddrP;

//
// Callback used to sort SCS register entries in ascending address
//
static Int32 compareRegAddr(const void *va, const void *vb) {

    const SCSRegAddr *a = va;
    const SCSRegAddr *b = vb;

    return a->lowAddr<b->lowAddr ? -1 : 1;
}

//
// Insert SCS region into the passed domain at the standard location
//
void armSysCreateSCSRegion(armP arm, memDomainP domain) {

    armSCSRegId id;
    SCSRegAddr  scsRegAddrs[SCS_ID(Size)];
    Uns32       prevLow = PPB_LOW;
    Uns32       numRegs = 0;
    Uns32       i;

    // remove any existing PPB region mapping
    vmirtUnaliasMemory(domain, PPB_LOW, PPB_HIGH);

    // install system registers
    for(id=0; id<SCS_ID(Size); id++) {

        // is the system register supported on this variant?
        if(armGetSysRegSupported(id, arm)) {

            const scsRegAttrs *attrs   = &scsRegInfo[id];
            SCSRegAddrP        regAddr = &scsRegAddrs[numRegs++];

            // save register bounds
            regAddr->lowAddr  = attrs->address;
            regAddr->highAddr = regAddr->lowAddr + attrs->numWords*4 - 1;

            // install callbacks to implement the register
            vmirtMapCallbacks(
                domain, regAddr->lowAddr, regAddr->highAddr, readSys, writeSys,
                (void *)attrs
            );
        }
    }

    // sort register descriptions in ascending address order
    qsort(scsRegAddrs, numRegs, sizeof(scsRegAddrs[0]), compareRegAddr);

    // fill unmapped sections in PPB region with default callback
    for(i=0; i<numRegs; i++) {

        SCSRegAddrP regAddr = &scsRegAddrs[i];

        if(regAddr->lowAddr > prevLow) {
            vmirtMapCallbacks(
                domain, prevLow, regAddr->lowAddr-1, readPPB, writePPB, 0
            );
        }

        prevLow = regAddr->highAddr+1;
    }

    // map final PPB subregion using default callbacks
    if(prevLow<PPB_HIGH) {
        vmirtMapCallbacks(domain, prevLow, PPB_HIGH, readPPB, writePPB, 0);
    }
}


////////////////////////////////////////////////////////////////////////////////
// DEBUGGER ACCESS TO SYSTEM REGISTERS
////////////////////////////////////////////////////////////////////////////////

//
// Perform a privileged-mode read of the system register
//
Bool armReadSysRegPriv(armSCSRegId id, armP arm, Uns32 *result) {

    // get the read callback to apply
    armSCSReadFn readCB = getSysPrivRegReadCallBack(id, arm);

    // either apply the callback or set the result to 0
    if(readCB) {
        *(Uns32*)result = readCB(arm, id, 0);
        return True;
    } else {
        *(Uns32*)result = 0;
        return False;
    }
}

//
// Perform a privileged-mode write of the system register
//
Bool armWriteSysRegPriv(armSCSRegId id, armP arm, Uns32 value) {

    // get the write callback to apply
    armSCSWriteFn writeCB = getSysPrivRegWriteCallBack(id, arm);

    // apply the callback if it is found
    if(writeCB) {
        writeCB(arm, id, value, 0);
        return True;
    } else {
        return False;
    }
}


////////////////////////////////////////////////////////////////////////////////
// SYSTEM REGISTER PROGRAMMER'S VIEW
////////////////////////////////////////////////////////////////////////////////

//
// Callback to obtain the value of a system register
//
static VMI_VIEW_VALUE_FN(scsRegViewCB) {

    // get the processor for this view object
    vmiViewObjectP baseObject      = vmiviewGetViewObjectParent(object);
    vmiViewObjectP processorObject = vmiviewGetViewObjectParent(baseObject);
    armP           arm             = vmirtGetViewObjectUserData(processorObject);
    UnsPS          id              = (UnsPS)clientData;

    armReadSysRegPriv(id, arm, (Uns32*)buffer);

    return VMI_VVT_UNS32;
}

//
// Add programmer's view of system register
//
void armAddSysRegisterView(
    armSCSRegId    id,
    armP           arm,
    vmiViewObjectP baseObject,
    const char    *name
) {
    accessAction   action = AA_PRIV_READ;
    armSCSReadInfo info   = {regDesc : {id:id}};

    if(!getSysReadInfo(arm, &info, id, action)) {

        // unknown register

    } else if(!info.cb && VMI_ISNOREG(info.rs)) {

        // register not readable on this variant

    } else {

        // extract fields from id (where known)
        const scsRegAttrs *attrs = &scsRegInfo[id];

        // create register description string
        char description[32];
        snprintf(description, sizeof(description), "0x%08x", attrs->address);

        // create new register object
        vmiViewObjectP regObject = vmirtAddViewObject(
            baseObject, name, description
        );

        // either reference the value directly or use a callback
        if(info.cb) {
            vmirtSetViewObjectValueCallback(regObject, scsRegViewCB, (void *)id);
        } else {
            Uns32 *refValue = &arm->scs.regs[id];
            vmirtSetViewObjectRefValue(regObject, VMI_VVT_UNS32, refValue);
        }
    }
}


