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
#include "vmi/vmiAttrs.h"
#include "vmi/vmiMt.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armDecode.h"
#include "armEmit.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMode.h"
#include "armMorph.h"
#include "armStructure.h"
#include "armRegisters.h"
#include "armUtils.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_EMIT"


////////////////////////////////////////////////////////////////////////////////
// SPECIAL HANDLING FOR PC UPDATES
////////////////////////////////////////////////////////////////////////////////

//
// Is the passed register the link register?
//
inline static Bool isLinkReg(vmiReg ra) {
    return VMI_REG_EQUAL(ra, ARM_LR);
}

//
// Invalidate derived flags dependent on ZF
//
inline static void invalidateZF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validHI = False;
}

//
// Invalidate derived flags dependent on NF
//
inline static void invalidateNF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validLT = False;
}

//
// Invalidate derived flags dependent on CF
//
inline static void invalidateCF(armMorphStateP state) {
    state->arm->validHI = False;
}

//
// Invalidate derived flags dependent on VF
//
inline static void invalidateVF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validLT = False;
}

//
// Invalidate any derived flags if a flag register is being assigned
//
static void invalidateDerivedReg(armMorphStateP state, vmiReg rd) {
    if(VMI_REG_EQUAL(rd, ARM_ZF)) {
        invalidateZF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_NF)) {
        invalidateNF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_CF)) {
        invalidateCF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_VF)) {
        invalidateVF(state);
    }
}

//
// Invalidate derived flags for all flags assigned in the flags structure
//
static void invalidateDerivedFlags(armMorphStateP state, vmiFlagsCP flags) {
    if(flags) {
        invalidateDerivedReg(state, flags->f[vmi_CF]);
        invalidateDerivedReg(state, flags->f[vmi_ZF]);
        invalidateDerivedReg(state, flags->f[vmi_SF]);
        invalidateDerivedReg(state, flags->f[vmi_OF]);
    }
}

//
// Called when register is assigned a variable value
//
static void setVariable(armMorphStateP state, vmiReg rd, Bool isReturn) {

    armP arm = state->arm;

    // assignment to a flag should invalidate any derived flags
    invalidateDerivedReg(state, rd);

    // possibly a special jump if PC is being assigned
    if(VMI_REG_EQUAL(rd, ARM_PC)) {

        Uns32 version   = ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
        Bool  interwork = False;

        switch(state->attrs->interwork) {

            case ARM_IW_L4:
                // always an interworking instruction
                interwork = True;
                break;

            case ARM_IW_ARM_V7:
                // an interworking instruction if ARMv7 or above and an ARM
                // instruction that does not set flags (not Thumb)
                interwork = ((version>=7) && !IN_THUMB_MODE(arm) && !state->info.f);
                break;

            default:
                // never an interworking instruction (or an explicit
                // interworking instruction)
                break;
        }

        state->pcSet   = isReturn ? ASPC_R15_RET : ASPC_R15;
        state->setMode = interwork;
    }
}

//
// Called to mask result value if required
//
static void maskResult(vmiReg rd) {

    if(VMI_REG_EQUAL(rd, ARM_SP)) {

        // sp_main is 4-byte aligned
        vmimtBinopRC(ARM_GPR_BITS, vmi_AND, rd, ~3, 0);

    } else if(VMI_REG_EQUAL(rd, ARM_BANK_SP)) {

        // sp_process is 4-byte aligned
        vmimtBinopRC(ARM_GPR_BITS, vmi_AND, rd, ~3, 0);
    }
}

//
// Called when register is assigned a constant value
//
static Bool setConstant(armMorphStateP state, vmiReg rd, Uns32 c) {

    // assignment to a flag should invalidate any derived flags
    invalidateDerivedReg(state, rd);

    // special jump if PC is being assigned
    if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return False;
    } else {
        state->pcSet       = ASPC_IMM;
        state->pcImmediate = c;
        return True;
    }
}


////////////////////////////////////////////////////////////////////////////////
// MODE SWITCH
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to switch state between normal and Thumb mode
//
void armEmitInterwork(armMorphStateP state) {

    // toggle Thumb bit in PSR
    vmimtBinopRC(ARM_GPR_BITS, vmi_XOR, ARM_PSR, PSR_THUMB, 0);

    // make call to switch to ARM mode (will immediately take exception)
    vmimtArgProcessor();
    vmimtCall((vmiCallFn)armSwitchMode);
}


