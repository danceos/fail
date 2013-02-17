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
#include "vmi/vmiMessage.h"

// model header files
#include "armConfig.h"
#include "armDecode.h"
#include "armEmit.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMessage.h"
#include "armMorph.h"
#include "armMorphFunctions.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armUtils.h"
#include "armVariant.h"
#include "armVM.h"
#include "armVFP.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_MORPH"


////////////////////////////////////////////////////////////////////////////////
// EXCEPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Generic exception callback
//
#define EXCEPTION_FN(_NAME) void _NAME(armP arm, Uns32 thisPC)
typedef EXCEPTION_FN((*exceptionFn));

//
// Emit call to exception function
//
static void emitExceptionCall(armMorphStateP state, exceptionFn cb) {

    Uns32 bits = ARM_GPR_BITS;

    // emit processor argument
    armEmitArgProcessor(state);
    armEmitArgSimPC(state, bits);
    armEmitCall(state, (vmiCallFn)cb);

    // terminate the current block
    armEmitEndBlock();
}


////////////////////////////////////////////////////////////////////////////////
// UNIMPLEMENTED/UNDEFINED INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Called for an unimplemented instruction
//
static void unimplemented(armP arm, Uns32 thisPC) {

    vmiMessage("F", CPU_PREFIX"_UII",
        SRCREF_FMT "unimplemented instruction",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Default morpher callback for unimplemented instructions
//
static void emitUnimplemented(armMorphStateP state) {
    armEmitArgProcessor(state);
    armEmitArgSimPC(state, ARM_GPR_BITS);
    armEmitCall(state, (vmiCallFn)unimplemented);
    armEmitExit();
}

//
// Called for UsageFault exception
//
static void usageFault(armP arm, Uns32 thisPC) {

    vmiMessage("E", CPU_PREFIX"_UDI",
        SRCREF_FMT "usage fault - enable simulated exceptions "
        "to continue simulation if this behavior is desired",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Emit code for a UsageFault exception
//
static void emitUsageFault(armMorphStateP state, Uns32 reason) {

    Uns32 bits = ARM_GPR_BITS;

    // update UFSR (CFSR)
    armEmitBinopRC(state, bits, vmi_OR, ARM_SCS_REG(SCS_ID(CFSR)), reason, 0);

    // handle UsageFault
    if(state->arm->simEx) {
        emitExceptionCall(state, armUsageFault);
    } else {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, bits);
        armEmitCall(state, (vmiCallFn)usageFault);
        armEmitExit();
    }
}

//
// Called for an instruction that isn't supported on this variant
//
static void notVariantMessage(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_NSV",
        SRCREF_FMT "not supported on this variant",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Morpher callback for instructions that are not valid on this processor
// variant
//
static ARM_MORPH_FN(emitNotVariant) {

    // report the offending instruction in verbose mode
    if(state->arm->verbose) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn)notVariantMessage);
    }

    // take UsageFault exception
    emitUsageFault(state, EXC_UNDEF_UNDEFINSTR);
}

//
// Called for an undefined coprocessor instruction
//
static void undefinedCPMessage(armP arm, Uns32 thisPC) {

    vmiMessage("a", CPU_PREFIX"_UCA",
        SRCREF_FMT "unsupported coprocessor access",
        SRCREF_ARGS(arm, thisPC)
    );
}


////////////////////////////////////////////////////////////////////////////////
// FLAGS
////////////////////////////////////////////////////////////////////////////////

const static vmiFlags flagsCIn = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

const static vmiFlags flagsCOut = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

// Macro accessors for flags
#define FLAGS_CIN  &flagsCIn
#define FLAGS_COUT &flagsCOut

//
// Return condition flags associated with an instruction, or null if no flags
// are applicable
//
static vmiFlagsCP getFlagsOrNull(armMorphStateP state) {
    return state->info.f ? state->attrs->flagsRW : 0;
}

//
// Return condition flags associated with an instruction, or FLAGS_CIN if no
// flags are applicable
//
static vmiFlagsCP getFlagsOrCIn(armMorphStateP state, vmiReg rd) {
    if(!state->info.f) {
        return state->attrs->flagsR;
    } else if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return state->attrs->flagsRW;
    } else {
        return state->attrs->flagsR;
    }
}

//
// If the instruction sets flags from the shifter output, return FLAGS_COUT
//
static vmiFlagsCP getShifterCOut(armMorphStateP state, vmiReg rd) {
    if(!state->info.f) {
        return FLAGS_CIN;
    } else if(!state->attrs->shiftCOut) {
        return FLAGS_CIN;
    } else if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return FLAGS_COUT;
    } else {
        return FLAGS_CIN;
    }
}

//
// Do the flags specify that the shifter sets the carry out?
//
inline static Bool shifterSetsCOut(vmiFlagsCP flags) {
    return (flags==FLAGS_COUT);
}

////////////////////////////////////////////////////////////////////////////////
// REGISTER ACCESS UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Register access macros
//
#define GET_RD(_S, _R) getRD(_S, _S->info._R)
#define GET_RS(_S, _R) getRS(_S, _S->info._R)

// VFP register access macros
#define GET_VFP_REG(_S, _R, _SZ)   getVFPReg((_S), (_S)->info._R, (_SZ))
#define GET_VFP_SREG(_S, _R)   getVFPReg((_S), (_S)->info._R, 4)
#define GET_VFP_DREG(_S, _R)   getVFPReg((_S), (_S)->info._R, 8)
#define GET_VFP_SCALAR(_S, _R, _IDX) getVFPReg((_S), (2*((_S)->info._R))+(_IDX), 4)

//
// Return vmiReg for GPR with the passed index
// NOTE: PC is a temporary, others are true registers
//
static vmiReg getGPR(armMorphStateP state, Uns32 index) {
    return index==ARM_REG_PC ? ARM_PC : ARM_REG(index);
}

//
// Return vmiReg for target GPR with the passed index
//
static vmiReg getRD(armMorphStateP state, Uns32 index) {
    return getGPR(state, index);
}

//
// Return vmiReg for source GPR with the passed index
//
static vmiReg getRS(armMorphStateP state, Uns32 index) {

    vmiReg r = getGPR(state, index);

    // ensure PC is created on demand if required
    if(VMI_REG_EQUAL(r, ARM_PC)) {
        armEmitGetPC(state);
    }

    return r;
}

//
// Return vmiReg object for low half of 32-bit register
//
inline static vmiReg getR32Lo(vmiReg reg32) {
    return reg32;
}

//
// Return vmiReg object for high half of 32-bit register
//
inline static vmiReg getR32Hi(vmiReg reg32) {
    return VMI_REG_DELTA(reg32, sizeof(Uns16));
}

//
// Return vmiReg object for low half of 64-bit register
//
inline static vmiReg getR64Lo(vmiReg reg64) {
    return reg64;
}

//
// Return vmiReg object for high half of 64-bit register
//
inline static vmiReg getR64Hi(vmiReg reg64) {
    return VMI_REG_DELTA(reg64, sizeof(Uns32));
}

//
// Return vmiReg for VFP register, addressed as a Single or Double precision
//
static vmiReg getVFPReg(armMorphStateP state, Uns32 regNum, Uns32 ebytes) {

    VMI_ASSERT(ebytes==2 || ebytes==4 || ebytes == 8, "Invalid VFP ebytes %d", ebytes);

    armP   arm          = state->arm;
    vmiReg reg          = VMI_NOREG;
    Uns32  vfpRegisters = SCS_FIELD(arm, MVFR0, A_SIMD_Registers);
    Uns32  numRegs      = (vfpRegisters == 1) ? ARM_VFP16_REG_NUM : ARM_VFP32_REG_NUM;


    if(!vfpRegisters) {

        //  VFP Registers are not present
        emitUsageFault(state, EXC_UNDEF_UNDEFINSTR);

    } else if((regNum*ebytes) >= (numRegs*ARM_VFP_REG_BYTES)) {

        // Register number is out of range of the implemented number of registers
        emitUsageFault(state, EXC_UNDEF_UNDEFINSTR);

    } else {

        // get the indicated register
        switch (ebytes) {
            case 1:  reg = ARM_BREG(regNum); break;
            case 2:  reg = ARM_HREG(regNum); break;
            case 4:  reg = ARM_WREG(regNum); break;
            case 8:  reg = ARM_DREG(regNum); break;
            default: VMI_ABORT("Invalid ebytes %d", ebytes);
        }
    }

    return reg;
}


////////////////////////////////////////////////////////////////////////////////
// INSTRUCTION SKIPPING UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Save the skip label on the info structure
//
inline static void setSkipLabel(armMorphStateP state, vmiLabelP label) {
    state->skipLabel = label;
}

//
// Return the skip label on the info structure
//
inline static vmiLabelP getSkipLabel(armMorphStateP state) {
    return state->skipLabel;
}

//
// Insert a label
//
inline static void emitLabel(vmiLabelP label) {

    if(label) {
        armEmitInsertLabel(label);
    }
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return the version of the instruction set implemented by the processor
//
inline static Uns32 getInstructionVersion(armP arm) {
    return ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
}

//
// Return the constant value adjusted so that when added to the PC the result
// is always word aligned
//
inline static Uns32 alignConstWithPC(armMorphStateP state, Uns32 c) {
    return c - (state->info.thisPC & 2);
}


////////////////////////////////////////////////////////////////////////////////
// TEMPORARIES
////////////////////////////////////////////////////////////////////////////////

//
// Get the current active temporary
//
inline static vmiReg getTemp(armMorphStateP state) {
    VMI_ASSERT(state->tempIdx<ARM_TEMP_NUM, "%s: no more temporaries", FUNC_NAME);
    return ARM_TEMP(state->tempIdx);
}

//
// Allocate a new 32-bit temporary
//
inline static vmiReg newTemp32(armMorphStateP state) {
    vmiReg temp = getTemp(state);
    state->tempIdx++;
    return temp;
}

//
// Free a 32-bit temporary
//
inline static void freeTemp32(armMorphStateP state) {
    VMI_ASSERT(state->tempIdx, "%s: temporary overflow", FUNC_NAME);
    state->tempIdx--;
}

//
// Allocate a new 64-bit temporary
//
inline static vmiReg newTemp64(armMorphStateP state) {
    vmiReg temp = newTemp32(state); newTemp32(state);
    return temp;
}

//
// Free a 64-bit temporary
//
inline static void freeTemp64(armMorphStateP state) {
    freeTemp32(state); freeTemp32(state);
}

//
// Allocate a new 128-bit temporary
//
inline static vmiReg newTemp128(armMorphStateP state) {
    vmiReg temp = newTemp64(state); newTemp64(state);
    return temp;
}

//
// Free a 128-bit temporary
//
inline static void freeTemp128(armMorphStateP state) {
    freeTemp64(state); freeTemp64(state);
}

////////////////////////////////////////////////////////////////////////////////
// TEMPORARY FLAGS
////////////////////////////////////////////////////////////////////////////////

//
// Return vmiFlags generating sign and overflow in temporary
//
static vmiFlags getSFOFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define overflow and sign flags
    vmiReg of = tf;
    vmiReg sf = VMI_REG_DELTA(of, 1);

    // define vmiFlags structure using the overflow and sign flags
    flags.f[vmi_SF] = sf;
    flags.f[vmi_OF] = of;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating overflow in temporary
//
static vmiFlags getOFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define vmiFlags structure using the overflow flag
    flags.f[vmi_OF] = tf;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating zero flag in temporary
//
static vmiFlags getZFFlags(vmiReg tf) {

    vmiFlags flags  = VMI_NOFLAGS;

    // define vmiFlags structure using the carry flag
    flags.f[vmi_ZF] = tf;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating and using carry in temporary
//
static vmiFlags getCFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define vmiFlags structure using the carry flag
    flags.cin       = tf;
    flags.f[vmi_CF] = tf;

    // return vmiFlags structure
    return flags;
}

////////////////////////////////////////////////////////////////////////////////
// CONDITIONAL INSTRUCTION EXECUTION
////////////////////////////////////////////////////////////////////////////////

//
// For conditions, this enumeration describes the circumstances under which the
// condition is satisfied
//
typedef enum armCondOpE {
    ACO_ALWAYS = 0,     // condition always True
    ACO_FALSE  = 1,     // condition satisfied if flag unset
    ACO_TRUE   = 2,     // condition satisfied if flag set
} armCondOp;

//
// For conditions, this structure describes a flag and a value for a match
//
typedef struct armCondS {
    vmiReg    flag;
    armCondOp op;
} armCond, *armCondP;

//
// Emit code to prepare a conditional operation and return an armCond structure
// giving the offset of a flag to compare against
//
static armCond emitPrepareCondition(armMorphStateP state) {

    const static armCond condTable[ARM_C_LAST] = {
        [ARM_C_EQ] = {ARM_ZF_CONST, ACO_TRUE  },    // ZF==1
        [ARM_C_NE] = {ARM_ZF_CONST, ACO_FALSE },    // ZF==0
        [ARM_C_CS] = {ARM_CF_CONST, ACO_TRUE  },    // CF==1
        [ARM_C_CC] = {ARM_CF_CONST, ACO_FALSE },    // CF==0
        [ARM_C_MI] = {ARM_NF_CONST, ACO_TRUE  },    // NF==1
        [ARM_C_PL] = {ARM_NF_CONST, ACO_FALSE },    // NF==0
        [ARM_C_VS] = {ARM_VF_CONST, ACO_TRUE  },    // VF==1
        [ARM_C_VC] = {ARM_VF_CONST, ACO_FALSE },    // VF==0
        [ARM_C_HI] = {ARM_HI_CONST, ACO_TRUE  },    // (CF==1) && (ZF==0)
        [ARM_C_LS] = {ARM_HI_CONST, ACO_FALSE },    // (CF==0) || (ZF==1)
        [ARM_C_GE] = {ARM_LT_CONST, ACO_FALSE },    // NF==VF
        [ARM_C_LT] = {ARM_LT_CONST, ACO_TRUE  },    // NF!=VF
        [ARM_C_GT] = {ARM_LE_CONST, ACO_FALSE },    // (ZF==0) && (NF==VF)
        [ARM_C_LE] = {ARM_LE_CONST, ACO_TRUE  },    // (ZF==1) || (NF!=VF)
        [ARM_C_AL] = {VMI_NOREG,    ACO_ALWAYS},    // always
        [ARM_C_NV] = {VMI_NOREG,    ACO_ALWAYS}     // always (historically never)
   };

    // get the table entry corresponding to the instruction condition
    armCond cond = condTable[state->info.cond];
    vmiReg  tf   = cond.flag;
    armP    arm  = state->arm;

    switch(state->info.cond) {

        case ARM_C_AL:
        case ARM_C_NV:
             // unconditional execution
             break;

        case ARM_C_EQ:
        case ARM_C_NE:
        case ARM_C_CS:
        case ARM_C_CC:
        case ARM_C_MI:
        case ARM_C_PL:
        case ARM_C_VS:
        case ARM_C_VC:
            // basic flags, always valid
            break;

        case ARM_C_GT:      // (ZF==0) && (NF==VF)
        case ARM_C_LE:      // (ZF==1) || (NF!=VF)
            // derived LE flag
            if(!arm->validLE) {
                arm->validLE = True;
                armEmitBinopRRR(state, 8, vmi_XOR, tf, ARM_NF, ARM_VF, 0);
                armEmitBinopRR(state, 8, vmi_OR, tf, ARM_ZF, 0);
            }
            break;

        case ARM_C_GE:      // NF==VF
        case ARM_C_LT:      // NF!=VF
            // derived LT flag
            if(!arm->validLT) {
                arm->validLT = True;
                armEmitBinopRRR(state, 8, vmi_XOR, tf, ARM_NF, ARM_VF, 0);
            }
            break;

        case ARM_C_HI:      // (CF==1) && (ZF==0)
        case ARM_C_LS:      // (CF==0) || (ZF==1)
            // derived HI flag
            if(!arm->validHI) {
                arm->validHI = True;
                armEmitBinopRRR(state, 8, vmi_ANDN, tf, ARM_CF, ARM_ZF, 0);
            }
            break;

        default:
            // not reached
            VMI_ABORT("%s: unimplemented condition", FUNC_NAME);
    }

    // return the condition description
    return cond;
}

//
// Create code to jump to a new label if the instruction is conditional
//
static vmiLabelP emitStartSkip(armMorphStateP state) {

    armCond   cond   = emitPrepareCondition(state);
    vmiLabelP doSkip = 0;

    if(cond.op!=ACO_ALWAYS) {
        doSkip = armEmitNewLabel();
        armEmitCondJumpLabel(cond.flag, cond.op==ACO_FALSE, doSkip);
    }

    return doSkip;
}

//
// Force all derived flag values to be regenerated
//
static void resetDerivedFlags(armMorphStateP state) {

    armP arm = state->arm;

    arm->validHI = False;
    arm->validLT = False;
    arm->validLE = False;
}


////////////////////////////////////////////////////////////////////////////////
// ARGUMENT GENERATION
////////////////////////////////////////////////////////////////////////////////

//
// Map from armShiftOp to vmiBinop
//
static vmiBinop mapShiftOp(armShiftOp so) {
    switch(so) {
        case ARM_SO_LSL: return vmi_SHL;
        case ARM_SO_LSR: return vmi_SHR;
        case ARM_SO_ASR: return vmi_SAR;
        case ARM_SO_ROR: return vmi_ROR;
        default: VMI_ABORT("%s: unimplemented case", FUNC_NAME); return 0;
    }
}

//
// Generate register argument by shifting register 'ra' by constant, placing
// result in register 't'
//
static void getShiftedRC(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiFlagsCP     shiftCOut
) {
    Uns32    shift = state->info.c;
    vmiBinop op    = mapShiftOp(state->info.so);

    armEmitBinopRRC(state, ARM_GPR_BITS, op, t, ra, shift, shiftCOut);
}

//
// Generate register argument by shifting register 'ra' by register 'rs',
// placing result in register 't'
//
static void getShiftedRR(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiReg         rs,
    vmiFlagsCP     shiftCOut
) {
    vmiBinop op = mapShiftOp(state->info.so);

    armEmitSetShiftMask();
    armEmitBinopRRR(state, ARM_GPR_BITS, op, t, ra, rs, shiftCOut);
}

//
// Generate register argument by rotate right of register 'ra' through carry
//
static void getExtendedR(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiFlagsCP     shiftCOut
) {
    armEmitBinopRRC(state, ARM_GPR_BITS, vmi_RCR, t, ra, 1, shiftCOut);
}


////////////////////////////////////////////////////////////////////////////////
// UNOPS
////////////////////////////////////////////////////////////////////////////////

#define vmiFlagsCPU vmiFlagsCP __attribute__ ((unused))

//
// Macro for emission of declarations for generic unop
//
#define EMIT_UNOP_DECLS(_S)                         \
    vmiReg      rd        = GET_RD(_S, r1);         \
    vmiUnop     op        = _S->attrs->unop;        \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic unop
//
#define EMIT_UNOP_CALL(_S, _F, _S1)                 \
    _F(_S, ARM_GPR_BITS, op, rd, _S1, opFlags)

//
// Macro for emission of generic unop with unshifted argument
//
#define EMIT_UNOP_NO_SHIFT(_S, _F, _S1)             \
    EMIT_UNOP_DECLS(_S);                            \
    EMIT_UNOP_CALL(_S, _F, _S1)

//
// Macro for emission of generic unop with shifted argument
//
#define EMIT_UNOP_SHIFT(_S, _T)                     \
    EMIT_UNOP_DECLS(_S);                            \
    _T;                                             \
    EMIT_UNOP_CALL(_S, armEmitUnopRR, rd);

//
// Unop with immediate
//
ARM_MORPH_FN(armEmitUnopI) {

    Uns32 c = state->info.c;

    // emit register/constant unop with unshifted argument
    EMIT_UNOP_NO_SHIFT(state, armEmitUnopRC, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Unop with register
//
ARM_MORPH_FN(armEmitUnopR) {
    EMIT_UNOP_NO_SHIFT(state, armEmitUnopRR, GET_RS(state, r2));
}

//
// Unop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitUnopRSI) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRC(state, rd, GET_RS(state, r2), shiftCOut)
    );
}

//
// Unop with register shifted by register addressing mode
//
ARM_MORPH_FN(armEmitUnopRSR) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRR(state, rd, GET_RS(state, r2), GET_RS(state, r3), shiftCOut)
    );
}

