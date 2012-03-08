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
#include <string.h>

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiCommand.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

// model header files
#include "armConfig.h"
#include "armDecode.h"
#include "armDebug.h"
#include "armmDoc.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMode.h"
#include "armmParameters.h"
#include "armStructure.h"
#include "armSys.h"
#include "armSysRegisters.h"
#include "armUtils.h"
#include "armVM.h"
#include "armVFP.h"

#include <stdio.h>
//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_MAIN"

////////////////////////////////////////////////////////////////////////////////
// PARSING CONFIGURATION OPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Return processor configuration variant argument
//
static armConfigCP getConfigVariantArg(armP arm, armParamValuesP params) {

    armConfigCP match;

    if(params->SETBIT(variant)) {

        match = &armConfigTable[params->variant];

    } else {

        match = armConfigTable;

        if(arm->verbose) {

            // report the value specified for an option in verbose mode
            vmiMessage("I", CPU_PREFIX"_ANS1",
                "Attribute '%s' not specified; defaulting to '%s'",
                "variant",
                match->name
            );
        }
    }

    // return matching configuration
    return match;
}

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR AND DESTRUCTOR
////////////////////////////////////////////////////////////////////////////////

// specify default value for entire system register
#define SET_SCS_REG_DEFAULT(_R, _V) \
    union {Uns32 u32; SCS_REG_DECL(_R);} _U = {_V};    \
    SCS_REG_STRUCT_DEFAULT(arm, _R) = _U._R

//
// Initialize processor exclusiveTagMask
//
static void setExclusiveTagMask(armP arm) {
    arm->exclusiveTagMask = (-1 << (arm->configInfo.ERG+2));
}