////////////////////////////////////////////////////////////////////////////////
// INTEGER OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// Get delta applied to the current PC to get the value returned when PC is a
// source argument
//
inline static Uns32 getPCDelta(armMorphStateP state) {
    return (IN_THUMB_MODE(state->arm) ? 4 : 8);
}

//
// rd = simPC (true)
//
void armEmitGetTruePC(armMorphStateP state, vmiReg rd) {
    vmimtMoveRSimPC(ARM_GPR_BITS, rd);
}

//
// r15 = simPC (according to ARM pipeline)
//
void armEmitGetPC(armMorphStateP state) {
    if(!state->pcFetched) {
        state->pcFetched = True;
        vmimtMoveRSimPC(ARM_GPR_BITS, ARM_PC);
        vmimtBinopRC(ARM_GPR_BITS, vmi_ADD, ARM_PC, getPCDelta(state), 0);
    }
}

//
// rd = c
//
void armEmitMoveRC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    Uns64          c
) {
    if(!setConstant(state, rd, c)) {
        vmimtMoveRC(bits, rd, c);
    }
}

//
// rd = ra
//
void armEmitMoveRR(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         ra
) {
    setVariable(state, rd, isLinkReg(ra));
    vmimtMoveRR(bits, rd, ra);
    maskResult(rd);
}

//
// rd<destBits> = ra<srcBits>
//
void armEmitMoveExtendRR(
    armMorphStateP state,
    Uns32          destBits,
    vmiReg         rd,
    Uns32          srcBits,
    vmiReg         ra,
    Bool           signExtend
) {
    setVariable(state, rd, False);
    vmimtMoveExtendRR(destBits, rd, srcBits, ra, signExtend);
    maskResult(rd);
}

//
// rd = (flag==select1) ? c1 : c2
//
void armEmitCondMoveRCC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         flag,
    Bool           select1,
    vmiReg         rd,
    Uns64          c1,
    Uns64          c2
) {
    setVariable(state, rd, False);
    vmimtCondMoveRCC(bits, flag, select1, rd, c1, c2);
    maskResult(rd);
}

//
// rd = <unop> ra
//
void armEmitUnopRR(
    armMorphStateP state,
    Uns32          bits,
    vmiUnop        op,
    vmiReg         rd,
    vmiReg         ra,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, (op==vmi_MOV) && isLinkReg(ra));
    vmimtUnopRR(bits, op, rd, ra, flags);
    maskResult(rd);
}

//
// rd = <unop> c
//
void armEmitUnopRC(
    armMorphStateP state,
    Uns32          bits,
    vmiUnop        op,
    vmiReg         rd,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);

    // convert constant so that value can be moved directly
    if(op==vmi_MOV) {
        // no action
    } else if(op==vmi_NOT) {
        op = vmi_MOV;
        c  = ~c;
    } else {
        VMI_ABORT("unexpected unary opcode %u", op);
    }

    // operation is only required if the target is not the program counter or
    // if flags are needed
    if(!setConstant(state, rd, c) || flags) {
        vmimtUnopRC(bits, op, rd, c, flags);
        maskResult(rd);
    }
}

//
// rd = rd <binop> ra
//
void armEmitBinopRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRR(bits, op, rd, ra, flags);
    maskResult(rd);
}

//
// rd = rd <binop> c
//
void armEmitBinopRC(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRC(bits, op, rd, c, flags);
    maskResult(rd);
}

//
// rd = ra <binop> rb
//
void armEmitBinopRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    vmiReg         rb,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRRR(bits, op, rd, ra, rb, flags);
    maskResult(rd);
}

//
// rd = ra <binop> c
//
void armEmitBinopRRC(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRRC(bits, op, rd, ra, c, flags);
    maskResult(rd);
}

//
// Generate shift mask prefix (sets mask to 255)
//
void armEmitSetShiftMask(void) {
    vmimtSetShiftMask(255);
}