//
// Unop with register shifted by register addressing mode (Thumb variant)
//
ARM_MORPH_FN(armEmitUnopRSRT) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRR(state, rd, GET_RS(state, r1), GET_RS(state, r2), shiftCOut)
    );
}

//
// Unop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitUnopRX) {
    EMIT_UNOP_SHIFT(
        state,
        getExtendedR(state, rd, GET_RS(state, r2), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// BINOPS
////////////////////////////////////////////////////////////////////////////////

//
// Macro for emission of declarations for generic binop
//
#define EMIT_BINOP_DECLS(_S, _S1)                   \
    vmiReg      rd        = GET_RD(_S, r1);         \
    vmiReg      rs1       = GET_RS(_S, _S1);        \
    vmiBinop    op        = _S->attrs->binop;       \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic binop
//
#define EMIT_BINOP_CALL(_S, _F, _S2)                \
    _F(_S, ARM_GPR_BITS, op, rd, rs1, _S2, opFlags)

//
// Macro for emission of generic binop with unshifted argument
//
#define EMIT_BINOP_NO_SHIFT(_S, _F, _S1, _S2)       \
    EMIT_BINOP_DECLS(_S, _S1);                      \
    EMIT_BINOP_CALL(_S, _F, _S2)

//
// Macro for emission of generic binop with shifted argument
//
#define EMIT_BINOP_SHIFT(_S, _S1, _RS2, _T)         \
    EMIT_BINOP_DECLS(_S, _S1);                      \
    vmiReg _RS2 = newTemp32(_S);                    \
    _T;                                             \
    EMIT_BINOP_CALL(_S, armEmitBinopRRR, _RS2);     \
    freeTemp32(_S)

//
// Binop with immediate
//
ARM_MORPH_FN(armEmitBinopI) {

    Uns32 c = state->info.c;

    // emit register/constant binop with unshifted argument
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r2, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Binop with three registers
//
ARM_MORPH_FN(armEmitBinopR) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRR, r2, GET_RS(state, r3));
}

//
// Binop with two registers (Thumb variant)
//
ARM_MORPH_FN(armEmitBinopRT) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRR, r1, GET_RS(state, r2));
}

//
// Binop with register and immediate (Thumb variant)
//
ARM_MORPH_FN(armEmitBinopIT) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r1, state->info.c);
}

//
// Binop with register, program counter and immediate (ADR)
//
ARM_MORPH_FN(armEmitBinopADR) {
    Uns32 c = state->attrs->negate ? -state->info.c : state->info.c;
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r2, alignConstWithPC(state, c));
}

//
// Binop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitBinopRSI) {
    EMIT_BINOP_SHIFT(
        state, r2, rs2,
        getShiftedRC(state, rs2, GET_RS(state, r3), shiftCOut)
    );
}

//
// Binop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitBinopRX) {
    EMIT_BINOP_SHIFT(
        state, r2, rs2,
        getExtendedR(state, rs2, GET_RS(state, r3), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// CMPOPS
////////////////////////////////////////////////////////////////////////////////

//
// Macro for emission of declarations for generic cmpop
//
#define EMIT_CMPOP_DECLS(_S)                        \
    vmiReg      rd        = VMI_NOREG;              \
    vmiReg      rs1       = GET_RS(_S, r1);         \
    vmiBinop    op        = _S->attrs->binop;       \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic cmpop
//
#define EMIT_CMPOP_CALL(_S, _F, _S2)                \
    _F(_S, ARM_GPR_BITS, op, rd, rs1, _S2, opFlags)

//
// Macro for emission of generic cmpop with unshifted argument
//
#define EMIT_CMPOP_NO_SHIFT(_S, _F, _S2)            \
    EMIT_CMPOP_DECLS(_S);                           \
    EMIT_CMPOP_CALL(_S, _F, _S2)

//
// Macro for emission of generic cmpop with shifted argument
//
#define EMIT_CMPOP_SHIFT(_S, _RS2, _T)              \
    EMIT_CMPOP_DECLS(_S);                           \
    vmiReg _RS2 = newTemp32(_S);                    \
    _T;                                             \
    EMIT_CMPOP_CALL(_S, armEmitBinopRRR, _RS2);     \
    freeTemp32(_S)

//
// Cmpop with immediate
//
ARM_MORPH_FN(armEmitCmpopI) {

    Uns32 c = state->info.c;

    // emit register/constant cmpop with unshifted argument
    EMIT_CMPOP_NO_SHIFT(state, armEmitBinopRRC, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Cmpop with register
//
ARM_MORPH_FN(armEmitCmpopR) {
    EMIT_CMPOP_NO_SHIFT(state, armEmitBinopRRR, GET_RS(state, r2));
}

//
// Cmpop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitCmpopRSI) {
    EMIT_CMPOP_SHIFT(
        state, rs2,
        getShiftedRC(state, rs2, GET_RS(state, r2), shiftCOut)
    );
}

//
// Cmpop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitCmpopRX) {
    EMIT_CMPOP_SHIFT(
        state, rs2,
        getExtendedR(state, rs2, GET_RS(state, r2), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// MULTIPLY AND DIVIDE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for MUL instruction
//
ARM_MORPH_FN(armEmitMUL) {

    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rd    = GET_RD(state, r1);
    vmiReg     rm    = GET_RS(state, r2);
    vmiReg     rs    = GET_RS(state, r3);

    armEmitBinopRRR(state, ARM_GPR_BITS, vmi_IMUL, rd, rm, rs, flags);
}

//
// Emit code for SDIV and UDIV instructions
//
ARM_MORPH_FN(armEmitDIV) {

    vmiBinop op = state->attrs->binop;
    vmiReg   rd = GET_RD(state, r1);
    vmiReg   rn = GET_RS(state, r2);
    vmiReg   rm = GET_RS(state, r3);

    // record the target of the divide instruction (in case of exception)
    armEmitMoveRC(state, 8, ARM_DIVIDE_TARGET, state->info.r1);
    armEmitBinopRRR(state, ARM_GPR_BITS, op, rd, rn, rm, 0);
}

//
// Emit code for MLA or MLS instructions
//
static void emitMLAMLS(armMorphStateP state, vmiBinop op) {

    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rd    = GET_RD(state, r1);
    vmiReg     rm    = GET_RS(state, r2);
    vmiReg     rs    = GET_RS(state, r3);
    vmiReg     rn    = GET_RS(state, r4);
    vmiReg     t     = VMI_REG_EQUAL(rd, rn) ? getTemp(state) : rd;

    armEmitBinopRRR(state, ARM_GPR_BITS, vmi_IMUL, t, rm, rs, 0);
    armEmitBinopRRR(state, ARM_GPR_BITS, op, rd, t, rn, flags);
}

//
// MLA instruction
//
ARM_MORPH_FN(armEmitMLA) {
    emitMLAMLS(state, vmi_ADD);
}

//
// MLS instruction
//
ARM_MORPH_FN(armEmitMLS) {
    emitMLAMLS(state, vmi_RSUB);
}

//
// [SU]MULL instructions
//
ARM_MORPH_FN(armEmitMULL) {

    vmiBinop   op    = state->attrs->binop;
    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rdlo  = GET_RD(state, r1);
    vmiReg     rdhi  = GET_RD(state, r2);
    vmiReg     rm    = GET_RS(state, r3);
    vmiReg     rs    = GET_RS(state, r4);

    armEmitMulopRRR(state, ARM_GPR_BITS, op, rdhi, rdlo, rm, rs, flags);
}

//
// [SU]MLAL instructions
//
ARM_MORPH_FN(armEmitMLAL) {

    Uns32      bits   = ARM_GPR_BITS;
    vmiBinop   op     = state->attrs->binop;
    vmiFlagsCP flags  = getFlagsOrNull(state);
    vmiReg     rdlo   = GET_RS(state, r1);
    vmiReg     rdhi   = GET_RS(state, r2);
    vmiReg     rm     = GET_RS(state, r3);
    vmiReg     rs     = GET_RS(state, r4);
    vmiReg     t      = newTemp64(state);
    vmiReg     t164lo = getR64Lo(t);
    vmiReg     t164hi = getR64Hi(t);

    // perform initial multiply, result in temporary t164lo/t164hi
    armEmitMulopRRR(state, bits, op, t164hi, t164lo, rm, rs, 0);

    if(VMI_REG_EQUAL(rdhi, getR64Hi(rdlo))) {

        // rdlo/rdhi are an adjacent pair
        armEmitBinopRR(state, bits*2, vmi_ADD, rdlo, t164lo, flags);

    } else {

        // rdlo/rdhi are not an adjacent pair
        vmiReg t      = newTemp64(state);
        vmiReg t264lo = getR64Lo(t);
        vmiReg t264hi = getR64Hi(t);

        // move rdlo/rdhi into adjacent pair
        armEmitMoveRR(state, bits, t264hi, rdhi);
        armEmitMoveRR(state, bits, t264lo, rdlo);

        // perform addition using adjacent pair
        armEmitBinopRR(state, bits*2, vmi_ADD, t264lo, t164lo, flags);

        // move to rdlo/rdhi from adjacent pair
        armEmitMoveRR(state, bits, rdhi, t264hi);
        armEmitMoveRR(state, bits, rdlo, t264lo);

        freeTemp64(state);
    }

    freeTemp64(state);
}


////////////////////////////////////////////////////////////////////////////////
// BRANCH INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Return link address from state
//
static Uns32 getStateLinkPC(armMorphStateP state) {
    return state->nextPC | (IN_THUMB_MODE(state->arm) ? 1 : 0);
}

//
// Fill armJumpInfo structure with information about a jump to a target address
//
static void seedJumpInfo(
    armJumpInfoP   ji,
    armMorphStateP state,
    Bool           isLink,
    Bool           isReturn,
    Bool           isRelative
) {
    ji->linkReg = isLink ? ARM_LR : VMI_NOREG;
    ji->hint    = isRelative ? vmi_JH_RELATIVE : vmi_JH_NONE;
    ji->hint   |= isReturn ? vmi_JH_RETURN : isLink ? vmi_JH_CALL : vmi_JH_NONE;
    ji->linkPC  = isLink ? getStateLinkPC(state) : 0;
}

//
// Emit an explicit unconditional jump to target address
//
static void emitUncondJumpC(armMorphStateP state, Bool isLink) {

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, False, True);

    // do the jump
    armEmitUncondJump(state, &ji);
}

//
// Emit an explicit conditional jump to target address
//
static void emitCondJumpC(
    armMorphStateP state,
    vmiReg         tf,
    Bool           jumpIfTrue,
    Bool           isLink
) {
    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, False, True);

    // do the jump
    armEmitCondJump(state, &ji, tf, jumpIfTrue);
}

//
// Macro defining the body of a (possibly conditional) jump to target
//
#define COND_OR_UNCOND_BRANCH_BODY(_UNCOND_CB, _COND_CB)                    \
                                                                            \
    armCond cond   = emitPrepareCondition(state);                           \
    Bool    isLink = state->attrs->isLink;                                  \
                                                                            \
    if(cond.op==ACO_ALWAYS) {                                               \
        /* unconditional jump */                                            \
        _UNCOND_CB(state, isLink);                                          \
    } else {                                                                \
        /* conditional jump */                                              \
        _COND_CB(state, cond.flag, cond.op==ACO_TRUE, isLink);              \
    }                                                                       \

//
// Emit conditional branch to constant address, possibly with link
//
ARM_MORPH_FN(armEmitBranchC) {
    COND_OR_UNCOND_BRANCH_BODY(emitUncondJumpC, emitCondJumpC);
}

//
// Emit unconditional branch to register address, possibly with link
//
ARM_MORPH_FN(armEmitBranchR) {

    vmiReg ra     = GET_RS(state, r1);
    Bool   isLink = state->attrs->isLink;

    // switch mode if LSB of target address implies a different mode
    armEmitInterworkLSB(state, ra);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, state->info.r1==ARM_REG_LR, False);

    // do the jump
    armEmitUncondJumpReg(state, &ji, ra);
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Called for an unimplemented instruction
//
static void ignoreBKPT(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_BII",
        SRCREF_FMT "BKPT instruction ignored in nopBKPT compatability mode",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Emit call to perform CLZ
//
ARM_MORPH_FN(armEmitCLZ) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    armEmitUnopRR(state, ARM_GPR_BITS, vmi_CLZ, rd, rm, 0);
}

//
// Emit call to perform BKPT
//
ARM_MORPH_FN(armEmitBKPT) {
    if(state->arm->compatMode==COMPAT_CODE_SOURCERY) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn)ignoreBKPT);
    } else {
        emitExceptionCall(state, armBKPT);
    }
}

//
// Emit call to perform SWI
//
ARM_MORPH_FN(armEmitSWI) {
    emitExceptionCall(state, armSWI);
}


////////////////////////////////////////////////////////////////////////////////
// SPECIAL PURPOSE REGISTER ACCESS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for MRS
//
ARM_MORPH_FN(armEmitMRS) {

    Uns32       bits     = ARM_GPR_BITS;
    vmiReg      rd       = GET_RD(state, r1);
    vmiReg      rs       = VMI_NOREG;
    Bool        privMode = !IN_USER_MODE(state->arm);
    armSysRegId SYSm     = state->info.c;

    switch(SYSm>>3) {

        case 0: {

            // extract fields from PSR
            Uns32 mask = 0;

            // include exceptNum field if required
            if((SYSm&1) && privMode) {
                mask |= PSR_EXCEPT_NUM;
            }

            // include flags field if required
            if(!(SYSm&4)) {
                mask |= PSR_FLAGS;
            }

            // get masked result value
            if(mask) {
                armEmitArgProcessor(state);
                armEmitCallResult(state, (vmiCallFn)armReadCPSR, bits, rd);
                armEmitBinopRC(state, bits, vmi_AND, rd, mask, 0);
                rs = rd;
            }

            break;
        }

        case 1:

            // return SP_main or SP_process
            if(privMode) {

                // this code is mode specific
                armEmitValidateBlockMask(ARM_BM_USE_SP_PROCESS);

                // select source register
                if((SYSm&1) == USE_SP_PROCESS(state->arm)) {
                    rs = ARM_SP;
                } else {
                    rs = ARM_BANK_SP;
                }
            }

            break;

        case 2:

            // return PRIMASK, BASEPRI, FAULTMASK or CONTROL
            switch(SYSm&7) {
                case 0: rs = privMode ? ARM_PRIMASK   : VMI_NOREG; break;
                case 1: rs = privMode ? ARM_BASEPRI   : VMI_NOREG; break;
                case 2: rs = privMode ? ARM_BASEPRI   : VMI_NOREG; break;
                case 3: rs = privMode ? ARM_FAULTMASK : VMI_NOREG; break;
                case 4: rs = ARM_CONTROL;                          break;
                default:                                           break;
            }

            break;

        default:
            break;
    }

    // get the register
    armEmitMoveRR(state, bits, rd, rs);
}

