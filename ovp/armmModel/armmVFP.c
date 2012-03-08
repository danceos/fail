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

#include <math.h>

// VMI header files
#include "vmi/vmiRt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armFPConstants.h"
#include "armMessage.h"
#include "armVFP.h"
#include "armSysRegisters.h"
#include "armStructure.h"
#include "armUtils.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARMM_VFP"

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}


////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT CONTROL PREDICATES
////////////////////////////////////////////////////////////////////////////////

//
// Return the current rounding control
//
static vmiFPRC getRoundingControl(armP arm) {

    // map ARM rounding mode to VMI rounding mode
    static const vmiFPRC modeMap[] = {
        [0] = vmi_FPR_NEAREST,
        [1] = vmi_FPR_POS_INF,
        [2] = vmi_FPR_NEG_INF,
        [3] = vmi_FPR_ZERO
    };

    if(SCS_USE_CPUID(arm) && !SCS_FIELD(arm, MVFR0, VFP_RoundingModes)) {
        // if rounding modes are not supported, use vmi_FPR_NEAREST
        return vmi_FPR_NEAREST;
    } else {
        // return the mapped rounding mode
        return modeMap[FPSCR_FIELD(arm, RMode)];
    }
}

//
// Should the default NaN be returned in any calculation that produces a NaN?
//
inline static Bool returnDefaultNaN(armP arm) {
    return (
        // in VFP mode, return the default NaN if FPSCR.DN is set
        FPSCR_FIELD(arm, DN) ||
        // MVFR1.DefaultNaNMode=0 indicates hardware supports only the default
        // NaN mode
        (SCS_USE_CPUID(arm) && !SCS_FIELD(arm, MVFR1, DefaultNaNMode))
    );
}

//
// Is flush-to-zero mode enabled?
//
inline static Bool inFTZMode(armP arm) {
    return (
        // flush-to-zero is enabled if FPSCR.FZ is set
        FPSCR_FIELD(arm, FZ) ||
        // MVFR1.FlushToZeroMode=0 indicates hardware supports only
        // flush-to-zero mode
        (SCS_USE_CPUID(arm) && !SCS_FIELD(arm, MVFR1, FlushToZeroMode))
    );
}

//
// Is alternative half precision mode enabled?
//
inline static Bool inAHPMode(armP arm) {
    return FPSCR_FIELD(arm, AHP);
}

////////////////////////////////////////////////////////////////////////////////
// FLOATING REGISTER ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// Set control word
//
inline static void setControlWord(armP arm, armFPCW new) {
    if(arm->currentCW.u32 != new.u32) {
        arm->currentCW = new;
        vmirtSetFPControlWord((vmiProcessorP)arm, new.cw);
    }
}

//
// Update VFP control word to make it consistent with the FPSCR
//
static void updateVFPControlWord(armP arm) {

    // Mask off all exceptions */
    armFPCW newFPCW = {.cw.IM=1, .cw.DM=1, .cw.ZM=1, .cw.OM=1, .cw.UM=1, .cw.PM=1};

    // set rounding control, denormals-are-zero and flush-to-zero
    newFPCW.cw.RC  = getRoundingControl(arm);
    newFPCW.cw.DAZ = newFPCW.cw.FZ = inFTZMode(arm);

    // Set the new control word
    setControlWord(arm, newFPCW);

}

//
// Write VFP FPSCR register
//
void armWriteFPSCR(armP arm, Uns32 newValue, Uns32 writeMask) {

    Uns32 oldValue = FPSCR_REG(arm);

    // update raw register
    FPSCR_REG(arm) = ((oldValue & ~writeMask) | (newValue & writeMask));

    // set denormal input sticky flag
    arm->denormalInput = FPSCR_FIELD(arm, IDC);

    // set sticky flags
    vmiFPFlags sticky = {0};
    sticky.f.I = FPSCR_FIELD(arm, IOC);   // invalid operation flag
    sticky.f.Z = FPSCR_FIELD(arm, DZC);   // divide-by-zero flag
    sticky.f.O = FPSCR_FIELD(arm, OFC);   // overflow flag
    sticky.f.U = FPSCR_FIELD(arm, UFC);   // underflow flag
    sticky.f.P = FPSCR_FIELD(arm, IXC);   // inexact flag
    arm->sdfpSticky = sticky.bits;

    // set comparison flags
    arm->sdfpAFlags.NF = (FPSCR_FIELD(arm, N)) ? 1 : 0;
    arm->sdfpAFlags.ZF = (FPSCR_FIELD(arm, Z)) ? 1 : 0;
    arm->sdfpAFlags.CF = (FPSCR_FIELD(arm, C)) ? 1 : 0;
    arm->sdfpAFlags.VF = (FPSCR_FIELD(arm, V)) ? 1 : 0;

    // update simulator floating point control word
    updateVFPControlWord(arm);

}

