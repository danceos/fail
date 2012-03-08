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

// Standard header files
#include "string.h"
#include "stdio.h"

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiDbg.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiOSLib.h"
#include "vmi/vmiRt.h"

// model header files
#include "armRegisters.h"
#include "armStructure.h"
#include "armUtils.h"
#include "armSys.h"
#include "armVFP.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_DEBUG"


////////////////////////////////////////////////////////////////////////////////
// REGISTER GROUPS
////////////////////////////////////////////////////////////////////////////////

//
// This describes the register groups in the processor
//
typedef enum armRegGroupIdE {
    ARM_RG_CORE,        // Core group
    ARM_RG_CONTROL,     // control register group
    ARM_RG_SYSTEM,      // memory-mapped system register group
    ARM_RG_FPR,         // VFP register group
    ARM_RG_LAST         // KEEP LAST: for sizing
} armRegGroupId;

//
// This provides information about each group
//
static const vmiRegGroup groups[ARM_RG_LAST+1] = {
    [ARM_RG_CORE]    = {name: "Core"   },
    [ARM_RG_CONTROL] = {name: "Control"},
    [ARM_RG_SYSTEM]  = {name: "System" },
    [ARM_RG_FPR]     = {name: "VFP"    },
};

//
// Macro to specify a the group for a register
//
#define ARM_GROUP(_G) &groups[ARM_RG_##_G]


////////////////////////////////////////////////////////////////////////////////
// MACROS FOR REGISTER ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// Macro to specify a register that can be accessed using raw read/write
// callbacks
//
#define ARM_RAW_REG(_R, _G) \
    VMI_REG_RAW_READ_CB,        \
    VMI_REG_RAW_WRITE_CB,       \
    (void *)ARM_CPU_OFFSET(_R), \
    _G

//
// Core registers
//
#define ARM_CORE_REG(_I) ARM_RAW_REG(regs[_I], ARM_GROUP(CORE))

//
// Macro to specify the PC accessible for read/write
//
#define ARM_PC_RW readPC, writePC, 0, ARM_GROUP(CORE)

//
// Macro to specify access for the stack pointer register
//
#define ARM_SP_REG(_R) \
    VMI_REG_RAW_READ_CB,                \
    writeSP,                            \
    (void *)ARM_CPU_OFFSET(regs[_R]),   \
    ARM_GROUP(CORE)

//
// Macro to specify the control registers
//
#define ARM_CONTROL_RW(_ID) read##_ID, write##_ID, 0, ARM_GROUP(CONTROL)

//
// Some registers are hidden in gdb, but we allow access to them
//
#define ARM_GDB_HIDDEN_INDEX  99
#define IS_GDB_HIDDEN_REG(_I) ((_I)>=ARM_GDB_HIDDEN_INDEX)

//
// system registers do not have access
//
#define ARM_CP_INDEX        0x1000
#define IS_SCS_REG(_I)      ((_I)>=ARM_CP_INDEX)

//
// VFP registers - must check for existence in the variant
//
#define ARM_FPSCR_INDEX       24
#define ARM_VFP0_INDEX       700
#define ARM_VFP15_INDEX      715
#define IS_VFP_REG(_I)       ((_I)==ARM_FPSCR_INDEX || IS_VFP_DATA_REG(_I))
#define IS_VFP_DATA_REG(_I)  (((_I)>=ARM_VFP0_INDEX) && ((_I)<=ARM_VFP15_INDEX))
#define ARM_VFP_INDEX(_I)    ((_I)-ARM_VFP0_INDEX)
#define ARM_VFP_DATA_REG(_I)  ARM_RAW_REG(vregs.d[_I], ARM_GROUP(FPR))

////////////////////////////////////////////////////////////////////////////////
// DEBUGGER REGISTER INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Return current vmiRegInfoCP structure for the passed banked vmiRegInfoCP
//
vmiRegInfoCP getCurrentInfo(vmiRegInfoCP reg);

//
// Return system register id for vmiRegInfoCP
//
static armSCSRegId getSysId(vmiRegInfoCP reg) {
    if(!IS_SCS_REG(reg->gdbIndex)) {
        return SCS_ID(INVALID);
    } else {
        return (armSCSRegId)reg->userData;
    }
}

//
// Write callback for sp
//
static VMI_REG_WRITE_FN(writeSP) {
    armWriteSP((armP)processor, *(Uns32*)buffer);
    return True;
}

//
// Read callback for pc
//
static VMI_REG_READ_FN(readPC) {
    *(Uns32*)buffer = ((Uns32)vmirtGetPC(processor));
    return True;
}

//
// Write callback for pc
//
static VMI_REG_WRITE_FN(writePC) {
    Uns32 simPC = *(Uns32*)buffer;
    vmirtSetPC(processor, simPC & ~1);
    return True;
}