//
// Emit code for MSR
//
ARM_MORPH_FN(armEmitMSR) {

    Uns32       bits     = ARM_GPR_BITS;
    vmiReg      rs       = GET_RD(state, r1);
    Bool        privMode = !IN_USER_MODE(state->arm);
    armSysRegId SYSm     = state->info.c;

    switch(SYSm>>3) {

        case 0: {

            // insert fields in PSR
            Uns32 wrMask = 0;

            // include flags field if required
            if(!(SYSm&4)) {
                armPSRBits bits = state->info.psrbits;
                if (bits==ARM_PSRBITS_ALL || bits==ARM_PSRBITS_FLAGS) {
                    wrMask |= PSR_FLAGS;
                }
                if (bits==ARM_PSRBITS_ALL || bits==ARM_PSRBITS_GE) {
                    if (DSP_PRESENT(state->arm)) {
                        wrMask |= PSR_GE;
                    }
                }
            }

            // set masked result value
            if(wrMask) {
                armEmitArgProcessor(state);
                armEmitArgReg(state, bits, rs);
                armEmitArgUns32(state, wrMask);
                armEmitCall(state, (vmiCallFn)armWriteCPSR);
            }

            break;
        }

        case 1:

            // set SP_main or SP_process
            if(privMode) {

                vmiReg rd;

                // this code is mode specific
                armEmitValidateBlockMask(ARM_BM_USE_SP_PROCESS);

                // select destination register
                if((SYSm&1) == USE_SP_PROCESS(state->arm)) {
                    rd = ARM_SP;
                } else {
                    rd = ARM_BANK_SP;
                }

                // set the register
                armEmitMoveRR(state, bits, rd, rs);
            }

            break;

        case 2:

            // assign PRIMASK, BASEPRI, FAULTMASK or CONTROL
            if(privMode) {

                switch(SYSm&7) {

                    case 0:
                        armEmitArgProcessor(state);
                        armEmitArgReg(state, bits, rs);
                        armEmitCall(state, (vmiCallFn)armWritePRIMASK);
                        break;

                    case 1:
                        armEmitArgProcessor(state);
                        armEmitArgReg(state, bits, rs);
                        armEmitCall(state, (vmiCallFn)armWriteBASEPRI);
                        break;

                    case 2:
                        armEmitArgProcessor(state);
                        armEmitArgReg(state, bits, rs);
                        armEmitCall(state, (vmiCallFn)armWriteBASEPRI_MAX);
                        break;

                    case 3:
                        armEmitArgProcessor(state);
                        armEmitArgReg(state, bits, rs);
                        armEmitCall(state, (vmiCallFn)armWriteFAULTMASK);
                        break;

                    case 4:
                        armEmitArgProcessor(state);
                        armEmitArgReg(state, bits, rs);
                        armEmitCall(state, (vmiCallFn)armWriteCONTROL);
                        break;

                    default:
                        break;
                }
            }

            break;

        default:
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// HINT INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit NOP
//
ARM_MORPH_FN(armEmitNOP) {
    // no action
}

//
// Emit WFE
//
ARM_MORPH_FN(armEmitWFE) {

    vmiLabelP wait = armEmitNewLabel();
    vmiLabelP done = armEmitNewLabel();

    // jump to wait code if no event registered
    armEmitCondJumpLabel(ARM_EVENT, False, wait);

    // clear event register and finish
    armEmitMoveRC(state, 8, ARM_EVENT, 0);
    armEmitUncondJumpLabel(done);

    // here if halt is required
    armEmitInsertLabel(wait);

    // wait for event
    armEmitWait(state, AD_WFE);

    // here when done
    armEmitInsertLabel(done);
}

//
// Emit WFI
//
ARM_MORPH_FN(armEmitWFI) {

    vmiLabelP noWait = armEmitNewLabel();

    // don't stop if there are pending interrupts
    armEmitCompareRCJumpLabel(8, vmi_COND_NZ, ARM_PENDING, 0, noWait);

    // halt the processor at the end of this instruction
    armEmitWait(state, AD_WFI);

    // here if interrupt is currently pending
    emitLabel(noWait);
}

//
// Emit SEV
//
ARM_MORPH_FN(armEmitSEV) {
    armEmitArgProcessor(state);
    armEmitCall(state, (vmiCallFn)armSEV);
}


////////////////////////////////////////////////////////////////////////////////
// LOAD AND STORE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// This indicates that the register being loaded is also the base in an LDM
//
#define LDM_BASE_REG -1

//
// If this is an STM with the base register in the list and also writeback, is
// the value written the *final* value of the base register?
//
#define STM_WB_BASE_FINAL False

//
// Callback function for load/store
//
#define LOAD_STORE_FN(_NAME) void _NAME( \
    armMorphStateP state,   \
    vmiReg         base,    \
    Int32          offset   \
)
typedef LOAD_STORE_FN((*loadStoreFn));

//
// Callback function for one register of a load/store multiple
//
#define LOAD_STORE_M_FN(_NAME) void _NAME( \
    armMorphStateP state,           \
    vmiReg         base,            \
    Int32          offset,          \
    Uns32          r,               \
    Bool           isWBNotFirst,    \
    Int32          frameDelta       \
)
typedef LOAD_STORE_M_FN((*loadStoreMFn));

//
// Should increment/decrement be performed before access?
//
inline static Bool doBefore(armMorphStateP state) {
    armIncDec incDec = state->info.incDec;
    return ((incDec==ARM_ID_IB) || (incDec==ARM_ID_DA));
}

//
// Does load/store multiple increment?
//
inline static Bool doIncrement(armMorphStateP state) {
    return state->info.incDec & ARM_ID_I;
}

//
// Return the step to apply before load/store multiple
//
inline static Int32 getStepBefore(armMorphStateP state, Uns32 bytes) {
    return doBefore(state) ? bytes : 0;
}

//
// Return the step to apply after load/store multiple
//
inline static Int32 getStepAfter(armMorphStateP state, Uns32 bytes) {
    return doBefore(state) ? 0 : bytes;
}

//
// Return frame size for a load/store multiple
//
static Uns32 getFrameSize(armMorphStateP state) {

    Uns32 rList = state->info.rList;
    Uns32 mask;
    Uns32 r;
    Int32 size = 0;

    for(r=0, mask=1; r<ARM_GPR_NUM; r++, mask<<=1) {
        if(rList&mask) {
            size += ARM_GPR_BYTES;
        }
    }

    return size;
}

//
// Is the base register overwritten by a load multiple?
//
static Bool baseIsLoaded(armMorphStateP state, Bool isLoad) {

    Uns32 rmMask = (1<<state->info.r1);

    return (isLoad && (state->info.rList & rmMask));
}

//
// Is the base register not the highest (first) register in the list?
//
inline static Bool baseIsNotFirst(Uns32 rList, Uns32 rBase) {
    return rList & ((1<<rBase)-1);
}

//
// Perform iteration for a load/store multiple instruction, calling the passed
// callback for each access
//
static void emitLoadStoreMultiple(
    armMorphStateP state,
    loadStoreMFn   cb,
    Bool           isLoad
) {
    Uns32  bits       = ARM_GPR_BITS;
    Uns32  rList      = state->info.rList;
    Uns32  rBase      = state->info.r1;
    Bool   baseLoaded = baseIsLoaded(state, isLoad);
    vmiReg base       = GET_RS(state, r1);
    Bool   increment  = doIncrement(state);
    Int32  stepBefore = getStepBefore(state, ARM_GPR_BYTES);
    Int32  stepAfter  = getStepAfter(state, ARM_GPR_BYTES);
    Uns32  frameSize  = getFrameSize(state);
    Int32  offset     = increment ? 0 : -frameSize;
    Int32  frameDelta = increment ? frameSize : -frameSize;
    Bool   isNotFirst = state->info.wb && baseIsNotFirst(rList, rBase);
    Uns32  mask;
    Uns32  r;

    // load or store registers
    for(r=0, mask=1; r<ARM_GPR_NUM; r++, mask<<=1) {
        if(rList&mask) {
            Bool isWBNotFirst = isNotFirst && (r==rBase);
            Uns32 rt = (baseLoaded && (r==rBase)) ? LDM_BASE_REG : r;
            offset += stepBefore;
            cb(state, base, offset, rt, isWBNotFirst, frameDelta);
            offset += stepAfter;
        }
    }

    // perform base register update if required
    if(baseLoaded) {
        vmiReg rn = GET_RD(state, r1);
        armEmitMoveRR(state, bits, rn, getTemp(state));
    } else if(state->info.wb) {
        vmiReg rn = GET_RD(state, r1);
        armEmitBinopRRC(state, bits, vmi_ADD, rn, base, frameDelta, 0);
    }
}

//
// Macro to emit one register load for LDM
//
#define EMIT_LDM_REG(_S, _BASE, _OFFSET, _R)  {                     \
                                                                    \
    Uns32  bits = ARM_GPR_BITS;                                     \
    vmiReg rd   = (_R==LDM_BASE_REG) ? getTemp(_S) : getRD(_S, _R); \
                                                                    \
    armEmitLoadRRO(_S, bits, _OFFSET, rd, _BASE, False, True);      \
}

//
// Emit one register load for LDM
//
static LOAD_STORE_M_FN(emitLDMCB) {
    EMIT_LDM_REG(state, base, offset, r);
}

//
// If the source register is the PC, adjust to a 12-byte offset if required
//
static vmiReg emitPCDelta(armMorphStateP state, Uns32 r, vmiReg rs) {

    if((r==ARM_REG_PC) && state->arm->configInfo.STRoffsetPC12) {
        vmiReg t = getTemp(state);
        armEmitBinopRRC(state, ARM_GPR_BITS, vmi_ADD, t, rs, 4, 0);
        return t;
    } else {
        return rs;
    }
}

//
// Emit one register store for STM
//
static LOAD_STORE_M_FN(emitSTMCB) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rs   = getRS(state, r);

    // write the final value of the base register if it is in the register list
    if(STM_WB_BASE_FINAL && isWBNotFirst) {
        rs = getTemp(state);
        armEmitBinopRRC(state, bits, vmi_ADD, rs, base, frameDelta, 0);
    }

    // if source register is PC, allow for store offset adjustment
    rs = emitPCDelta(state, r, rs);

    // do the store
    armEmitStoreRRO(state, bits, offset, base, rs);
}

//
// Emit code for LDM (1)
//
ARM_MORPH_FN(armEmitLDM1) {
    emitLoadStoreMultiple(state, emitLDMCB, True);
}

//
// Emit code for STM (1)
//
ARM_MORPH_FN(armEmitSTM1) {
    emitLoadStoreMultiple(state, emitSTMCB, False);
}

//
// Should the LDR instruction do writeback?
//
inline static Bool doLDRWriteBack(armMorphStateP state) {
    return (state->info.wb && (state->info.r1!=state->info.r2));
}

//
// Is translation required? (LDRT, STRT)
//
inline static Bool doTranslate(armMorphStateP state) {
    return (state->info.tl && IN_PRIV_MPU_MODE(state->arm));
}

//
// If this is an LDRT/STRT instruction and we are currently in privileged mode
// with TLB enabled, emit code to switch to the user mode data memDomain
//
static void emitTranslateOn(armMorphStateP state) {
    if(doTranslate(state)) {
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)armVMSetUserPrivilegedModeDataDomain);
    }
}

//
// If this is after a LDRT/STRT instruction and we are currently in privileged
// mode with TLB enabled, emit code to switch back to the privileged mode data
// memDomain. Note that this code is not executed if the prior access causes an
// exception; in this case, mode is restored in armDataAbort.
//
static void emitTranslateOff(armMorphStateP state) {
    if(doTranslate(state)) {
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)armVMRestoreNormalDataDomain);
    }
}

//
// Emit code for LDR variant
//
static LOAD_STORE_FN(emitLDR) {

    Uns32  memBits = state->info.sz*8;
    Bool   isLong  = (memBits==64);
    vmiReg rd      = GET_RD(state, r1);
    Bool   xs      = state->info.xs;

    // start translation (LDRT, STRT)
    emitTranslateOn(state);

    // emit load
    if(isLong) {

        vmiReg rdH = GET_RD(state, r4);
        vmiReg rt  = getTemp(state);

        armEmitLoadRRRO(state, memBits, offset, rd, rdH, base, rt, xs, False);

    } else {

        armEmitLoadRRO(state, memBits, offset, rd, base, xs, False);
    }

    // end translation (LDRT, STRT)
    emitTranslateOff(state);
}

//
// Emit code for STR variant
//
static LOAD_STORE_FN(emitSTR) {

    Uns32  memBits = state->info.sz*8;
    Bool   isLong  = (memBits==64);
    Uns32  r1      = state->info.r1;
    vmiReg rs      = getRS(state, r1);

    // if source register is PC, allow for store offset adjustment
    rs = emitPCDelta(state, r1, rs);

    // start translation (LDRT, STRT)
    emitTranslateOn(state);

    // emit store
    if(isLong) {

        Uns32  r4  = state->info.r4;
        vmiReg rsH = getRS(state, r4);

        armEmitStoreRRRO(state, memBits, offset, base, rs, rsH);

    } else {

        armEmitStoreRRO(state, memBits, offset, base, rs);
    }

    // end translation (LDRT, STRT)
    emitTranslateOff(state);
}

//
// Emit code for LDR/STR with immediate offset
//
static void emitLDRSTRI(armMorphStateP state, loadStoreFn cb, Bool align) {

    Uns32  bits      = ARM_GPR_BITS;
    vmiReg base      = GET_RS(state, r2);
    Int32  offset    = state->info.c;
    Int32  memOffset = state->info.pi ? 0 : offset;

    // align the constant if the base register is the program counter (has
    // effect for PC-relative Thumb load instructions only)
    if(align && VMI_REG_EQUAL(base, ARM_PC)) {
        memOffset = alignConstWithPC(state, memOffset);
    }

    // emit register load or store
    cb(state, base, memOffset);

    // do writeback if required
    if(doLDRWriteBack(state)) {
        vmiReg ra = GET_RD(state, r2);
        armEmitBinopRC(state, bits, vmi_ADD, ra, offset, 0);
    }
}

//
// Emit code for LDR/STR variant
//
static void emitLDRSTRInt(armMorphStateP state, vmiReg offset, loadStoreFn cb) {

    Uns32    bits   = ARM_GPR_BITS;
    vmiReg   base   = GET_RS(state, r2);
    vmiReg   t      = newTemp32(state);
    vmiReg   memReg = state->info.pi ? base : t;
    vmiBinop op     = state->info.u ? vmi_ADD : vmi_SUB;

    // calculate incremented address
    armEmitBinopRRR(state, bits, op, t, base, offset, 0);

    // emit register load
    cb(state, memReg, 0);

    // do writeback if required
    if(doLDRWriteBack(state)) {
        vmiReg ra = GET_RD(state, r2);
        armEmitMoveRR(state, bits, ra, t);
    }

    freeTemp32(state);
}

//
// Emit code for LDR/STR with register offset
//
static void emitLDRSTRR(armMorphStateP state, loadStoreFn cb) {

    vmiReg rm = GET_RS(state, r3);

    emitLDRSTRInt(state, rm, cb);
}

//
// Emit code for LDR/STR with scaled register offset (LSL, LSR, ASR, ROR)
//
static void emitLDRSTRRSI(armMorphStateP state, loadStoreFn cb) {

    vmiReg t  = newTemp32(state);
    vmiReg rm = GET_RS(state, r3);

    getShiftedRC(state, t, rm, FLAGS_CIN);

    emitLDRSTRInt(state, t, cb);

    freeTemp32(state);
}

//
// Emit code for LDR with immediate offset
//
ARM_MORPH_FN(armEmitLDRI) {
    emitLDRSTRI(state, emitLDR, True);
}

//
// Emit code for LDR with register offset
//
ARM_MORPH_FN(armEmitLDRR) {
    emitLDRSTRR(state, emitLDR);
}

//
// Emit code for LDR with scaled register offset (LSL, LSR, ASR, ROR)
//
ARM_MORPH_FN(armEmitLDRRSI) {
    emitLDRSTRRSI(state, emitLDR);
}

//
// Emit code for STR with immediate offset
//
ARM_MORPH_FN(armEmitSTRI) {
    emitLDRSTRI(state, emitSTR, False);
}

//
// Emit code for STR with register offset
//
ARM_MORPH_FN(armEmitSTRR) {
    emitLDRSTRR(state, emitSTR);
}

//
// Emit code for STR with scaled register offset (LSL, LSR, ASR, ROR)
//
ARM_MORPH_FN(armEmitSTRRSI) {
    emitLDRSTRRSI(state, emitSTR);
}


////////////////////////////////////////////////////////////////////////////////
// DSP INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Minimum and maximum values for clamping
//
#define ARM_MIN 0x80000000
#define ARM_MAX 0x7fffffff