//
// Read VFP FPSCR register
//
Uns32 armReadFPSCR(armP arm) {

    // compose denormal input flag
    FPSCR_FIELD(arm, IDC) = arm->denormalInput;

    // compose remaining sticky flags
    vmiFPFlags sticky = {arm->sdfpSticky};
    FPSCR_FIELD(arm, IOC) = sticky.f.I;   // invalid operation flag
    FPSCR_FIELD(arm, DZC) = sticky.f.Z;   // divide-by-zero flag
    FPSCR_FIELD(arm, OFC) = sticky.f.O;   // overflow flag
    FPSCR_FIELD(arm, UFC) = sticky.f.U;   // underflow flag
    FPSCR_FIELD(arm, IXC) = sticky.f.P;   // inexact flag

    // compose comparison flags
    FPSCR_FIELD(arm, N) = (arm->sdfpAFlags.NF) ? 1 : 0;
    FPSCR_FIELD(arm, Z) = (arm->sdfpAFlags.ZF) ? 1 : 0;
    FPSCR_FIELD(arm, C) = (arm->sdfpAFlags.CF) ? 1 : 0;
    FPSCR_FIELD(arm, V) = (arm->sdfpAFlags.VF) ? 1 : 0;

    // return composed value
    return FPSCR_REG(arm);
}

////////////////////////////////////////////////////////////////////////////////
// EXCEPTION FLAG UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Validate that an exception is disabled
//
inline static void validateFTZ(armP arm) {
    VMI_ASSERT(
        inFTZMode(arm),
        "expected enabled flush-to-zero mode"
    );
}

//
// Set the denormal sticky bit (when flush-to-zero mode is enabled)
//
static void setDenormalStickyFTZ(armP arm) {
    validateFTZ(arm);
    arm->denormalInput = 1;
}

//
// Set the underflow sticky bit (when flush-to-zero mode is enabled)
//
static void setUnderflowStickyFTZ(armP arm) {
    validateFTZ(arm);
    const static vmiFPFlags underflow = {f:{U:1}};
    arm->sdfpSticky |= underflow.bits;
}

//
// Raise the denormal exception
//
static void raiseDenormal(armP arm) {
    arm->denormalInput = 1;
}

//
// Raise the underflow exception
//
static void raiseUnderflow(armP arm) {
    const static vmiFPFlags underflow = {f:{U:1}};
    arm->sdfpSticky |= underflow.bits;
}

//
// Raise the overflow exception
//
static void raiseOverflow(armP arm) {
    const static vmiFPFlags overflow = {f:{O:1}};
    arm->sdfpSticky |= overflow.bits;
}

//
// Raise the invalid operation exception
//
static void raiseInvalidOperation(armP arm) {
    const static vmiFPFlags invalidOperation = {f:{I:1}};
    arm->sdfpSticky |= invalidOperation.bits;
}

//
// Raise the inexact exception
//
static void raiseInexact(armP arm) {
    const static vmiFPFlags inexact = {f:{P:1}};
    arm->sdfpSticky |= inexact.bits;
}


////////////////////////////////////////////////////////////////////////////////
// HALF-PRECISION OPERATIONS
////////////////////////////////////////////////////////////////////////////////