//
// Read callback for PSR
//
static VMI_REG_READ_FN(readPSR) {
    *(Uns32*)buffer = armReadCPSR((armP)processor);
    return True;
}

//
// Write callback for PSR
//
static VMI_REG_WRITE_FN(writePSR) {
    armP arm = (armP)processor;
    armWriteCPSR(arm, *(Uns32*)buffer, PSR_ALL);
    return True;
}

//
// Read callback for FPSCR
//
static VMI_REG_READ_FN(readFPSCR) {
    *(Uns32*)buffer = armReadFPSCR((armP)processor);
    return True;
}

//
// Write callback for FPSCR
//
static VMI_REG_WRITE_FN(writeFPSCR) {
    armP arm = (armP)processor;
    armWriteFPSCR(arm, *(Uns32*)buffer, FPSCR_MASK);
    return True;
}

//
// Read callback for CONTROL
//
static VMI_REG_READ_FN(readControl) {
    *(Uns32*)buffer = armReadCONTROL((armP)processor);
    return True;
}

//
// Write callback for CONTROL
//
static VMI_REG_WRITE_FN(writeControl) {
    armP arm = (armP)processor;
    armWriteCONTROL(arm, *(Uns32*)buffer);
    return True;
}

//
// Read callback for PRIMASK
//
static VMI_REG_READ_FN(readPRIMASK) {
    armP arm = (armP)processor;
    *(Uns32*)buffer = arm->sregs.PRIMASK;
    return True;
}

//
// Write callback for PRIMASK
//
static VMI_REG_WRITE_FN(writePRIMASK) {
    armP arm = (armP)processor;
    armWritePRIMASK(arm, *(Uns32*)buffer);
    return True;
}


//
// Read callback for FAULTMASK
//
static VMI_REG_READ_FN(readFAULTMASK) {
    armP arm = (armP)processor;
    *(Uns32*)buffer = arm->sregs.FAULTMASK;
    return True;
}
//
// Write callback for FAULTMASK
//
static VMI_REG_WRITE_FN(writeFAULTMASK) {
    armP arm = (armP)processor;
    armWriteFAULTMASK(arm, *(Uns32*)buffer);
    return True;
}

//
// Read callback for FAULTMASK
//
static VMI_REG_READ_FN(readBASEPRI) {
    armP arm = (armP)processor;
    *(Uns32*)buffer = arm->sregs.BASEPRI;
    return True;
}

//
// Write callback for BASEPRI
//
static VMI_REG_WRITE_FN(writeBASEPRI) {
    armP arm = (armP)processor;
    armWriteBASEPRI(arm, *(Uns32*)buffer);
    return True;
}

//
// Read callback for banked register
//
static VMI_REG_READ_FN(readBank) {

    armP arm              = (armP)processor;
    Bool trueUseSPProcess = USE_SP_PROCESS(arm);
    Bool tempUseSPProcess = True;

    armSwitchRegs(arm, trueUseSPProcess, tempUseSPProcess);
    Bool result = vmiosRegRead(processor, getCurrentInfo(reg), buffer);
    armSwitchRegs(arm, tempUseSPProcess, trueUseSPProcess);

    return result;
}

//
// Write callback for banked register
//
static VMI_REG_WRITE_FN(writeBank) {

    armP arm              = (armP)processor;
    Bool trueUseSPProcess = USE_SP_PROCESS(arm);
    Bool tempUseSPProcess = True;

    armSwitchRegs(arm, trueUseSPProcess, tempUseSPProcess);
    Bool result = vmiosRegWrite(processor, getCurrentInfo(reg), buffer);
    armSwitchRegs(arm, tempUseSPProcess, trueUseSPProcess);

    return result;
}

//
// Read callback for system register
//
static VMI_REG_READ_FN(readSCS) {

    armP        arm = (armP)processor;
    armSCSRegId id  = getSysId(reg);

    if(!armReadSysRegPriv(id, arm, (Uns32*)buffer)) {

        return False;

    } else if(id!=SCS_ID(CPUID)) {

        return True;

    } else {

        union {Uns32 u32; SCS_REG_DECL(CPUID);} u = {*(Uns32*)buffer};

        if(!u.u32) {
            armArchitecture variant = arm->configInfo.arch;
             u.CPUID.ARCHITECTURE = ARM_VARIANT_ARCH(variant);
            *(Uns32*)buffer = u.u32;
        }

        return True;
    }
}

//
// Write callback for system register
//
static VMI_REG_WRITE_FN(writeSCS) {
    return armWriteSysRegPriv(getSysId(reg), (armP)processor, *(Uns32*)buffer);
}

