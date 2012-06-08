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

// VMI header files
#include "vmi/vmiCxt.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armDecode.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armStructure.h"
#include "armSysRegisters.h"
#include "armUtils.h"
#include "armVM.h"

//
// Return the version of the instruction set implemented by the processor
//
inline static Uns32 getInstructionVersion(armP arm) {
    return ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
}

//
// Return endianness of the processor, public routine
//
VMI_ENDIAN_FN(armGetEndian) {

    armP arm = (armP)processor;

    if(isFetch) {
        return arm->instructionEndian;
    } else if(SCS_FIELD(arm, AIRCR, ENDIANNESS)) {
        return MEM_ENDIAN_BIG;
    } else {
        return MEM_ENDIAN_LITTLE;
    }
}

//
// Set the initial endianness for the model
//
void armSetInitialEndian(armP arm, Bool isBigEndian) {
    SCS_FIELD(arm, AIRCR, ENDIANNESS) = isBigEndian;
}

//
// Return the next instruction address after 'thisPC'.
//
VMI_NEXT_PC_FN(armNextInstruction) {
    return (Uns32)(thisPC + armGetInstructionSize((armP)processor, thisPC));
}

//
// Return the name of a GPR
//
const char *armGetGPRName(armP arm, Uns32 index) {

    static const char *gprNames[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "sl", "fp", "ip", "sp", "lr", "pc"
    };

    return gprNames[index];
}

//
// Return the name of a CPR
//
const char *armGetCPRName(armP arm, Uns32 index) {

    static const char *cprNames[] = {
        "cr0",  "cr1",  "cr2",  "cr3",  "cr4",  "cr5",  "cr6",  "cr7",
        "cr8",  "cr9",  "cr10", "cr11", "cr12", "cr13", "cr14", "cr15"
    };

    return cprNames[index];
}

#define SWAP_REG(_R1, _R2)  {Uns32 _V = _R1; _R1= _R2; _R2 = _V;}
#define SWAP_GPR(_N, _SET)  SWAP_REG(arm->regs[_N], arm->bank.R##_N##_##_SET)

//
// Switch banked registers on switch to the passed mode
//
inline static void switchRegs(armP arm, Bool useSPProcess) {
    if(useSPProcess) {
        SWAP_GPR(13, process);
    }
}

//
// Switch banked registers on switch between the passed modes
//
void armSwitchRegs(armP arm, Bool oldUseSPProcess, Bool newUseSPProcess) {
    switchRegs(arm, oldUseSPProcess);
    switchRegs(arm, newUseSPProcess);
}

//
// Return current processor block mask
//
static armBlockMask getBlockMask(armP arm) {

    armBlockMask blockMask = 0;

    // get blockmask component selecting register bank
    if(USE_SP_PROCESS(arm)) {
        blockMask |= ARM_BM_USE_SP_PROCESS;
    }

    // include component for endianness
    if(SCS_FIELD(arm, AIRCR, ENDIANNESS)) {
        blockMask |= ARM_BM_BIG_ENDIAN;
    }

    // include component for alignment checking
    if(DO_UNALIGNED(arm)) {
        blockMask |= ARM_BM_UNALIGNED;
    }

    // include component for cp10 (and cp11) enable checking
    // NOTE: arm documentation specifies behavior is undefined if cp10 enable
    // differs from cp11 enable, so only cp10 enable is checked here
    Uns32 cp10Enable = SCS_FIELD(arm, CPACR, cp10);
    if(IN_USER_MODE(arm)) {
        if(cp10Enable&2) {blockMask |= ARM_BM_CP10;}
    } else {
        if(cp10Enable&1) {blockMask |= ARM_BM_CP10;}
    }

    // include CONTROL.FPCA bit
    if(CONTROL_FIELD(arm, FPCA)) {
        blockMask |= ARM_BM_FPCA;
    }

    // include FPCCR.ASPEN bit
    if(SCS_FIELD(arm, FPCCR, ASPEN)) {
        blockMask |= ARM_BM_ASPEN;
    }

    // include FPCCR.LSPACT bit
    if(SCS_FIELD(arm, FPCCR, LSPACT)) {
        blockMask |= ARM_BM_LSPACT;
    }

    return blockMask;
}

//
// Update processor block mask
//
void armSetBlockMask(armP arm) {
    vmirtSetBlockMask((vmiProcessorP)arm, getBlockMask(arm));
}

//
// Update processor mode when system state that affects it has been written
//
static void updateMode(armP arm) {

    // update block mask
    armSetBlockMask(arm);

    // switch mode if required
    armSwitchMode(arm);
}

