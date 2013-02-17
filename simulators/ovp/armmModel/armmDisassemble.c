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

// standard includes
#include <stdio.h>
#include <string.h>

// VMI header files
#include "vmi/vmiCxt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armDecode.h"
#include "armDisassemble.h"
#include "armDisassembleFormats.h"
#include "armFunctions.h"
#include "armStructure.h"
#include "armUtils.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_DISASS"

//
// This defines the minimum string width to use for the opcode
//
#define OP_WIDTH 7

//
// Condition names
//
static const char *condNames[] = {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", ""  , ""
};

//
// Condition names in an IT block
//
static const char *condNamesIT[] = {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", "al", "al"
};

//
// Limitation names
//
static const char *limNames[] = {
    "#0", "#1", "oshst", "osh", "#4",  "#5",  "nshst", "nsh",
    "#8", "#9", "ishst", "ish", "#12", "#13", "st",    "sy"
};

//
// Increment/decrement names
//
static const char *incDecNames[] = {
    [ARM_ID_NA]  = "",
    [ARM_ID_IA]  = "ia",
    [ARM_ID_IB]  = "ib",
    [ARM_ID_DA]  = "da",
    [ARM_ID_DB]  = "db",
    [ARM_ID_IAI] = "",
    [ARM_ID_IBI] = "",
    [ARM_ID_DBI] = "",
    [ARM_ID_DAI] = ""
};

//
// Flag action names
//
static const char *flagActionNames[] = {
    [ARM_FACT_NA]  = "",
    [ARM_FACT_BAD] = "",
    [ARM_FACT_IE]  = "ie",
    [ARM_FACT_ID]  = "id"
};

//
// Size names
//
static const char *sizeNames[] = {"", "b", "h", "", "", "", "", "", "d"};

//
// Shift names
//
static const char *shiftNames[] = {"???", "lsl", "lsr", "asr", "ror", "rrx"};

//
// Names for armSDFPType values
// Note most not used in single precision VFP-Only M profile
//
static const char *ftypeNames[] = {
    [ARM_SDFPT_NA]  = "",       // no floating point type
    [ARM_SDFPT_8]   = ".8",
    [ARM_SDFPT_16]  = ".16",
    [ARM_SDFPT_32]  = ".32",
    [ARM_SDFPT_64]  = ".64",
    [ARM_SDFPT_F16] = ".f16",
    [ARM_SDFPT_F32] = ".f32",
    [ARM_SDFPT_F64] = ".f64",
    [ARM_SDFPT_I8]  = ".i8",
    [ARM_SDFPT_I16] = ".i16",
    [ARM_SDFPT_I32] = ".i32",
    [ARM_SDFPT_I64] = ".i64",
    [ARM_SDFPT_P8]  = ".p8",
    [ARM_SDFPT_S8]  = ".s8",
    [ARM_SDFPT_S16] = ".s16",
    [ARM_SDFPT_S32] = ".s32",
    [ARM_SDFPT_S64] = ".s64",
    [ARM_SDFPT_U8]  = ".u8",
    [ARM_SDFPT_U16] = ".u16",
    [ARM_SDFPT_U32] = ".u32",
    [ARM_SDFPT_U64] = ".u64",
};

//
// Append the character to to the result
//
static void putChar(char **result, char ch) {

    // get the tail pointer
    char *tail = *result;

    // do the append
    *tail++ = ch;

    // add null terminator
    *tail = 0;

    // update the tail pointer
    *result = tail;
}

//
// Append the string to to the result
//
static void putString(char **result, const char *string) {

    // get the tail pointer
    char *tail = *result;
    char  ch;

    // do the append
    while((ch=*string++)) {
        *tail++ = ch;
    }

    // add null terminator
    *tail = 0;

    // update the tail pointer
    *result = tail;
}