//
// Static const array holding information about the registers in the cpu,
// used for debugger interaction
//
static const vmiRegInfo basicRegisters[] = {

    // current mode registers (visible in gdb)
    {"r0",            0, vmi_REG_NONE, 32, False, ARM_CORE_REG(0)        },
    {"r1",            1, vmi_REG_NONE, 32, False, ARM_CORE_REG(1)        },
    {"r2",            2, vmi_REG_NONE, 32, False, ARM_CORE_REG(2)        },
    {"r3",            3, vmi_REG_NONE, 32, False, ARM_CORE_REG(3)        },
    {"r4",            4, vmi_REG_NONE, 32, False, ARM_CORE_REG(4)        },
    {"r5",            5, vmi_REG_NONE, 32, False, ARM_CORE_REG(5)        },
    {"r6",            6, vmi_REG_NONE, 32, False, ARM_CORE_REG(6)        },
    {"r7",            7, vmi_REG_NONE, 32, False, ARM_CORE_REG(7)        },
    {"r8",            8, vmi_REG_NONE, 32, False, ARM_CORE_REG(8)        },
    {"r9",            9, vmi_REG_NONE, 32, False, ARM_CORE_REG(9)        },
    {"r10",          10, vmi_REG_NONE, 32, False, ARM_CORE_REG(10)       },
    {"r11",          11, vmi_REG_FP,   32, False, ARM_CORE_REG(11)       },
    {"r12",          12, vmi_REG_NONE, 32, False, ARM_CORE_REG(12)       },
    {"sp",           13, vmi_REG_SP,   32, False, ARM_SP_REG  (13)       },
    {"lr",           14, vmi_REG_NONE, 32, False, ARM_CORE_REG(14)       },
    {"pc",           15, vmi_REG_PC,   32, False, ARM_PC_RW              },
    {"fps",          24, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(FPSCR)  },
    {"cpsr",         25, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(PSR)    },

    // control and SP_process (not visible in gdb)
    {"control",     100, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(Control)},
    {"primask",     101, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(PRIMASK)},
    {"faultmask",   102, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(FAULTMASK)},
    {"basepri",     103, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(BASEPRI)},
    {"sp_process",  113, vmi_REG_SP,   32, False, ARM_CONTROL_RW(Bank)   },

    // VFP registers - double word view only (not visible in gdb)
    {"d0",          700, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(0) },
    {"d1",          701, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(1) },
    {"d2",          702, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(2) },
    {"d3",          703, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(3) },
    {"d4",          704, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(4) },
    {"d5",          705, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(5) },
    {"d6",          706, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(6) },
    {"d7",          707, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(7) },
    {"d8",          708, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(8) },
    {"d9",          709, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(9) },
    {"d10",         710, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(10)},
    {"d11",         711, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(11)},
    {"d12",         712, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(12)},
    {"d13",         713, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(13)},
    {"d14",         714, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(14)},
    {"d15",         715, vmi_REG_NONE, 64, False, ARM_VFP_DATA_REG(15)},

    {0},
};

//
// Return ARM register descriptions
//
static vmiRegInfoCP getRegisters(void) {

    static vmiRegInfo *allRegisters;

    if(!allRegisters) {

        armSysRegDesc desc;
        Uns32         basicNum = 0;
        Uns32         sysNum   = 0;
        Uns32         i;

        // count basic registers
        while(basicRegisters[basicNum].name) {
            basicNum++;
        }

        // count system registers
        desc.name = 0;
        while(armGetSysRegisterDesc(&desc)) {
            sysNum++;
        }

        // allocate full register information, including terminating NULL entry
        // TODO: This is never freed! Need to free description too
        allRegisters = STYPE_CALLOC_N(vmiRegInfo, basicNum+sysNum+1);

        // fill basic entries
        for(i=0; i<basicNum; i++) {
            allRegisters[i] = basicRegisters[i];
        }

        // fill system entries
        for(desc.name=0, i=0; armGetSysRegisterDesc(&desc); i++) {

            vmiRegInfo *reg = &allRegisters[basicNum+i];

            // fill basic fields
            reg->name     = desc.name;
            reg->usage    = vmi_REG_NONE;
            reg->bits     = 32;
            reg->readonly = False;
            reg->readCB   = readSCS;
            reg->writeCB  = writeSCS;
            reg->userData = (void *)desc.id;
            reg->group    = ARM_GROUP(SYSTEM);

            // synthesize a description
            char descStr[64];
            snprintf(descStr, 64, "Addr: 0x%08x  Priv:%s User:%s", desc.address, desc.privRW, desc.userRW);
            reg->description = strdup(descStr);

            // use address as gdb pseudo-index
            reg->gdbIndex = desc.address;
        }
    }

    // return register set
    return allRegisters;
}