//
// rdh:rdl = ra*rb
//
void armEmitMulopRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rdH,
    vmiReg         rdL,
    vmiReg         ra,
    vmiReg         rb,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rdL, False);
    setVariable(state, rdH, False);
    vmimtMulopRRR(bits, op, rdH, rdL, ra, rb, flags);
    maskResult(rdH);
    maskResult(rdL);
}
////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// Perform actions after a floating point operation
//
static void endFPOperation(armMorphStateP state, Bool setSticky) {

    if(setSticky) {
        // merge sticky flags
        vmimtBinopRR(8, vmi_OR, ARM_FP_STICKY, ARM_FP_FLAGS, 0);
    }
}

//
// fd = fa <fp ternnop> fb fc
//
void armEmitFTernopSimdRRRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFTernop     op,
    vmiReg         fd,
    vmiReg         fa,
    vmiReg         fb,
    vmiReg         fc,
    Bool           roundInt
) {

    // do the ternop
    vmimtFTernopSimdRRRR(type, num, op, fd, fa, fb, fc, ARM_FP_FLAGS, roundInt);

    // do epilogue actions
    endFPOperation(state, True);
}


//
// fd = fa <fp binop> fb
//
void armEmitFBinopSimdRRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFBinop      op,
    vmiReg         fd,
    vmiReg         fa,
    vmiReg         fb
) {

    // do the binop
    vmimtFBinopSimdRRR(type, num, op, fd, fa, fb, ARM_FP_FLAGS);

    // do epilogue actions
    endFPOperation(state, True);
}

//
// fd = <fp unop> fa
//
void armEmitFUnopSimdRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFUnop       op,
    vmiReg         fd,
    vmiReg         fa
) {

    // do the unop
    vmimtFUnopSimdRR(type, num, op, fd, fa, ARM_FP_FLAGS);

    // do epilogue actions
    endFPOperation(state, True);
}

//
// fd = <fp convert> fa
//
void armEmitFConvertRR(
    armMorphStateP state,
    vmiFType       destType,
    vmiReg         fd,
    vmiFType       srcType,
    vmiReg         fa,
    vmiFPRC        round
) {
    // conversions from integer values to longer values (integer or floating
    // point) never produce exceptions or require rounding
    Bool  srcIsFP       = !VMI_FTYPE_IS_INT(srcType);
    Uns32 srcBits       = VMI_FTYPE_BITS(srcType);
    Uns32 destBits      = VMI_FTYPE_BITS(destType);
    Bool  exceptOrRound = (srcIsFP || (srcBits>=destBits));

    // do the conversion
    vmimtFConvertRR(destType, fd, srcType, fa, round, ARM_FP_FLAGS);

    // do epilogue actions if exceptions were possible
    if(exceptOrRound) {
        endFPOperation(state, True);
    }
}

//
// compare fa to fb, setting relation and flags
//
void armEmitFCompareRR(
    armMorphStateP state,
    vmiFType type,
    vmiReg   relation,
    vmiReg   fa,
    vmiReg   fb,
    Bool     allowQNaN,
    Bool     setSticky
) {

    // do the compare
    vmimtFCompareRR(type, relation, fa, fb, ARM_FP_FLAGS, allowQNaN);

    // do epilogue actions
    endFPOperation(state, setSticky);
}


////////////////////////////////////////////////////////////////////////////////
// LOAD AND STORE OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// Return processor endianness (and add blockMask check to validate it if
// required)
//
static memEndian getEndian(armMorphStateP state, Uns32 bits) {

    armP arm = state->arm;

    // validate endianness for memory operations wider than a byte
    if(arm->checkEndian && (bits>8)) {
        vmimtValidateBlockMask(ARM_BM_BIG_ENDIAN);
    }

    // return current endianness
    return armGetEndian((vmiProcessorP)arm, False);
}

//
// Indicate whether instruction requires alignment checking (and add blockMask
// check to validate it if required)
//
static Bool getAlign(armMorphStateP state, Uns32 bits) {

    armP arm = state->arm;

    // validate alignment checking for memory operations wider than a byte
    if(arm->checkUnaligned && (bits>8)) {
        vmimtValidateBlockMask(ARM_BM_UNALIGNED);
    }

    // alignment checking required if ua is not ARM_UA_UNALIGNED
    return !(DO_UNALIGNED(arm) && (state->info.ua==ARM_UA_UNALIGNED));
}