//
// Emit code for to perform rd = rs1 op rs2, clamping result and setting Q
// flag if overflow
//
static void emitOpSetQClamp(
    armMorphStateP state,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    Uns32     bits       = ARM_GPR_BITS;
    vmiFlags  flags      = getSFOFFlags(getTemp(state));
    vmiLabelP noOverflow = armEmitNewLabel();

    // do the operation, setting flags
    armEmitBinopRRR(state, bits, op, rd, rs1, rs2, &flags);

    // skip clamping and Q flag update unless there was an overflow
    armEmitCondJumpLabel(flags.f[vmi_OF], False, noOverflow);

    // clamp depending on sign of result
    armEmitCondMoveRCC(state, bits, flags.f[vmi_SF], True, rd, ARM_MAX, ARM_MIN);

    // set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // here if the operation didn't overflow
    emitLabel(noOverflow);
}

//
// Emit code to perform rd = rs1 op rs2, setting Q flag if overflow
//
static void emitOpSetQ(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    vmiReg   tf    = getTemp(state);
    vmiFlags flags = getOFFlags(tf);

    // do the operation, setting flags
    armEmitBinopRRR(state, bits, op, rd, rs1, rs2, &flags);

    // set the sticky Q flag of there was overflow
    armEmitBinopRR(state, 8, vmi_OR, ARM_QF, tf, 0);
}

//
// Emit code for 16 x 16 = 32 multiply
//
static void emitMul1632(
    armMorphStateP state,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    vmiReg rdlo = getR32Lo(rd);
    vmiReg rdhi = getR32Hi(rd);

    // do 16 x 16 = 32 multiply
    armEmitMulopRRR(state, ARM_GPR_BITS/2, vmi_IMUL, rdhi, rdlo, rs1, rs2, 0);
}

//
// Emit code for QADD/QSUB
//
static void emitQADDSUB(armMorphStateP state, vmiBinop op) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = GET_RS(state, r2);
    vmiReg rs2 = GET_RS(state, r3);

    emitOpSetQClamp(state, op, rd, rs1, rs2);
}

//
// Emit code for QDADD/QDSUB
//
static void emitQDADDSUB(armMorphStateP state, vmiBinop op) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = GET_RS(state, r2);
    vmiReg rs2 = GET_RS(state, r3);
    vmiReg t   = newTemp32(state);

    emitOpSetQClamp(state, vmi_ADD, t, rs2, rs2);
    emitOpSetQClamp(state, op, rd, rs1, t);

    freeTemp32(state);
}

//
// Emit code for QADD
//
ARM_MORPH_FN(armEmitQADD) {
    emitQADDSUB(state, vmi_ADD);
}

//
// Emit code for QSUB
//
ARM_MORPH_FN(armEmitQSUB) {
    emitQADDSUB(state, vmi_SUB);
}

//
// Emit code for QDADD
//
ARM_MORPH_FN(armEmitQDADD) {
    emitQDADDSUB(state, vmi_ADD);
}

//
// Emit code for QDSUB
//
ARM_MORPH_FN(armEmitQDSUB) {
    emitQDADDSUB(state, vmi_SUB);
}

//
// Emit code for SMLA<x><y>
//
static void emitSMLA(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rs1  = VMI_REG_DELTA(GET_RS(state, r2), xDelta);
    vmiReg rs2  = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg rs3  = GET_RS(state, r4);
    vmiReg t    = newTemp32(state);

    // do 16 x 16 = 32 multiply
    emitMul1632(state, t, rs1, rs2);

    // do the accumulate, setting Q flag on overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, rs3, t);

    // free multiply result temporary
    freeTemp32(state);
}

//
// Emit code for SMLABB
//
ARM_MORPH_FN(armEmitSMLABB) {
    emitSMLA(state, 0, 0);
}

//
// Emit code for SMLABT
//
ARM_MORPH_FN(armEmitSMLABT) {
    emitSMLA(state, 0, 2);
}

//
// Emit code for SMLATB
//
ARM_MORPH_FN(armEmitSMLATB) {
    emitSMLA(state, 2, 0);
}

//
// Emit code for SMLATT
//
ARM_MORPH_FN(armEmitSMLATT) {
    emitSMLA(state, 2, 2);
}

//
// Emit code for SMLAL<x><y>
//
static void emitSMLAL(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rdlo  = GET_RD(state, r1);
    vmiReg   rdhi  = GET_RD(state, r2);
    vmiReg   rs1   = VMI_REG_DELTA(GET_RS(state, r3), xDelta);
    vmiReg   rs2   = VMI_REG_DELTA(GET_RS(state, r4), yDelta);
    vmiReg   t     = newTemp64(state);
    vmiReg   tlo32 = getR64Lo(t);
    vmiReg   thi32 = getR64Hi(t);
    vmiFlags flags = getCFFlags(getTemp(state));

    // do 16 x 16 = 32 multiply and extend to 64 bits
    emitMul1632(state, t, rs1, rs2);
    armEmitMoveExtendRR(state, bits*2, t, bits, tlo32, True);

    // do the accumulate
    armEmitBinopRR(state, bits, vmi_ADD, rdlo, tlo32, &flags);
    armEmitBinopRR(state, bits, vmi_ADC, rdhi, thi32, &flags);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMLABB
//
ARM_MORPH_FN(armEmitSMLALBB) {
    emitSMLAL(state, 0, 0);
}

//
// Emit code for SMLABT
//
ARM_MORPH_FN(armEmitSMLALBT) {
    emitSMLAL(state, 0, 2);
}

//
// Emit code for SMLATB
//
ARM_MORPH_FN(armEmitSMLALTB) {
    emitSMLAL(state, 2, 0);
}

//
// Emit code for SMLATT
//
ARM_MORPH_FN(armEmitSMLALTT) {
    emitSMLAL(state, 2, 2);
}

//
// Emit code for SMLAW<y>
//
static void emitSMLAW(armMorphStateP state, Uns32 yDelta) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RD(state, r1);
    vmiReg rs1   = GET_RS(state, r2);
    vmiReg rs2   = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg rs3   = GET_RS(state, r4);
    vmiReg t     = newTemp64(state);
    vmiReg tlo32 = getR64Lo(t);
    vmiReg thi32 = getR64Hi(t);
    vmiReg t48   = VMI_REG_DELTA(t, 2);

    // sign extend rs2 and place result in temporary
    armEmitMoveExtendRR(state, bits, t, bits/2, rs2, True);

    // do 32 x 32 = 64 multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, thi32, tlo32, t, rs1, 0);

    // do the accumulate, setting Q flag on overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, t48, rs3);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMLAWB
//
ARM_MORPH_FN(armEmitSMLAWB) {
    emitSMLAW(state, 0);
}

//
// Emit code for SMLAWT
//
ARM_MORPH_FN(armEmitSMLAWT) {
    emitSMLAW(state, 2);
}

//
// Emit code for emitSMUL<x><y>
//
static void emitSMUL(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = VMI_REG_DELTA(GET_RS(state, r2), xDelta);
    vmiReg rs2 = VMI_REG_DELTA(GET_RS(state, r3), yDelta);

    emitMul1632(state, rd, rs1, rs2);
}

//
// Emit code for SMULBB
//
ARM_MORPH_FN(armEmitSMULBB) {
    emitSMUL(state, 0, 0);
}

//
// Emit code for SMULBT
//
ARM_MORPH_FN(armEmitSMULBT) {
    emitSMUL(state, 0, 2);
}

//
// Emit code for SMULTB
//
ARM_MORPH_FN(armEmitSMULTB) {
    emitSMUL(state, 2, 0);
}

//
// Emit code for SMULTT
//
ARM_MORPH_FN(armEmitSMULTT) {
    emitSMUL(state, 2, 2);
}

//
// Emit code for SMULW<y>
//
static void emitSMULW(armMorphStateP state, Uns32 yDelta) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RD(state, r1);
    vmiReg rs1   = GET_RS(state, r2);
    vmiReg rs2   = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg t     = newTemp64(state);
    vmiReg tlo32 = getR64Lo(t);
    vmiReg thi32 = getR64Hi(t);
    vmiReg t48   = VMI_REG_DELTA(t, 2);

    // sign extend rs2 and place result in temporary
    armEmitMoveExtendRR(state, bits, t, bits/2, rs2, True);

    // do 32 x 32 = 64 multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, thi32, tlo32, t, rs1, 0);

    // assign the result
    armEmitMoveRR(state, bits, rd, t48);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMULWB
//
ARM_MORPH_FN(armEmitSMULWB) {
    emitSMULW(state, 0);
}

//
// Emit code for SMULWT
//
ARM_MORPH_FN(armEmitSMULWT) {
    emitSMULW(state, 2);
}

////////////////////////////////////////////////////////////////////////////////
// COPROCESSOR INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morpher callback for undefined coprocessor instructions
//
ARM_MORPH_FN(armEmitUsageFaultCP) {

    // generate assertion
    armEmitArgProcessor(state);
    armEmitArgSimPC(state, ARM_GPR_BITS);
    armEmitCall(state, (vmiCallFn)undefinedCPMessage);

    // take UndefinedInstruction exception
    emitUsageFault(state, EXC_UNDEF_NOCP);
}



////////////////////////////////////////////////////////////////////////////////
// MOVE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// MOVW instruction
//
ARM_MORPH_FN(armEmitMOVW) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);

    armEmitMoveRC(state, bits, rd, state->info.c);
}

//
// MOVT instruction
//
ARM_MORPH_FN(armEmitMOVT) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);

    if(!VMI_REG_EQUAL(rd, ARM_PC)) {

        vmiReg rdH = VMI_REG_DELTA(rd, 2);

        armEmitMoveRC(state, bits/2, rdH, state->info.c);

    } else {

        vmiReg rs   = getRS(state, ARM_REG_PC);
        vmiReg tmp  = newTemp32(state);
        vmiReg tmpH = VMI_REG_DELTA(tmp, 2);

        armEmitMoveRR(state, bits, tmp, rs);
        armEmitMoveRC(state, bits/2, tmpH, state->info.c);
        armEmitMoveRR(state, bits, rd, tmp);

        freeTemp32(state);
    }
}

////////////////////////////////////////////////////////////////////////////////
// MULTIPLY INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// [SU]MAAL instructions
//
ARM_MORPH_FN(armEmitMAAL) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiBinop op    = state->attrs->binop;
    vmiReg   rdlo  = GET_RS(state, r1);
    vmiReg   rdhi  = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   rs    = GET_RS(state, r4);
    vmiReg   t     = newTemp64(state);
    vmiReg   t64lo = getR64Lo(t);
    vmiReg   t64hi = getR64Hi(t);
    vmiFlags tf    = getCFFlags(getTemp(state));

    // perform initial multiply, result in temporary t64lo/t64hi
    armEmitMulopRRR(state, bits, op, t64hi, t64lo, rm, rs, 0);

    // add in rdhi
    armEmitBinopRR(state, bits, vmi_ADD, t64lo, rdhi, &tf);
    armEmitBinopRC(state, bits, vmi_ADC, t64hi, 0,    &tf);

    // add in rdlo
    armEmitBinopRR(state, bits, vmi_ADD, t64lo, rdlo, &tf);
    armEmitBinopRC(state, bits, vmi_ADC, t64hi, 0,    &tf);

    // move result to rdlo/rdhi
    armEmitMoveRR(state, bits, rdhi, t64hi);
    armEmitMoveRR(state, bits, rdlo, t64lo);

    freeTemp64(state);
}

////////////////////////////////////////////////////////////////////////////////
// SYNCHRONIZATION INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Generate exclusive access tag address ra+offset in register rtag
//
static void generateEATag(
    armMorphStateP state,
    Uns32          offset,
    vmiReg         rtag,
    vmiReg         ra
) {
    Uns32 bits = ARM_GPR_BITS;
    Uns32 mask = state->arm->exclusiveTagMask;

    armEmitBinopRRC(state, bits, vmi_ADD, rtag, ra, offset, 0);
    armEmitBinopRC(state, bits, vmi_AND, rtag, mask, 0);
}

//
// Emit code to start exclusive access to address ra+offset
//
inline static void startEA(armMorphStateP state, Uns32 offset, vmiReg ra) {
    generateEATag(state, offset, ARM_EA_TAG, ra);
}

//
// Validate the exclusive access and jump to label 'done' if it is invalid,
// setting rd to 1
//
static vmiLabelP validateEA(
    armMorphStateP state,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rd
) {
    Uns32     bits    = ARM_GPR_BITS;
    Uns32     memBits = state->info.sz*8;
    vmiLabelP done    = armEmitNewLabel();
    vmiLabelP ok      = armEmitNewLabel();
    vmiReg    t       = getTemp(state);

    // generate any store exception prior to exclusive access tag check
    armEmitTryStoreRC(state, memBits, offset, ra);

    // generate exclusive access tag for this address
    generateEATag(state, offset, t, ra);

    // do load and store tags match?
    armEmitCompareRR(state, bits, vmi_COND_EQ, ARM_EA_TAG, t, t);

    // commit store if tags match
    armEmitCondJumpLabel(t, True, ok);

    // indicate store failed
    armEmitMoveRC(state, bits, rd, 1);

    // jump to instruction end
    armEmitUncondJumpLabel(done);

    // here to commit store
    armEmitInsertLabel(ok);

    return done;
}

//
// Do actions required to terminate exclusive access
//
static void clearEA(armMorphStateP state) {

    // exclusiveTag becomes ARM_NO_TAG to indicate no active access
    armEmitMoveRC(state, ARM_GPR_BITS, ARM_EA_TAG, ARM_NO_TAG);
}

//
// Do actions required to complete exclusive access
//
static void endEA(armMorphStateP state, vmiReg rd, vmiLabelP done) {

    // indicate store succeeded
    armEmitMoveRC(state, ARM_GPR_BITS, rd, 0);

    // insert target label for aborted stores
    armEmitInsertLabel(done);

    // terminate exclusive access
    clearEA(state);
}

//
// Emit code for LDREX*
//
ARM_MORPH_FN(armEmitLDREX) {

    // indicate LDREX is now active at address r2+offset
    startEA(state, state->info.c, GET_RS(state, r2));

    // emit load
    armEmitLDRI(state);
}

//
// Emit code for STREX*
//
ARM_MORPH_FN(armEmitSTREX) {

    // validate STREX attempt at address r3+offset
    vmiReg    rd   = GET_RD(state, r1);
    vmiLabelP done = validateEA(state, state->info.c, GET_RS(state, r3), rd);

    // move down rt and rn so that they are in the required positions for a
    // normal store
    state->info.r1 = state->info.r2;
    state->info.r2 = state->info.r3;

    // emit store
    armEmitSTRI(state);

    // complete STREX attempt
    endEA(state, rd, done);
}

//
// Emit code for CLREX
//
ARM_MORPH_FN(armEmitCLREX) {
    clearEA(state);
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Function to set PRIMASK and/or FAULTMASK
//
static void setMasks(armP arm, armFlagAffect faff) {

    Bool updateState = False;

    // set PRIMASK if required
    if((faff&ARM_FAFF_I) && !arm->sregs.PRIMASK) {
        arm->sregs.PRIMASK = 1;
        updateState        = True;
    }

    // set FAULTMASK if required
    if((faff&ARM_FAFF_F) && !arm->sregs.FAULTMASK && (arm->executionPriority>-1)) {
        arm->sregs.FAULTMASK = 1;
        updateState          = True;
    }

    // update processor state if required
    if(updateState) {
        armRefreshExecutionPriority(arm);
    }
}

//
// Function to clear PRIMASK and/or FAULTMASK
//
static void clearMasks(armP arm, armFlagAffect faff) {

    Bool updateState = False;

    // set PRIMASK if required
    if((faff&ARM_FAFF_I) && arm->sregs.PRIMASK) {
        arm->sregs.PRIMASK = 0;
        updateState        = True;
    }

    // set FAULTMASK if required
    if((faff&ARM_FAFF_F) && arm->sregs.FAULTMASK) {
        arm->sregs.FAULTMASK = 0;
        updateState          = True;
    }

    // update processor state if required
    if(updateState) {
        armRefreshExecutionPriority(arm);
    }
}

//
// Emit code for CPS
//
ARM_MORPH_FN(armEmitCPS) {

    if(!IN_USER_MODE(state->arm)) {

        armFlagAffect faff = state->info.faff;
        Bool          set  = (state->info.fact==ARM_FACT_ID);

        // emit call to set or clear the masks
        armEmitArgProcessor(state);
        armEmitArgUns32(state, faff);
        armEmitCall(state, set ? (vmiCallFn)setMasks : (vmiCallFn)clearMasks);

        // terminate the current code block
        armEmitEndBlock();
    }
}


////////////////////////////////////////////////////////////////////////////////
// BRANCH INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for CPZ/CBNZ
//
ARM_MORPH_FN(armEmitCBZ) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rn   = GET_RS(state, r1);
    vmiReg tf   = getTemp(state);

    // do the comparison
    armEmitCompareRC(state, bits, vmi_COND_Z, rn, 0, tf);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, False, False, True);

    // do the jump
    armEmitCondJump(state, &ji, tf, state->attrs->jumpIfTrue);
}