//
// Write CONTROL.SP_PROCESS field
//
void armWriteSPProcess(armP arm, Bool newUseSPProcess) {

    Bool oldUseSPProcess = USE_SP_PROCESS(arm);

    if(oldUseSPProcess!=newUseSPProcess) {

        // update CONTROL.SP_PROCESS
        USE_SP_PROCESS(arm) = newUseSPProcess;

        // switch banked registers
        armSwitchRegs(arm, oldUseSPProcess, newUseSPProcess);

        // update block mask
        armSetBlockMask(arm);
    }
}

//
// Read CPSR register
//
Uns32 armReadCPSR(armP arm) {

    // seed flag fields from flag structure
    PSR_FIELD(arm, Z) = arm->aflags.ZF;
    PSR_FIELD(arm, N) = arm->aflags.NF;
    PSR_FIELD(arm, C) = arm->aflags.CF;
    PSR_FIELD(arm, V) = arm->aflags.VF;
    PSR_FIELD(arm, Q) = arm->oflags.QF;

    // seed if-then state fields
    PSR_FIELD(arm, IT10) = arm->itStateRT & 0x3;
    PSR_FIELD(arm, IT72) = arm->itStateRT >> 2;

    // return derived value
    return arm->sregs.PSR.reg;
}

//
// Read CONTROL register
//
Uns32 armReadCONTROL(armP arm) {

    // return value
    return arm->sregs.CONTROL.reg;
}

//
// Write CPSR register
//
void armWriteCPSR(armP arm, Uns32 value, Uns32 mask) {

    // save current processor mode
    Bool oldHandlerMode = IN_HANDLER_MODE(arm);
    Bool oldCPSRT       = PSR_FIELD(arm, T);

    if (!DSP_PRESENT(arm)) {
        // GE fields reserved when DSP not implemented
        mask &= ~PSR_GE;
    }

    // set new register value (writable bits only)
    arm->sregs.PSR.reg = (value & mask) | (arm->sregs.PSR.reg & ~mask);

    // get new processor mode
    Bool newHandlerMode = IN_HANDLER_MODE(arm);
    Bool newCPSRT       = PSR_FIELD(arm, T);

    // update arithmetic flag fields if required
    if(mask & PSR_FLAGS) {
        arm->aflags.ZF = PSR_FIELD(arm, Z);
        arm->aflags.NF = PSR_FIELD(arm, N);
        arm->aflags.CF = PSR_FIELD(arm, C);
        arm->aflags.VF = PSR_FIELD(arm, V);
        arm->oflags.QF = PSR_FIELD(arm, Q);
    }

    // update if-then state if required
    if(mask & PSR_IT) {
        arm->itStateRT = (PSR_FIELD(arm, IT10) | (PSR_FIELD(arm, IT72<<2)));
    }

    // update processor mode if required
    if((oldHandlerMode!=newHandlerMode) || (oldCPSRT!=newCPSRT)) {
        updateMode(arm);
    }
}

//
// Write PRIMASK register
//
void armWritePRIMASK(armP arm, Uns32 value) {

    value &= PRIMASK_MASK;

    if(arm->sregs.PRIMASK != value) {
        arm->sregs.PRIMASK = value;
        armRefreshExecutionPriority(arm);
    }
}

//
// Write BASEPRI register
//
void armWriteBASEPRI(armP arm, Uns32 value) {

    value &= (BASEPRI_MASK & arm->priorityMask);

    if(arm->sregs.BASEPRI != value) {
        arm->sregs.BASEPRI = value;
        armRefreshExecutionPriority(arm);
    }
}

//
// Write BASEPRI register (using BASEPRI_MAX semantics)
//
void armWriteBASEPRI_MAX(armP arm, Uns32 value) {

    value &= (BASEPRI_MASK & arm->priorityMask);

    if(value && ((value<arm->sregs.BASEPRI) || !arm->sregs.BASEPRI)) {
        arm->sregs.BASEPRI = value;
        armRefreshExecutionPriority(arm);
    }
}

//
// Write FAULTMASK register
//
void armWriteFAULTMASK(armP arm, Uns32 value) {

    value &= FAULTMASK_MASK;

    if((arm->executionPriority>-1) && (value!=arm->sregs.FAULTMASK)) {
        arm->sregs.FAULTMASK = value;
        armRefreshExecutionPriority(arm);
    }
}

//
// update the CONTROL.FPCA bit to the value. If it changes then set the block mask
//
void armUpdateFPCA(armP arm, Bool value) {

    // if CONTROL.FPCA changes set new value and set the block mask
    if (CONTROL_FIELD(arm, FPCA) != value) {

        CONTROL_FIELD(arm, FPCA) = value;
        armSetBlockMask(arm);

    }
}

