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

#include <stdio.h>
#include <string.h>

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiPorts.h"

// model header files
#include "armConfig.h"
#include "armDecode.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMessage.h"
#include "armStructure.h"
#include "armSys.h"
#include "armSysRegisters.h"
#include "armUtils.h"
#include "armVariant.h"
#include "armVM.h"
#include "armVFP.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_EXCEPTION"

//
// This value is used when no exception priority is specified
//
#define EXC_NO_PRIORITY 0x7fffffff


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return processor endianness
//
inline static memEndian getEndian(armP arm) {
    return armGetEndian((vmiProcessorP)arm, False);
}

//
// Is the processor halted?
//
inline static Bool isHalted(armP arm) {
    return vmirtIsHalted((vmiProcessorP)arm);
}

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Return current program counter
//
inline static const char *getName(armP arm) {
    return vmirtProcessorName((vmiProcessorP)arm);
}

//
// Get vector table base address
//
inline static Uns32 getVectorTable(armP arm) {
    return SCS_REG_UNS32(arm, VTOR) & 0x3fffff80;
}

//
// Jump to address
//
inline static void setPC(armP arm, Uns32 newPC) {
    vmirtSetPC((vmiProcessorP)arm, newPC);
}

//
// Jump to exception vector
//
inline static void setPCException(armP arm, Uns32 newPC) {
    vmirtSetPCException((vmiProcessorP)arm, newPC);
}

//
// Is the processor in handler mode (note special case when sleep on ISR return)
//
inline static Bool inHandlerMode(armP arm) {
    return IN_HANDLER_MODE(arm) && !arm->sleepOnExit;
}

//
// Load a value from the passed memory domain, returning a boolean indicating if
// the load succeeded
//
static Bool loadWord(armP arm, memDomainP domain, Uns32 address, Uns32 *result) {

    // do the read
    *result = vmirtRead4ByteDomain(domain, address, getEndian(arm), MEM_AA_TRUE);

    // indicate whether the write succeeded
    return (arm->derivedException==AEN_None);
}

//
// Store a value to the passed memory domain, returning a boolean indicating if
// the store succeeded
//
static Bool storeWord(armP arm, memDomainP domain, Uns32 address, Uns32 value) {

    // do the write
    vmirtWrite4ByteDomain(domain, address, getEndian(arm), value, MEM_AA_TRUE);

    // indicate whether the write succeeded
    return (arm->derivedException==AEN_None);
}