//
// Emit code for TBB/TBH
//
ARM_MORPH_FN(armEmitTB) {

    Uns32  bits    = ARM_GPR_BITS;
    Uns32  sz      = state->info.sz;
    Uns32  memBits = sz*8;
    vmiReg rn      = GET_RS(state, r1);
    vmiReg rm      = GET_RS(state, r2);
    vmiReg t1      = getTemp(state);
    Uns32  shift   = 0;

    // convert from size to shift
    while(sz>1) {
        sz >>= 1;
        shift++;
    }

    // get offset, shifted if required
    armEmitBinopRRC(state, bits, vmi_SHL, t1, rm, shift, 0);

    // compose full address
    armEmitBinopRR(state, bits, vmi_ADD, t1, rn, 0);

    // load zero-extended offset from table and double it
    armEmitLoadRRO(state, memBits, 0, t1, t1, False, False);
    armEmitBinopRR(state, bits, vmi_ADD, t1, t1, 0);

    // add to effective PC to get target address
    armEmitBinopRR(state, bits, vmi_ADD, t1, getRS(state, ARM_REG_PC), 0);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, False, False, True);

    // do the jump
    armEmitUncondJumpReg(state, &ji, t1);
}


////////////////////////////////////////////////////////////////////////////////
// BASIC MEDIA INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Common routine for USAD8 and USADA8
//
static void emitUSAD8Int(armMorphStateP state, vmiReg ra) {

    Uns32  bits     = ARM_GPR_BITS;
    Uns32  partBits = 8;
    vmiReg rd       = GET_RD(state, r1);
    vmiReg rn       = GET_RS(state, r2);
    vmiReg rm       = GET_RS(state, r3);
    vmiReg t1       = newTemp32(state);
    vmiReg t2       = newTemp32(state);
    vmiReg t3       = getTemp(state);
    Uns32  i;

    // clear result accumulators
    armEmitMoveRR(state, bits, t1, ra);

    for(i=0; i<(bits/partBits); i++) {

        // get zero-extended arguments in temporaries
        armEmitMoveExtendRR(state, bits, t2, partBits, rn, False);
        armEmitMoveExtendRR(state, bits, t3, partBits, rm, False);

        // perform subtraction
        armEmitBinopRR(state, bits, vmi_SUB, t2, t3, 0);

        // get absolute result
        armEmitUnopRR(state, bits, vmi_ABS, t2, t2, 0);

        // update accumulated result
        armEmitBinopRR(state, bits, vmi_ADD, t1, t2, 0);

        // step to next register pair
        rn = VMI_REG_DELTA(rn, 1);
        rm = VMI_REG_DELTA(rm, 1);
    }

    // assign result register
    armEmitMoveRR(state, bits, rd, t1);

    // free allocated temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Common routine for SBFX and UBFX
//
static void emitBFXInt(armMorphStateP state, vmiBinop op) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    Uns32  lsb  = state->info.c;
    Uns32  msb  = lsb + state->info.w - 1;

    if(msb<ARM_GPR_BITS) {

        Uns32 leftShift  = bits - msb - 1;
        Uns32 rightShift = leftShift + lsb;

        armEmitBinopRRC(state, bits, vmi_SHL, rd, rn, leftShift,  0);
        armEmitBinopRRC(state, bits, op,      rd, rd, rightShift, 0);
    }
}

//
// Emit code for USAD8
//
ARM_MORPH_FN(armEmitUSAD8) {
    emitUSAD8Int(state, VMI_NOREG);
}

//
// Emit code for USADA8
//
ARM_MORPH_FN(armEmitUSADA8) {
    emitUSAD8Int(state, GET_RS(state, r4));
}


//
// Emit code for SBFX
//
ARM_MORPH_FN(armEmitSBFX) {
    emitBFXInt(state, vmi_SAR);
}

//
// Emit code for BFC
//
ARM_MORPH_FN(armEmitBFC) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RS(state, r1);
    Uns32  lsb   = state->info.c;
    Int32  width = state->info.w;

    if(width<=0) {

        // no action

    } else if(width==bits) {

        // optimize to a move
        armEmitMoveRC(state, bits, rd, 0);

    } else {

        // general case
        Uns32 mask1 = ((1<<width)-1);
        Uns32 mask2 = ~(mask1<<lsb);

        armEmitBinopRC(state, bits, vmi_AND, rd, mask2, 0);
    }
}

//
// Emit code for BFI
//
ARM_MORPH_FN(armEmitBFI) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RS(state, r1);
    vmiReg rs    = GET_RS(state, r2);
    Uns32  lsb   = state->info.c;
    Int32  width = state->info.w;

    if(width<=0) {

        // no action

    } else if(width==bits) {

        // optimize to a move
        armEmitMoveRR(state, bits, rd, rs);

    } else {

        // general case
        vmiReg t     = getTemp(state);
        Uns32  mask1 = ((1<<width)-1);
        Uns32  mask2 = ~(mask1<<lsb);

        armEmitBinopRRC(state, bits, vmi_AND, t, rs, mask1, 0);
        armEmitBinopRC(state, bits, vmi_SHL, t, lsb, 0);
        armEmitBinopRC(state, bits, vmi_AND, rd, mask2, 0);
        armEmitBinopRR(state, bits, vmi_OR, rd, t, 0);
    }
}

//
// Emit code for UBFX
//
ARM_MORPH_FN(armEmitUBFX) {
    emitBFXInt(state, vmi_SHR);
}


////////////////////////////////////////////////////////////////////////////////
// PARALLEL ADD/SUBTRACT INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to clear GE flags in the CPSR if required
//
static void emitClearGEFlags(armMorphStateP state) {

    if(state->attrs->setGE) {
        armEmitBinopRC(state, ARM_GPR_BITS, vmi_AND, ARM_PSR, ~PSR_GE30, 0);
    }
}

//
// Emit code to perform part of a parallel operation
//
static void emitParallelBinopInt(
    armMorphStateP state,
    Uns32          partBits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rn,
    vmiReg         rm,
    Uns32          delta1,
    Uns32          delta2,
    Uns32          apsrMask
) {
    Bool     setGE   = state->attrs->setGE;
    Bool     sextend = state->attrs->sextend;
    Bool     halve   = state->attrs->halve;
    vmiFlags flags   = VMI_NOFLAGS;
    vmiReg   tf      = newTemp32(state);
    vmiReg   rnTmp;
    vmiReg   rmTmp;
    vmiReg   rdTmp;
    Uns32    opBits;

    // set up flags to generate CPSR greater-or-equal bits if required
    if(!(setGE || halve)) {
        // no action
    } else if(sextend) {
        flags.f[vmi_SF] = tf;
    } else {
        flags.f[vmi_CF] = flags.cin = tf;
    }

    // get shift to appropriate part of each register
    rd = VMI_REG_DELTA(rd, delta1);
    rn = VMI_REG_DELTA(rn, delta1);
    rm = VMI_REG_DELTA(rm, delta2);

    // argument format depends on whether sign extension is required
    if(sextend) {

        // sign extension required: perform operation on extended arguments
        opBits = ARM_GPR_BITS;
        rdTmp  = newTemp32(state);
        rnTmp  = rdTmp;
        rmTmp  = getTemp(state);

        // sign extend arguments into temporaries
        armEmitMoveExtendRR(state, opBits, rnTmp, partBits, rn, True);
        armEmitMoveExtendRR(state, opBits, rmTmp, partBits, rm, True);

    } else {

        // use original unextended arguments
        opBits = partBits;
        rdTmp  = rd;
        rnTmp  = rn;
        rmTmp  = rm;
    }

    // do the operation
    armEmitBinopRRR(state, opBits, op, rdTmp, rnTmp, rmTmp, &flags);

    // update CPSR greater-or-equal bits if required
    if(setGE) {

        vmiLabelP done = armEmitNewLabel();

        // skip flag update if required
        armEmitCondJumpLabel(tf, (sextend || (op!=vmi_ADD)), done);

        // include apsrMask in CPSR register
        armEmitBinopRC(state, ARM_GPR_BITS, vmi_OR, ARM_PSR, apsrMask, 0);

        // jump to here if flag update is not required
        armEmitInsertLabel(done);
    }

    // halve results if required
    if(!halve) {
        // no action
    } else if(sextend) {
        armEmitBinopRC(state, opBits, vmi_SHR, rdTmp, 1, 0);
    } else {
        armEmitBinopRC(state, opBits, vmi_RCR, rdTmp, 1, &flags);
    }

    // write back temporary if required
    if(sextend) {
        armEmitMoveRR(state, partBits, rd, rdTmp);
        freeTemp32(state);
    }

    // free temporary flag
    freeTemp32(state);
}

//
// Emit code for parallel add/subtract of 8-bit data
//
ARM_MORPH_FN(armEmitParallelBinop8) {

    Uns32    bits = ARM_GPR_BITS/4;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    vmiBinop op   = state->attrs->binop;

    // clear GE flags
    emitClearGEFlags(state);

    // perform the parallel operations
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 0, 0, PSR_GE0);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 1, 1, PSR_GE1);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 2, 2, PSR_GE2);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 3, 3, PSR_GE3);
}

//
// Emit code for parallel add/subtract of 16-bit data
//
ARM_MORPH_FN(armEmitParallelBinop16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    Bool     exch = state->attrs->exchange;
    vmiBinop op1  = state->attrs->binop;
    vmiBinop op2  = state->attrs->binop2;
    vmiReg   tmp  = newTemp32(state);

    // save rm in temporary if exchange required and it is clobbered
    if(exch && VMI_REG_EQUAL(rd, rm)) {
        armEmitMoveRR(state, ARM_GPR_BITS, tmp, rm);
        rm = tmp;
    }

    // clear GE flags
    emitClearGEFlags(state);

    // perform the parallel operations
    emitParallelBinopInt(state, bits, op1, rd, rn, rm, 0, exch?2:0, PSR_GE10);
    emitParallelBinopInt(state, bits, op2, rd, rn, rm, 2, exch?0:2, PSR_GE32);

    // free temporary
    freeTemp32(state);
}


////////////////////////////////////////////////////////////////////////////////
// PACKING, UNPACKING, SATURATION AND REVERSAL INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SSAT/SSAT16
//
static void emitSSAT(
    armMorphStateP state,
    vmiBinop       so,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         rn
) {
    vmiReg    t1    = newTemp32(state);
    vmiReg    tf    = getTemp(state);
    Uns32     shift = state->info.c;
    Uns32     width = state->info.w;
    vmiFlags  flags = getZFFlags(tf);
    vmiLabelP done  = armEmitNewLabel();

    // create bitmasks
    Uns32 signMask = 1<<(width-1);
    Uns32 maxMask  = signMask-1;
    Uns32 propMask = ~maxMask;

    // mask propMask to the operand width
    if(bits!=32) {
        propMask &= (1<<bits)-1;
    }

    // get shifted argument
    armEmitBinopRRC(state, bits, so, rd, rn, shift, 0);

    // mask with propMask
    armEmitBinopRRC(state, bits, vmi_AND, t1, rd, propMask, &flags);

    // no action if the top bits are all zero
    armEmitCondJumpLabel(tf, True, done);

    // no action if the top bits are all one
    armEmitCompareRCJumpLabel(bits, vmi_COND_EQ, t1, propMask, done);

    // saturation required: set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // is the argument signed?
    armEmitCompareRC(state, bits, vmi_COND_S, rd, 0, tf);

    // saturate result based on original sign
    armEmitCondMoveRCC(state, bits, tf, True, rd, propMask, maxMask);

    // jump to here when done
    armEmitInsertLabel(done);

    // free temporaries
    freeTemp32(state);
}

//
// Emit code for USAT/USAT16
//
static void emitUSAT(
    armMorphStateP state,
    vmiBinop       so,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         rn
) {
    vmiReg    tf    = getTemp(state);
    Uns32     shift = state->info.c;
    Uns32     width = state->info.w;
    vmiLabelP done  = armEmitNewLabel();

    // create bitmasks
    Uns32 maxMask  = (1<<width)-1;
    Uns32 propMask = ~maxMask;

    // mask propMask to the operand width
    if(bits!=32) {
        propMask &= (1<<bits)-1;
    }

    // get shifted argument
    armEmitBinopRRC(state, bits, so, rd, rn, shift, 0);

    // no action if the top bits are all zero
    armEmitTestRCJumpLabel(bits, vmi_COND_Z, rd, propMask, done);

    // saturation required: set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // is the argument signed?
    armEmitCompareRC(state, bits, vmi_COND_S, rd, 0, tf);

    // saturate result based on original sign
    armEmitCondMoveRCC(state, bits, tf, True, rd, 0, maxMask);

    // jump to here when done
    armEmitInsertLabel(done);
}

//
// These specify byte offsets equivalent to ROR values in SXTAH etc
//
const static Uns32 rorDelta1[] = {0, 1, 2, 3};
const static Uns32 rorDelta2[] = {2, 3, 0, 1};

//
// Extract byte into temporary with sign extension
//
static void emitExtByte(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    Bool           sextend
) {
    Uns32 bits   = ARM_GPR_BITS;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 8, VMI_REG_DELTA(rm,delta1), sextend);
}

//
// Extract word into temporary with sign extension
//
static void emitExtWord(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    Bool           sextend
) {
    Uns32 bits   = ARM_GPR_BITS;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];

    // if the word straddles the register boundary, handle it specially
    if(delta1==3) {
        armEmitBinopRRC(state, bits, vmi_ROR, t1, rm, 24, 0);
        rm     = t1;
        delta1 = 0;
    }

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 16, VMI_REG_DELTA(rm,delta1), sextend);
}

//
// Extract byte pair into temporaries with sign extension
//
static void emitExtBytePair(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    vmiReg         t2,
    Bool           sextend
) {
    Uns32 bits   = 16;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];
    Uns32 delta2 = rorDelta2[dIndex];

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 8, VMI_REG_DELTA(rm,delta1), sextend);
    armEmitMoveExtendRR(state, bits, t2, 8, VMI_REG_DELTA(rm,delta2), sextend);
}

//
// Emit code for SXTAB/UXTAB
//
static void emitXTAB(armMorphStateP state, Bool sextend) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = getTemp(state);

    // extract extended byte into temporary
    emitExtByte(state, rm, t1, sextend);

    // create result from 32-bit addition
    armEmitBinopRRR(state, bits, vmi_ADD, rd, rn, t1, 0);
}

//
// Emit code for SXTAH/UXTAH
//
static void emitXTAH(armMorphStateP state, Bool sextend) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = getTemp(state);

    // extract extended word into temporary
    emitExtWord(state, rm, t1, sextend);

    // create result from 32-bit addition
    armEmitBinopRRR(state, bits, vmi_ADD, rd, rn, t1, 0);
}

//
// Emit code for SXTAB16/UXTAB16
//
static void emitXTAB16(armMorphStateP state, Bool sextend) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rnL  = GET_RS(state, r2);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rnH  = VMI_REG_DELTA(rnL, 2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = newTemp32(state);
    vmiReg t2   = getTemp(state);

    // extract extended bytes into temporaries
    emitExtBytePair(state, rm, t1, t2, sextend);

    // create result from two 16-bit additions
    armEmitBinopRRR(state, bits, vmi_ADD, rdL, rnL, t1, 0);
    armEmitBinopRRR(state, bits, vmi_ADD, rdH, rnH, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SXTB16/UXTB16
//
static void emitXTB16(armMorphStateP state, Bool sextend) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rm   = GET_RS(state, r2);
    vmiReg t1   = newTemp32(state);
    vmiReg t2   = getTemp(state);

    // extract extended bytes into temporaries
    emitExtBytePair(state, rm, t1, t2, sextend);

    // create result from two 16-bit moves
    armEmitMoveRR(state, bits, rdL, t1);
    armEmitMoveRR(state, bits, rdH, t2);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SXTB/UXTB
//
static void emitXTB(armMorphStateP state, Bool sextend) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    emitExtByte(state, rm, rd, sextend);
}

//
// Emit code for SXTH/UXTH
//
static void emitXTH(armMorphStateP state, Bool sextend) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    emitExtWord(state, rm, rd, sextend);
}

//
// Emit code for PKHBT
//
ARM_MORPH_FN(armEmitPKHBT) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rd    = GET_RD(state, r1);
    vmiReg   rn    = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   t1    = newTemp32(state);
    vmiReg   t2    = getTemp(state);
    Uns32    shift = state->info.c;
    vmiBinop so    = mapShiftOp(state->info.so);

    // create lower half of the result
    armEmitBinopRRC(state, bits, vmi_AND, t1, rn, 0xffff, 0);

    // create upper half of the result
    armEmitBinopRRC(state, bits, so, t2, rm, shift, 0);

    // mask upper half of the result if required
    if(shift<16) {
        armEmitBinopRC(state, bits, vmi_AND, t2, 0xffff0000, 0);
    }

    // create combined result
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for PKHTB
//
ARM_MORPH_FN(armEmitPKHTB) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rd    = GET_RD(state, r1);
    vmiReg   rn    = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   t1    = newTemp32(state);
    vmiReg   t2    = getTemp(state);
    Uns32    shift = state->info.c;
    vmiBinop so    = mapShiftOp(state->info.so);

    // create upper half of the result
    armEmitBinopRRC(state, bits, vmi_AND, t1, rn, 0xffff0000, 0);

    // create lower half of the result
    armEmitBinopRRC(state, bits, so, t2, rm, shift, 0);
    armEmitBinopRC(state, bits, vmi_AND, t2, 0xffff, 0);

    // create combined result
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SSAT
//
ARM_MORPH_FN(armEmitSSAT) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop so   = mapShiftOp(state->info.so);
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitSSAT(state, so, bits, rd, rn);
}