//
// Append comma separator, unless previous character is space or comma (allows
// for omitted optional arguments)
//
static void putComma(char **result) {

    char *tail = *result;

    switch(tail[-1]) {
        case ',':
        case ' ':
            break;
        default:
            putChar(result, ',');
            break;
    }
}

//
// Append right bracket, overwriting any previous comma (allows for omitted
// optional arguments)
//
static void putRBracket(char **result) {

    char *tail = *result;

    switch(tail[-1]) {
        case ',':
            tail[-1] = ']';
            break;
        default:
            putChar(result, ']');
            break;
    }
}

//
// Emit hexadecimal argument using format 0x%x
//
static void put0x(char **result, Uns32 value) {
    char tmp[32];
    sprintf(tmp, "0x%x", value);
    putString(result, tmp);
}

//
// Emit hexadecimal Uns64 argument using format 0x%x
//
static void put0xll(char **result, Uns64 value) {
    char tmp[32];
    sprintf(tmp, "0x"FMT_640Nx, value);
    putString(result, tmp);
}

//
// Emit unsigned argument
//
static void putU(char **result, Uns32 value) {
    char tmp[32];
    sprintf(tmp, "%u", value);
    putString(result, tmp);
}

//
// Emit floating point argument
//
static void putF(char **result, Flt64 value) {
    char tmp[32];
    sprintf(tmp, "%g", value);
    putString(result, tmp);
}

//
// Emit signed argument
//
static void putD(char **result, Uns32 value) {
    char tmp[32];
    sprintf(tmp, "%d", value);
    putString(result, tmp);
}

//
// Emit unsigned argument preceded with hash
//
static void putHashU(char **result, Uns32 value) {
    putChar(result, '#');
    putU(result, value);
}

//
// Emit constant 0 floating point argument preceded with hash
//
static void putHashF0(char **result) {
    putString(result, "#0.0");
}

//
// Emit signed argument preceded with hash, unless optional and zero
//
static void putHashD(char **result, Uns32 value, Bool opt) {
    if(!opt || value) {
        putChar(result, '#');
        putD(result, value);
    }
}

//
// Emit minus sign if required
//
static void putMinus(char **result, Bool uBit) {
    if(!uBit) {
        putChar(result, '-');
    }
}

//
// Emit instruction format if required
//
static void putInstruction(armP arm, armInstructionInfoP info, char **result) {

    char tmp[32];

    // emit basic opcode string
    sprintf(tmp, info->bytes==2 ? "%04x     " : "%08x ", info->instruction);
    putString(result, tmp);
}

//
// Emit GPR argument
//
static void putGPR(armP arm, char **result, Uns32 r) {
    putString(result, armGetGPRName(arm, r));
}

//
// Emit GPR argument, or flags if r15 and UAL mode
//
static void putGPROrFlagsIfR15(armP arm, char **result, Uns32 r) {
    if(!arm->UAL || (r!=15)) {
        putString(result, armGetGPRName(arm, r));
    } else {
        putString(result, "APSR_nzcv");
    }
}

//
// Emit singleword register argument
//
static void putSR(armP arm, char **result, Uns32 r) {
    putChar(result, 's');
    putU(result, r);
}

//
// Emit consecutive pair of singleword registers argument
//
static void putSSR(armP arm, char **result, Uns32 r) {
    putChar(result, 's');
    putU(result, r);
    putString(result, ",s");
    putU(result, r+1);
}

//
// Emit doubleword register argument
//
static void putDR(armP arm, char **result, Uns32 r) {
    putChar(result, 'd');
    putU(result, r);
}

//
// Emit scalar argument
//
static void putScalar(armP arm, char **result, Uns32 d, Uns32 index) {
    putChar(result, 'd');
    putU(result, d);
    putChar(result, '[');
    putU(result, index);
    putChar(result, ']');
}

//
// Emit CPR argument
//
static void putCPR(armP arm, char **result, Uns32 r) {
    putString(result, armGetCPRName(arm, r));
}

