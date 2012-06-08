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

#ifndef ARM_EMIT_H
#define ARM_EMIT_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armMode.h"
#include "armTypeRefs.h"


////////////////////////////////////////////////////////////////////////////////
// INTERWORKING
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to interwork if required, depending on LSB of target address
//
void armEmitInterworkLSB(armMorphStateP state, vmiReg ra);


////////////////////////////////////////////////////////////////////////////////
// INTEGER OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// rd = simPC (true)
//
void armEmitGetTruePC(armMorphStateP state, vmiReg rd);

//
// r15 = simPC (accoring to ARM pipeline)
//
void armEmitGetPC(armMorphStateP state);

//
// rd = c
//
void armEmitMoveRC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    Uns64          c
);

//
// rd = ra
//
void armEmitMoveRR(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         ra
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

//
// Generate shift mask prefix (sets mask to 255)
//
void armEmitSetShiftMask(void);

//
// rdh:rdl = ra*rb
//
void armEmitMulopRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rdh,
    vmiReg         rdl,
    vmiReg         ra,
    vmiReg         rb,
    vmiFlagsCP     flags
);


////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT OPCODES
////////////////////////////////////////////////////////////////////////////////

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
);

//
// fd = fa <binop> fb
//
void armEmitFBinopSimdRRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFBinop      op,
    vmiReg         fd,
    vmiReg         fa,
    vmiReg         fb
);

//
// fd = <unop> fa
//
void armEmitFUnopSimdRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFUnop       op,
    vmiReg         fd,
    vmiReg         fa
);

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
);

//
// compare fa to fb, setting relation and flags
//
void armEmitFCompareRR(
    armMorphStateP state,
    vmiFType       type,
    vmiReg         relation,
    vmiReg         fa,
    vmiReg         fb,
    Bool           allowQNaN,
    Bool           setSticky
);


////////////////////////////////////////////////////////////////////////////////
// LOAD AND STORE OPCODES
////////////////////////////////////////////////////////////////////////////////

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
);

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
);

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
);

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
);

//
// trystore mem[ra+offset]  (when ra!=VMI_NOREG)
// trystore mem[offset]     (when ra==VMI_NOREG)
//
void armEmitTryStoreRC(
    armMorphStateP state,
    Uns32          bits,
    Addr           offset,
    vmiReg         ra
);


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
);

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
);


////////////////////////////////////////////////////////////////////////////////
// INTER-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Structure holding information about a jump
//
typedef struct armJumpInfoS {
    vmiReg      linkReg;    // link register
    vmiJumpHint hint;       // call hint
    Uns32       linkPC;     // address to put in link register
} armJumpInfo, *armJumpInfoP;

//
// Perform an unconditional direct jump.
//
void armEmitUncondJump(
    armMorphStateP state,
    armJumpInfoP   ji
);

//
// Perform an unconditional indirect jump.
//
void armEmitUncondJumpReg(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         toReg
);

//
// Perform a conditional direct jump if the condition flag is non-zero
// (jumpIfTrue) or zero (not jumpIfTrue).
//
void armEmitCondJump(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         flag,
    Bool           jumpIfTrue
);

//
// Perform an implicit unconditional direct jump if required
//
void armEmitImplicitUncondJump(armMorphStateP state);


////////////////////////////////////////////////////////////////////////////////
// INTRA-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Create and return a new label.
//
vmiLabelP armEmitNewLabel(void);

//
// Insert a label previously created by vmimtNewLabel at the current location.
//
void armEmitInsertLabel(vmiLabelP label);

//
// Perform an unconditional jump to the passed local label.
//
void armEmitUncondJumpLabel(vmiLabelP toLabel);

//
// Perform a conditional jump if the condition flag is non-zero (jumpIfTrue)
// or zero (not jumpIfTrue). The target location is the passed local label.
//
void armEmitCondJumpLabel(vmiReg flag, Bool jumpIfTrue, vmiLabelP toLabel);

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
);

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
);


////////////////////////////////////////////////////////////////////////////////
//// CALLBACK FUNCTION INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Add processor argument to the stack frame
//
void armEmitArgProcessor(armMorphStateP state);

//
// Add Uns32 argument to the stack frame
//
void armEmitArgUns32(armMorphStateP state, Uns32 arg);

//
// Add register argument to the stack frame
//
void armEmitArgReg(armMorphStateP state, Uns32 bits, vmiReg r);

//
// Add program counter argument to the stack frame
//
void armEmitArgSimPC(armMorphStateP state, Uns32 bits);

//
// Make a call with all current stack frame arguments
//
void armEmitCall(armMorphStateP state, vmiCallFn arg);

//
// As above but generate a function result (placed in rd)
//
void armEmitCallResult(
    armMorphStateP state,
    vmiCallFn      arg,
    Uns32          bits,
    vmiReg         rd
);


////////////////////////////////////////////////////////////////////////////////
// SIMULATOR CONTROL
////////////////////////////////////////////////////////////////////////////////

//
// Stop simulation of the current processor
//
void armEmitExit(void);

//
// Emit code to wait for the passed reason
//
void armEmitWait(armMorphStateP state, armDisable reason);

//
// Terminate the current block
//
void armEmitEndBlock(void);

//
// Emit code to validate the current block mode
//
void armEmitValidateBlockMask(armBlockMask blockMask);

#endif