//
// Emit code for SSAT16
//
ARM_MORPH_FN(armEmitSSAT16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiBinop so   = vmi_SHR;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitSSAT(state, so, bits, VMI_REG_DELTA(rd, 0), VMI_REG_DELTA(rn, 0));
    emitSSAT(state, so, bits, VMI_REG_DELTA(rd, 2), VMI_REG_DELTA(rn, 2));
}

//
// Emit code for USAT
//
ARM_MORPH_FN(armEmitUSAT) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop so   = mapShiftOp(state->info.so);
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitUSAT(state, so, bits, rd, rn);
}

//
// Emit code for USAT16
//
ARM_MORPH_FN(armEmitUSAT16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiBinop so   = vmi_SHR;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitUSAT(state, so, bits, VMI_REG_DELTA(rd, 0), VMI_REG_DELTA(rn, 0));
    emitUSAT(state, so, bits, VMI_REG_DELTA(rd, 2), VMI_REG_DELTA(rn, 2));
}

//
// Emit code for SXTAB
//
ARM_MORPH_FN(armEmitSXTAB) {
    emitXTAB(state, True);
}

//
// Emit code for UXTAB
//
ARM_MORPH_FN(armEmitUXTAB) {
    emitXTAB(state, False);
}

//
// Emit code for SXTAB16
//
ARM_MORPH_FN(armEmitSXTAB16) {
    emitXTAB16(state, True);
}

//
// Emit code for UXTAB16
//
ARM_MORPH_FN(armEmitUXTAB16) {
    emitXTAB16(state, False);
}

//
// Emit code for SXTAH
//
ARM_MORPH_FN(armEmitSXTAH) {
    emitXTAH(state, True);
}

//
// Emit code for UXTAH
//
ARM_MORPH_FN(armEmitUXTAH) {
    emitXTAH(state, False);
}

//
// Emit code for SXTB
//
ARM_MORPH_FN(armEmitSXTB) {
    emitXTB(state, True);
}

//
// Emit code for UXTB
//
ARM_MORPH_FN(armEmitUXTB) {
    emitXTB(state, False);
}

//
// Emit code for SXTB16
//
ARM_MORPH_FN(armEmitSXTB16) {
    emitXTB16(state, True);
}

//
// Emit code for UXTB16
//
ARM_MORPH_FN(armEmitUXTB16) {
    emitXTB16(state, False);
}

//
// Emit code for SXTH
//
ARM_MORPH_FN(armEmitSXTH) {
    emitXTH(state, True);
}

//
// Emit code for UXTH
//
ARM_MORPH_FN(armEmitUXTH) {
    emitXTH(state, False);
}

//
// Emit code for SEL
//
ARM_MORPH_FN(armEmitSEL) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1  = newTemp32(state);
    vmiReg t2  = getTemp(state);

    // select GE bits from CPSR
    armEmitBinopRRC(state, bits, vmi_SHR, t1, ARM_PSR, 16, 0);
    armEmitBinopRC (state, bits, vmi_AND, t1, 0xf, 0);

    // convert from 0..15 to bitmask
    armEmitBinopRC(state, bits, vmi_IMUL, t1, 0x00204081, 0);
    armEmitBinopRC(state, bits, vmi_AND,  t1, 0x01010101, 0);
    armEmitBinopRC(state, bits, vmi_IMUL, t1, 0xff,       0);

    // get components from each source
    armEmitBinopRRR(state, bits, vmi_AND,  t2, rn, t1, 0);
    armEmitBinopRRR(state, bits, vmi_ANDN, t1, rm, t1, 0);

    // assign result register
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for REV
//
ARM_MORPH_FN(armEmitREV) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    armEmitUnopRR(state, bits, vmi_SWP, rd, rm, 0);
}

//
// Emit code for REV16
//
ARM_MORPH_FN(armEmitREV16) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rmL  = GET_RS(state, r2);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rmH  = VMI_REG_DELTA(rmL, 2);

    armEmitUnopRR(state, bits, vmi_SWP, rdL, rmL, 0);
    armEmitUnopRR(state, bits, vmi_SWP, rdH, rmH, 0);
}

//
// Emit code for REVSH
//
ARM_MORPH_FN(armEmitREVSH) {

    Uns32  bits = 16;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    armEmitUnopRR(state, bits, vmi_SWP, rd, rm, 0);
    armEmitMoveExtendRR(state, ARM_GPR_BITS, rd, bits, rd, True);
}

//
// This array will hold bit-reversed byte values for RBIT
//
static Uns8 rbit8[256];

//
// Return bit-reversed value
//
static Uns32 doRBIT(Uns32 value) {

    union {Uns32 u32; Uns8 u8[4];} u1 = {value};
    union {Uns32 u32; Uns8 u8[4];} u2;

    // generate reversed result a byte at a time using the lookup table
    u2.u8[0] = rbit8[u1.u8[3]];
    u2.u8[1] = rbit8[u1.u8[2]];
    u2.u8[2] = rbit8[u1.u8[1]];
    u2.u8[3] = rbit8[u1.u8[0]];

    // return the reversed result
    return u2.u32;
}

//
// Emit code for RBIT
//
ARM_MORPH_FN(armEmitRBIT) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    static Bool init;

    // set up rbit8 table if required
    if(!init) {

        Uns32 i;

        for(i=0; i<256; i++) {

            Uns8  byte   = i;
            Uns8  result = 0;
            Uns32 j;

            for(j=0; j<8; j++) {
                result = (result<<1) | (byte&1);
                byte >>= 1;
            }

            rbit8[i] = result;
        }

        init = True;
    }

    // emit embedded call to perform operation
    armEmitArgReg(state, bits, rm);
    armEmitCallResult(state, (vmiCallFn)doRBIT, bits, rd);
}

////////////////////////////////////////////////////////////////////////////////
// SIGNED MULTIPLY INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to perform dual multiply, with results in t1 and t2
//
static void emitDualMultiply(
    armMorphStateP state,
    vmiReg         rnL,
    vmiReg         rmL,
    vmiReg         t1,
    vmiReg         t2
) {
    Uns32  bits = ARM_GPR_BITS;
    vmiReg rnH  = VMI_REG_DELTA(rnL, 2);
    vmiReg rmH  = VMI_REG_DELTA(rmL, 2);
    vmiReg t3   = getTemp(state);

    // exchange arguments if required
    if(state->attrs->exchange) {
        vmiReg tmp = rmL; rmL = rmH; rmH = tmp;
    }

    // do first multiply
    armEmitMoveExtendRR(state, bits, t1, 16, rnL, True);
    armEmitMoveExtendRR(state, bits, t3, 16, rmL, True);
    armEmitBinopRR(state, bits, vmi_IMUL, t1, t3, 0);

    // do second multiply
    armEmitMoveExtendRR(state, bits, t2, 16, rnH, True);
    armEmitMoveExtendRR(state, bits, t3, 16, rmH, True);
    armEmitBinopRR(state, bits, vmi_IMUL, t2, t3, 0);
}

//
// Emit code for SMLAD/SMLSD
//
ARM_MORPH_FN(armEmitSMLXD) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rnL  = GET_RS(state, r2);
    vmiReg   rmL  = GET_RS(state, r3);
    vmiReg   ra   = GET_RS(state, r4);
    vmiReg   t1   = newTemp32(state);
    vmiReg   t2   = newTemp32(state);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // combine results, setting CPSR.Q if overflow (ADD only, special case of
    // 0x8000*0x8000 + 0x8000*0x8000)
    if(op==vmi_ADD) {
        emitOpSetQ(state, bits, op, t1, t1, t2);
    } else {
        armEmitBinopRR(state, bits, op, t1, t2, 0);
    }

    // accumulate, setting CPSR.Q if overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, ra, t1);

    // free temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Emit code for SMUAD/SMUSD
//
ARM_MORPH_FN(armEmitSMUXD) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rnL  = GET_RS(state, r2);
    vmiReg   rmL  = GET_RS(state, r3);
    vmiReg   t1   = newTemp32(state);
    vmiReg   t2   = newTemp32(state);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // generate result, setting CPSR.Q if overflow (ADD only)
    if(op==vmi_ADD) {
        emitOpSetQ(state, bits, op, rd, t1, t2);
    } else {
        armEmitBinopRRR(state, bits, op, rd, t1, t2, 0);
    }

    // free temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Emit code for SMLALD/SMLSLD
//
ARM_MORPH_FN(armEmitSMLXLD) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiBinop op    = state->attrs->binop;
    vmiReg   rdLo  = GET_RD(state, r1);
    vmiReg   rdHi  = GET_RD(state, r2);
    vmiReg   rnL   = GET_RS(state, r3);
    vmiReg   rmL   = GET_RS(state, r4);
    vmiReg   t1    = newTemp64(state);
    vmiReg   t2    = newTemp32(state);
    vmiReg   tf    = t2;
    vmiFlags flags = getCFFlags(tf);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // extend results to 64 bits
    armEmitMoveExtendRR(state, 64, t1, bits, t1, True);
    armEmitMoveExtendRR(state, 64, t2, bits, t2, True);

    // add/subtract extended results
    armEmitBinopRR(state, 64, op, t1, t2, 0);

    // add total to accumulator
    armEmitBinopRR(state, bits, vmi_ADD, rdLo, VMI_REG_DELTA(t1, 0), &flags);
    armEmitBinopRR(state, bits, vmi_ADC, rdHi, VMI_REG_DELTA(t1, 4), &flags);

    // free temporaries
    freeTemp32(state);
    freeTemp64(state);
}

//
// Emit code for SMMLX
//
ARM_MORPH_FN(armEmitSMMLX) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    vmiReg   t1L  = newTemp64(state);
    vmiReg   t1H  = VMI_REG_DELTA(t1L, 4);

    // do the multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, t1H, t1L, rn, rm, 0);

    // accumulate if required
    if(state->attrs->accumulate) {

        vmiReg t2L = getTemp(state);
        vmiReg t2H = VMI_REG_DELTA(t2L, 4);

        armEmitMoveRC(state, bits, t2L, 0);
        armEmitMoveRR(state, bits, t2H, GET_RS(state, r4));

        armEmitBinopRR(state, 64, op, t1L, t2L, 0);
    }

    // round if required
    if(state->attrs->round) {
        armEmitBinopRC(state, 64, vmi_ADD, t1L, 0x80000000, 0);
    }

    // assign to result
    armEmitMoveRR(state, bits, rd, t1H);

    // free temporary
    freeTemp64(state);
}

////////////////////////////////////////////////////////////////////////////////
// VFP Utility functions
////////////////////////////////////////////////////////////////////////////////

//
// Emit NOCP Usage Fault if FPU is disabled
//
#define VFP_DISABLED(_S) emitUsageFault(state, EXC_UNDEF_NOCP); return False;

//
// This is the ARM CheckVFPEnabled psuedo-code primitive
//
static Bool checkVFPEnabled(armMorphStateP state) {

    armP  arm        = state->arm;
    Bool  inUserMode = IN_USER_MODE(arm);
    Uns32 cp10Enable = SCS_FIELD(arm, CPACR, cp10);

    // this code block is dependent on the enable state of coprocessor 10
    armEmitValidateBlockMask(ARM_BM_CP10);

    // check CPACR for permission to use cp10 (and cp11) in the current user/privilege mode
    if(inUserMode && !(cp10Enable&2)) {
        VFP_DISABLED(state);
    } else if(!inUserMode && !(cp10Enable&1)) {
        VFP_DISABLED(state);
    }

    return True;
}

//
// This is the ARM ExecuteFPCheck primitive
//
static Bool executeFPCheck(armMorphStateP state) {

    armP  arm = state->arm;

    VMI_ASSERT(FPU_PRESENT(arm), "FP instruction when no FPU present");

    if (!checkVFPEnabled(state)) {

        // Access to CP10 not enabled
        return False;

    } else {

        Bool endMorphBlock = False;

        // this code block is dependent on the CONTROL.FPCA, FPCCR.ASPEN and FPCCR.LSPACT bits
        armEmitValidateBlockMask(ARM_BM_FPCA|ARM_BM_ASPEN|ARM_BM_LSPACT);

        // If FP lazy context save is enabled then save state
        if (SCS_FIELD(arm, FPCCR, LSPACT)) {

            // Preserve the FP state in the saved stack space
            armEmitArgProcessor(state);
            armEmitCall(state, (vmiCallFn)armPreserveFPState);

            // preserveFPState will clear LSPACT so should end block
            endMorphBlock = True;
        }

        // If ASPEN is on then check is FPCA is off. If it is, then this is the
        // first FP instruction in this context
        if (SCS_FIELD(arm, FPCCR, ASPEN) && !CONTROL_FIELD(arm, FPCA)) {

            Uns32 bits = ARM_GPR_BITS;

            // copy FPDSCR values to FPSCR
            armEmitArgProcessor(state);
            armEmitArgReg(state, bits, ARM_SCS_REG(SCS_ID(FPDSCR)));
            armEmitArgUns32(state, SCS_WRITE_MASK_FPDSCR);
            armEmitCall(state, (vmiCallFn)armWriteFPSCR);

            // Set CONTROL.FPCA bit
            armEmitBinopRC(state, 32, vmi_OR, ARM_CONTROL, CONTROL_FPCA, 0);

            // CONTROL.FPCA has changed so should end block
            endMorphBlock = True;
        }

        if (endMorphBlock) {
            // terminate the code block (CONTROL.FPCA or FPCCR.LSPACT has changed)
            armEmitEndBlock();
        }
    }

    // VFP is available
    return True;

}

//
// Load the Flt64 register with the floating point constant value of 2^n
//
static vmiReg getFPConstPower2Flt64(armMorphStateP state, Uns32 n) {

    Uns64  twoN = (1ULL << n);
    vmiReg r    = newTemp64(state);

    union {Flt64 f64; Uns64 u64;} u = {f64:twoN};

    armEmitMoveRC(state, 64, r, u.u64);

    return r;
}

//
// Load the Flt80 register with the floating point constant value of 2^n
//
static vmiReg getFPConstPower2Flt80(armMorphStateP state, Uns32 n) {

    Uns64  twoN = (1ULL << n);
    vmiReg r    = newTemp128(state);

    union {Flt80 f80; Uns64 u64; Uns16 u16[8];} u = {f80:twoN};

    armEmitMoveRC(state, 64, r, u.u64);
    armEmitMoveRC(state, 16, VMI_REG_DELTA(r, 8), u.u16[4]);

    return r;
}

//
// Get floating point operation type for ARM integer type from size of operand in bytes
//
static vmiFType bytesToIType(Uns32 ebytes, Bool isSigned) {
    switch(ebytes) {
        case 2:
            return isSigned ? vmi_FT_16_INT : vmi_FT_16_UNS;
        case 4:
            return isSigned ? vmi_FT_32_INT : vmi_FT_32_UNS;
        default:
            VMI_ABORT("%s: unimplemented size for floating point type: %d bytes", FUNC_NAME, ebytes);
            return 0;    // Not reached
    }
}

//
// Get floating point operation type for ARM floating point type from size of operand in bytes
//
static vmiFType bytesToFType(Uns32 ebytes) {
    switch(ebytes) {
        case 4:
            return vmi_FT_32_IEEE_754;
        case 8:
            return vmi_FT_64_IEEE_754;
        default:
            VMI_ABORT("%s: unimplemented size for floating point type: %d bytes", FUNC_NAME, ebytes);
            return 0;    // Not reached
    }
}


//
// This is FPNeg from psuedo-code
// Negate the value in the register by toggling the sign bit
//
static void FPNeg(armMorphStateP state, vmiReg dest, vmiReg src, Uns32 size) {

    Uns64 signBit = 1ULL << (size-1);

    armEmitBinopRRC(state, size, vmi_XOR, dest, src, signBit, 0);
}

//
// This is FixedToFP from psuedo-code
// Note: fpscr_controlled selection is done automatically in armEmit... call
//
static void fixedToFP(
    armMorphStateP state,
    vmiReg         result,
    Uns32          resultBytes,
    vmiReg         operand,
    Uns32          operandBytes,
    Uns32          fracBits,
    Bool           isSigned,
    Bool           roundToNearest
) {
    vmiFPRC  round       = roundToNearest ? vmi_FPR_NEAREST : vmi_FPR_CURRENT;
    vmiFType operandType = bytesToIType(operandBytes, isSigned);

    VMI_ASSERT(operandBytes==4 || operandBytes==2, "operand size in bytes %d must be 2 or 4", operandBytes);
    VMI_ASSERT(fracBits <= operandBytes*8, "fracBits = %d must be <= %d", fracBits, operandBytes*8);

    if (fracBits) {

        // Fixed point to floating point - must scale by fracBits
        Bool   singlePrec = (resultBytes == 4);
        vmiReg t          = newTemp64(state);
        vmiReg power2     = getFPConstPower2Flt64(state, fracBits);

        // Convert integer to double precision floating point (rounding mode unused here)
        armEmitFConvertRR(state, vmi_FT_64_IEEE_754, t, operandType, operand, vmi_FPR_CURRENT);

        if (singlePrec) {

            // Get result / 2^fracBits (always exact, so no rounding)
            armEmitFBinopSimdRRR(state, vmi_FT_64_IEEE_754, 1, vmi_FDIV, t, t, power2);

            // Convert to single precision (may be rounded)
            armEmitFConvertRR(state, vmi_FT_32_IEEE_754, result, vmi_FT_64_IEEE_754, t, round);

        } else {

            // result = value / 2^fracBits (always exact, so no rounding)
            armEmitFBinopSimdRRR(state, vmi_FT_64_IEEE_754, 1, vmi_FDIV, result, t, power2);
        }

    } else {

        // Just convert integer to target floating point type (may be rounded)
        vmiFType resultType = bytesToFType(resultBytes);

        armEmitFConvertRR(state, resultType, result, operandType, operand, round);
    }
}