//
// mem[ra+offset] = rb  (when ra!=VMI_NOREG)
// mem[offset]    = rb  (when ra==VMI_NOREG)
//
void armEmitStoreRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rb
) {
    memEndian endian = getEndian(state, bits);
    Bool      align  = getAlign(state, bits);

    // emit single store
    vmimtStoreRRO(bits, offset, ra, rb, endian, align);
}

//
// mem[ra+offset] = rbH:rbL  (when ra!=VMI_NOREG)
// mem[offset]    = rbH:rbL  (when ra==VMI_NOREG)
//
void armEmitStoreRRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rbL,
    vmiReg         rbH
) {
    Uns32     regBits = ARM_GPR_BITS;
    armP      arm     = state->arm;
    memEndian endian  = getEndian(state, bits);
    Bool      align   = getAlign(state, bits);

    // generate exception for misaligned or inaccessible address if required
    if(!arm->configInfo.align64as32) {
        vmimtTryStoreRC(bits, offset, ra, align);
    }

    // emit two word stores
    vmimtStoreRRO(regBits, offset,   ra, rbL, endian, align);
    vmimtStoreRRO(regBits, offset+4, ra, rbH, endian, align);
}

//
// rd = mem[ra+offset]  (when ra!=VMI_NOREG)
// rd = mem[offset]     (when ra==VMI_NOREG)
//
void armEmitLoadRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         rd,
    vmiReg         ra,
    Bool           signExtend,
    Bool           isReturn
) {
    Uns32     regBits = ARM_GPR_BITS;
    memEndian endian  = getEndian(state, bits);
    Bool      align   = getAlign(state, bits);

    // emit single load
    vmimtLoadRRO(regBits, bits, offset, rd, ra, endian, signExtend, align);
    setVariable(state, rd, isReturn);
    maskResult(rd);
}

//
// rdH:rdL = mem[ra+offset]  (when ra!=VMI_NOREG)
// rdH:rdL = mem[offset]     (when ra==VMI_NOREG)
//
void armEmitLoadRRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         rdL,
    vmiReg         rdH,
    vmiReg         ra,
    vmiReg         rt,
    Bool           signExtend,
    Bool           isReturn
) {
    Uns32     regBits = ARM_GPR_BITS;
    armP      arm     = state->arm;
    memEndian endian  = getEndian(state, bits);
    Bool      align   = getAlign(state, bits);

    // generate exception for misaligned or inaccessible address if required
    if(!arm->configInfo.align64as32) {
        vmimtTryLoadRC(bits, offset, ra, align);
    }

    // emit two word loads (the first into a temporary in case the second fails)
    vmimtLoadRRO(regBits, regBits, offset,   rt,  ra, endian, False, align);
    vmimtLoadRRO(regBits, regBits, offset+4, rdH, ra, endian, False, align);
    vmimtMoveRR(regBits, rdL, rt);
    setVariable(state, rdL, isReturn);
    setVariable(state, rdH, isReturn);
}

//
// trystore mem[ra+offset]  (when ra!=VMI_NOREG)
// trystore mem[offset]     (when ra==VMI_NOREG)
//
void armEmitTryStoreRC(
    armMorphStateP state,
    Uns32          bits,
    Addr           offset,
    vmiReg         ra
) {
    vmimtTryStoreRC(bits, offset, ra, getAlign(state, bits));
}


////////////////////////////////////////////////////////////////////////////////
// COMPARE OPERATIONS
////////////////////////////////////////////////////////////////////////////////

//
// flag = ra <cond> rb
//
void armEmitCompareRR(
    armMorphStateP state,
    Uns32          bits,
    vmiCondition   cond,
    vmiReg         ra,
    vmiReg         rb,
    vmiReg         flag
) {
    vmimtCompareRR(bits, cond, ra, rb, flag);
}

//
// flag = ra <cond> c
//
void armEmitCompareRC(
    armMorphStateP state,
    Uns32          bits,
    vmiCondition   cond,
    vmiReg         ra,
    Uns64          c,
    vmiReg         flag
) {
    vmimtCompareRC(bits, cond, ra, c, flag);
}


////////////////////////////////////////////////////////////////////////////////
// INTER-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to clear ITSTATE if required
//
static void emitClearITState(armMorphStateP state) {
    if(state->arm->itStateMT) {
        armEmitMoveRC(state, 8, ARM_IT_STATE, 0);
    }
}