//
// Convert from half-precision to single-precision
//
Uns32 armFPHalfToSingle(armP arm, Uns16 half) {

    Int32 exponent   = ARM_FP16_EXPONENT(half);
    Uns32 fraction   = ARM_FP16_FRACTION(half);
    Bool  sign       = ARM_FP16_SIGN(half);
    Bool  expAll1    = (exponent==ARM_FP16_EXP_ONES);
    Bool  isZero     = (!exponent && !fraction);
    Bool  isInfinity = (expAll1 && !fraction);
    Bool  isNaN      = (expAll1 && !isInfinity);
    Bool  useAHP     = inAHPMode(arm);

    if(isInfinity && !useAHP) {

        // infinities require exponent correction
        exponent = ARM_FP32_EXPONENT(ARM_FP32_PLUS_INFINITY);
        fraction = ARM_FP32_FRACTION(ARM_FP32_PLUS_INFINITY);

    } else if(isZero) {

        // zero values require no special processing

    } else if(isNaN && !useAHP) {

        // argument is an SNaN if MSB of fraction is clear
        Bool isSNaN = !(fraction & (1 << (ARM_FP16_EXP_SHIFT-1)));

        // raise Invalid Operation exception if an SNaN
        if(isSNaN) {
            raiseInvalidOperation(arm);
        }

        if(returnDefaultNaN(arm)) {

            // default QNaN is required
            exponent = ARM_FP32_EXPONENT(ARM_QNAN_DEFAULT_32);
            fraction = ARM_FP32_FRACTION(ARM_QNAN_DEFAULT_32);
            sign     = ARM_FP32_SIGN(ARM_QNAN_DEFAULT_32);

        } else {

            // NaNs require exponent correction
            exponent = ARM_FP32_EXP_ONES;

            // NaNs require fraction correction
            fraction  = fraction << (ARM_FP32_EXP_SHIFT - ARM_FP16_EXP_SHIFT);
            fraction |= ARM_QNAN_MASK_32;
        }

    } else {

        // normalize a denormal value if required
        if(!exponent) {

            // shift up until implicit MSB is 1
            do {
                fraction <<= 1;
                exponent--;
            } while(!(fraction & (1<<ARM_FP16_EXP_SHIFT)));

            // correct exponent and mask off implicit MSB
            exponent++;
            fraction &= ~(1<<ARM_FP16_EXP_SHIFT);
        }

        // rebase exponent and fraction
        exponent +=  (ARM_FP32_EXP_BIAS  - ARM_FP16_EXP_BIAS );
        fraction <<= (ARM_FP32_EXP_SHIFT - ARM_FP16_EXP_SHIFT);
    }

    // compose single-precision result
    return (
        (sign     << ARM_FP32_SIGN_SHIFT) |
        (exponent << ARM_FP32_EXP_SHIFT ) |
        fraction
    );
}