//
// This is FPToFixed from psuedo-code
// Note: fpscr_controlled selection is done automatically in armEmit... call
//
static void FPToFixed (
    armMorphStateP state,
    vmiReg         result,
    Uns32          resultBytes,
    vmiReg         operand,
    Uns32          operandBytes,
    Uns32          fracBits,
    Bool           isSigned,
    Bool           roundTowardsZero
) {
    vmiFPRC  round  = roundTowardsZero ? vmi_FPR_ZERO : vmi_FPR_CURRENT;
    vmiFType opType = bytesToFType(operandBytes);

    VMI_ASSERT(operandBytes==4 || operandBytes==8, "operand size in bytes %d must be 8 or 4", operandBytes);
    VMI_ASSERT(resultBytes==4 || resultBytes==2, "result size in bytes %d must be 2 or 4", resultBytes);
    VMI_ASSERT(fracBits <= resultBytes*8, "fracBits = %d must be <= %d", fracBits, resultBytes*8);

    if (fracBits) {

        // Scale by fracBits
        vmiReg power2 = getFPConstPower2Flt80(state, fracBits);
        vmiReg t      = newTemp128(state);

        // Convert operand to 80 bit fp value (rounding mode unused here)
        armEmitFConvertRR(state, vmi_FT_80_X87, t, opType, operand, vmi_FPR_CURRENT);

        // t = t * 2^fracBits (always bigger, so no rounding)
        armEmitFBinopSimdRRR(state, vmi_FT_80_X87, 1, vmi_FMUL, t, t, power2);

        // Use temporary as operand source
        opType  = vmi_FT_80_X87;
        operand = t;
    }

    // convert to required type
    vmiFType resultType = bytesToIType(resultBytes, isSigned);
    armEmitFConvertRR(state, resultType, result, opType, operand, round);
}

//
// Set FPSCR flags according to the vmiFPRelation value
//
static void setFPSCRFlags(armP arm, vmiFPRelation relation) {

    if (relation == vmi_FPRL_UNORDERED) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.NF = 0;
        arm->sdfpAFlags.CF = arm->sdfpAFlags.VF = 1;
    } else if (relation == vmi_FPRL_EQUAL) {
        arm->sdfpAFlags.NF = arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.CF = 1;
    } else if (relation == vmi_FPRL_LESS) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.CF= arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.NF = 1;
    } else if (relation == vmi_FPRL_GREATER) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.NF= arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.CF = 1;
    } else {
        VMI_ABORT("unsupported fp relation result 0x%x", relation);
    }

}

//
// Assign APSR VCMPflags from FPSCR flags
//
static void getFPSCRFlags(armP arm) {

    arm->aflags = arm->sdfpAFlags;

}

////////////////////////////////////////////////////////////////////////////////
// VFP VLD and VST
////////////////////////////////////////////////////////////////////////////////

//
// Callback function for V load/store
//
#define V_LOAD_STORE_FN(_NAME) void _NAME( \
    armMorphStateP state,   \
    Uns32          memBits, \
    vmiReg         base,    \
    Int32          offset,  \
    vmiReg         rd       \
)
typedef V_LOAD_STORE_FN((*VLoadStoreFn));


//
// Swap rdH and rd if endian is big
//
static inline void endianSwapRegs(armMorphStateP state, vmiReg *rdH, vmiReg *rd) {

    memEndian endian = armGetEndian((vmiProcessorP)state->arm, False);

    if (endian == MEM_ENDIAN_BIG) {

        vmiReg t;

        t    = *rdH;
        *rdH = *rd;
        *rd  = t;
    }
}

//
// Emit code to load a VFP register from memory address [base]+offset
//
static V_LOAD_STORE_FN(emitVLoad)  {

    if (memBits == 2*ARM_GPR_BITS) {

        vmiReg rdH = getR64Hi(rd);

        endianSwapRegs(state, &rdH, &rd);
        armEmitLoadRRRO(state, memBits, offset, rd, rdH, base, getTemp(state), False, False);

    } else if (memBits <= ARM_GPR_BITS) {

        armEmitLoadRRO(state, memBits, offset, rd, base, False, False);

    } else {

        VMI_ABORT("Invalid memBits %d", memBits);

    }
}

//
// Emit code to store a VFP register to memory address [base]+offset
// When double word register adjust register depending on endian
//
static V_LOAD_STORE_FN(emitVStore) {

    if (memBits == 2*ARM_GPR_BITS) {

        vmiReg rdH = getR64Hi(rd);

        endianSwapRegs(state, &rdH, &rd);
        armEmitStoreRRRO(state, memBits, offset, base, rd, rdH);

    } else if (memBits <= ARM_GPR_BITS) {

        armEmitStoreRRO(state, memBits, offset, base, rd);

    } else {

        VMI_ABORT("Invalid memBits %d", memBits);

    }
}


//
// Common code for a VLDR or VSTR instruction
//
static void emitVLoadStore(armMorphStateP state, VLoadStoreFn cb, Bool isLoad) {

    if(executeFPCheck(state)) {

        Uns32  ebytes  = state->attrs->ebytes;
        Uns32  memBits = ebytes*8;
        vmiReg rd      = GET_VFP_REG(state, r1, ebytes);
        vmiReg base    = GET_RS(state, r2);
        Int32  offset  = state->info.c;
        if (isLoad) {

            // For loads only: align the constant if the base register is the
            // program counter (has effect for PC-relative Thumb load instructions only)
            if(VMI_REG_EQUAL(base, ARM_PC)) {
                offset = alignConstWithPC(state, offset);
            }

        }

        (*cb) (state, memBits, base, offset, rd);

    }
}

//
// Emit code for VLDR S or D, [RN, #imm]
//
ARM_MORPH_FN(armEmitVLDR) {
    emitVLoadStore(state,  emitVLoad, True);
}

//
// Emit code for VLDR S or D, [RN, #imm]
//
ARM_MORPH_FN(armEmitVSTR) {
    emitVLoadStore(state,  emitVStore, False);
}

//
// Common code for Load or Store Multiple using the passed call back function
//
static void emitVLoadStoreM(armMorphStateP state, VLoadStoreFn cb) {

    if(executeFPCheck(state)) {

        Uns32  ebytes     = state->attrs->ebytes;
        Uns32  memBits    = ebytes*8;
        vmiReg base       = GET_RS(state, r1);
        Uns32  nregs      = state->info.nregs;
        Bool   increment  = doIncrement(state);
        Uns32  frameSize  = nregs * ebytes;
        Int32  frameDelta = increment ? frameSize : -frameSize;
        Int32  offset     = increment ? 0 : -frameSize;
        Int32  stepBefore = getStepBefore(state, ebytes);
        Int32  stepAfter  = getStepAfter (state, ebytes);
        Uns32  r;

        // load or store registers
        for(r=0; r<nregs; r++) {

            vmiReg rd = GET_VFP_REG(state, r2+r, ebytes);

            offset += stepBefore;
            (*cb) (state, memBits, base, offset, rd);
            offset += stepAfter;
        }

        // perform base register update if required
        if(state->info.wb) {

            armEmitBinopRC(state, ARM_GPR_BITS, vmi_ADD, base, frameDelta, 0);

        }
    }
}

//
// Emit code for VLDM RN{!}, <S or D reg list>
//
ARM_MORPH_FN(armEmitVLDM) {
    emitVLoadStoreM(state, emitVLoad);
}

//
// Emit code for VSTM RN{!}, <S or D reg list>
//
ARM_MORPH_FN(armEmitVSTM) {
    emitVLoadStoreM(state, emitVStore);
}

////////////////////////////////////////////////////////////////////////////////
// VFP VMOV, etc instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for VMRS
//
ARM_MORPH_FN(armEmitVMRS) {

    Uns32 bits = ARM_GPR_BITS;

    // FPSCR is accessible if cp10/11 access is enabled
    if(!executeFPCheck(state)) {
        // no action
    } else if(state->info.r1==ARM_REG_PC) {
        // assign CPSR flags from FPSCR flags
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)getFPSCRFlags);

        // terminate the code block (derived flags are invalid)
        armEmitEndBlock();
    } else {
        vmiReg rd = GET_RD(state, r1);

        armEmitArgProcessor(state);
        armEmitCallResult(state, (vmiCallFn)armReadFPSCR, bits, rd);
    }
}

//
// Emit code for VMSR
//
ARM_MORPH_FN(armEmitVMSR) {

    Uns32 bits = ARM_GPR_BITS;

    // FPSCR is accessible in user mode
    if(executeFPCheck(state)) {
        armEmitArgProcessor(state);
        armEmitArgReg(state, bits, GET_RS(state, r1));
        armEmitArgUns32(state, FPSCR_MASK);
        armEmitCall(state, (vmiCallFn)armWriteFPSCR);

        // terminate the code block (block masks or floating point
        //  mode may have changed)
        armEmitEndBlock();
    }
}

//
// Emit code for VMOV.F32 Sd, Sm
//
ARM_MORPH_FN(armEmitVMOVR_VFP) {

    if(executeFPCheck(state)) {

        vmiReg rd     = GET_VFP_SREG(state, r1);
        vmiReg rs     = GET_VFP_SREG(state, r2);

        armEmitMoveRR(state, 32, rd, rs);
    }
}

//
// Emit code for VMOV.F32 Sd, # or VMOV.F64 Dd, #
//
ARM_MORPH_FN(armEmitVMOVI_VFP) {

    if(executeFPCheck(state)) {

        vmiReg rd     = GET_VFP_SREG(state, r1);
        Uns64  mi     = state->info.sdfpMI.u64;

        armEmitMoveRC(state, 32, rd, mi);
    }
}