//
// Emit fpscr argument (M-profile only supports "fpscr" on vmrs/vmsr)
//
static void putFPSCR(armP arm, char **result) {
    putString(result, "fpscr");
}

//
// Emit jump target
//
static void putTarget(char **result, Uns32 value) {
    char tmp[32];
    sprintf(tmp, "%x", value);
    putString(result, tmp);
}

//
// Emit shiftop argument
//
static void putShift(char **result, armShiftOp so) {
    putString(result, shiftNames[so]);
}

//
// Emit constant shift if non-zero
//
static void putShiftC(char **result, armShiftOp so, Uns32 value) {
    if(value) {
        putShift(result, so);
        putChar(result, ' ');
        putHashD(result, value, False);
    }
}

//
// Emit constant shift equivalent to size if non-zero
//
static void putSizeShift(char **result, Uns32 sz) {

    Uns32 shift = 0;

    while(sz>1) {
        sz >>= 1;
        shift++;
    }

    putShiftC(result, ARM_SO_LSL, shift);
}

//
// Emit writeback argument
//
static void putWB(char **result, Bool pi, Bool wb) {
    if(wb && !pi) {
        putChar(result, '!');
    }
}

//
// Emit register list argument
//
static void putRList(armP arm, char **result, Uns32 rList) {

    Uns32 r;
    Uns32 mask;
    Bool  first = True;

    putChar(result, '{');

    for(r=0, mask=0x0001; r<16; r++, mask<<=1) {
        if(rList&mask) {
            if(!first) {
                putChar(result, ',');
            }
            putGPR(arm, result, r);
            first = False;
        }
    }

    putChar(result, '}');
}

//
// Return string for _<bits> of MSR instruction defined
// by the instruction's mask field
//
static char *msrBits(armPSRBits bits) {

    char *s;

    switch (bits) {
        case ARM_PSRBITS_GE:    s = "_g";      break;
        // Support for the GE bits was added by Arm in Q3 2010 so now
        // this should be s = "_nzcvq", but toolchains do not seem to support it yet
        // so continue using the (deprecated) empty string for now
        case ARM_PSRBITS_FLAGS: s = NULL;  break;
        case ARM_PSRBITS_ALL:   s = "_nzcvqg"; break;
        default:                s = NULL;      break;
    }

    return s;
}

//
// Emit special register argument
//
static void putSpecialReg(char **result, armSysRegId SYSm, armPSRBits bits) {

    const char *regName;
    const char *bitsName = NULL;

    switch(SYSm) {
        case ASRID_APSR:        regName = "APSR";  bitsName = msrBits(bits); break;
        case ASRID_IAPSR:       regName = "IAPSR"; bitsName = msrBits(bits); break;
        case ASRID_EAPSR:       regName = "EAPSR"; bitsName = msrBits(bits); break;
        case ASRID_XPSR:        regName = "XPSR";  bitsName = msrBits(bits); break;
        case ASRID_IPSR:        regName = "IPSR";        break;
        case ASRID_EPSR:        regName = "EPSR";        break;
        case ASRID_IEPSR:       regName = "IEPSR";       break;
        case ASRID_MSP:         regName = "MSP";         break;
        case ASRID_PSP:         regName = "PSP";         break;
        case ASRID_PRIMASK:     regName = "PRIMASK";     break;
        case ASRID_BASEPRI:     regName = "BASEPRI";     break;
        case ASRID_BASEPRI_MAX: regName = "BASEPRI_MAX"; break;
        case ASRID_FAULTMASK:   regName = "FAULTMASK";   break;
        case ASRID_CONTROL:     regName = "CONTROL";     break;
        default:                regName = "RSVD";        break;
    }

    putString(result, regName);
    if (bitsName != NULL) putString(result, bitsName);
}