//
// update the FPCCR.LSPACT bit to the value. If it changes then set the block mask
//
void armUpdateLSPACT(armP arm, Bool value) {

    // if FPCCR.LSPACT changes set new value and set the block mask
    if (SCS_FIELD(arm, FPCCR, LSPACT) != value) {

        SCS_FIELD(arm, FPCCR, LSPACT) = value;
        armSetBlockMask(arm);

    }
}

//
// Write CONTROL register
//
void armWriteCONTROL(armP arm, Uns32 value) {

    armCONTROL u = {reg:value};

    // update CONTROL.FPCA (FPCA is only implemented if FPU is present)
    if (FPU_PRESENT(arm)) {
        armUpdateFPCA(arm, u.fields.FPCA);
    }

    // update CONTROL.threadUnpriv
    Bool oldUserMode = IN_USER_MODE(arm);
    CONTROL_FIELD(arm, threadUnpriv) = u.fields.threadUnpriv;
    Bool newUserMode = IN_USER_MODE(arm);

    // handle any switch to user mode
    if(oldUserMode!=newUserMode) {
        updateMode(arm);
    }

    // CONTROL.useSP_Process may only be modified in Thread mode
    if(!IN_HANDLER_MODE(arm)) {
        armWriteSPProcess(arm, u.fields.useSP_Process);
    }
}

//
// Write SP register
//
void armWriteSP(armP arm, Uns32 value) {
    arm->regs[ARM_REG_SP] = value & ~3;
}

//
// Switch processor mode if required
//
void armSwitchMode(armP arm) {

    armMode mode;

    if(!IN_THUMB_MODE(arm)) {

        // invalid mode
        mode = ARM_MODE_ARM;
        SCS_FIELD(arm, CFSR, INVSTATE) = 1;

    } else {

        mode = 0;

        // user/kernel mode mask
        if(IN_USER_MODE(arm)) {
            mode |= ARM_MODE_U;
        }

        // add MPU enable mask
        if(!MPU_ENABLED(arm)) {
            // no action
        } else if(arm->executionPriority>=0) {
            mode |= ARM_MODE_MPU;
        } else if(SCS_FIELD(arm, MPU_CONTROL, HFNMIENA)) {
            mode |= ARM_MODE_MPU;
        }
    }

    // switch mode if required
    if(mode != arm->mode) {
        vmirtSetMode((vmiProcessorP)arm, mode);
        arm->mode = mode;
    }
}


////////////////////////////////////////////////////////////////////////////////
// PROCESSOR RUN STATE TRANSITION HANDLING
////////////////////////////////////////////////////////////////////////////////

//
// If this memory access callback is triggered, abort any active load linked
//
static VMI_MEM_WATCH_FN(abortEA) {
    armAbortExclusiveAccess((armP)userData);
}

//
// Install or remove the exclusive access monitor callback
//
static void updateExclusiveAccessCallback(armP arm, Bool install) {

    memDomainP domain  = vmirtGetProcessorDataDomain((vmiProcessorP)arm);
    Uns32      simLow  = arm->exclusiveTag;
    Uns32      simHigh = simLow + ~arm->exclusiveTagMask;

    // install or remove a watchpoint on the current exclusive access address
    if(install) {
        vmirtAddWriteCallback(domain, simLow, simHigh, abortEA, arm);
    } else {
        vmirtRemoveWriteCallback(domain, simLow, simHigh, abortEA, arm);
    }
}

//
// Abort any active exclusive access
//
void armAbortExclusiveAccess(armP arm) {

    if(arm->exclusiveTag != ARM_NO_TAG) {

        // remove callback on exclusive access monitor region
        updateExclusiveAccessCallback(arm, False);

        // clear exclusive tag (AFTER updateExclusiveAccessCallback)
        arm->exclusiveTag = ARM_NO_TAG;
    }
}

//
// This is called on simulator context switch (when this processor is either
// about to start or about to stop simulation)
//
VMI_IASSWITCH_FN(armContextSwitchCB) {

    armP arm = (armP)processor;

    // establish a watchpoint on a pending exclusive address to detect if that
    // address is written elsewhere.
    if(arm->exclusiveTag != ARM_NO_TAG) {
        updateExclusiveAccessCallback(arm, state==RS_SUSPEND);
    }

    // if a processor is about to be run, make state consistent with any changes
    // that may have occurred while the processor was suspended
    if(state==RS_RUN) {
        armSetBlockMask(arm);
    }
}