//
// Emit code for VMOV RT, SN
//
ARM_MORPH_FN(armEmitVMOVRS) {

    if(executeFPCheck(state)) {

        vmiReg rd = GET_RD(state, r1);
        vmiReg rs = GET_VFP_SREG(state, r2);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit code for VMOV SN, RT
//
ARM_MORPH_FN(armEmitVMOVSR) {

    if(executeFPCheck(state)) {

        vmiReg rd = GET_VFP_SREG(state, r1);
        vmiReg rs = GET_RS(state, r2);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit code for VMOV RT, RT2, DN
//
ARM_MORPH_FN(armEmitVMOVRRD) {

    if(executeFPCheck(state)) {

        vmiReg rdL = GET_RD(state, r1);
        vmiReg rdH = GET_RD(state, r2);
        vmiReg rsL = GET_VFP_DREG(state, r3);
        vmiReg rsH = getR64Hi(rsL);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV DN, RT, RT2
//
ARM_MORPH_FN(armEmitVMOVDRR) {

    if(executeFPCheck(state)) {

        vmiReg rdL = GET_VFP_DREG(state, r1);
        vmiReg rdH = getR64Hi(rdL);
        vmiReg rsL = GET_RS(state, r2);
        vmiReg rsH = GET_RS(state, r3);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV RT, RT2, SM, SM1
//
ARM_MORPH_FN(armEmitVMOVRRSS) {

    if(executeFPCheck(state)) {

        vmiReg rdL = GET_RD(state, r1);
        vmiReg rdH = GET_RD(state, r2);
        vmiReg rsL = GET_VFP_SREG(state, r3);
        vmiReg rsH = GET_VFP_SREG(state, r3+1);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV SM, SM1, RT, RT2
//
ARM_MORPH_FN(armEmitVMOVSSRR) {

    if(executeFPCheck(state)) {

        vmiReg rdL = GET_VFP_SREG(state, r1);
        vmiReg rdH = GET_VFP_SREG(state, r1+1);
        vmiReg rsL = GET_RS(state, r2);
        vmiReg rsH = GET_RS(state, r3);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV DD[x], RT
//
ARM_MORPH_FN(armEmitVMOVZR) {

    if(executeFPCheck(state)) {

        Uns32  index  = state->info.index;
        vmiReg rd     = GET_VFP_SCALAR(state, r1, index);
        vmiReg rs     = GET_RS(state, r2);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit code for VMOV RT, DD[x]
//
ARM_MORPH_FN(armEmitVMOVRZ) {

    if(executeFPCheck(state)) {

        Uns32  index  = state->info.index;
        vmiReg rd     = GET_RS(state, r1);
        vmiReg rs     = GET_VFP_SCALAR(state, r2, index);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}


////////////////////////////////////////////////////////////////////////////////
// VFP Data-Processing Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for VFP single precision floating point <binop>, with 3 regs
//
ARM_MORPH_FN(armEmitVFPBinop) {

    if(executeFPCheck(state)) {

        vmiReg    r1     = GET_VFP_SREG(state, r1);
        vmiReg    r2     = GET_VFP_SREG(state, r2);
        vmiReg    r3     = GET_VFP_SREG(state, r3);
        vmiFBinop op     = state->attrs->fbinop;
        Bool      negate = state->attrs->negate;

        armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, op, r1, r2, r3);

        // If negate attribute is selected negate the result
        if (negate) FPNeg(state, r1, r1, 32);
    }
}

//
// Emit code for single precision VFP floating point <Unop>, with 2 regs
//
ARM_MORPH_FN(armEmitVFPUnop) {

    if(executeFPCheck(state)) {

        vmiReg    r1     = GET_VFP_SREG(state, r1);
        vmiReg    r2     = GET_VFP_SREG(state, r2);
        vmiFBinop op     = state->attrs->funop;

        armEmitFUnopSimdRR(state, vmi_FT_32_IEEE_754, 1, op, r1, r2);

    }
}

//
// Emit code for VFP VABS Sd, Sm
// Simply mask off sign bit
//
ARM_MORPH_FN(armEmitVABS_VFP) {

    if(executeFPCheck(state)) {

        vmiReg    r1     = GET_VFP_SREG(state, r1);
        vmiReg    r2     = GET_VFP_SREG(state, r2);

        armEmitBinopRRC(state, 32, vmi_ANDN, r1, r2, 0x80000000, 0);

    }
}

//
// Emit code for VFP VNEG Sd, Sm or VNEG Dd, Dm
//
ARM_MORPH_FN(armEmitVNEG_VFP) {

    if(executeFPCheck(state)) {

        vmiReg    r1     = GET_VFP_SREG(state, r1);
        vmiReg    r2     = GET_VFP_SREG(state, r2);

        // Take negative value by simply toggling the sign bit
        FPNeg(state, r1, r2, 32);

    }
}

//
// Emit code for VFP floating point Multiply (negate) accumulate/subtract instruction
//
ARM_MORPH_FN(armEmitVMulAcc_VFP) {

    if(executeFPCheck(state)) {

        vmiReg    r1      = GET_VFP_SREG(state, r1);
        vmiReg    r2      = GET_VFP_SREG(state, r2);
        vmiReg    r3      = GET_VFP_SREG(state, r3);
        vmiFBinop op      = state->attrs->fbinop;
        Bool      negate  = state->attrs->negate;
        vmiReg    product = newTemp32(state);
        vmiReg    acc     = r1;

        VMI_ASSERT(op==vmi_FSUB || op==vmi_FADD, "Invalid fbinop");

        // Perform Multiply to temporary
        armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FMUL, product, r2, r3);

        if (negate) {

            // Negate the accumulation value before adding/subtracting
            acc = newTemp64(state);
            FPNeg(state, acc, r1, 32);

        }

        // To implement subtraction, negate the value being subtracted and use vmi_FADD
        if (op == vmi_FSUB) FPNeg(state, product, product, 32);

        // Add the (possibly negated) product to/from the accumlate value (+/- r1)
        armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FADD, r1, acc, product);
    }
}

//
// Emit code for VFP single precision fused multiply accumulate
//
ARM_MORPH_FN(armEmitVFusedMAC) {

    if(executeFPCheck(state)) {

        vmiReg    rd       = GET_VFP_SREG(state, r1);
        vmiReg    addend   = rd;
        vmiReg    op1      = GET_VFP_SREG(state, r2);
        vmiReg    op2      = GET_VFP_SREG(state, r3);
        Bool      negate   = state->attrs->negate;
        Bool      subtract = state->attrs->subtract;

        if (subtract) {
            // Use negative of op1
            vmiReg t = newTemp32(state);
            FPNeg(state, t, op1, 32);
            op1 = t;
        }

        if (negate) {
            // Use negative of r1 as addend
            vmiReg t = newTemp32(state);
            FPNeg(state, t, addend, 32);
            addend = t;
        }

        // Do rd = addend + (op1 * op2) with no rounding of intermediate results
        armEmitFTernopSimdRRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FMADD, rd, op1, op2, addend, False);

    }
}


////////////////////////////////////////////////////////////////////////////////
// VFP VCMP Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Common code to execute VFP VCMP instructions
//
static void emitVCmpVFP(armMorphStateP state, vmiReg rd, vmiReg rm) {

    Bool   allowQNaN = state->attrs->allowQNaN;
    vmiReg relation  = getTemp(state);

    // Compare the floating point operands, getting vmiFPRelation result in the register relation
    armEmitFCompareRR(state, vmi_FT_32_IEEE_754, relation, rd, rm, allowQNaN, True);

    // Set the FPSCR N, Z, C, V flags according to the result
    armEmitArgProcessor(state);
    armEmitArgReg(state, 8, relation);
    armEmitCall(state, (vmiCallFn) setFPSCRFlags);

}

//
// Emit code for VFP VCMP/VCMPE instruction: VCMP{E} Sd, Sm
//
ARM_MORPH_FN(armEmitVCMP_VFP) {

    if(executeFPCheck(state)) {

        vmiReg rd = GET_VFP_SREG(state, r1);
        vmiReg rm = GET_VFP_SREG(state, r2);

        emitVCmpVFP(state, rd, rm);

    }
}

//
// Emit code for VFP VCMP/VCMPE immediate 0.0 instruction: VCMP{E} Sd, #0.0
// Note: VCMP instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCMP0_VFP) {

    if(executeFPCheck(state)) {

        vmiReg rd = GET_VFP_SREG(state, r1);
        vmiReg rm = newTemp32(state);

        armEmitMoveRC(state, 32, rm, 0);
        emitVCmpVFP(state, rd, rm);
    }
}


////////////////////////////////////////////////////////////////////////////////
// VFP VCVT Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for VFP Half to Single precision conversion: VCVTT.F32.F16 Sd, Sm or VCVTB.F32.F16 Sd, Sm
//
ARM_MORPH_FN(armEmitVCVT_SH_VFP) {

    if(executeFPCheck(state)) {

        Uns32  top = state->attrs->highhalf ? 1 : 0;
        vmiReg rd  = GET_VFP_SREG(state, r1);
        vmiReg rm  = GET_VFP_SREG(state, r2);

        // point to top half of register when VCVTT
        if (top) rm = VMI_REG_DELTA(rm, 2);

        armEmitArgProcessor(state);         // argument 1: processor
        armEmitArgReg(state, 16, rm);       // argument 2: value in rm
        armEmitCallResult(state, (vmiCallFn)armFPHalfToSingle, 32, rd);
    }
}

//
// Emit code for VFP Single to Half precision conversion: VCVTT.F16.F32 Sd, Sm or VCVTB.F16.F32 Sd, Sm
//
ARM_MORPH_FN(armEmitVCVT_HS_VFP) {

    if(executeFPCheck(state)) {

        Uns32  top = state->attrs->highhalf ? 1 : 0;
        vmiReg rd  = GET_VFP_SREG(state, r1);
        vmiReg rm  = GET_VFP_SREG(state, r2);

        // point to top half of register when VCVTT
        if (top) rd = VMI_REG_DELTA(rd, 2);

        armEmitArgProcessor(state);         // argument 1: processor
        armEmitArgReg(state, 32, rm);// argument 2: value
        armEmitCallResult(state, (vmiCallFn)armFPSingleToHalf, 16, rd);
    }
}

//
// Emit code for VFP single precision Floating point to Fixed point (32 or 16 bit) conversion: VCVT
// Note: r1 always equals r2 and the size of the fixed point value is defined by ebytes
// Note: sextend indicates if 16 bit result should be sign extended
// Note: roundFPSCR indicates if the normal rounding mode or round to nearest mode is used
//
ARM_MORPH_FN(armEmitVCVT_XF_VFP) {

    if(executeFPCheck(state)) {

        VMI_ASSERT(state->info.r1== state->info.r2, "r1 must be same as r2");

        Uns32   fracBits   = state->info.c;
        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   fxBytes    = state->attrs->ebytes;
        vmiReg  rd         = GET_VFP_SREG(state, r1);

        FPToFixed(state, rd, fxBytes, rd, 4, fracBits, sextend, !roundFPSCR);

        if (fxBytes != 4) {
            // sign extend to fill destination registeru
            armEmitMoveExtendRR(state, 32, rd, fxBytes*8, rd, sextend);
        }
    }
}

//
// Emit code for VFP single precsion Floating point to signed or unsigned 32 bit Integer conversion: VCVT
//
ARM_MORPH_FN(armEmitVCVT_IF_VFP) {

    if(executeFPCheck(state)) {

        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        vmiReg  rd         = GET_VFP_SREG(state, r1);
        vmiReg  rm         = GET_VFP_SREG(state, r2);

        FPToFixed(state, rd, 4, rm, 4, 0, sextend, !roundFPSCR);
    }
}

//
// Emit code for VFP Fixed point to single precision Floating point conversion: VCVT
// Note: r1 always equals r2 and the size of the fixed point value is defined by ebytes
//
ARM_MORPH_FN(armEmitVCVT_FX_VFP) {

    if(executeFPCheck(state)) {

        VMI_ASSERT(state->info.r1== state->info.r2, "r1 must be same as r2");

        Uns32   fracBits   = state->info.c;
        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   fxBytes    = state->attrs->ebytes;
        vmiReg  rd         = GET_VFP_SREG(state, r1);

        fixedToFP(state, rd, 4, rd, fxBytes, fracBits, sextend, !roundFPSCR);
    }
}

//
// Emit code for VFP Integer to single precision Floating point conversion: VCVT
//
ARM_MORPH_FN(armEmitVCVT_FI_VFP) {

    if(executeFPCheck(state)) {

        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        vmiReg  rd         = GET_VFP_SREG(state, r1);
        vmiReg  rm         = GET_VFP_SREG(state, r2);

        fixedToFP(state, rd, 4, rm, 4, 0, sextend, !roundFPSCR);
    }
}

////////////////////////////////////////////////////////////////////////////////
// DEPRECATED DISASSEMBLY MODE
////////////////////////////////////////////////////////////////////////////////

//
// Disassembly mode: SWI 99/9999 terminates the test
//
#define ARM_SWI_EXIT_CODE_THUMB  99
#define ARM_SWI_EXIT_CODE_NORMAL 9999

//
// Should morphing be disabled? (disassembly test mode only)
//
static Bool disableMorph(armMorphStateP state) {

    armP arm = state->arm;

    if(!ARM_DISASSEMBLE(arm)) {
        return False;
    } else if(state->info.type!=ARM_IT_SWI) {
        return True;
    } else if(
        ( IN_THUMB_MODE(arm) && (state->info.c==ARM_SWI_EXIT_CODE_THUMB)) ||
        (!IN_THUMB_MODE(arm) && (state->info.c==ARM_SWI_EXIT_CODE_NORMAL))
    ) {
        armEmitExit();
        return True;
    } else {
        return True;
    }
}


////////////////////////////////////////////////////////////////////////////////
// MORPHER MAIN ROUTINES
////////////////////////////////////////////////////////////////////////////////

//
// Advance to the new IT state
//
static Uns8 itAdvance(Uns8 oldState) {
    Uns8 newState = (oldState & 0xe0) | ((oldState<<1) & 0x1f);
    return (newState & 0xf) ? newState : 0;
}

//
// Emit code to update ITSTATE if required when the instruction completes (note
// that jumps and branches take special action to reset ITSTATE elsewhere)
//
static void emitUpdateITState(armMorphStateP state) {

    armP arm = state->arm;

    if(state->info.it) {
        arm->itStateMT = state->info.it;
        armEmitMoveRC(state, 8, ARM_IT_STATE, arm->itStateMT);
    } else if(arm->itStateMT) {
        arm->itStateMT = itAdvance(arm->itStateMT);
        armEmitMoveRC(state, 8, ARM_IT_STATE, arm->itStateMT);
    }
}

//
// Default morpher callback for implemented instructions
//
static void emitImplemented(armMorphStateP state) {

    // start code to skip this instruction conditionally unless it can be
    // emitted as a conditional jump
    setSkipLabel(state, !state->attrs->condJump ? emitStartSkip(state) : 0);

    // generate instruction code
    state->attrs->morphCB(state);

    // generate implicit jump to next instruction if required
    armEmitImplicitUncondJump(state);

    // insert conditional instruction skip target label
    emitLabel(getSkipLabel(state));

    // update ITSTATE if required when the instruction completes
    emitUpdateITState(state);
}

//
// Check whether the VFP floating-point type is not supported at the minimum required level
//
static Bool supportVFP(armMorphStateP state, armSDFPType dt, Uns32 minLevel) {

    armP arm = state->arm;
    Bool ok;

    VMI_ASSERT (state->attrs->iType==ARM_TY_VFP, "Called with non-VFP instruction");

    if (!FPU_PRESENT(state->arm)) {
        ok = False;
    } else  if (dt == ARM_SDFPT_F32) {
        ok = SCS_FIELD(arm, MVFR0, SinglePrecision) >= minLevel;
    } else if (dt == ARM_SDFPT_F64) {
        ok = SCS_FIELD(arm, MVFR0, DoublePrecision) >= minLevel;
    } else {
        ok = True;
    }

    return ok;
}

//
// Determine whether a feature is supported by ISAR
//
static Bool supportedByISAR(armP arm, armMorphStateP state) {
    switch(state->info.isar) {
        case ARM_ISAR_DIV:
            return ARM_ISAR(0, Divide_instrs);
        case ARM_ISAR_BKPT:
            return ARM_ISAR(0, Debug_instrs);
        case ARM_ISAR_CBZ:
            return ARM_ISAR(0, CmpBranch_instrs);
        case ARM_ISAR_BFC:
            return ARM_ISAR(0, BitField_instrs);
        case ARM_ISAR_CLZ:
            return ARM_ISAR(0, BitCount_instrs);
        case ARM_ISAR_SWP:
            return ARM_ISAR(0, Swap_instrs);
        case ARM_ISAR_BXJ:
            return ARM_ISAR(1, Jazelle_instrs);
        case ARM_ISAR_BX:
            return ARM_ISAR(1, Interwork_instrs)>0;
        case ARM_ISAR_BLX:
            return ARM_ISAR(1, Interwork_instrs)>1;
        case ARM_ISAR_MOVT:
            return ARM_ISAR(1, Immediate_instrs);
        case ARM_ISAR_IT:
            return ARM_ISAR(1, IfThen_instrs);
        case ARM_ISAR_SXTB:
            return ARM_ISAR(1, Extend_instrs)>0;
        case ARM_ISAR_SXTAB:
            return ARM_ISAR(1, Extend_instrs)>1;
        case ARM_ISAR_SXTB16:
            return (ARM_ISAR(1, Extend_instrs)>1) && DSP_PRESENT(arm);
        case ARM_ISAR_SRS:
            return ARM_ISAR(1, Except_AR_instrs);
        case ARM_ISAR_LDM_UR:
            return ARM_ISAR(1, Except_instrs);
        case ARM_ISAR_SETEND:
            return ARM_ISAR(1, Endian_instrs);
        case ARM_ISAR_REV:
            return ARM_ISAR(2, Reversal_instrs)>0;
        case ARM_ISAR_RBIT:
            return ARM_ISAR(2, Reversal_instrs)>1;
        case ARM_ISAR_MRS_AR:
            return ARM_ISAR(2, PSR_AR_instrs);
        case ARM_ISAR_UMULL:
            return ARM_ISAR(2, MultU_instrs)>0;
        case ARM_ISAR_UMAAL:
            return ARM_ISAR(2, MultU_instrs)>1;
        case ARM_ISAR_SMULL:
            return ARM_ISAR(2, MultS_instrs)>0;
        case ARM_ISAR_SMLABB:
            return ARM_ISAR(2, MultS_instrs)>1;
        case ARM_ISAR_SMLAD:
            return ARM_ISAR(2, MultS_instrs)>2;
        case ARM_ISAR_MLA:
            return ARM_ISAR(2, Mult_instrs)>0;
        case ARM_ISAR_MLS:
            return ARM_ISAR(2, Mult_instrs)>1;
        case ARM_ISAR_PLD:
            return ARM_ISAR(2, MemHint_instrs)>0;
        case ARM_ISAR_PLI:
            return ARM_ISAR(2, MemHint_instrs)>2;
        case ARM_ISAR_LDRD:
            return ARM_ISAR(2, LoadStore_instrs);
        case ARM_ISAR_NOP:
            return ARM_ISAR(3, TrueNOP_instrs);
        case ARM_ISAR_MOVLL:
            return ARM_ISAR(3, ThumbCopy_instrs);
        case ARM_ISAR_TBB:
            return ARM_ISAR(3, TabBranch_instrs);
        case ARM_ISAR_LDREX:
            return ARM_ISAR(3, SynchPrim_instrs)>0;
        case ARM_ISAR_CLREX:
            return (ARM_ISAR(3, SynchPrim_instrs)>1) || (ARM_ISAR(4, SynchPrim_instrs_frac)==3);
        case ARM_ISAR_LDREXD:
            return ARM_ISAR(3, SynchPrim_instrs)>1;
        case ARM_ISAR_SVC:
            return ARM_ISAR(3, SVC_instrs);
        case ARM_ISAR_SSAT:
            return ARM_ISAR(3, SIMD_instrs)>0;
        case ARM_ISAR_PKHBT:
            return DSP_PRESENT(arm);
        case ARM_ISAR_QADD:
            return ARM_ISAR(3, Saturate_instrs);
        case ARM_ISAR_MRS_M:
            return ARM_ISAR(4, PSR_M_instrs);
        case ARM_ISAR_DMB:
            return ARM_ISAR(4, Barrier_instrs);
        case ARM_ISAR_LDRBT:
            return ARM_ISAR(4, Unpriv_instrs)>0;
        case ARM_ISAR_LDRHT:
            return ARM_ISAR(4, Unpriv_instrs)>1;
        case ARM_ISAR_VMRS:
            return FPU_PRESENT(arm);
        case ARM_ISAR_VFPV2:
            return supportVFP(state, state->info.dt1, 1);
        case ARM_ISAR_VFPV3:
            return supportVFP(state, state->info.dt1, 2);
        case ARM_ISAR_VFPFMAC:
            return FPU_PRESENT(arm) && SCS_FIELD(arm, MVFR1, VFP_FusedMAC);
        case ARM_ISAR_VFPSQRT:
            return SCS_FIELD(arm, MVFR0, SquareRoot) && supportVFP(state, state->info.dt1, 1);
        case ARM_ISAR_VFPDIV:
            return SCS_FIELD(arm, MVFR0, Divide) && supportVFP(state, state->info.dt1, 1);
        case ARM_ISAR_VFPCVT2:
            return supportVFP(state, state->info.dt1, 1) && supportVFP(state, state->info.dt2, 1);
        case ARM_ISAR_VFPCVT3:
            return supportVFP(state, state->info.dt1, 2) && supportVFP(state, state->info.dt2, 2);
        case ARM_ISAR_VFPHP:
            return FPU_PRESENT(arm) && SCS_FIELD(arm, MVFR1, VFP_HalfPrecision);
        default:
            VMI_ABORT("unimplemented case");
            return False;
    }
}

//
// Return a boolean indicating whether the processor supports the required
// architecture
//
static Bool supportedOnVariant(armP arm, armMorphStateP state) {

    armArchitecture configVariant   = arm->configInfo.arch;
    armArchitecture requiredVariant = state->info.support;

    if(ARM_INSTRUCTION_VERSION(requiredVariant) > getInstructionVersion(arm)) {
        return False;
    } else if(!ARM_SUPPORT(configVariant, requiredVariant & ~ARM_MASK_VERSION)) {
        return False;
    } else if(state->info.isar && SCS_USE_CPUID(arm)) {
        return supportedByISAR(arm, state);
    } else {
        return True;
    }
}

//
// Create code for the ARM instruction at the simulated address referenced
// by 'thisPC'.
//
VMI_MORPH_FN(armMorphInstruction) {

    armP          arm = (armP)processor;
    armMorphState state;

    // seed morph-time ITSTATE if this is the first instruction in a code block
    if(firstInBlock) {
        arm->itStateMT = arm->itStateRT;
    }

    // get instruction and instruction type
    armDecode(arm, thisPC, &state.info);

    // get morpher attributes for the decoded instruction and initialize other
    // state fields
    state.attrs       = &armMorphTable[state.info.type];
    state.arm         = arm;
    state.nextPC      = state.info.thisPC + state.info.bytes;
    state.skipLabel   = 0;
    state.tempIdx     = 0;
    state.pcFetched   = False;
    state.pcSet       = ASPC_NA;
    state.pcImmediate = 0;
    state.setMode     = False;

    // if this is the first instruction in the block, mark all derived flags as
    // invalid
    if(firstInBlock) {
        resetDerivedFlags(&state);
    }

    if(disableMorph(&state)) {
        // no action if in disassembly mode
    } else if(!supportedOnVariant(arm, &state)) {
        // instruction not supported on this variant
        emitNotVariant(&state);
    } else if(state.attrs->morphCB) {
        // translate the instruction
        emitImplemented(&state);
    } else if(state.info.type==ARM_IT_LAST) {
        // take UsageFault exception
        emitUsageFault(&state, EXC_UNDEF_UNDEFINSTR);
    } else {
        // here if no morph callback specified
        emitUnimplemented(&state);
    }
}

//
// Snap instruction fetch addresses to the appropriate boundary
//
VMI_FETCH_SNAP_FN(armFetchSnap) {

    armP  arm  = (armP)processor;
    Uns32 snap = IN_THUMB_MODE(arm) ? 1 : 3;

    return thisPC & ~snap;
}