//
// Emit flags argument (CPS instruction)
//
static void putFlags(char **result, armFlagAffect faff) {
    if(faff & ARM_FAFF_A) {putChar(result, 'a');}
    if(faff & ARM_FAFF_I) {putChar(result, 'i');}
    if(faff & ARM_FAFF_F) {putChar(result, 'f');}
}

//
// Emit optional mode argument (CPS instruction)
//
static void putOptMode(char **result, Bool ma, Uns32 c) {
    if(ma) {putHashU(result, c);}
}

//
// Emit limitation (DMB, DSB and ISB instructions)
//
static void putLim(char **result, Uns32 lim) {
    putString(result, limNames[lim]);
}

//
// Emit true/else condition list for IT block
//
static void putIT(char **result, Uns32 it) {

    Bool negate = (it&0x10) ? False : True;

    while(it & 0x7) {
        Bool select = (it & 0x8) ? True : False;
        putChar(result, (select ^ negate) ? 't' : 'e');
        it <<= 1;
    }
}

//
// Emit a single register in a register list
// May be a scalar which may or may not have a valid index
//
static void putRListReg(char **result, Uns32 rn, char regType, Bool scalar, Int32 index) {

    putChar(result, regType);
    putU(result, rn);
    if (scalar) {
        putChar(result, '[');
        if (index >= 0) putU(result, (Uns32) index);
        putChar(result, ']');
    }
}

//
// Emit SIMD/VFP type register list argument
//
static void putSdfpRList(
    char **result,
    Uns32 nRegs,
    Uns32 incr,
    Uns32 rn,
    char regType,
    Bool scalar,
    Int32 index,
    Bool forceList
) {

    putChar(result, '{');
    putRListReg(result, rn, regType, scalar, index);

    if (incr > 1 || forceList) {

        // when there is an incr (or when forced) use a list of regs rx, ry,...rz
        int i;

        // If forcing a list make sure incr is valid
        if (incr == 0) incr = 1;

        for (i = 1; i < nRegs; i++) {
            putChar(result, ',');
            putRListReg(result, rn+(incr*i), regType, scalar, index);
        }

    } else {

        // When there is no incr, disassembler uses rx-rz notation for multiple registers
        if (nRegs > 1) {
            putChar(result, '-');
            putRListReg(result, rn+nRegs-1, regType, scalar, index);
        }

    }

    putChar(result, '}');
}

//
// Emit a SIMD/VFP Modified Immediate constant value
//
static void putSdfpMI(char **result, armSdfpMItype mi, armSDFPType ftype)
{
    putChar(result, '#');

    switch (ftype) {

        case ARM_SDFPT_I8:
            putD(result, mi.u8.b0);
            break;
        case ARM_SDFPT_I16:
            putD(result, mi.u16.h0);
            break;
        case ARM_SDFPT_I32:
            putD(result, mi.u32.w0);
            break;
        case ARM_SDFPT_I64:
            put0xll(result, mi.u64);
            break;
        case ARM_SDFPT_F32:
            putF(result, mi.f32);
            break;
        case ARM_SDFPT_F64:
            putF(result, mi.f64);
            break;
        default:
            // Only above values for dt1 are supported for instructions with modified immediate constants
            VMI_ABORT("Unsupported ftype %d withSIMD/VFP modified immediate constant", ftype);

    }
}