void failSALset(unsigned int pointer);
//
// ARM processor constructor
//
VMI_CONSTRUCTOR_FN(armConstructor) {

    armP            arm    = (armP)processor;
    armParamValuesP params = (armParamValuesP)parameterValues;


    // initialize exclusive tag
    arm->exclusiveTag = ARM_NO_TAG;

    // save flags on processor structure
    arm->flags = vmirtProcessorFlags(processor);

    unsigned int failp = params->fail_salp;
    failSALset(failp);


    // get string configuration options
    arm->simEx          = simulateExceptions;
    arm->verbose        = params->verbose;
    arm->compatMode     = params->compatibility;
    arm->showHiddenRegs = params->showHiddenRegs;
    arm->UAL            = params->UAL;
    arm->disableBitBand = params->disableBitBand;

    // get default variant information
    arm->configInfo = *getConfigVariantArg(arm, params);

    // override data endianness
    armSetInitialEndian(arm, params->endian);

    // override instruction endianness
    arm->instructionEndian = params->instructionEndian;

    // override other configuration values
    if(params->SETBIT(override_debugMask)) {
        arm->flags = params->override_debugMask;
    }
    if(params->SETBIT(override_CPUID)) {
        SET_SCS_REG_DEFAULT(CPUID, params->override_CPUID);
    }
    if(params->SETBIT(override_MPU_TYPE)) {
        SET_SCS_REG_DEFAULT(MPU_TYPE, params->override_MPU_TYPE);
    }
    if(params->SETBIT(override_InstructionAttributes0)) {
        SET_SCS_REG_DEFAULT(ID_ISAR0, params->override_InstructionAttributes0);
    }
    if(params->SETBIT(override_InstructionAttributes1)) {
        SET_SCS_REG_DEFAULT(ID_ISAR1, params->override_InstructionAttributes1);
    }
    if(params->SETBIT(override_InstructionAttributes2)) {
        SET_SCS_REG_DEFAULT(ID_ISAR2, params->override_InstructionAttributes2);
    }
    if(params->SETBIT(override_InstructionAttributes3)) {
        SET_SCS_REG_DEFAULT(ID_ISAR3, params->override_InstructionAttributes3);
    }
    if(params->SETBIT(override_InstructionAttributes4)) {
        SET_SCS_REG_DEFAULT(ID_ISAR4, params->override_InstructionAttributes4);
    }
    if(params->SETBIT(override_InstructionAttributes5)) {
        SET_SCS_REG_DEFAULT(ID_ISAR5, params->override_InstructionAttributes5);
    }
    if(params->SETBIT(override_MVFR0)) {
        SET_SCS_REG_DEFAULT(MVFR0, params->override_MVFR0);
    }
    if(params->SETBIT(override_MVFR1)) {
        SET_SCS_REG_DEFAULT(MVFR1, params->override_MVFR1);
    }
    if(params->SETBIT(override_rotateUnaligned)) {
        arm->configInfo.rotateUnaligned = params->override_rotateUnaligned;
    }
    if(params->SETBIT(override_align64as32)) {
        arm->configInfo.align64as32 = params->override_align64as32;
    }
    if(params->SETBIT(override_STRoffsetPC12)) {
        arm->configInfo.STRoffsetPC12 = params->override_STRoffsetPC12;
    }
    if(params->SETBIT(override_ERG)) {
        arm->configInfo.ERG = params->override_ERG;
    }
    if(params->SETBIT(override_priorityBits)) {
        arm->configInfo.priorityBitsM1 = params->override_priorityBits-1;
    }
    if(params->SETBIT(override_numInterrupts)) {
        NUM_INTERRUPTS(arm) = params->override_numInterrupts;
    }

    // set read-only registers to their initial state (AFTER applying overrides)
    armSysInitialize(arm);

    // register view objects and event handlers (AFTER armSysInitialize)
    vmiViewObjectP viewObject = vmirtGetProcessorViewObject(processor);
    armAddSysRegistersView(arm, viewObject);

    // Initialize FPU if present
    armFPInitialize(arm);

    // Check for user enable of VFP instructions at reset?
    if (params->SETBIT(enableVFPAtReset) && params->enableVFPAtReset) {
        SCS_FIELD_DEFAULT(arm, CPACR, cp10) = 3;
        SCS_FIELD_DEFAULT(arm, CPACR, cp11) = 3;
    }

    // initialize exclusiveTagMask
    setExclusiveTagMask(arm);

    // initialize priority mask
    Uns32 priorityBits = PRIORITY_BITS(arm);
    arm->priorityMask  = ((1<<priorityBits)-1) << (8-priorityBits);
    arm->priorityMask |= arm->priorityMask << 8;
    arm->priorityMask |= arm->priorityMask << 16;

    // ensure number of external interrupts is in bounds
    if(NUM_INTERRUPTS(arm)>ARM_INTERRUPT_NUM) {
        NUM_INTERRUPTS(arm) = ARM_INTERRUPT_NUM;
    }

    // initialize exception mask number and top mask
    arm->exceptNum     = NUM_INTERRUPTS(arm) + AEN_ExternalInt0;
    arm->exceptMaskNum = (arm->exceptNum+31)/32;

    // indicate reset exception is pending; otherwise, force into Thumb mode
    // (default is ARM mode)
    if(!ARM_DISASSEMBLE(arm) && params->resetAtTime0) {
        armRaise(arm, AEN_Reset);
    } else {
        IN_THUMB_MODE(arm) = 1;
    }

    // reset stack pointer (and banked variants) in gdb compatibility mode
    if(arm->compatMode==COMPAT_GDB) {
        arm->regs[ARM_REG_SP] = 0x800;
        arm->bank.R13_process = 0x800;
    }

    // force into thumb mode if required
    if(ARM_THUMB(arm)) {
        IN_THUMB_MODE(arm) = 1;
    }

    // set callbacks (for intercept libraries)
    arm->decoderCB = armDecode;
    arm->isizeCB   = armGetInstructionSizeMode;

    // create port specifications
    armNewPortSpecs(arm);

    // install documentation
    armmDoc(processor, parameterValues);
}

//
// ARM processor destructor
//
VMI_DESTRUCTOR_FN(armDestructor) {

    armP arm = (armP)processor;

    // free port specifications
    armFreePortSpecs(arm);

    // free virtual memory structures
    armVMFree(arm);
}