//
// Convert from single-precision to half-precision
//
Uns16 armFPSingleToHalf(armP arm, Uns32 single) {

    Int32 exponent   = ARM_FP32_EXPONENT(single);
    Uns32 fraction   = ARM_FP32_FRACTION(single);
    Bool  sign       = ARM_FP32_SIGN(single);
    Bool  expAll1    = (exponent==ARM_FP32_EXP_ONES);
    Bool  isZero     = (!exponent && !fraction);
    Bool  isDenormal = (!exponent &&  fraction);
    Bool  isInfinity = (expAll1 && !fraction);
    Bool  isNaN      = (expAll1 && !isInfinity);
    Bool  useAHP     = inAHPMode(arm);

    if(isInfinity) {

        if(useAHP) {

            // alternative half precision has a different infinity format
            exponent = ARM_FP16_EXPONENT(ARM_FP16_AHP_INFINITY);
            fraction = ARM_FP16_FRACTION(ARM_FP16_AHP_INFINITY);

            raiseInvalidOperation(arm);

        } else {

            // IEEE infinities require exponent correction
            exponent = ARM_FP16_EXPONENT(ARM_FP16_PLUS_INFINITY);
            fraction = ARM_FP16_FRACTION(ARM_FP16_PLUS_INFINITY);
        }

    } else if(isZero) {

        // zero values require no special processing

    } else if(isNaN) {

        // argument is an SNaN if MSB of fraction is clear
        Bool isSNaN = !(fraction & (1 << (ARM_FP32_EXP_SHIFT-1)));

        // raise Invalid Operation exception if using Alternative Half Precision
        // format or argument is an SNaN
        if(isSNaN || useAHP) {
            raiseInvalidOperation(arm);
        }

        if(useAHP) {

            // if using Alternative Half Precision format, NaNs go to zero
            exponent = 0;
            fraction = 0;

        } else if(returnDefaultNaN(arm)) {

            // default QNaN is required
            exponent = ARM_FP16_EXPONENT(ARM_QNAN_DEFAULT_16);
            fraction = ARM_FP16_FRACTION(ARM_QNAN_DEFAULT_16);
            sign     = ARM_FP16_SIGN(ARM_QNAN_DEFAULT_16);

        } else {

            // NaNs require exponent correction
            exponent = ARM_FP16_EXP_ONES;

            // NaNs require fraction correction
            fraction  = fraction >> (ARM_FP32_EXP_SHIFT - ARM_FP16_EXP_SHIFT);
            fraction |= ARM_QNAN_MASK_16;
        }

    } else if(isDenormal && inFTZMode(arm)) {

        // flush denormals to zero if required
        setDenormalStickyFTZ(arm);
        exponent = 0;
        fraction = 0;

    } else {

        // split value into sign, unrounded mantissa and exponent
        union {Uns32 u32; Flt32 f32;} u = {single};
        Flt32 result   = u.f32;
        Flt32 mantissa = sign ? -result : result;
        exponent = 0;
        while(mantissa<1.0) {
            mantissa *= 2.0; exponent--;
        }
        while(mantissa>=2.0) {
            mantissa /= 2.0; exponent++;
        }

        // bias the exponent so that the minimum exponent becomes 1, lower
        // values 0 (indicating possible underflow)
        exponent -= ARM_FP16_EXP_MIN;
        while(exponent<0) {
            exponent++;
            mantissa /= 2.0;
        }

        // get the unrounded mantissa as an integer and the "units in last
        // place" rounding error
        Flt32 mantissaExp2F = mantissa*(1<<ARM_FP16_EXP_SHIFT);
        fraction            = mantissaExp2F;
        Flt32 error         = mantissaExp2F - fraction;

        // there is an underflow exception if the exponent is too small before
        // rounding and the result is inexact
        if(!exponent && error) {
            raiseUnderflow(arm);
        }

        Bool roundUp;
        Bool overflowToInf;

        // now round using the required rounding mode
        switch(getRoundingControl(arm)) {

            case vmi_FPR_NEAREST:
                roundUp       = ((error>0.5) || ((error==0.5) && (fraction&1)));
                overflowToInf = True;
                break;

            case vmi_FPR_POS_INF:
                roundUp       = (error && !sign);
                overflowToInf = !sign;
                break;

            case vmi_FPR_NEG_INF:
                roundUp       = (error && sign);
                overflowToInf = sign;
                break;

            default:
                roundUp       = False;
                overflowToInf = False;
                break;
        }

        if(roundUp) {

            fraction++;

            // check for round up from denormalized to normalized
            if(fraction==(1<<ARM_FP16_EXP_SHIFT)) {
                exponent = 1;
            }

            // check for round up to next exponent
            if(fraction==(1<<(ARM_FP16_EXP_SHIFT+1))) {
                exponent++;
                fraction /= 2;
            }
        }

        // deal with overflow and generate result
        if(!useAHP) {

            // IEEE format result required
            if(exponent>=((1<<ARM_FP16_EXP_BITS)-1)) {

                if(overflowToInf) {
                    exponent = ARM_FP16_EXPONENT(ARM_FP16_PLUS_INFINITY);
                    fraction = ARM_FP16_FRACTION(ARM_FP16_PLUS_INFINITY);
                } else {
                    exponent = ARM_FP16_EXPONENT(ARM_FP16_MAX_NORMAL);
                    fraction = ARM_FP16_FRACTION(ARM_FP16_MAX_NORMAL);
                }

                raiseOverflow(arm);

                // Note this is not in the psuedo code but is needed to match
                // behavior of reference simulator
                raiseInexact(arm);

            }

        } else {

            // alternative half precision result required
            if(exponent>=(1<<ARM_FP16_EXP_BITS)) {

                exponent = ARM_FP16_EXPONENT(ARM_FP16_AHP_INFINITY);
                fraction = ARM_FP16_FRACTION(ARM_FP16_AHP_INFINITY);

                raiseInvalidOperation(arm);

                // avoid an inexact exception
                error = 0.0;
            }
        }

        // deal with inexact exception
        if(error) {
            raiseInexact(arm);
        }
    }

    // compose half-precision result
    return (
        (sign     << ARM_FP16_SIGN_SHIFT) |
        (exponent << ARM_FP16_EXP_SHIFT ) |
        fraction
    );
}


////////////////////////////////////////////////////////////////////////////////
// INFINITY/ZERO CHECK
////////////////////////////////////////////////////////////////////////////////