//
// Load a value from the default system address map domain, returning a boolean
// indicating if the load succeeded
//
static Bool loadWordDefault(armP arm, Uns32 address, Uns32 *result) {

    // save current derived exception and context
    armExceptNum oldDerivedException = arm->derivedException;
    armExceptCxt oldContext          = arm->exceptionContext;
    Bool         ok;

    // reset derived exception and context
    arm->derivedException = AEN_None;
    arm->exceptionContext = AEC_ReadVector;

    // do the load using default address map, restoring previous derived
    // exception if it succeeds
    if((ok=loadWord(arm, arm->dds.system, address, result))) {
        arm->derivedException = oldDerivedException;
    }

    // restore previous context
    arm->exceptionContext = oldContext;

    return ok;
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
// Suspend the executing processor
//
static void suspend(armP arm, Uns32 haltPC, armDisable reason) {

    arm->disableReason |= reason;

    setPC(arm, haltPC);

    vmirtHalt((vmiProcessorP)arm);
}

//
// Enter lockup state
//
static void lockup(armP arm, Uns32 haltPC) {

    suspend(arm, haltPC, AD_LOCKUP);

    writeNet(arm, arm->lockup, 1);

    if(ARM_DEBUG_EXCEPT(arm)) {
        vmiMessage("W", CPU_PREFIX "LOCKUP",
            "ARM %s lock up at 0x%08x\n",
            getName(arm), haltPC
        );
    }
}

//
// If the processor was waiting for the passed reason, restart it and return
// True; otherwise, return False
//
static Bool restart(armP arm, armDisable reason) {

    if(arm->disableReason) {

        arm->disableReason &= ~reason;

        if(!arm->disableReason) {
            vmirtRestartNext((vmiProcessorP)arm);
            return True;
        }
    }

    return False;
}


////////////////////////////////////////////////////////////////////////////////
// EXCEPTION INFORMATION (FOR DEBUGGER INTEGRATION)
////////////////////////////////////////////////////////////////////////////////

//
// Helper macro for creation of exception entry in exceptions table
//
#define ARM_EXCEPTION_INFO(_D) [AEN_##_D] = {name:#_D, code:AEN_##_D}

//
// Helper macro for creation of interrupt entry in exceptions table
//
#define ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, MINOR_S, MINOR_N) \
    [(_MAJOR_N*16)+MINOR_N+16] = {              \
        name : "ExternalInt"#_MAJOR_S#MINOR_S,  \
        code : (_MAJOR_N*16)+MINOR_N+16         \
    }

//
// Helper macro for creation of 16 interrupt entries in exceptions table
//
#define ARM_INTERRUPT_INFOx16(_MAJOR_S, _MAJOR_N) \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 0,  0), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 1,  1), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 2,  2), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 3,  3), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 4,  4), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 5,  5), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 6,  6), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 7,  7), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 8,  8), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, 9,  9), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, a, 10), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, b, 11), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, c, 12), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, d, 13), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, e, 14), \
    ARM_INTERRUPT_INFO(_MAJOR_S, _MAJOR_N, f, 15)  \

//
// Table of exception mode descriptors
//
static const vmiExceptionInfo exceptions[AEN_LAST] = {
    ARM_EXCEPTION_INFO(Reset),
    ARM_EXCEPTION_INFO(NMI),
    ARM_EXCEPTION_INFO(HardFault),
    ARM_EXCEPTION_INFO(MemManage),
    ARM_EXCEPTION_INFO(BusFault),
    ARM_EXCEPTION_INFO(UsageFault),
    ARM_EXCEPTION_INFO(SVCall),
    ARM_EXCEPTION_INFO(DebugMonitor),
    ARM_EXCEPTION_INFO(PendSV),
    ARM_EXCEPTION_INFO(SysTick),
    ARM_INTERRUPT_INFOx16(00, 0),
    ARM_INTERRUPT_INFOx16(01, 1),
    ARM_INTERRUPT_INFOx16(02, 2),
    ARM_INTERRUPT_INFOx16(03, 3),
    ARM_INTERRUPT_INFOx16(04, 4),
    ARM_INTERRUPT_INFOx16(05, 5),
    ARM_INTERRUPT_INFOx16(06, 6),
    ARM_INTERRUPT_INFOx16(07, 7),
    ARM_INTERRUPT_INFOx16(08, 8),
    ARM_INTERRUPT_INFOx16(09, 9),
    ARM_INTERRUPT_INFOx16(0a, 10),
    ARM_INTERRUPT_INFOx16(0b, 11),
    ARM_INTERRUPT_INFOx16(0c, 12),
    ARM_INTERRUPT_INFOx16(0d, 13),
    ARM_INTERRUPT_INFOx16(0e, 14),
    ARM_INTERRUPT_INFOx16(0f, 15),
    ARM_INTERRUPT_INFOx16(10, 16),
    ARM_INTERRUPT_INFOx16(11, 17),
    ARM_INTERRUPT_INFOx16(12, 18),
    ARM_INTERRUPT_INFOx16(13, 19),
    ARM_INTERRUPT_INFOx16(14, 20),
    ARM_INTERRUPT_INFOx16(15, 21),
    ARM_INTERRUPT_INFOx16(16, 22),
    ARM_INTERRUPT_INFOx16(17, 23),
    ARM_INTERRUPT_INFOx16(18, 24),
    ARM_INTERRUPT_INFOx16(19, 25),
    ARM_INTERRUPT_INFOx16(1a, 26),
    ARM_INTERRUPT_INFOx16(1b, 27),
    ARM_INTERRUPT_INFOx16(1c, 28),
    ARM_INTERRUPT_INFOx16(1d, 29),
    ARM_INTERRUPT_INFOx16(1e, 30)
};

//
// Return the current processor exception
//
VMI_GET_EXCEPTION_FN(armGetException) {
    armP arm = (armP)processor;
    return &exceptions[arm->enabledException];
}

//
// Exception mode iterator
//
VMI_EXCEPTION_INFO_FN(armExceptionInfo) {

    armP               arm    = (armP)processor;
    Uns32              intNum = NUM_INTERRUPTS(arm);
    armExceptNum       last   = intNum+16;
    vmiExceptionInfoCP end    = exceptions+last;
    vmiExceptionInfoCP this;

    // on the first call, start with the first member of the table
    if(!prev) {
        prev = exceptions-1;
    }

    // search for the next member with seeded name
    for(this=prev+1; this!=end; this++) {
        if(this->name) {
            return this;
        }
    }

    // no more exceptions
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// MODE INFORMATION (FOR DEBUGGER INTEGRATION)
////////////////////////////////////////////////////////////////////////////////

//
// Table of processor mode descriptions
//
static const vmiModeInfo modes[] = {
    {name:"Thread",  code:0},
    {name:"Handler", code:1},
    {0}
};

//
// Processor mode iterator
//
VMI_MODE_INFO_FN(armModeInfo) {

    // on the first call, start with the first member of the table
    if(!prev) {
        prev = modes-1;
    }

    vmiModeInfoCP this = prev+1;

    return this->name ? this : 0;
}

//
// Return the current processor mode
//
VMI_GET_MODE_FN(armGetMode) {
    armP arm = (armP)processor;
    return &modes[IN_HANDLER_MODE(arm)];
}


////////////////////////////////////////////////////////////////////////////////
// EXCEPTION ACTIONS
////////////////////////////////////////////////////////////////////////////////

//
// This specifies valid combinations for exception return
//
typedef enum armExcReturnTypeE {
    AXRT_HANDLER_MAIN   =  1,
    AXRT_THREAD_MAIN    =  9,
    AXRT_THREAD_PROCESS = 13
} armExcReturnType;

//
// This macro returns a mask to select sub-priority bits
//
#define SUB_PRI_MASK(_A) ((2<<SCS_FIELD(_A, AIRCR, PRIGROUP))-1)

//
// Return the priority of the numbered exception
// AEN_Reset, AEN_NMI and AEN_HardFault have fixed group priority; other
// exceptions have group priority extracted from xPriority
//
inline static Int32 groupPriority(armP arm, armExceptNum num) {
    return num<4 ? num-4 : (arm->xPriority[num] & ~SUB_PRI_MASK(arm));
}

//
// Return the sub-priority of the numbered exception
// NOTE: AEN_Reset, AEN_NMI and AEN_HardFault always return zero
//
inline static Uns32 subPriority(armP arm, armExceptNum num) {
    return arm->xPriority[num] & SUB_PRI_MASK(arm);
}

//
// Is the numbered exception enabled?
//
inline static Bool isEnabled(armP arm, armExceptNum num) {
    return (
        EX_MASK_GET(arm->xEnable, num) &&
        (groupPriority(arm, num)<arm->executionPriority)
    );
}

//
// Return the number of any pending enabled exception that should be taken now,
// assuming the processor is in the mode specified
//
static armExceptNum exceptionEnabled(armP arm, Bool handlerMode) {

    armExceptNum num = arm->enabledException;

    // return the selected exception if it has a priority number less than the
    // current priority
    if(num==AEN_None) {
        return num;
    } else if(!handlerMode) {
        return num;
    } else if(groupPriority(arm, num)<arm->executionPriority) {
        return num;
    } else {
        return AEN_None;
    }
}

//
// Take exception immediately if required (may require processor restart)
//
static void takeExceptionNow(armP arm) {

    armExceptNum exceptNum = exceptionEnabled(arm, inHandlerMode(arm));

    if(exceptNum && (!arm->disableReason || restart(arm, AD_WFI|AD_WFE))) {
        vmirtDoSynchronousInterrupt((vmiProcessorP)arm);
    }
}

//
// Type representing number and priority of an exception
//
typedef struct exceptionDescS {
    armExceptNum num;       // exception number
    Int32        groupPri;  // exception group priority
    Uns32        subPri;    // exception sub-priority
} exceptionDesc, *exceptionDescP;

//
// Update exceptionDesc to the passed exception if it is the highest priority
// one found so far
//
static void selectHighestPriority(
    armP           arm,
    exceptionDescP desc,
    armExceptNum   num
) {
    Int32 groupPri = groupPriority(arm, num);
    Uns32 subPri   = subPriority(arm, num);

    if(
        // this is the highest group priority exception
        (groupPri<desc->groupPri) ||
        // this is the highest sub-priority exception
        ((groupPri==desc->groupPri) && (subPri<desc->subPri))
    ) {
        desc->num      = num;
        desc->groupPri = groupPri;
        desc->subPri   = subPri;
    }
}

//
// Recalculate the boosted execution priority
//
static void refreshBoostedPriority(armP arm) {

    Int32 oldPriority = arm->executionPriority;
    Int32 newPriority = arm->unboostedPriority;
    Int32 BASEPRI     = arm->sregs.BASEPRI;

    // allow for FAULTMASK, PRIMASK and BASEPRI execution priority boost
    if(arm->sregs.FAULTMASK && (newPriority>-1)) {
        newPriority = -1;
    } else if(arm->sregs.PRIMASK && (newPriority>0)) {
        newPriority = 0;
    } else if(BASEPRI && (newPriority>BASEPRI)) {
        newPriority = BASEPRI;
    }

    // has execution priority changed?
    if(oldPriority != newPriority) {

        // report execution priority change
        if(ARM_DEBUG_EXCEPT(arm)) {
            vmiPrintf(
                "Execution priority %d -> %d\n",
                oldPriority, newPriority
            );
        }

        // save the new priority
        arm->executionPriority = newPriority;

        // if the MPU is generally enabled but disabled when in a handler with
        // negative execution priority (MPU_CONTROL.HFNMIENA=0), the MPU must be
        // *disabled* when the old priority is non-negative and the new priority
        // is negative, and *enabled* if the priority is negative and the new
        // priority is non-negative
        if(MPU_ENABLED(arm) && !SCS_FIELD(arm, MPU_CONTROL, HFNMIENA)) {

            Bool oldNegative = (oldPriority<0);
            Bool newNegative = (newPriority<0);

            if(oldNegative!=newNegative) {
                armSwitchMode(arm);
            }
        }
    }
}

//
// Recalculate the unboosted execution priority
//
static void refreshUnboostedPriority(armP arm) {

    exceptionDesc desc = {num:AEN_None, groupPri:EXC_NO_PRIORITY};
    Uns32         index;

    // process array of active exceptions, high to low priority
    for(index=0; index<arm->exceptMaskNum; index++) {

        Uns32 entry = arm->xActive[index];

        if(entry) {

            Uns32 bit  = 0;
            Uns32 mask = 1;

            do {

                // is this bit selected?
                if(entry & mask) {

                    // calculate armExceptNum for this index/bit pair
                    armExceptNum num = (index*32) + bit;

                    // select the exception if it is higher priority than any
                    // found so far
                    selectHighestPriority(arm, &desc, num);

                    // remove the selected bit from the mask
                    entry &= ~mask;
                }

                // step to the next bit
                mask <<= 1;
                bit++;

            } while(entry);
        }
    }

    // save the new unboosted priority
    arm->unboostedPriority = desc.groupPri;

    // refresh the boosted priority
    refreshBoostedPriority(arm);
}

//
// Calculate the highest-priority pending exception number
//
static void refreshPendingException(armP arm) {

    exceptionDesc enabledDesc = {num:AEN_None, groupPri:EXC_NO_PRIORITY};
    exceptionDesc pendingDesc = {num:AEN_None, groupPri:EXC_NO_PRIORITY};
    Uns32         index;

    // assume no interrupts are pending initially
    arm->pendingInterrupt = False;

    // process array of pending exceptions, high to low priority
    for(index=0; index<arm->exceptMaskNum; index++) {

        Uns32 entry = arm->xPend[index];

        if(entry) {

            Uns32 bit  = 0;
            Uns32 mask = 1;

            do {

                // is this bit selected?
                if(entry & mask) {

                    // calculate armExceptNum for this index/bit pair
                    armExceptNum num = (index*32) + bit;

                    // set ICSR.ISRPENDING if this is a pending interrupt
                    if(num>=AEN_ExternalInt0) {
                        arm->pendingInterrupt = True;
                    }

                    // find the highest priority pending exception (regardless
                    // of enabled)
                    selectHighestPriority(arm, &pendingDesc, num);

                    // find the highest priority enabled exception
                    if(isEnabled(arm, num)) {
                        selectHighestPriority(arm, &enabledDesc, num);
                    }

                    // remove the selected bit from the mask
                    entry &= ~mask;
                }

                // step to the next bit
                mask <<= 1;
                bit++;

            } while(entry);
        }
    }

    // has highest-priority pending exception changed?
    if(arm->pendingException != pendingDesc.num) {

        // report execution priority change
        if(ARM_DEBUG_EXCEPT(arm)) {
            vmiPrintf(
                "Pending exception %u -> %u (priority %d)\n",
                arm->pendingException, pendingDesc.num, pendingDesc.groupPri
            );
        }

        // save the highest-priority pending exception
        arm->pendingException = pendingDesc.num;
    }

    // has highest-priority enabled exception changed?
    if(arm->enabledException != enabledDesc.num) {

        // report execution priority change
        if(ARM_DEBUG_EXCEPT(arm)) {
            vmiPrintf(
                "Enabled exception %u -> %u (priority %d)\n",
                arm->enabledException, enabledDesc.num, enabledDesc.groupPri
            );
        }

        // save the highest-priority pending exception
        arm->enabledException = enabledDesc.num;
    }
}

//
// Derive value of ICSR.RETTOBASE
//
Bool armGetRetToBase(armP arm) {

    Uns32 index;

    // process array of active exceptions
    for(index=0; index<arm->exceptMaskNum; index++) {

        Uns32 entry = arm->xActive[index];

        if(entry) {

            Uns32 bit  = 0;
            Uns32 mask = 1;

            do {

                // is this bit selected?
                if(entry & mask) {

                    // calculate armExceptNum for this index/bit pair
                    armExceptNum thisNum = (index*32) + bit;

                    // return False if the active exception is not the currently
                    // executing one
                    if(thisNum != PSR_FIELD(arm, exceptNum)) {
                        return False;
                    }

                    // remove the selected bit from the mask
                    entry &= ~mask;
                }

                // step to the next bit
                mask <<= 1;
                bit++;

            } while(entry);
        }
    }

    // no other exceptions are active
    return True;
}

//
// ARMv7-M ClearExclusiveLocal implementation
//
inline static void clearExclusiveLocal(armP arm) {
    arm->exclusiveTag = ARM_NO_TAG;
}

//
// ARMv7-M ARM SetEventRegister implementation
//
inline static void setEventRegister(armP arm) {
    arm->eventRegister = !restart(arm, AD_WFE);
}

//
// ARMv7-M ARM Raise implementation
//
static void raise(armP arm, armExceptNum num) {

    // restart the processor on reset, or if it is waiting-for-event and
    // SCR.SEVONPEND is set
    if(num==AEN_Reset) {
        restart(arm, AD_WFE|AD_WFI|AD_LOCKUP);
    } else if(!EX_MASK_GET(arm->xPend, num) && SCS_FIELD(arm, SCR, SEVONPEND)) {
        restart(arm, AD_WFE);
    }

    // set pending bit AFTER restart
    EX_MASK_SET_1(arm->xPend, num);

    // refresh pending exception
    refreshPendingException(arm);
}

//
// ARMv7-M ARM Raise implementation, takes effect next instruction
//
inline static void raiseNext(armP arm, armExceptNum num) {

    raise(arm, num);
}

//
// ARMv7-M ARM Raise implementation, takes effect immediately
//
inline static void raiseNow(armP arm, armExceptNum num) {

    raise(arm, num);

    // refresh pending exception
    takeExceptionNow(arm);
}

//
// ARMv7-M ARM Lower implementation
//
static void lower(armP arm, armExceptNum num) {

    EX_MASK_SET_0(arm->xPend, num);

    // refresh pending exception
    if(num==arm->enabledException) {
        refreshPendingException(arm);
    }
}

//
// ARMv7-M ARM Activate implementation
//
static void activate(armP arm, armExceptNum num) {

    EX_MASK_SET_0(arm->xPend,   num);
    EX_MASK_SET_1(arm->xActive, num);

    arm->nestedActivation++;

    // refresh execution priority and pending exception
    refreshUnboostedPriority(arm);
    refreshPendingException(arm);
}

//
// ARMv7-M ARM Deactivate implementation
//
static void deactivate(armP arm, armExceptNum num) {

    EX_MASK_SET_0(arm->xActive, num);

    // clear FAULTMASK on any return except NMI
    if(PSR_FIELD(arm, exceptNum) != AEN_NMI) {
        arm->sregs.FAULTMASK = 0;
    }

    arm->nestedActivation--;

    // refresh execution priority and pending exception
    refreshUnboostedPriority(arm);
    refreshPendingException(arm);
}

//
// ARMv7-M ARM DeactivateAll implementation
//
static void deactivateAll(armP arm) {

    Uns32 i;

    // set all exceptions inactive
    for(i=0; i<arm->exceptMaskNum; i++) {
        arm->xPend  [i] = 0;
        arm->xActive[i] = 0;
    }

    arm->nestedActivation = 0;

    // refresh execution priority and pending exception
    refreshUnboostedPriority(arm);
    refreshPendingException(arm);
}

//
// This is the UpdateFPCCR psuedo-code function
// Called on exception entry when lazy context save is active
//
static void updateFPCCR(armP arm, Uns32 frameptr, memDomainP domain) {

    if (CONTROL_FIELD(arm, FPCA) && SCS_FIELD(arm, FPCCR, LSPEN)) {

        Uns32 fpFramePtr = frameptr + 0x20;
        Uns32 pri        = arm->executionPriority;

        // Save the pointer to the unfilled space on the stack for the FP state
        // (also save the domain to use with the pointer)
        SCS_FIELD(arm, FPCAR, ADDRESS) = fpFramePtr >> 3;
        arm->FPCARdomain = domain;

        armUpdateLSPACT(arm, 1);

        SCS_FIELD(arm, FPCCR, USER)    = IN_USER_MODE(arm);
        SCS_FIELD(arm, FPCCR, THREAD)  = !IN_HANDLER_MODE(arm);
        SCS_FIELD(arm, FPCCR, HFRDY)   = pri > -1;
        SCS_FIELD(arm, FPCCR, BFRDY)   = EX_MASK_GET(arm->xEnable, AEN_BusFault)     && (pri > arm->xPriority[AEN_BusFault]);
        SCS_FIELD(arm, FPCCR, MMRDY)   = EX_MASK_GET(arm->xEnable, AEN_MemManage)    && (pri > arm->xPriority[AEN_MemManage]);
        SCS_FIELD(arm, FPCCR, MONRDY)  = EX_MASK_GET(arm->xEnable, AEN_DebugMonitor) && (pri > arm->xPriority[AEN_DebugMonitor]);

    }
}

//
// Run-time check if the VFP is currently enabled for the current mode
// TODO: Should this use the mode being returned to??
//
static Bool doVFPEnabled(armP arm) {

    Bool  inUserMode = IN_USER_MODE(arm);
    Uns32 cp10Enable = SCS_FIELD(arm, CPACR, cp10);
    Bool  enabled    = True;

    // check CPACR for permission to use cp10 (and cp11) in the current user/privilege mode
    if(inUserMode && !(cp10Enable&2)) {
        enabled = False;
    } else if(!inUserMode && !(cp10Enable&1)) {
        enabled = False;
    }

    if (!enabled) {
        // VFP disabled - usage fault
        SCS_FIELD(arm, CFSR, NOCP) = 1;
        arm->derivedException = AEN_UsageFault;
        return False;
    }

    return True;

}

//
// Save the FP state on the exception stack frame
//
static Bool pushFP(armP arm, Uns32 fpFramePtr, memDomainP domain) {

    Uns32 i;

    for (i = 0; i < 16; i++) {
        if (!storeWord(arm, domain, fpFramePtr+(i*4), FP_REG(arm, i)))
            return False;
    }

    if (!storeWord(arm, domain, fpFramePtr+64, FPSCR_REG(arm))) {
        return False;
    }

    return True;

}

//
// ARMv7-M ARM PushStack implementation
//
static void pushStack(armP arm, memDomainP domain, Uns32 returnAddress) {

    // NOTE: stack pointer currently in arm->regs[ARM_REG_SP] is always
    // consistent with CONTROL.useSP_Process at this point
    // TODO: When this is updated to support FP, must also update armmCpuHelper
    Bool  haveFPExt     = FPU_PRESENT(arm);
    Bool  fpCtxtActive  = haveFPExt && CONTROL_FIELD(arm, FPCA);
    Uns32 frameSize     = fpCtxtActive ? 0x68 : 0x20;
    Uns32 frameptr      = arm->regs[ARM_REG_SP] - frameSize;
    Bool  frameptralign = (SCS_FIELD(arm, CCR, STKALIGN) || fpCtxtActive) && (frameptr&4);

    // allow for alignment
    if(frameptralign) {
        frameptr &= ~4;
    }

    // save modified stack pointer
    arm->regs[ARM_REG_SP] = frameptr;

    // get the PSR and modify the align4 field
    armPSR tmpPSR = {reg:armReadCPSR(arm)};
    tmpPSR.fields.align4 = frameptralign;
    Uns32 value = tmpPSR.reg;

    // save the registers, terminating on derived exception
    if (storeWord(arm, domain, frameptr,      arm->regs[0]         ) &&
        storeWord(arm, domain, frameptr+0x4,  arm->regs[1]         ) &&
        storeWord(arm, domain, frameptr+0x8,  arm->regs[2]         ) &&
        storeWord(arm, domain, frameptr+0xc,  arm->regs[3]         ) &&
        storeWord(arm, domain, frameptr+0x10, arm->regs[12]        ) &&
        storeWord(arm, domain, frameptr+0x14, arm->regs[ARM_REG_LR]) &&
        storeWord(arm, domain, frameptr+0x18, returnAddress        ) &&
        storeWord(arm, domain, frameptr+0x1c, value                ))
    {
        // if normal stack context successfully saved then save FP context if indicated
        if (fpCtxtActive) {
            if (SCS_FIELD(arm, FPCCR, LSPEN) == 0) {
                if (doVFPEnabled(arm)) {
                    pushFP(arm, frameptr+0x20, domain);
                }
            } else {
                updateFPCCR(arm, frameptr, domain);
            }
        }
    }

    // determine exception return type
    armExcReturnType returnType;

    if(inHandlerMode(arm)) {
        returnType = AXRT_HANDLER_MAIN;
    } else if(!USE_SP_PROCESS(arm)) {
        returnType = AXRT_THREAD_MAIN;
    } else {
        returnType = AXRT_THREAD_PROCESS;
    }

    // update LR for correct return behavior
    if (fpCtxtActive) {
        // Bit 4 = 0 indicates fp context was active
        arm->regs[ARM_REG_LR] = 0xffffffe0 | returnType;
    } else {
        arm->regs[ARM_REG_LR] = 0xfffffff0 | returnType;
    }
}

//
// Restore the FP state from the exception stack frame
//
static Bool popFP(armP arm, Uns32 frameptr, Uns32 *fp, memDomainP domain, Bool *restore) {

    *restore = False;

    if (SCS_FIELD(arm, FPCCR, LSPACT)) {

        // State in FP is still valid, do not need to restore
        armUpdateLSPACT(arm, 0);

    } else if (!doVFPEnabled(arm)) {

        // VFP disabled - usage fault generated
        return False;

    } else {

        // Restore fp state, aborting if error occurs
        Uns32 fpFramePtr = frameptr + 0x20;
        Uns32 i;

        for (i = 0; i < 16; i++) {
            if (!loadWord(arm, domain, fpFramePtr+(i*4), fp+i))
                return False;
        }

        if (!loadWord(arm, domain, fpFramePtr+64, fp+16)) {
            return False;
        }

        *restore = True;
    }

    return True;
}

//
// ARMv7-M ARM PopStack implementation
//
static void popStack(armP arm, Uns32 targetPC, memDomainP domain, Bool requiredHandlerMode) {

    // these hold intermediate values prior to commit
    Uns32 r0, r1, r2, r3, r12, lr, pc, psr, fp[17];
    Bool restoreFP = False;

    // NOTE: stack pointer currently in arm->regs[ARM_REG_SP] is always
    // consistent with CONTROL.useSP_Process at this point
    Bool  haveFPExt     = FPU_PRESENT(arm);
    Bool  fpca          = (targetPC & 0x10) == 0;
    Bool  fpCtxtActive  = haveFPExt && fpca;
    Uns32 frameSize     = fpCtxtActive ? 0x68 : 0x20;
    Uns32 frameptr      = arm->regs[ARM_REG_SP];

    // load stack frame, terminating on derived exception
    if(
        loadWord(arm, domain, frameptr,      &r0 ) &&
        loadWord(arm, domain, frameptr+0x4,  &r1 ) &&
        loadWord(arm, domain, frameptr+0x8,  &r2 ) &&
        loadWord(arm, domain, frameptr+0xc,  &r3 ) &&
        loadWord(arm, domain, frameptr+0x10, &r12) &&
        loadWord(arm, domain, frameptr+0x14, &lr ) &&
        loadWord(arm, domain, frameptr+0x18, &pc ) &&
        loadWord(arm, domain, frameptr+0x1c, &psr) &&
        (!fpCtxtActive || popFP(arm, frameptr, fp, domain, &restoreFP))
    ) {

        // extract PSR fields
        armPSR tmpPSR            = {reg:psr};
        Bool   actualHandlerMode = tmpPSR.fields.exceptNum && True;

        // if fp exists restore the fpca value that existed on exception entry
        if (haveFPExt) {
            armUpdateFPCA(arm, fpca);
        }

        // 1. must be returning to Thumb mode
        // 2. must be returning to consistent Handler mode
        if(!(tmpPSR.fields.T && (requiredHandlerMode==actualHandlerMode))) {

            SCS_FIELD(arm, CFSR, INVPC) = 1;
            arm->derivedException = AEN_UsageFault;

        } else {

            // commit loaded values
            arm->regs[0]          = r0;
            arm->regs[1]          = r1;
            arm->regs[2]          = r2;
            arm->regs[3]          = r3;
            arm->regs[12]         = r12;
            arm->regs[ARM_REG_LR] = lr;
            if (restoreFP) {
                Uns32 i;
                for (i = 0; i < 16; i++) {
                    FP_REG(arm, i) = fp[i];
                }
                FPSCR_REG(arm) = fp[16];
            }

            // commit PC (NOTE: least significant bit should be zero, but we mask it
            // here to be sure)
            setPC(arm, pc & ~1);

            // adjust stack pointer
            // NOTE: stack pointer currently in arm->regs[ARM_REG_SP] is
            // always consistent with CONTROL.useSP_Process at this point
            arm->regs[ARM_REG_SP] += frameSize;

            // restore 4-byte alignment if required
            if(tmpPSR.fields.align4  && (SCS_FIELD(arm, CCR, STKALIGN) || fpCtxtActive)) {
                arm->regs[ARM_REG_SP] |= 4;
            }

            // write valid PSR bits
            armWriteCPSR(arm, psr, PSR_ALL);
        }
    }
}

//
// Do reset exception
//
static void takeReset(armP arm) {

    Uns32 table = getVectorTable(arm);

    // these hold intermediate values prior to commit
    Uns32 sp, pc;

    // get initial values of spMain and PC
    if(
        loadWordDefault(arm, table,   &sp) &&
        loadWordDefault(arm, table+4, &pc)
    ) {
        // reset stack pointers and link register
        arm->regs[ARM_REG_SP] = sp & 0xfffffffc;
        arm->bank.R13_process = 0;
        arm->regs[ARM_REG_LR] = 0xffffffff;

        // set PSR
        armPSR psr = {fields:{T:pc&1}};
        armWriteCPSR(arm, psr.reg, PSR_ALL);

        // force other special registers to reset value (effects of this are
        // accounted for in deactivateAll below)
        arm->sregs.CONTROL.reg = 0;
        arm->sregs.PRIMASK     = 0;
        arm->sregs.FAULTMASK   = 0;
        arm->sregs.BASEPRI     = 0;

        // reset system state and VM structures
        armSysReset(arm);
        armVMReset(arm);
        armFPReset(arm);

        // Set the block mask since regs with fields in block mask have been reset
        armSetBlockMask(arm);

        // deactivate all exceptions
        deactivateAll(arm);

        // processor is not in exclusive access mode
        clearExclusiveLocal(arm);
        setEventRegister(arm);

        // jump to reset address (Thumb mode bit masked off)
        setPCException(arm, pc & ~1);
    }
}

//
// Do common exception actions
//
static void takeException(armP arm, armExceptNum exceptionNumber) {

    Uns32 table = getVectorTable(arm);
    Uns32 pc;

    // get vector address
    if(loadWordDefault(arm, table+(4*exceptionNumber), &pc)) {

        // set PSR (all fields except flags)
        armPSR psr = {fields:{T:pc&1, exceptNum:exceptionNumber}};
        armWriteCPSR(arm, psr.reg, PSR_NOT_FLAGS);

        // Clear CONTROL.FPCA if FP present
        // Note: Documentation says to set to 1 here but that makes no sense!!!!
        if (FPU_PRESENT(arm)) {
            armUpdateFPCA(arm, 0);
        }

        // current stack is Main
        armWriteSPProcess(arm, False);

        // activate exception
        activate(arm, exceptionNumber);

        // processor is not in exclusive access mode
        clearExclusiveLocal(arm);
        setEventRegister(arm);

        // if this is an interrupt, signal to any external interrupt controller
        // that it is being serviced
        if(exceptionNumber>=AEN_ExternalInt0) {
            writeNet(arm, arm->intISS, exceptionNumber-AEN_ExternalInt0);
        }

        // jump to exception vector (Thumb mode bit masked off)
        setPCException(arm, pc & ~1);
    }
}

//
// Escalate priority to AEN_HardFault if this exception is not enabled or lower
// or equal priority to the execution priority
//
static armExceptNum escalatePriority(armP arm, armExceptNum exceptionNumber) {

    if(isEnabled(arm, exceptionNumber)) {
        return exceptionNumber;
    } else if(exceptionNumber==AEN_DebugMonitor) {
        SCS_FIELD(arm, HFSR, DEBUGEVT) = 1;
    } else {
        SCS_FIELD(arm, HFSR, FORCED) = 1;
    }

    return AEN_HardFault;
}

//
// Return the memory domain to use for pushStack/popStack when executing with
// the passed priority
//
static memDomainP getPriorityStackDomain(armP arm, Int32 priority) {
    if(!MPU_ENABLED(arm)) {
        return arm->dds.system;
    } else if(priority>=0) {
        return arm->dds.vmPriv;
    } else if(SCS_FIELD(arm, MPU_CONTROL, HFNMIENA)) {
        return arm->dds.vmPriv;
    } else {
        return arm->dds.system;
    }
}

//
// Do common actions at start of attempt to take exception - actual exception
// taken may be an escalated one
//
static void startException(
    armP         arm,
    armExceptNum exceptionNumber,
    Uns32        returnAddress
) {
    Int32 oldPriority = arm->executionPriority;
    Bool  lockupDerived;

    if(arm->exceptionContext == AEC_PreserveFPState) {
        // TODO: Remove this when this exception is fully modeled
        vmiMessage("E", CPU_PREFIX "_LFPSPE", "Exceptions during lazy FP state preservation not supported yet");
        vmirtStop();
    }

    // restore normal data domain for this mode (in case this exception is a
    // result of LDRT or STRT, for example)
    if(arm->restoreDomain) {
        armVMRestoreNormalDataDomain(arm);
    }

    // escalate priority  to AEN_HardFault if this exception is not enabled
    // or lower or equal priority to the execution priority
    exceptionNumber = escalatePriority(arm, exceptionNumber);

    // indicate that we are entering the exception prologue
    arm->exceptionContext = AEC_PushStack;
    arm->derivedException = AEN_None;

    // do exception-specific actions
    if(exceptionNumber==AEN_Reset) {

        // any derived exception here causes lockup
        lockupDerived = True;

        // do reset operations
        takeReset(arm);

    } else {

        // any derived exception here causes lockup only if the current priority
        // is negative
        lockupDerived = (oldPriority<0);

        // get the appropriate memory domain to use for vector reads and
        // pushStack writes
        Int32      priority = groupPriority(arm, exceptionNumber);
        memDomainP domain   = getPriorityStackDomain(arm, priority);

        // save register state
        pushStack(arm, domain, returnAddress);

        // take the exception unless pushStack created unrecoverable errors
        if(!(lockupDerived && arm->derivedException)) {
            takeException(arm, exceptionNumber);
        }
    }

    // if there is a derived exception, either activate it or enter lockup state
    if(!arm->derivedException) {
        // no action
    } else if(lockupDerived) {
        lockup(arm, 0xffffffff);
    } else {
        raiseNext(arm, escalatePriority(arm, arm->derivedException));
    }

    // indicate exception prologue is no longer active
    arm->exceptionContext = AEC_None;
}

//
// Take a derived exception (uses tail chaining)
//
static void derivedException(
    armP         arm,
    armExceptNum exceptionNumber,
    Uns32        targetPC
) {
    // save target address in LR
    arm->regs[ARM_REG_LR] = targetPC;

    // deactivate the running exception
    deactivate(arm, PSR_FIELD(arm, exceptNum));

    // take the derived exception (tail-chained)
    takeException(arm, escalatePriority(arm, exceptionNumber));
}

//
// Should the processor sleet on exit from the exception?
//
inline static Bool doSleepOnExit(armP arm, armExceptNum exceptNum) {
    return EX_IS_INTERRUPT(exceptNum) && SCS_FIELD(arm, SCR, SLEEPONEXIT);
}

//
// Handle exception return
//
void armExceptionReturn(armP arm, Uns32 targetPC) {

    armExceptNum     returningExceptionNumber = PSR_FIELD(arm, exceptNum);
    armExcReturnType type                     = targetPC & EXC_RETURN_TYPE;
    Bool             handlerMode              = (type==AXRT_HANDLER_MAIN);

    // sanity check we are being called in handler mode
    VMI_ASSERT(
        inHandlerMode(arm),
        "require handler mode"
    );

    // sanity check this is indeed an exception return
    VMI_ASSERT(
        (targetPC&EXC_RETURN_MAGIC) == EXC_RETURN_MAGIC,
        "targetPC is not an exception return address"
    );

    // initially assume no derived exception
    arm->derivedException = AEN_None;

    if(!EX_MASK_GET(arm->xActive, returningExceptionNumber)) {

        // returning from inactive handler
        SCS_FIELD(arm, CFSR, INVPC) = 1;
        arm->derivedException = AEN_UsageFault;

    } else switch(type) {

        case AXRT_HANDLER_MAIN:
            // return to Handler
            if(arm->nestedActivation==1) {
                SCS_FIELD(arm, CFSR, INVPC) = 1;
                arm->derivedException = AEN_UsageFault;
            } else {
                armWriteSPProcess(arm, False);
            }
            break;

        case AXRT_THREAD_MAIN:
            // return to Thread using Main stack
            if((arm->nestedActivation!=1) && SCS_FIELD(arm, CCR, NONBASETHRDENA)) {
                SCS_FIELD(arm, CFSR, INVPC) = 1;
                arm->derivedException = AEN_UsageFault;
            } else {
                armWriteSPProcess(arm, False);
            }
            break;

        case AXRT_THREAD_PROCESS:
            // return to Thread using Process stack
            if((arm->nestedActivation!=1) && SCS_FIELD(arm, CCR, NONBASETHRDENA)) {
                SCS_FIELD(arm, CFSR, INVPC) = 1;
                arm->derivedException = AEN_UsageFault;
            } else {
                armWriteSPProcess(arm, True);
            }
            break;

        default:
            // illegal EXC_RETURN
            SCS_FIELD(arm, CFSR, INVPC) = 1;
            arm->derivedException = AEN_UsageFault;
            break;
    }

    if(!arm->derivedException) {

        // get data domain *before* deactivating the exception (for popStack)
        memDomainP domain = getPriorityStackDomain(arm, arm->executionPriority);

        // deactivate the terminating exception (NOTE: this can update execution
        // priority and so affect the stack domain)
        deactivate(arm, returningExceptionNumber);

        if(exceptionEnabled(arm, handlerMode)) {

            // take tail-chained exception
            arm->regs[ARM_REG_LR] = targetPC;
            takeException(arm, arm->enabledException);

        } else if(doSleepOnExit(arm, returningExceptionNumber)) {

            // return to top level with SCR.SLEEPONEXIT set should suspend the
            // processor (reawaken it when the next interrupt comes in)
            arm->regs[ARM_REG_LR] = targetPC;
            suspend(arm, getPC(arm), AD_WFI);
            arm->sleepOnExit = True;

        } else {

            // restore register state (may set arm->derivedException)
            arm->exceptionContext = AEC_PopStack;
            popStack(arm, targetPC, domain, handlerMode);
            arm->exceptionContext = AEC_None;

            // error in load operations or inconsistent IPSR - reincrement
            // nestedActivation (to undo the effect of deactivate above)
            if(arm->derivedException) {
                arm->nestedActivation++;
            }
        }

        // processor is not in exclusive access mode
        clearExclusiveLocal(arm);
        setEventRegister(arm);
    }

    // handle any derived exception as a tail-chained one at higher activation
    // depth
    if(!arm->derivedException) {
        // no action
    } else if((arm->executionPriority<0) && (returningExceptionNumber==AEN_NMI)) {
        lockup(arm, 0xffffffff);
    } else {
        derivedException(arm, arm->derivedException, targetPC);
    }
}

//
// Perform actions required for alignment fault
//
inline static void doAlign(armP arm) {
    startException(arm, AEN_UsageFault, getPC(arm));
}

//
// Do breakpoint exception (prefetch abort)
//
void armBKPT(armP arm, Uns32 thisPC) {

    if(arm->executionPriority<0) {
        lockup(arm, thisPC);
    } else {
        startException(arm, AEN_DebugMonitor, thisPC);
    }
}

//
// Do software exception
//
void armSWI(armP arm, Uns32 thisPC) {

    if(arm->executionPriority<0) {
        lockup(arm, thisPC);
    } else {
        startException(arm, AEN_SVCall, thisPC+2);
    }
}

//
// Do UsageFault exception
//
void armUsageFault(armP arm, Uns32 thisPC) {

    if(arm->executionPriority<0) {
        lockup(arm, thisPC);
    } else {
        startException(arm, AEN_UsageFault, thisPC);
    }
}

//
// Do BusFault exception
//
void armBusFault(armP arm, Uns32 faultAddress, memPriv priv) {

    Int32 priority    = arm->executionPriority;
    Bool  ignoreFault = False;
    Bool  lockUpAtFFF = False;

    // clear valid bits
    SCS_FIELD(arm, CFSR, BFARVALID) = 0;

    // update fault status
    if(priv==MEM_PRIV_X) {
        SCS_FIELD(arm, CFSR, IBUSERR) = 1;
    } else if(arm->exceptionContext==AEC_PushStack) {
        SCS_FIELD(arm, CFSR, STKERR) = 1;
    } else if(arm->exceptionContext==AEC_PopStack) {
        SCS_FIELD(arm, CFSR, UNSTKERR) = 1;
    } else if(arm->exceptionContext==AEC_ReadVector) {
        SCS_FIELD(arm, HFSR, VECTTBL) = 1;
        lockUpAtFFF = True;
    } else {

        SCS_FIELD(arm, CFSR, PRECISERR) = 1;
        SCS_FIELD(arm, CFSR, BFARVALID) = 1;
        SCS_REG_UNS32(arm, BFAR)        = faultAddress;

        // ignore this fault if priority is negative and CCR.BFHFNMIGN is set
        ignoreFault = (priority<0) && SCS_FIELD(arm, CCR, BFHFNMIGN);
    }

    if(ignoreFault) {
        // no action if fault is ignored for some reason
    } else if(priority<0) {
        arm->derivedException = AEN_HardFault;
        lockup(arm, lockUpAtFFF ? 0xffffffff : getPC(arm));
    } else if(arm->exceptionContext == AEC_ReadVector) {
        arm->derivedException = AEN_HardFault;
    } else if(arm->exceptionContext != AEC_None) {
        arm->derivedException = AEN_BusFault;
        if(arm->exceptionContext == AEC_PreserveFPState) {
            // TODO: Remove this when this exception is fully modeled
            vmiMessage("E", CPU_PREFIX "_LFPSPE", "Exceptions during lazy FP state preservation not supported yet");
            vmirtStop();
        }
    } else {
        startException(arm, AEN_BusFault, getPC(arm));
    }
}

//
// Do MemManage exception
//
void armMemoryAbort(armP arm, Uns32 faultAddress, memPriv priv) {

    // clear valid bits
    SCS_FIELD(arm, CFSR, MMARVALID) = 0;

    // update fault status
    if(priv==MEM_PRIV_X) {
        SCS_FIELD(arm, CFSR, IACCVIOL)  = 1;
    } else if(arm->exceptionContext==AEC_PushStack) {
        SCS_FIELD(arm, CFSR, MSTKERR)   = 1;
    } else if(arm->exceptionContext==AEC_PopStack) {
        SCS_FIELD(arm, CFSR, MUNSTKERR) = 1;
    } else {
        SCS_FIELD(arm, CFSR, DACCVIOL)  = 1;
        SCS_FIELD(arm, CFSR, MMARVALID) = 1;
        SCS_REG_UNS32(arm, MMAR)        = faultAddress;
    }

    // either take immediately or handle as a derived exception later
    if(arm->executionPriority<0) {
        arm->derivedException = AEN_MemManage;
        lockup(arm, getPC(arm));
    } else if(arm->exceptionContext != AEC_None) {
        arm->derivedException = AEN_MemManage;
        if(arm->exceptionContext == AEC_PreserveFPState) {
            // TODO: Remove this when this exception is fully modeled
            vmiMessage("E", CPU_PREFIX "_LFPSPE", "Exceptions during lazy FP state preservation not supported yet");
            vmirtStop();
        }
    } else {
        startException(arm, AEN_MemManage, getPC(arm));
    }
}

//
// Refresh execution priority on any state change which affects it
//
void armRefreshExecutionPriority(armP arm) {

    // refresh execution priority
    Int32 oldPriority = arm->executionPriority;
    refreshBoostedPriority(arm);
    Int32 newPriority = arm->executionPriority;

    // if execution priority has been lowered, determine whether pending
    // exception is now active
    if((arm->pendingException!=AEN_None) && (newPriority>oldPriority)) {

        armExceptNum num      = arm->pendingException;
        Int32        priority = groupPriority(arm, num);

        if(priority<newPriority) {

            // report execution priority change
            if(ARM_DEBUG_EXCEPT(arm)) {
                vmiPrintf(
                    "Enabled exception %u -> %u (priority %d)\n",
                    arm->enabledException, num, priority
                );
            }

            // save the highest-priority pending exception
            arm->enabledException = num;

            // take exceptions now if required
            takeExceptionNow(arm);
        }
    }
}

//
// Refresh pending exception state on any state change which affects it
//
void armRefreshPendingException(armP arm) {

    // refresh pending exception
    refreshPendingException(arm);

    // take exceptions now if required
    takeExceptionNow(arm);
}

//
// Refresh execution priority and pending exception state on any state change
// which affects them
//
void armRefreshExecutionPriorityPendingException(armP arm) {

    // refresh execution priority
    refreshUnboostedPriority(arm);
    refreshPendingException(arm);

    // take exceptions now if required
    takeExceptionNow(arm);
}


////////////////////////////////////////////////////////////////////////////////
// PRIVILEGE, ALIGNMENT AND FETCH EXCEPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Read privilege exception handler
//
VMI_RD_PRIV_EXCEPT_FN(armRdPrivExceptionCB) {

    armP        arm       = (armP)processor;
    Uns32       faultAddr = (Uns32)address;
    memPriv     priv      = MEM_PRIV_R;
    armVMAction action    = armVMMiss(arm, priv, faultAddr, bytes, attrs);

    if(action == MA_OK) {
        memDomainP domain = vmirtGetProcessorDataDomain(processor);
        vmirtReadNByteDomain(domain, faultAddr, value, bytes, 0, attrs);
    }
}

//
// Write privilege exception handler
//
VMI_WR_PRIV_EXCEPT_FN(armWrPrivExceptionCB) {

    armP        arm       = (armP)processor;
    Uns32       faultAddr = (Uns32)address;
    memPriv     priv      = MEM_PRIV_W;
    armVMAction action    = armVMMiss(arm, priv, faultAddr, bytes, attrs);

    if(action == MA_OK) {
        memDomainP domain = vmirtGetProcessorDataDomain(processor);
        vmirtWriteNByteDomain(domain, faultAddr, value, bytes, 0, attrs);
    }
}

//
// Read alignment exception handler
//
VMI_RD_ALIGN_EXCEPT_FN(armRdAlignExceptionCB) {

    armP arm = (armP)processor;

    SCS_FIELD(arm, CFSR, UNALIGNED) = 1;
    doAlign(arm);

    return 0;
}

//
// Write alignment exception handler
//
VMI_WR_ALIGN_EXCEPT_FN(armWrAlignExceptionCB) {

    armP arm = (armP)processor;

    SCS_FIELD(arm, CFSR, UNALIGNED) = 1;
    doAlign(arm);

    return 0;
}

//
// Read abort exception handler (generated by calls to icmAbortRead in memory
// read callbacks)
//
VMI_RD_ABORT_EXCEPT_FN(armRdAbortExceptionCB) {

    armP arm = (armP)processor;

    if(isFetch) {
        armBusFault(arm, address, MEM_PRIV_X);
    } else {
        armBusFault(arm, address, MEM_PRIV_R);
    }
}

//
// Write abort exception handler (generated by calls to icmAbortWrite in memory
// write callbacks)
//
VMI_WR_ABORT_EXCEPT_FN(armWrAbortExceptionCB) {

    armP arm = (armP)processor;

    armBusFault(arm, address, MEM_PRIV_W);
}

//
// Validate instruction fetch from the passed address
//
static Bool validateFetchAddress(
    armP  arm,
    Uns32 thisPC,
    Uns32 snap,
    Bool  complete
) {
    vmiProcessorP  processor = (vmiProcessorP)arm;
    memAccessAttrs attrs     = complete ? MEM_AA_TRUE : MEM_AA_FALSE;
    armVMAction    action;

    if(vmirtIsExecutable(processor, thisPC)) {

        // no exception pending
        return True;

    } else if((action=armVMMiss(arm, MEM_PRIV_X, thisPC, 2, attrs)) == MA_EXCEPTION) {

        // permission exception of some kind, handled by armVMMiss, so no
        // further action required here.
        return False;

    } else {

        // no exception pending
        return True;
    }
}

//
// This is called by the simulator when fetching from an instruction address.
// It gives the model an opportunity to take an exception instead.
//
VMI_IFETCH_FN(armIFetchExceptionCB) {

    armP         arm       = (armP)processor;
    Uns32        thisPC    = (Uns32)address;
    Uns32        snap      = IN_THUMB_MODE(arm) ? 1 : 3;
    armExceptNum exceptNum = exceptionEnabled(arm, inHandlerMode(arm));
    Bool         fetchOK;

    if(exceptNum!=AEN_None) {

        // exception pending
        fetchOK = False;

        if(!complete) {

            // no action

        } else if(!arm->sleepOnExit) {

            // update registers to complete exception if required
            startException(arm, exceptNum, thisPC);

        } else {

            // restart after SCR.SLEEPONEXIT, taking tail-chained exception
            arm->sleepOnExit = False;

            // indicate that we are entering the exception epilogue
            arm->exceptionContext = AEC_PopStack;
            arm->derivedException = AEN_None;

            // take tail-chained exception
            takeException(arm, exceptNum);

            // indicate exception epilogue is no longer active
            arm->exceptionContext = AEC_None;
        }

    } else if(!IN_THUMB_MODE(arm)) {

        // unsupported ARM mode execution
        fetchOK = False;

        // update registers to complete exception if required
        if(complete) {
            armUsageFault(arm, thisPC);
        }

    } else if(!validateFetchAddress(arm, thisPC, snap, complete)) {

        // fetch exception (handled in validateFetchAddress)
        fetchOK = False;

    } else if((thisPC+2) & (MIN_PAGE_SIZE-1)) {

        // simPC isn't two bytes before page end - success
        fetchOK = True;

    } else if(armGetInstructionSize(arm, thisPC) == 2) {

        // instruction at simPC is a two-byte instruction - success
        fetchOK = True;

    } else if(!validateFetchAddress(arm, thisPC+2, snap, complete)) {

        // fetch exception (handled in validateFetchAddress)
        fetchOK = False;

    } else {

        // no exception
        fetchOK = True;
    }

    // return appropriate result
    if(fetchOK) {
        return VMI_FETCH_NONE;
    } else if(complete) {
        return VMI_FETCH_EXCEPTION_COMPLETE;
    } else {
        return VMI_FETCH_EXCEPTION_PENDING;
    }
}

//
// This is called by the simulator on a simulated arithmetic exception
//
VMI_ARITH_EXCEPT_FN(armArithExceptionCB) {

    armP arm = (armP)processor;

    if(exceptionContext==VMI_EXCEPT_CXT_CALL) {

        // not expecting any arithmetic exceptions in calls from morphed code
        return VMI_NUMERIC_UNHANDLED;

    } else switch(exceptionType) {

        case VMI_INTEGER_DIVIDE_BY_ZERO:
            // handle divide-by-zero
            if(SCS_FIELD(arm, CCR, DIV_0_TRP)) {
                SCS_FIELD(arm, CFSR, DIVBYZERO) = 1;
                armUsageFault(arm, getPC(arm));
            } else {
                arm->regs[arm->divideTarget] = 0;
            }
            return VMI_NUMERIC_ABORT;

        case VMI_INTEGER_OVERFLOW:
            // handle overflow (MIN_INT / -1)
            arm->regs[arm->divideTarget] = 0x80000000;
            return VMI_NUMERIC_ABORT;

        default:
            // not expecting any other arithmetic exception types
            return VMI_NUMERIC_UNHANDLED;
    }
}


////////////////////////////////////////////////////////////////////////////////
// TICK TIMER
////////////////////////////////////////////////////////////////////////////////

//
// Return lower 32 bits of the instruction count
//
inline static Uns64 getThisICount(armP arm) {
    return vmirtGetICount((vmiProcessorP)arm);
}

//
// Is the tick timer enabled?
//
static Bool timerEnabled(armP arm) {
    return (
        !arm->disableTimerRVR0 &&
        SCS_FIELD(arm, SYST_CSR, ENABLE) &&
        SCS_FIELD(arm, SYST_CSR, CLKSOURCE)
    );
}

//
// Must the model timer be set?
//
static Bool setModelTimer(armP arm) {
    return (
        timerEnabled(arm) &&
        (
            // must set model timer if SysTick interrupt is enabled
            SCS_FIELD(arm, SYST_CSR, TICKINT) ||
            // must set model timer if COUNTFLAG must be set on wrap
            !SCS_FIELD(arm, SYST_CSR, COUNTFLAG) ||
            // must set model timer if reset value will change from before
            (SCS_REG_UNS32(arm, SYST_RVR) != (arm->timerModulus-1))
        )
    );
}

//
// Return calculated SYST_CVR register value
//
static Uns32 getSYST_CVR(armP arm) {

    if(timerEnabled(arm)) {

        Uns64 current      = getThisICount(arm);
        Uns32 timerModulus = arm->timerModulus;
        Uns32 delta        = (current-arm->timerBase) % timerModulus;

        return delta ? timerModulus-delta : 0;

    } else {

        return SCS_REG_UNS32(arm, SYST_CVR);
    }
}

//
// Set SYST_CVR register value, enabling interrupts if required
//
static void setSYST_CVR(armP arm, Uns32 SYST_CVR) {

    Uns32 SYST_RVR = SCS_REG_UNS32(arm, SYST_RVR);

    // if both SYST_CVR and SYST_RVR are zero, timer is disabled
    arm->disableTimerRVR0 = !(SYST_CVR || SYST_RVR);

    if(timerEnabled(arm)) {

        // allow for reset to SYST_RVR on decrement through zero
        if(!SYST_CVR) {
            SYST_CVR = arm->timerModulus = SYST_RVR+1;
        }

        // set base instruction count
        arm->timerBase = getThisICount(arm) - arm->timerModulus + SYST_CVR;

    } else {

        // timer stopped, record current value
        SCS_REG_UNS32(arm, SYST_CVR) = SYST_CVR;
    }

    // set model interrupt if required
    if(setModelTimer(arm)) {

        Uns32 timerDelta = SYST_CVR-1;

        // show tick timer timeout delta
        if(ARM_DEBUG_EXCEPT(arm)) {
            vmiPrintf(
                "Tick timer expiry scheduled in %u instructions\n",
                timerDelta
            );
        }

        vmirtSetICountInterrupt((vmiProcessorP)arm, timerDelta);

    } else {

        vmirtClearICountInterrupt((vmiProcessorP)arm);
    }
}

//
// Read SYST_CVR register
//
Uns32 armReadSYST_CVR(armP arm) {
    return getSYST_CVR(arm);
}

//
// Write SYST_CVR register
//
void armWriteSYST_CVR(armP arm, Uns32 newValue) {

    // SYST_CSR.COUNTFLAG is cleared by a write to SYST_CVR
    SCS_FIELD(arm, SYST_CSR, COUNTFLAG) = 0;

    // set new count value, accounting for pending exceptions
    setSYST_CVR(arm, newValue);
}

//
// Read SYST_CSR register
//
Uns32 armReadSYST_CSR(armP arm) {

    Uns32 oldValue = SCS_REG_UNS32(arm, SYST_CSR);

    // action only required if SYST_CSR.COUNTFLAG is set
    if(SCS_FIELD(arm, SYST_CSR, COUNTFLAG)) {

        // get current value of count register prior to SYST_CSR update
        Uns32 SYST_CVR = getSYST_CVR(arm);

        // SYST_CSR.COUNTFLAG is cleared by a read of SYST_CSR
        SCS_FIELD(arm, SYST_CSR, COUNTFLAG) = 0;

        // restore original count value, accounting for pending exceptions
        setSYST_CVR(arm, SYST_CVR);
    }

    return oldValue;
}

//
// Write SYST_CSR register
//
void armWriteSYST_CSR(armP arm, Uns32 newValue) {

    // get current value of count register prior to SYST_CSR update
    Uns32 SYST_CVR = getSYST_CVR(arm);
    Uns32 oldValue = SCS_REG_UNS32(arm, SYST_CSR);

    // update the register
    SCS_REG_UNS32(arm, SYST_CSR) = (
        (oldValue & ~SCS_WRITE_MASK_SYST_CSR) |
        (newValue &  SCS_WRITE_MASK_SYST_CSR)
    );

    // restore original count value, accounting for pending exceptions
    setSYST_CVR(arm, SYST_CVR);
}

//
// Write SYST_RVR register
//
void armWriteSYST_RVR(armP arm, Uns32 newValue) {

    // mask to allowed range
    newValue &= SCS_WRITE_MASK_SYST_RVR;

    // action only required if SYST_RVR has changed
    if(newValue != SCS_REG_UNS32(arm, SYST_RVR)) {

        // get current value of count register prior to SYST_CSR update
        Uns32 SYST_CVR = getSYST_CVR(arm);

        // update the register
        SCS_REG_UNS32(arm, SYST_RVR) = newValue;

        // restore original count value, accounting for pending exceptions
        setSYST_CVR(arm, SYST_CVR);
    }
}

//
// This is called when the tick timer has expired. It should set the interrupt
// pending bit for this exception. The exception is subsequently handled in the
// instruction fetch handler, armIFetchExceptionCB.
//
VMI_ICOUNT_FN(armICountPendingCB) {

    armP  arm      = (armP)processor;
    Uns32 SYST_RVR = SCS_REG_UNS32(arm, SYST_RVR);

    // report tick timer expiry
    if(ARM_DEBUG_EXCEPT(arm)) {
        vmiPrintf("Tick timer expired\n");
    }

    // sanity check that the timer is enabled
    VMI_ASSERT(timerEnabled(arm), "timer interrupt, but timer was disabled");

    // indicate timer has expired
    SCS_FIELD(arm, SYST_CSR, COUNTFLAG) = 1;

    // if interrupt generation is enabled, raise SysTick exception
    if(SCS_FIELD(arm, SYST_CSR, TICKINT)) {
        raiseNow(arm, AEN_SysTick);
    }

    // this is the length of the countdown timer cycle
    arm->timerModulus = SYST_RVR+1;

    // schedule the next interrupt event, including one instruction from this
    // cycle
    setSYST_CVR(arm, SYST_RVR ? arm->timerModulus+1 : 0);
}


////////////////////////////////////////////////////////////////////////////////
// EXTERNAL INTERRUPTS
////////////////////////////////////////////////////////////////////////////////

//
// Called by the simulator when an external reset is raised
//
static VMI_NET_CHANGE_FN(externalReset) {
    raiseNow((armP)processor, AEN_Reset);
}

//
// Called by the simulator when an external reset is raised
//
static VMI_NET_CHANGE_FN(externalNMI) {
    raiseNow((armP)processor, AEN_NMI);
}


static void externalInterrupt(armP arm, Uns32 intNum, Bool raise) {

    armExceptNum exceptNum = intNum+16;

    if(intNum>=NUM_INTERRUPTS(arm)) {

        // interrupt out of range
        vmiMessage("W", CPU_PREFIX "NET",
            "ARM external interrupt ID is %u but must be 0..%u - ignored",
            intNum, NUM_INTERRUPTS(arm)-1
        );

    } else if(raise) {

        // interrupt being raised
        raiseNow(arm, exceptNum);

    } else {

        // interrupt being lowered
        lower(arm, exceptNum);
    }
}

//
// Called by the simulator when an external scalar interrupt is raised.
// In this model the net value is either zero (low) or non-zero (high)
//

static VMI_NET_CHANGE_FN(externalScalarInterrupt) {

    armP  arm    = (armP)processor;
    UnsPS intNum = (UnsPS)userData;

    externalInterrupt(arm, intNum, newValue);
}

//
// Called by the simulator when an external vector interrupt is raised.
// In this model the value encodes the interrupt number in bits 31-1,
// and the level in bit0. The model ignores an interrupt number if too big.
//
static VMI_NET_CHANGE_FN(externalVectorInterrupt) {

    armP  arm    = (armP)processor;
    Uns32 intNum = (newValue>>1);
    Bool  raise  = newValue&1;

    externalInterrupt(arm, intNum, raise);
}

//
// Called by the simulator when an external memory prefetch abort is raised
//
static VMI_NET_CHANGE_FN(externalPAbort) {
    armBusFault((armP)processor, 0, MEM_PRIV_X);
}

//
// Called by the simulator when an external memory data abort is raised
//
static VMI_NET_CHANGE_FN(externalDAbort) {
    // TODO : newValue needs to be the (data) address that failed
    armBusFault((armP)processor, newValue, MEM_PRIV_RW);
}

//
// Called by the simulator when an external event is raised
//
static VMI_NET_CHANGE_FN(externalEvent) {
    setEventRegister((armP)processor);
}

//
// Raise an exception
//
void armRaise(armP arm, armExceptNum num) {
    raiseNow(arm, num);
}

//
// Perform SEV instruction actions
//
void armSEV(armP arm) {
    writeNet(arm, arm->eventOut, 1);
}


////////////////////////////////////////////////////////////////////////////////
// NET PORTS
////////////////////////////////////////////////////////////////////////////////

//
// Return number of members of an array
//
#define NUM_MEMBERS(_A) (sizeof(_A)/sizeof((_A)[0]))

//
// Create port specifications
//
void armNewPortSpecs(armP arm) {

    // declare template port structure for fixed ports
    vmiNetPort template[] = {

        // output ports
        {"sysResetReq",vmi_NP_OUTPUT, (void*)0, 0, &arm->sysResetReq },
        {"intISS",     vmi_NP_OUTPUT, (void*)0, 0, &arm->intISS      },
        {"eventOut",   vmi_NP_OUTPUT, (void*)0, 0, &arm->eventOut    },
        {"lockup",     vmi_NP_OUTPUT, (void*)0, 0, &arm->lockup      },

        // input ports
        {"int",        vmi_NP_INPUT,  (void*)2, externalVectorInterrupt },
        {"reset",      vmi_NP_INPUT,  (void*)0, externalReset           },
        {"nmi",        vmi_NP_INPUT,  (void*)0, externalNMI             },
        {"pabort",     vmi_NP_INPUT,  (void*)0, externalPAbort          },
        {"dabort",     vmi_NP_INPUT,  (void*)0, externalDAbort          },
        {"eventIn",    vmi_NP_INPUT,  (void*)0, externalEvent           },
    };

    // calculate number of members
    Uns32 numFixed      = NUM_MEMBERS(template);
    Uns32 numInterrupts = NUM_INTERRUPTS(arm);
    Uns32 i;

    // allocate permanent port structure (including terminator)
    vmiNetPortP result = STYPE_CALLOC_N(vmiNetPort, numFixed+numInterrupts+1);

    // fill fixed members, replacing names with an allocated copy
    for(i=0; i<numFixed; i++) {
        result[i]      = template[i];
        result[i].name = strdup(result[i].name);
    }

    // fill interrupt members
    for(i=0; i<numInterrupts; i++) {

        vmiNetPortP this = result+numFixed+i;

        // assemble temporary name
        char netName[8];
        sprintf(netName, "int%u", i);

        // fill port, replacing name with an allocated copy
        this->name        = strdup(netName);
        this->type        = vmi_NP_INPUT;
        this->userData    = (void*)(UnsPS)i;
        this->netChangeCB = externalScalarInterrupt;
    }

    // save ports on processor
    arm->netPorts = result;
}

//
// Free net port list
//
void armFreePortSpecs(armP arm) {

    Uns32 i;

    for(i=0; arm->netPorts[i].name; i++) {
        STYPE_FREE(arm->netPorts[i].name);
    }

    STYPE_FREE(arm->netPorts);
}

//
// Get the next net port
//
VMI_NET_PORT_SPECS_FN(armGetNetPortSpec) {

    armP        arm = (armP)processor;
    vmiNetPortP this;

    if(!prev) {
        this = arm->netPorts;
    } else {
        this = prev + 1;
    }

    return this->name ? this : 0;
}


////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT SUPPORT
////////////////////////////////////////////////////////////////////////////////

//
// This is the ARM PreserveFPState primitive
//
void armPreserveFPState(armP arm) {

// TODO: accesses need to be done at priv level indicated by FPCCR
//    Bool       isPriv     = SCS_FIELD(arm, FPCCR, USER) == 0;
    Uns32      fpFramePtr = SCS_REG_UNS32(arm, FPCAR);
    memDomainP domain     = arm->FPCARdomain;

    // Sanity checks
    VMI_ASSERT(domain!=0, "FPCAR memory domain is NULL");
    VMI_ASSERT(SCS_FIELD(arm, FPCCR, LSPACT), "FPCCR.LSPACT not active");

    // Clear FPCRdomain since it cannot be used again until LSPACT is set,
    // and it will be reset at that time
    arm->FPCARdomain = 0;

    // TODO: exceptions occurring during this routine need special handling
    // For now just do an assertion if an exception occurs at this point
    armExceptCxt oldContext = arm->exceptionContext;
    arm->exceptionContext = AEC_PreserveFPState;

    // Push the floating point state onto the space allocated on the stack for it
    pushFP(arm, fpFramePtr, domain);

    // Turn off the LSPACT flag
    armUpdateLSPACT(arm, 0);

    // restore previous context
    arm->exceptionContext = oldContext;

}
