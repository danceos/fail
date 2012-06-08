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

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiParameters.h"
#include "vmi/vmiMessage.h"

#include "armFunctions.h"
#include "armConfig.h"
#include "armmParameters.h"
#include "armStructure.h"
#include "armVariant.h"

static vmiEnumParameter compatTable[] = {
    {"ISA",       COMPAT_ISA},
    {"gdb",       COMPAT_GDB},
    {"nopBKPT",   COMPAT_CODE_SOURCERY},
    { 0, 0 }
};

//
// Table of parameter specs
//
static vmiParameter formals[] = {

    VMI_ENUM_PARAM_SPEC(  armParamValues, variant,          NULL, "Select variant (either a generic ISA or a specific model)"), // filled in by getVariantList()
    VMI_ENDIAN_PARAM_SPEC(armParamValues, endian, "Model endian"),
    VMI_ENUM_PARAM_SPEC(  armParamValues, compatibility,    compatTable, "Specify compatibility mode (default: ISA)"),

    VMI_BOOL_PARAM_SPEC(  armParamValues, verbose,          1, "Specify verbosity of output" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, showHiddenRegs,   0, "Show hidden registers during register tracing" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, UAL,              1, "Disassemble using UAL syntax" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, enableVFPAtReset, 0, "Enable vector floating point (VFP) instructions at reset. (Enables cp10/11 in CPACR)" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, disableBitBand,   0, "Disable bit banding"),
    VMI_BOOL_PARAM_SPEC(  armParamValues, resetAtTime0,     1, "Reset the model at time=0 (NB: default=1)" ),

    VMI_UNS32_PARAM_SPEC(  armParamValues, override_CPUID                , 0, 0, VMI_MAXU32, "Override system CPUID register"),
    VMI_UNS32_PARAM_SPEC(  armParamValues, override_MPU_TYPE             , 0, 0, VMI_MAXU32, "Override system MPU_TYPE register"),

    VMI_ENDIAN_PARAM_SPEC(armParamValues, instructionEndian, "The ARMv7-M is defined to always fetch instructions in little endian order; this attribute allows the profile-defined instruction endianness to be overridden if required"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_debugMask             , 0, 0, VMI_MAXU32, "Specifies debug mask, enabling debug output for model components"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_MPUType               , 0, 0, VMI_MAXU32, "Override MPUType register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes0, 0, 0, VMI_MAXU32, "Override InstructionAttributes0 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes1, 0, 0, VMI_MAXU32, "Override InstructionAttributes1 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes2, 0, 0, VMI_MAXU32, "Override InstructionAttributes2 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes3, 0, 0, VMI_MAXU32, "Override InstructionAttributes3 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes4, 0, 0, VMI_MAXU32, "Override InstructionAttributes4 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes5, 0, 0, VMI_MAXU32, "Override InstructionAttributes5 register"),

    VMI_UNS32_PARAM_SPEC( armParamValues, override_MVFR0,                  0, 0, VMI_MAXU32, "Override MVFR0 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_MVFR1,                  0, 0, VMI_MAXU32, "Override MVFR1 register"),

    VMI_UNS32_PARAM_SPEC( armParamValues, override_rotateUnaligned       , 0, 0, 1, "Specifies that data from unaligned loads by LDR, LDRT or SWP should be rotated (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_align64as32           , 0, 0, 1, "Specifies that 64:bit loads and stores are aligned to 32:bit boundaries (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_STRoffsetPC12         , 0, 0, 1, "Specifies that STR/STR of PC should do so with 12:byte offset from the current instruction (if 1), otherwise an 8:byte offset is used"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_ERG                   , 0, 0, 1024, "Specifies exclusive reservation granule"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_priorityBits          , 0, 0, 8   , "Specifies number of priority bits in BASEPRI etc (1-8, default is 3)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_numInterrupts         , 0, 0, 4096, "Specifies number of external interrupt lines (0-495, default is 16)"),
    
VMI_UNS32_PARAM_SPEC( armParamValues, fail_salp         , 0, 0, VMI_MAXU32, "Address of Fail* SAL Object"),

    VMI_END_PARAM
};

static Uns32 countVariants(void) {
    armConfigCP cfg = armConfigTable;
    Uns32         i   = 0;
    while(cfg->name) {
        cfg++;
        i++;
    }
    return i;
}

//
// First time through, malloc and fill the variant list from the config table
//
static vmiEnumParameterP getVariantList() {
    static vmiEnumParameterP list = NULL;
    if (!list) {
        Uns32 v = 1+ countVariants();
        list = STYPE_CALLOC_N(vmiEnumParameter, v);
        vmiEnumParameterP prm;
        armConfigCP    cfg;
        Uns32 i;
        for (i = 0, cfg = armConfigTable, prm = list;
             cfg->name;
             i++, cfg++, prm++) {
            prm->name = cfg->name;
            prm->value = i;
        }
    }
    return list;
}

//
// First time through, fill the formals table
//
static vmiParameterP getFormals(void) {
    static Bool first = True;
    if(first) {
        first = False;
        formals[0].u.enumParam.legalValues = getVariantList();
    }
    return formals;
}


//
// Function to iterate the parameter specs
//
VMI_PROC_PARAM_SPECS_FN(armGetParamSpec) {
    if(!prev) {
        return getFormals();
    } else {
        prev++;
        if (prev->name)
            return prev;
        else
            return 0;
    }
}

//
// Get the size of the parameter values table
//
VMI_PROC_PARAM_TABLE_SIZE_FN(armParamValueSize) {
    return sizeof(armParamValues);
}