//
// Return True if the single-precision floating point values op1 and op2 are 0
// and infinity (in either order), setting the denormal sticky bit if so
//
Bool armFPInfinityAndZero(armP arm, Uns32 op1, Uns32 op2) {

    Bool op1Zero = ((op1 & 0x7f800000) == 0);
    Bool op2Zero = ((op2 & 0x7f800000) == 0);
    Bool op1Inf  = ((op1 & 0x7fffffff) == 0x7f800000);
    Bool op2Inf  = ((op2 & 0x7fffffff) == 0x7f800000);

    if((op1Zero && op2Inf) || (op2Zero && op1Inf)) {

        // check if an op is denormal number
        if ((op1Zero && (op1 & 0x007fffff) != 0) ||
            (op2Zero && (op2 & 0x007fffff) != 0))
        {
            // raise denormal exception
            raiseDenormal(arm);
        }

        return True;
    }

    return False;
}


////////////////////////////////////////////////////////////////////////////////
// QNAN/TINY HANDLING
////////////////////////////////////////////////////////////////////////////////

//
// Is the argument a 32-bit QNaN?
//
inline static Bool is32BitQNaN(vmiFPNaNArgP arg) {
    return arg->isFlt32 && (arg->u32 & ARM_QNAN_MASK_32);
}

//
// Is the argument a 64-bit QNaN?
//
inline static Bool is64BitQNaN(vmiFPNaNArgP arg) {
    return !arg->isFlt32 && (arg->u64 & ARM_QNAN_MASK_64);
}

//
// Is the argument a 32-bit SNaN?
//
inline static Bool is32BitSNaN(vmiFPNaNArgP arg) {
    return arg->isFlt32 && !(arg->u32 & ARM_QNAN_MASK_32);
}

//
// Is the argument a 64-bit SNaN?
//
inline static Bool is64BitSNaN(vmiFPNaNArgP arg) {
    return !arg->isFlt32 && !(arg->u64 & ARM_QNAN_MASK_64);
}

//
// Handle 32-bit QNaN result
//
static VMI_FP_QNAN32_RESULT_FN(handleQNaN32) {

    armP arm = (armP)processor;

    if(!NaNArgNum || returnDefaultNaN(arm)) {

        return ARM_QNAN_DEFAULT_32;

    } else {

        Uns32 i;

        // PASS 1: if any argument is an 32-bit SNaN, return that (as a QNaN)
        for(i=0; i<NaNArgNum; i++) {
            if(is32BitSNaN(&NaNArgs[i])) {
                return NaNArgs[i].u32 | ARM_QNAN_MASK_32;
            }
        }

        // PASS 2: if any argument is a 32-bit QNaN, return that
        for(i=0; i<NaNArgNum; i++) {
            if(is32BitQNaN(&NaNArgs[i])) {
                return NaNArgs[i].u32;
            }
        }

        // otherwise, if no argument is a 32-bit NaN (e.g. this is a conversion
        // from 64-bit) return the calculated result
        return QNaN32;
    }
}

//
// Handle 64-bit QNaN result
//
static VMI_FP_QNAN64_RESULT_FN(handleQNaN64) {

    armP arm = (armP)processor;

    if(!NaNArgNum || returnDefaultNaN(arm)) {

        return ARM_QNAN_DEFAULT_64;

    } else {

        Uns32 i;

        // PASS 1: if any argument is an 64-bit SNaN, return that (as a QNaN)
        for(i=0; i<NaNArgNum; i++) {
            if(is64BitSNaN(&NaNArgs[i])) {
                return NaNArgs[i].u64 | ARM_QNAN_MASK_64;
            }
        }

        // PASS 2: if any argument is a 64-bit QNaN, return that
        for(i=0; i<NaNArgNum; i++) {
            if(is64BitQNaN(&NaNArgs[i])) {
                return NaNArgs[i].u64;
            }
        }

        // otherwise, if no argument is a 64-bit NaN (e.g. this is a conversion
        // from 32-bit) return the calculated result
        return QNaN64;
    }
}

//
// Is the passed value a QNaN or SNaN?
//
static Bool isNaN80(vmiFP80Arg value) {

    // decompose vmiFP80Arg into mantissa and sign
    union {vmiFP80Arg fp80; struct {Uns64 mantissa; Uns16 expSign;};} u = {
        value
    };

    if((u.expSign & 0x7fff)!=0x7fff) {
        // exponent most be all 1's
        return False;
    } else if(u.mantissa==0x8000000000000000ULL) {
        // +infinity, -infinity
        return False;
    } else {
        // NaNs have some non-zero bit in the mantissa
        return u.mantissa;
    }
}