//
// Return current vmiRegInfoCP structure for the passed banked vmiRegInfoCP
//
vmiRegInfoCP getCurrentInfo(vmiRegInfoCP reg) {

    Uns32        index = reg->gdbIndex % 100;
    vmiRegInfoCP info;

    for(info=getRegisters(); info->name; info++) {
        if(info->gdbIndex == index) {
            return info;
        }
    }

    return 0;
}

//
// Is the passed register supported on this processor?
//
static Bool isRegSupported(armP arm, vmiRegInfoCP reg, Bool gdbFrame) {

    if(gdbFrame && IS_GDB_HIDDEN_REG(reg->gdbIndex)) {

        // if this is a GDB frame request then registers that should be hidden
        // from GDB should be ignored
        return False;

    } else if(IS_VFP_REG(reg->gdbIndex)) {

        // VFP registers are supported if in variant
        return FPU_PRESENT(arm);

    } else if(IS_SCS_REG(reg->gdbIndex)) {

        // system registers are supported only if the associated unit is present
        return armGetSysRegSupported(getSysId(reg), arm);

    }

    // other registers are always supported
    return True;

}

//
// Return next supported register on this processor
//
static vmiRegInfoCP getNextRegister(armP arm, vmiRegInfoCP reg, Bool gdbFrame) {

    do {
        if(!reg) {
            reg = getRegisters();
        } else if((reg+1)->name) {
            reg = reg+1;
        } else {
            reg = 0;
        }
    } while(reg && !isRegSupported(arm, reg, gdbFrame));

    return reg;
}

//
// Is the passed register group supported on this processor?
//
static Bool isGroupSupported(armP arm, vmiRegGroupCP group) {

    vmiRegInfoCP info = 0;

    while((info=getNextRegister(arm, info, False))) {
        if(info->group == group) {
            return True;
        }
    }

    return False;
}

//
// Return next supported group on this processor
//
static vmiRegGroupCP getNextGroup(armP arm, vmiRegGroupCP group) {

    do {
        if(!group) {
            group = groups;
        } else if((group+1)->name) {
            group = group+1;
        } else {
            group = 0;
        }
    } while(group && !isGroupSupported(arm, group));

    return group;
}

//
// Register structure iterator
//
VMI_REG_INFO_FN(armRegInfo) {
    return getNextRegister((armP)processor, prev, gdbFrame);
}

//
// Register group iterator
//
VMI_REG_GROUP_FN(armRegGroup) {
    return getNextGroup((armP)processor, prev);
}


////////////////////////////////////////////////////////////////////////////////
// REGISTER DUMP INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Dump processor registers
//
VMI_DEBUG_FN(armDumpRegisters) {

    armP         arm            = (armP)processor;
    Bool         showHiddenRegs = arm->showHiddenRegs;
    Uns32        nameWidth      = showHiddenRegs ? 10 : 7;
    vmiRegInfoCP info           = 0;

    while((info=getNextRegister(arm, info, False))) {

        if(IS_SCS_REG(info->gdbIndex)) {

            // ignore system registers

        } else if(IS_VFP_DATA_REG(info->gdbIndex)) {

            // print VFP regs if enabled
            if(ARM_DUMP_SDFP_REG(arm)) {

                const char *fmt = "        %-*s 0x" FMT_640Nx "\n";
                Uns64 value;

                // read and print register value
                vmiosRegRead(processor, info, &value);
                vmiPrintf(fmt, nameWidth, info->name, value);
            }

        } else if(!IS_GDB_HIDDEN_REG(info->gdbIndex) || showHiddenRegs) {

            const char *fmt;
            Uns32 value;

            // read register value
            vmiosRegRead(processor, info, &value);

            // select approriate format string
            if((info->usage==vmi_REG_SP) || (info->usage==vmi_REG_PC)) {
                fmt = "        %-*s 0x%-8x 0x%x\n";
            } else {
                fmt = "        %-*s 0x%-8x %u\n";
            }

            // print register using selected format
            vmiPrintf(fmt, nameWidth, info->name, value, value);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// SCS PROGRAMMER'S VIEW
////////////////////////////////////////////////////////////////////////////////

//
// Add programmer's view of all system registers
//
void armAddSysRegistersView(armP arm, vmiViewObjectP processorObject) {

    vmiRegInfoCP info = 0;

    // create coprocessor 15 child object
    vmiViewObjectP baseObject = vmirtAddViewObject(processorObject, "SCS", 0);

    while((info=getNextRegister(arm, info, False))) {

        if(IS_SCS_REG(info->gdbIndex)) {

            const char *name = info->name;
            armSCSRegId id   = getSysId(info);

            armAddSysRegisterView(id, arm, baseObject, name);
        }
    }
}