//
// Set address mask to mask off the bottom bit of the target address
//
static void emitAddressMask(void) {
    vmimtSetAddressMask(~1);
}

//
// Perform an unconditional direct jump.
//
void armEmitUncondJump(
    armMorphStateP state,
    armJumpInfoP   ji
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtUncondJump(
        ji->linkPC,
        state->info.t,
        ji->linkReg,
        ji->hint
    );
}

//
// Perform an unconditional indirect jump.
//
void armEmitUncondJumpReg(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         toReg
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtUncondJumpReg(
        ji->linkPC,
        toReg,
        ji->linkReg,
        ji->hint
    );
}

//
// Perform a conditional direct jump if the condition flag is non-zero
// (jumpIfTrue) or zero (not jumpIfTrue).
//
void armEmitCondJump(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         flag,
    Bool           jumpIfTrue
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtCondJump(
        flag,
        jumpIfTrue,
        ji->linkPC,
        state->info.t,
        ji->linkReg,
        ji->hint
    );
}

//
// Jump to the label if the register masked with the mask is non-zero (if
// jumpIfNonZero) or zero (if !jumpIfNonZero)
//
static void emitJumpLabelOnMask(
    armMorphStateP state,
    Uns32          bits,
    vmiLabelP      label,
    vmiReg         reg,
    Uns32          mask,
    Bool           jumpIfNonZero
) {
    vmiCondition cond = jumpIfNonZero ? vmi_COND_NZ : vmi_COND_Z;
    vmimtTestRCJumpLabel(bits, cond, reg, mask, label);
}

//
// Emit code to interwork if required, depending on LSB of target address, or
// handle exception return
//
void armEmitInterworkLSB(armMorphStateP state, vmiReg ra) {

    Uns32     bits        = ARM_GPR_BITS;
    armP      arm         = state->arm;
    vmiLabelP noSwitch    = armEmitNewLabel();
    Bool      inThumbMode = IN_THUMB_MODE(arm);

    // special behavior is required if this is a possible handler mode exception
    // return
    if(!IN_USER_MODE(arm)) {

        vmiLabelP noExceptionReturn = vmimtNewLabel();

        // jump past exception return unless target address is the magic value
        vmimtCompareRCJumpLabel(
            bits, vmi_COND_C, ra, EXC_RETURN_MAGIC, noExceptionReturn
        );

        // jump past exception return unless processor is in handler mode
        vmimtTestRCJumpLabel(
            bits, vmi_COND_Z, ARM_PSR, PSR_EXCEPT_NUM, noExceptionReturn
        );

        // do exception return
        vmimtArgProcessor();
        vmimtArgReg(bits, ra);
        vmimtCall((vmiCallFn)armExceptionReturn);

        // here if not an exception return
        vmimtInsertLabel(noExceptionReturn);
    }

    // skip mode switch unless mode has changed
    emitJumpLabelOnMask(state, bits, noSwitch, ra, 1, inThumbMode);

    // switch processor mode
    armEmitInterwork(state);

    // here if no mode switch required
    armEmitInsertLabel(noSwitch);
}

//
// Check for mode switch depending on setting of bottom bit of target address
//
static void emitCheckSetMode(armMorphStateP state) {

    if(state->info.f) {

        // instruction variants that assign the PC and set flags are illegal
        Uns32 bits = ARM_GPR_BITS;

        // update UFSR (CFSR)
        vmimtBinopRC(
            bits, vmi_OR, ARM_SCS_REG(SCS_ID(CFSR)), EXC_UNDEF_UNDEFINSTR, 0
        );

        // take UsageFault exception
        vmimtArgProcessor();
        vmimtArgSimPC(bits);
        vmimtCall((vmiCallFn)armUsageFault);

    } else {

        // switch mode if LSB of PC implies a different mode
        if(state->setMode) {
            armEmitInterworkLSB(state, ARM_PC);
        }

        // mask off LSB of target address
        emitAddressMask();
    }
}