//
// Handle 16-bit indeterminate result
//
static VMI_FP_IND16_RESULT_FN(handleIndeterminate16) {

    Uns32 result;

    if(isNaN80(value)) {
        result = 0;
    } else if(value.bytes[VMI_FP_80_BYTES-1] & 0x80) {
        result = isSigned ? ARM_MIN_INT16 : ARM_MIN_UNS16;
    } else {
        result = isSigned ? ARM_MAX_INT16 : ARM_MAX_UNS16;
    }

    return result;
}

//
// Handle 32-bit indeterminate result
//
static VMI_FP_IND32_RESULT_FN(handleIndeterminate32) {

    Uns32 result;

    if(isNaN80(value)) {
        result = 0;
    } else if(value.bytes[VMI_FP_80_BYTES-1] & 0x80) {
        result = isSigned ? ARM_MIN_INT32 : ARM_MIN_UNS32;
    } else {
        result = isSigned ? ARM_MAX_INT32 : ARM_MAX_UNS32;
    }

    return result;
}

//
// Enumeration of tiny values that might be returned by flushing
//
typedef enum tinyValueE {
    TV_PLUS_0,
    TV_MINUS_0,
    TV_LAST
} tinyValue;

//
// These contain tiny values
//
static vmiFP80Arg tinyValues32[TV_LAST];

//
// Initialize constant tiny 32-bit values
//
static void initTinyValue32(tinyValue tv, Uns32 pattern) {
    union {Uns32 u32; Flt32 f32;} u = {pattern};
    tinyValues32[tv].f80 = u.f32;
}

//
// Initialize constant tiny values
//
static void initTinyValues(void) {
    initTinyValue32(TV_PLUS_0,  0x00000000);
    initTinyValue32(TV_MINUS_0, 0x80000000);
}

//
// Handle 32-bit tiny result
//
static VMI_FP_TINY_RESULT_FN(handleTinyResult) {

    // set underflow sticky bit
    setUnderflowStickyFTZ((armP)processor);

    // return appropriately-signed zero
    Bool isNegative = value.bytes[VMI_FP_80_BYTES-1] & 0x80;
    return tinyValues32[isNegative ? TV_MINUS_0 : TV_PLUS_0];
}

//
// Handle 64-bit tiny argument
//
static VMI_FP_TINY_ARGUMENT_FN(handleTinyArgument) {

    // set denormal sticky bit
    setDenormalStickyFTZ((armP)processor);

    // return appropriately-signed zero
    Bool isNegative = value.bytes[VMI_FP_80_BYTES-1] & 0x80;
    return tinyValues32[isNegative ? TV_MINUS_0 : TV_PLUS_0];
}


////////////////////////////////////////////////////////////////////////////////
// RESET AND INITIALIZATION
////////////////////////////////////////////////////////////////////////////////

//
// Call on initialization
//
void armFPInitialize(armP arm) {

    // initialize current control word to an empty value
    arm->currentCW.u32 = -1;

    // Initialize FP registers if present (AFTER armSysInitialize)
    if (SCS_USE_CPUID(arm) && FPU_PRESENT(arm)) {

        // initialize constant tiny values
        initTinyValues();

        // set up QNaN values and handlers
        vmirtConfigureFPU(
            (vmiProcessorP)arm,
            ARM_QNAN_DEFAULT_32,
            ARM_QNAN_DEFAULT_64,
            0,  // indeterminate value not used
            0,  // indeterminate value not used
            0,  // indeterminate value not used
            handleQNaN32,
            handleQNaN64,
            handleIndeterminate16,
            handleIndeterminate32,
            0,  // 64-bit indeterminate value handler not required
            handleTinyResult,
            handleTinyArgument,
            True
        );

        // Set the write mask for CPACR to allow updating field cp10 and cp11
        SCS_MASK_FIELD(arm, CPACR, cp10) = 3;
        SCS_MASK_FIELD(arm, CPACR, cp11) = 3;

    }
}

//
// Call on reset
//
void armFPReset(armP arm) {

    // no action unless floating point is implemented
    if(SCS_USE_CPUID(arm) && FPU_PRESENT(arm)) {
        armWriteFPSCR(arm, 0, FPSCR_MASK);
    }
}