//
// Generate instruction disassembly using the format string
//
static void disassembleFormat(
    armP                arm,
    armInstructionInfoP info,
    char              **result,
    const char         *format
) {
    const char *opcode = info->opcode;

    // in UAL mode, some instances of MOV instruction get reclassed as shift
    // pseudo-ops
    if(!arm->UAL) {
        // no action
    } else if((info->type==ARM_IT_MOV_RM_SHFT_IMM) && info->c) {
        opcode = shiftNames[info->so];
        format = FMT_R1_R2_SIMM;
    } else if(info->type==ARM_IT_MOV_RM_SHFT_RS) {
        opcode = shiftNames[info->so];
        format = FMT_R1_R2_R3;
    } else if(info->type==ARM_IT_MOV_RM_RRX) {
        opcode = shiftNames[info->so];
        format = FMT_R1_R2;
    }

    // generate instruction pattern
    putInstruction(arm, info, result);

    // get offset at opcode start
    const char *opcodeStart = *result;

    // generate opcode text
    putString(result, opcode);

    if(arm->UAL) {
        // disassemble using UAL syntax
        if(info->xs) putChar(result, 's');
        if(info->ea) putString(result, "ex");
        putString(result, sizeNames[info->sz]);
        if(info->f==ARM_SF_V) putChar(result, 's');
        putString(result, incDecNames[info->incDec]);
        putString(result, condNames[info->cond]);
        putString(result, flagActionNames[info->fact]);
        if(info->tl) putChar(result, 't');
        if(info->ll) putChar(result, 'l');
        if(info->it) putIT(result, info->it);
        putString(result, ftypeNames[info->dt1]);
        putString(result, ftypeNames[info->dt2]);
    } else {
        // disassemble using pre-UAL syntax
        putString(result, condNames[info->cond]);
        putString(result, incDecNames[info->incDec]);
        putString(result, flagActionNames[info->fact]);
        if(info->xs) putChar(result, 's');
        if(info->ea) putString(result, "ex");
        putString(result, sizeNames[info->sz]);
        if(info->tl) putChar(result, 't');
        if(info->ll) putChar(result, 'l');
        if(info->f==ARM_SF_V) putChar(result, 's');
        if(info->it) putIT(result, info->it);
        putString(result, ftypeNames[info->dt1]);
        putString(result, ftypeNames[info->dt2]);
    }

    if(*format) {

        // calculate length of opcode text
        const char *opcodeEnd = *result;
        Uns32       len       = opcodeEnd-opcodeStart;
        char        ch;

        // pad to minimum width
        for(; len<OP_WIDTH; len++) {
            putChar(result, ' ');
        }

        // emit space before arguments
        putChar(result, ' ');

        // this defines whether the next argument is optional
        Bool nextOpt = False;

        // generate arguments in appropriate format
        while((ch=*format++)) {

            // is this argument optional?
            Bool opt = nextOpt;

            // assume subsequent argument is mandatory
            nextOpt = False;

            switch(ch) {
                case EMIT_R1:
                    putGPR(arm, result, info->r1);
                    break;
                case EMIT_R2:
                    putGPR(arm, result, info->r2);
                    break;
                case EMIT_R3:
                    putGPR(arm, result, info->r3);
                    break;
                case EMIT_R4:
                    putGPR(arm, result, info->r4);
                    break;
                case EMIT_S1:
                    putSR(arm, result, info->r1);
                    break;
                case EMIT_S2:
                    putSR(arm, result, info->r2);
                    break;
                case EMIT_S3:
                    putSR(arm, result, info->r3);
                    break;
                case EMIT_D1:
                    putDR(arm, result, info->r1);
                    break;
                case EMIT_D2:
                    putDR(arm, result, info->r2);
                    break;
                case EMIT_D3:
                    putDR(arm, result, info->r3);
                    break;
                case EMIT_Z1:
                    putScalar(arm, result, info->r1, info->index);
                    break;
                case EMIT_Z2:
                    putScalar(arm, result, info->r2, info->index);
                    break;
                case EMIT_SS1:
                    putSSR(arm, result, info->r1);
                    break;
                case EMIT_SS3:
                    putSSR(arm, result, info->r3);
                    break;
                case EMIT_CU:
                    putHashU(result, info->c);
                    break;
                case EMIT_CS:
                    putHashD(result, info->c, opt);
                    break;
                case EMIT_CX:
                    put0x(result, info->c);
                    break;
                case EMIT_T:
                    putTarget(result, info->t);
                    break;
                case EMIT_SHIFT:
                    putShift(result, info->so);
                    break;
                case EMIT_SHIFT_C:
                    putShiftC(result, info->so, info->c);
                    break;
                case EMIT_CPNUM:
                    putU(result, info->cpNum);
                    break;
                case EMIT_COP1:
                    putU(result, info->cpOp1);
                    break;
                case EMIT_COP2:
                    putU(result, info->cpOp2);
                    break;
                case EMIT_CR1:
                    putCPR(arm, result, info->r1);
                    break;
                case EMIT_CR2:
                    putCPR(arm, result, info->r2);
                    break;
                case EMIT_CR3:
                    putCPR(arm, result, info->r3);
                    break;
                case EMIT_OPT:
                    putU(result, info->c);
                    break;
                case EMIT_WB:
                    putWB(result, info->pi, info->wb);
                    break;
                case EMIT_RLIST:
                    putRList(arm, result, info->rList);
                    break;
                case EMIT_U:
                    putMinus(result, info->u);
                    break;
                case EMIT_SR:
                    putSpecialReg(result, info->c, info->psrbits);
                    break;
                case EMIT_FLAGS:
                    putFlags(result, info->faff);
                    break;
                case EMIT_OPT_MODE:
                    putOptMode(result, info->ma, info->c);
                    break;
                case EMIT_LIM:
                    putLim(result, info->c);
                    break;
                case EMIT_WIDTH:
                    putHashD(result, info->w, False);
                    break;
                case EMIT_ITC:
                    putString(result, condNamesIT[info->it>>4]);
                    break;
                case EMIT_SZSHIFT:
                    putSizeShift(result, info->sz);
                    break;
                case EMIT_R1F:
                    putGPROrFlagsIfR15(arm, result, info->r1);
                    break;
                case EMIT_FPSCR:
                    putFPSCR(arm, result);
                    break;
                case EMIT_C0F:
                    putHashF0(result);
                    break;
                case EMIT_SIMD_RL:
                    putSdfpRList(result, info->nregs, 0, info->r2, 'd', False, 0, False);
                    break;
                case EMIT_SDFP_MI:
                    putSdfpMI(result, info->sdfpMI, info->dt1);
                    break;
                case EMIT_VFP_RL:
                    putSdfpRList(result, info->nregs, 0, info->r2, 's', False, 0, False);
                    break;
                case '1':
                    if(!info->pi) format++;
                    break;
                case '2':
                    if(info->pi) format++;
                    break;
                case ',':
                    putComma(result);
                    break;
                case ']':
                    putRBracket(result);
                    break;
                case '*':
                    nextOpt = True;
                    break;
                default:
                    putChar(result, ch);
                    break;
            }
        }
    }

    // strip trailing whitespace and commas
    char *tail = (*result)-1;
    while((*tail == ' ') || (*tail == ',')) {
        *tail-- = 0;
    }
}

//
// ARM disassembler, decoded instruction interface
//
const char *armDisassembleInfo(armP arm, armInstructionInfoP info) {

    // static buffer to hold disassembly result
    static char result[256];
    const char *format = info->format;
    char       *tail   = result;

    // disassemble using the format for the type
    if(format) {
        disassembleFormat(arm, info, &tail, format);
    } else if(info->bytes==2) {
        sprintf(result, "0x%04x", info->instruction);
    } else {
        sprintf(result, "0x%08x", info->instruction);
    }

    // return the result
    return result;
}

//
// ARM disassembler, VMI interface
//
VMI_DISASSEMBLE_FN(armDisassemble) {

    // static buffer to hold disassembly result
    armP               arm = (armP)processor;
    armInstructionInfo info;

    // get instruction and instruction type
    arm->itStateMT = arm->itStateRT;
    armDecode(arm, thisPC, &info);

    // return disassembled instruction
    return armDisassembleInfo(arm, &info);
}