//
// Perform an implicit unconditional direct jump if required
//
void armEmitImplicitUncondJump(armMorphStateP state) {

    switch(state->pcSet) {

        case ASPC_NA:
            // no action unless PC was set by the instruction
            break;

        case ASPC_R15:
            // indirect jump to current value in R15
            emitCheckSetMode(state);
            emitClearITState(state);
            vmimtUncondJumpReg(0, ARM_PC, VMI_NOREG, vmi_JH_NONE);
            break;

        case ASPC_R15_RET:
            // return to current value in R15
            emitCheckSetMode(state);
            emitClearITState(state);
            vmimtUncondJumpReg(0, ARM_PC, VMI_NOREG, vmi_JH_RETURN);
            break;

        case ASPC_IMM:
            // direct jump to immediate address
            emitClearITState(state);
            vmimtUncondJump(0, state->pcImmediate, VMI_NOREG, vmi_JH_NONE);
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// INTRA-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Create and return a new label
//
vmiLabelP armEmitNewLabel(void) {
    return vmimtNewLabel();
}

//
// Insert a label previously created by vmimtNewLabel at the current location.
//
void armEmitInsertLabel(vmiLabelP label) {
    vmimtInsertLabel(label);
}

//
// Perform an unconditional jump to the passed local label.
//
void armEmitUncondJumpLabel(vmiLabelP toLabel) {
    vmimtUncondJumpLabel(toLabel);
}

//
// Perform a conditional jump if the condition flag is non-zero (jumpIfTrue)
// or zero (not jumpIfTrue). The target location is the passed local label.
//
void armEmitCondJumpLabel(vmiReg flag, Bool jumpIfTrue, vmiLabelP toLabel) {
    vmimtCondJumpLabel(flag, jumpIfTrue, toLabel);
}

//
// Test the register value by performing bitwise AND with the passed constant
// value, and jump to 'toLabel' if condition 'cond' is satisfied.
//
void armEmitTestRCJumpLabel(
    Uns32        bits,
    vmiCondition cond,
    vmiReg       r,
    Uns64        c,
    vmiLabelP    toLabel
) {
    vmimtTestRCJumpLabel(bits, cond, r, c, toLabel);
}

//
// Compare the register value by performing subtraction of the passed constant
// value, and jump to 'toLabel' if condition 'cond' is satisfied.
//
void armEmitCompareRCJumpLabel(
    Uns32        bits,
    vmiCondition cond,
    vmiReg       r,
    Uns64        c,
    vmiLabelP    toLabel
) {
    vmimtCompareRCJumpLabel(bits, cond, r, c, toLabel);
}


////////////////////////////////////////////////////////////////////////////////
// CALLBACK FUNCTION INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Add processor argument to the stack frame
//
void armEmitArgProcessor(armMorphStateP state) {
    vmimtArgProcessor();
}

//
// Add Uns32 argument to the stack frame
//
void armEmitArgUns32(armMorphStateP state, Uns32 arg) {
    vmimtArgUns32(arg);
}

//
// Add register argument to the stack frame
//
void armEmitArgReg(armMorphStateP state, Uns32 bits, vmiReg r) {
    vmimtArgReg(bits, r);
}

//
// Add program counter argument to the stack frame
//
void armEmitArgSimPC(armMorphStateP state, Uns32 bits) {
    vmimtArgSimPC(bits);
}

//
// Make a call with all current stack frame arguments
//
void armEmitCall(armMorphStateP state, vmiCallFn arg) {
    vmimtCall(arg);
}

//
// As above but generate a function result (placed in rd)
//
void armEmitCallResult(
    armMorphStateP state,
    vmiCallFn      arg,
    Uns32          bits,
    vmiReg         rd
) {
    setVariable(state, rd, False);
    vmimtCallResult(arg, bits, rd);
    maskResult(rd);
}


////////////////////////////////////////////////////////////////////////////////
// SIMULATOR CONTROL
////////////////////////////////////////////////////////////////////////////////

//
// Stop simulation of the current processor
//
void armEmitExit(void) {
    vmimtExit();
}

//
// Emit code to wait for the passed reason
//
void armEmitWait(armMorphStateP state, armDisable reason) {
    vmimtBinopRC(8, vmi_OR, ARM_DISABLE_REASON, reason, 0);
    vmimtHalt();
}

//
// Terminate the current block
//
void armEmitEndBlock(void) {
    vmimtEndBlock();
}

//
// Emit code to validate the current block mode
//
void armEmitValidateBlockMask(armBlockMask blockMask) {
    vmimtValidateBlockMask(blockMask);
}


