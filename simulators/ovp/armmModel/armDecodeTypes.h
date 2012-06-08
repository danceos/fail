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

#ifndef ARM_DECODE_TYPES_H
#define ARM_DECODE_TYPES_H

// basic number types
#include "hostapi/impTypes.h"

// model header files
#include "armVariant.h"


//
// Instruction type for an instruction with a single variant
//
#define ITYPE_SINGLE(_NAME) \
    ARM_IT_##_NAME

//
// Instruction types for normal instructions like ADC
//
#define ITYPE_SET_ADC(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_RRX,            \
    ARM_IT_##_NAME##_IT,                \
    ARM_IT_##_NAME##_RT

//
// Instruction types for normal instructions like LDC
//
#define ITYPE_SET_LDC(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_UNINDEXED

//
// Instruction types for normal instructions like LDR
//
#define ITYPE_SET_LDR(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM

//
// Instruction types for normal instructions like LDRH
//
#define ITYPE_SET_LDRH(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM                 \

//
// Instruction types for normal instructions like MOV
//
#define ITYPE_SET_MOV(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_SHFT_RS,        \
    ARM_IT_##_NAME##_RM_RRX,            \
    ARM_IT_##_NAME##_RM_SHFT_RST

//
// Instruction types for DSP instructions like SMLA<x><y>
//
#define ITYPE_SET_SMLA_XY(_NAME) \
     ARM_IT_##_NAME##BB,                \
     ARM_IT_##_NAME##BT,                \
     ARM_IT_##_NAME##TB,                \
     ARM_IT_##_NAME##TT

//
// Instruction types for DSP instructions like SMLAW<y>
//
#define ITYPE_SET_SMLAW_Y(_NAME) \
     ARM_IT_##_NAME##B,                 \
     ARM_IT_##_NAME##T

//
// Instruction types for normal instructions like PLD
//
#define ITYPE_SET_PLD(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM

//
// Instruction types for parallel add/subtract Media instructions
//
#define ITYPE_SET_PAS(_NAME) \
    ARM_IT_S##_NAME,                    \
    ARM_IT_Q##_NAME,                    \
    ARM_IT_SH##_NAME,                   \
    ARM_IT_U##_NAME,                    \
    ARM_IT_UQ##_NAME,                   \
    ARM_IT_UH##_NAME

//
// Instruction types for Media instructions with optional argument exchange
//
#define ITYPE_MEDIA_X(_NAME) \
    ARM_IT_##_NAME,                     \
    ARM_IT_##_NAME##X

//
// Instruction types for Media instructions with optional result rounding
//
#define ITYPE_MEDIA_R(_NAME) \
    ARM_IT_##_NAME,                     \
    ARM_IT_##_NAME##R

//
// Instruction types for VFP instructions with D and S variants
//
#define ITYPE_VFP_DS(_NAME)       \
    ARM_IT_##_NAME##_D,           \
    ARM_IT_##_NAME##_S

//
// Instruction type enumeration
//
typedef enum armInstructionTypeE {

    ////////////////////////////////////////////////////////////////////////////
    // ARM INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SET_ADC (ADC),
    ITYPE_SET_ADC (ADD),
    ITYPE_SET_ADC (AND),
    ITYPE_SET_ADC (BIC),
    ITYPE_SET_ADC (EOR),
    ITYPE_SET_MOV (MOV),
    ITYPE_SET_ADC (MUL),
    ITYPE_SET_MOV (MVN),
    ITYPE_SET_MOV (NEG),
    ITYPE_SET_ADC (ORN),
    ITYPE_SET_ADC (ORR),
    ITYPE_SET_ADC (RSB),
    ITYPE_SET_ADC (RSC),
    ITYPE_SET_ADC (SBC),
    ITYPE_SET_ADC (SUB),

    // ARMv6T2 move instructions
    ITYPE_SINGLE (MOVT),
    ITYPE_SINGLE (MOVW),

    // multiply instructions
    ITYPE_SINGLE (MLA  ),
    ITYPE_SINGLE (MLS  ),
    ITYPE_SINGLE (MUL  ),
    ITYPE_SINGLE (SMLAL),
    ITYPE_SINGLE (SMULL),
    ITYPE_SINGLE (UMAAL),
    ITYPE_SINGLE (UMLAL),
    ITYPE_SINGLE (UMULL),

    // compare instructions
    ITYPE_SET_ADC (CMN),
    ITYPE_SET_ADC (CMP),
    ITYPE_SET_ADC (TEQ),
    ITYPE_SET_ADC (TST),

    // branch instructions
    ITYPE_SINGLE (B   ),
    ITYPE_SINGLE (BL  ),
    ITYPE_SINGLE (BLX2),
    ITYPE_SINGLE (BX  ),

    // miscellaneous instructions
    ITYPE_SINGLE (BKPT),
    ITYPE_SINGLE (CLZ ),
    ITYPE_SINGLE (SWI ),

    // load and store instructions
    ITYPE_SET_LDR  (LDR  ),
    ITYPE_SET_LDR  (LDRB ),
    ITYPE_SET_LDR  (LDRBT),
    ITYPE_SET_LDRH (LDRH ),
    ITYPE_SET_LDRH (LDRSB),
    ITYPE_SET_LDRH (LDRSH),
    ITYPE_SET_LDR  (LDRT ),
    ITYPE_SET_LDR  (STR  ),
    ITYPE_SET_LDR  (STRB ),
    ITYPE_SET_LDR  (STRBT),
    ITYPE_SET_LDRH (STRH ),
    ITYPE_SET_LDR  (STRT ),

    // load and store multiple instructions
    ITYPE_SINGLE (LDM1),
    ITYPE_SINGLE (STM1),

    // ARMv6T2 load and store instructions
    ITYPE_SET_LDRH (LDRHT ),
    ITYPE_SET_LDRH (LDRSBT),
    ITYPE_SET_LDRH (LDRSHT),
    ITYPE_SET_LDRH (STRHT ),

    // synchronization primitives
    ITYPE_SINGLE (LDREX ),
    ITYPE_SINGLE (LDREXB),
    ITYPE_SINGLE (LDREXH),
    ITYPE_SINGLE (STREX ),
    ITYPE_SINGLE (STREXB),
    ITYPE_SINGLE (STREXH),

    // coprocessor instructions
    ITYPE_SINGLE  (CDP ),
    ITYPE_SINGLE  (CDP2),
    ITYPE_SET_LDC (LDC ),
    ITYPE_SET_LDC (LDC2),
    ITYPE_SINGLE  (MCR ),
    ITYPE_SINGLE  (MCR2),
    ITYPE_SINGLE  (MRC ),
    ITYPE_SINGLE  (MRC2),
    ITYPE_SET_LDC (STC ),
    ITYPE_SET_LDC (STC2),

    // status register access instructions
    ITYPE_SINGLE (MRS),
    ITYPE_SINGLE (MSR),

    // hints
    ITYPE_SINGLE (NOP  ),
    ITYPE_SINGLE (YIELD),
    ITYPE_SINGLE (WFE  ),
    ITYPE_SINGLE (WFI  ),
    ITYPE_SINGLE (SEV  ),
    ITYPE_SINGLE (DBG  ),

    // ARMv6 miscellaneous instructions
    ITYPE_SINGLE (CPS   ),
    ITYPE_SINGLE (CLREX ),
    ITYPE_SINGLE (DSB   ),
    ITYPE_SINGLE (ISB   ),

    // ARMv7 hint instructions
    ITYPE_SET_PLD (PLD),
    ITYPE_SET_PLD (PLI),
    ITYPE_SINGLE  (DMB),

    ////////////////////////////////////////////////////////////////////////////
    // DSP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SINGLE (QADD ),
    ITYPE_SINGLE (QDADD),
    ITYPE_SINGLE (QDSUB),
    ITYPE_SINGLE (QSUB ),

    // multiply instructions
    ITYPE_SET_SMLA_XY (SMLA ),
    ITYPE_SET_SMLA_XY (SMLAL),
    ITYPE_SET_SMLAW_Y (SMLAW),
    ITYPE_SET_SMLA_XY (SMUL ),
    ITYPE_SET_SMLAW_Y (SMULW),

    // load and store instructions
    ITYPE_SET_LDRH (LDRD),
    ITYPE_SET_LDRH (STRD),

    // coprocessor instructions
    ITYPE_SINGLE (MCRR ),
    ITYPE_SINGLE (MCRR2),
    ITYPE_SINGLE (MRRC ),
    ITYPE_SINGLE (MRRC2),

    ////////////////////////////////////////////////////////////////////////////
    // MEDIA INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // basic instructions
    ITYPE_SINGLE (USAD8 ),
    ITYPE_SINGLE (USADA8),
    ITYPE_SINGLE (SBFX  ),
    ITYPE_SINGLE (BFC   ),
    ITYPE_SINGLE (BFI   ),
    ITYPE_SINGLE (UBFX  ),

    // parallel add/subtract instructions
    ITYPE_SET_PAS (ADD16),
    ITYPE_SET_PAS (ASX  ),
    ITYPE_SET_PAS (SAX  ),
    ITYPE_SET_PAS (SUB16),
    ITYPE_SET_PAS (ADD8 ),
    ITYPE_SET_PAS (SUB8 ),

    // packing, unpacking, saturation and reversal instructions
    ITYPE_SINGLE (PKHBT  ),
    ITYPE_SINGLE (PKHTB  ),
    ITYPE_SINGLE (SSAT   ),
    ITYPE_SINGLE (SSAT16 ),
    ITYPE_SINGLE (USAT   ),
    ITYPE_SINGLE (USAT16 ),
    ITYPE_SINGLE (SXTAB  ),
    ITYPE_SINGLE (UXTAB  ),
    ITYPE_SINGLE (SXTAB16),
    ITYPE_SINGLE (UXTAB16),
    ITYPE_SINGLE (SXTAH  ),
    ITYPE_SINGLE (UXTAH  ),
    ITYPE_SINGLE (SXTB   ),
    ITYPE_SINGLE (UXTB   ),
    ITYPE_SINGLE (SXTB16 ),
    ITYPE_SINGLE (UXTB16 ),
    ITYPE_SINGLE (SXTH   ),
    ITYPE_SINGLE (UXTH   ),
    ITYPE_SINGLE (SEL    ),
    ITYPE_SINGLE (REV    ),
    ITYPE_SINGLE (REV16  ),
    ITYPE_SINGLE (RBIT   ),
    ITYPE_SINGLE (REVSH  ),

    // signed multiply instructions
    ITYPE_MEDIA_X (SMLAD ),
    ITYPE_MEDIA_X (SMUAD ),
    ITYPE_MEDIA_X (SMLSD ),
    ITYPE_MEDIA_X (SMUSD ),
    ITYPE_MEDIA_X (SMLALD),
    ITYPE_MEDIA_X (SMLSLD),
    ITYPE_MEDIA_R (SMMLA ),
    ITYPE_MEDIA_R (SMMUL ),
    ITYPE_MEDIA_R (SMMLS ),

    // VFP data processing instructions
    ITYPE_SINGLE (VMLA_VFP),
    ITYPE_SINGLE (VMLS_VFP),
    ITYPE_SINGLE (VNMLS_VFP),
    ITYPE_SINGLE (VNMLA_VFP),
    ITYPE_SINGLE (VMUL_VFP),
    ITYPE_SINGLE (VNMUL_VFP),
    ITYPE_SINGLE (VADD_VFP),
    ITYPE_SINGLE (VSUB_VFP),
    ITYPE_SINGLE (VDIV_VFP),
    ITYPE_SINGLE (VFMA_VFP),
    ITYPE_SINGLE (VFMS_VFP),
    ITYPE_SINGLE (VFNMS_VFP),
    ITYPE_SINGLE (VFNMA_VFP),
    ITYPE_SINGLE (VMOVI_VFP),
    ITYPE_SINGLE (VMOVR_VFP),
    ITYPE_SINGLE (VABS_VFP),
    ITYPE_SINGLE (VNEG_VFP),
    ITYPE_SINGLE (VSQRT_VFP),
    ITYPE_SINGLE (VCVTBFH_VFP),
    ITYPE_SINGLE (VCVTTFH_VFP),
    ITYPE_SINGLE (VCVTBHF_VFP),
    ITYPE_SINGLE (VCVTTHF_VFP),
    ITYPE_SINGLE (VCMP_VFP),
    ITYPE_SINGLE (VCMPE_VFP),
    ITYPE_SINGLE (VCMP0_VFP),
    ITYPE_SINGLE (VCMPE0_VFP),
    ITYPE_SINGLE (VCVTFU_VFP),
    ITYPE_SINGLE (VCVTFS_VFP),
    ITYPE_SINGLE (VCVTFXUH_VFP),
    ITYPE_SINGLE (VCVTFXUW_VFP),
    ITYPE_SINGLE (VCVTFXSH_VFP),
    ITYPE_SINGLE (VCVTFXSW_VFP),
    ITYPE_SINGLE (VCVTUF_VFP),
    ITYPE_SINGLE (VCVTRUF_VFP),
    ITYPE_SINGLE (VCVTSF_VFP),
    ITYPE_SINGLE (VCVTRSF_VFP),
    ITYPE_SINGLE (VCVTXFSH_VFP),
    ITYPE_SINGLE (VCVTXFSW_VFP),
    ITYPE_SINGLE (VCVTXFUH_VFP),
    ITYPE_SINGLE (VCVTXFUW_VFP),

    // Extension register load/store instructions
    ITYPE_VFP_DS (VSTMIA),
    ITYPE_VFP_DS (VSTMIAW),
    ITYPE_VFP_DS (VSTR),
    ITYPE_VFP_DS (VSTMDBW),
    ITYPE_VFP_DS (VPUSH),
    ITYPE_VFP_DS (VLDMIA),
    ITYPE_VFP_DS (VLDMIAW),
    ITYPE_VFP_DS (VPOP),
    ITYPE_VFP_DS (VLDR),
    ITYPE_VFP_DS (VLDMDBW),

    // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
    ITYPE_SINGLE (VMRS),
    ITYPE_SINGLE (VMSR),
    ITYPE_SINGLE (VMOVRS),
    ITYPE_SINGLE (VMOVSR),
    ITYPE_SINGLE (VMOVZR),
    ITYPE_SINGLE (VMOVRZ),

    // 64-bit transfer instructions between ARM core regs and extension regs
    ITYPE_SINGLE (VMOVRRD),
    ITYPE_SINGLE (VMOVDRR),
    ITYPE_SINGLE (VMOVRRSS),
    ITYPE_SINGLE (VMOVSSRR),

    ////////////////////////////////////////////////////////////////////////////
    // THUMB INSTRUCTIONS (WHEN DISTINCT FROM ARM INSTRUCTIONS)
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SINGLE (ADD4),
    ITYPE_SINGLE (ADD6),
    ITYPE_SINGLE (ADD7),
    ITYPE_SINGLE (SUB4),
    ITYPE_SINGLE (MOV3),

    // address instructions
    ITYPE_SINGLE (ADD_ADR),
    ITYPE_SINGLE (SUB_ADR),

    // branch instructions
    ITYPE_SINGLE (CBNZ),
    ITYPE_SINGLE (CBZ ),
    ITYPE_SINGLE (TB  ),

    // divide instructions
    ITYPE_SINGLE (SDIV),
    ITYPE_SINGLE (UDIV),

    // KEEP LAST
    ITYPE_SINGLE (LAST)

} armInstructionType;

//
// Condition code enumeration
//
typedef enum armConditionE {

    ARM_C_EQ,       // ZF==1
    ARM_C_NE,       // ZF==0
    ARM_C_CS,       // CF==1
    ARM_C_CC,       // CF==0
    ARM_C_MI,       // NF==1
    ARM_C_PL,       // NF==0
    ARM_C_VS,       // VF==1
    ARM_C_VC,       // VF==0
    ARM_C_HI,       // (CF==1) && (ZF==0)
    ARM_C_LS,       // (CF==0) || (ZF==1)
    ARM_C_GE,       // NF==VF
    ARM_C_LT,       // NF!=VF
    ARM_C_GT,       // (ZF==0) && (NF==VF)
    ARM_C_LE,       // (ZF==1) || (NF!=VF)
    ARM_C_AL,       // always
    ARM_C_NV,       // never

    // KEEP LAST
    ARM_C_LAST

} armCondition;

//
// This defines whether the instruction sets flags
//
typedef enum armSetFlagsE {
    ARM_SF_0,       // don't set flags
    ARM_SF_V,       // set flags, show in disassembly using "s" suffix
    ARM_SF_I,       // set flags, not shown in instruction disassembly
    ARM_SF_IT,      // only when not in if-then block
} armSetFlags;

//
// This defines shift operations
//
typedef enum armShiftOpE {
    ARM_SO_NA,      // no shift operation
    ARM_SO_LSL,     // logical shift left
    ARM_SO_LSR,     // logical shift right
    ARM_SO_ASR,     // arithmetic shift right
    ARM_SO_ROR,     // rotate right
    ARM_SO_RRX      // rotate right with extend
} armShiftOp;

//
// This defines increment/decrement actions
//
typedef enum armIncDecE {
    ARM_ID_NA  = 0x0,                               // no increment/decrement spec present
    ARM_ID_D   = 0x0,                               // decrement
    ARM_ID_I   = 0x1,                               // increment
    ARM_ID_A   = 0x0,                               // after
    ARM_ID_B   = 0x2,                               // before
    ARM_ID_NS  = 0x4,                               // not shown in disassembly
    ARM_ID_P   = 0x8,                               // increment/decrement spec present
    ARM_ID_DA  = ARM_ID_P | ARM_ID_D  | ARM_ID_A,   // decrement after
    ARM_ID_IA  = ARM_ID_P | ARM_ID_I  | ARM_ID_A,   // increment after
    ARM_ID_DB  = ARM_ID_P | ARM_ID_D  | ARM_ID_B,   // decrement before
    ARM_ID_IB  = ARM_ID_P | ARM_ID_I  | ARM_ID_B,   // increment before
    ARM_ID_IAI = ARM_ID_IA | ARM_ID_NS,             // IA, not shown in disassembly
    ARM_ID_IBI = ARM_ID_IB | ARM_ID_NS,             // IB, not shown in disassembly
    ARM_ID_DAI = ARM_ID_DA | ARM_ID_NS,             // DA, not shown in disassembly
    ARM_ID_DBI = ARM_ID_DB | ARM_ID_NS              // DB, not shown in disassembly
} armIncDec;

//
// This defines bits in a field mask
//
typedef enum armSRFieldMaskE {
    ARM_SR_C = 0x1, // control field mask bit
    ARM_SR_X = 0x2, // extension field mask bit
    ARM_SR_S = 0x4, // status field mask bit
    ARM_SR_F = 0x8  // flags field mask bit
} armSRFieldMask;

//
// This defines actions to be taken for unaligned memory accesses
//
typedef enum armUnalignedActionE {
    ARM_UA_DABORT,      // take data abort exception
    ARM_UA_ROTATE,      // rotate if unaligned (some ARMv4 and ARMv5 reads)
    ARM_UA_ALIGN,       // force alignment
    ARM_UA_UNALIGNED,   // allow unaligned access
} armUnalignedAction;

//
// This specifies the effect on any interrupt flags of this instruction
//
typedef enum armFlagActionE {
    ARM_FACT_NA  = 0,    // no flag action
    ARM_FACT_BAD = 1,    // (illegal value)
    ARM_FACT_IE  = 2,    // interrupts enabled
    ARM_FACT_ID  = 3     // interrupts disabled
} armFlagAction;

//
// This specifies flags affected by this instruction
//
typedef enum armFlagAffectE {
    ARM_FAFF_NA = 0x0,  // no flags affected
    ARM_FAFF_F  = 0x1,  // F flag affected
    ARM_FAFF_I  = 0x2,  // I flag affected
    ARM_FAFF_A  = 0x4,  // A flag affected
} armFlagAffect;

//
// This specifies system registers for MSR/MRS
//
typedef enum armSysRegIdE {
    ASRID_APSR        = 0,
    ASRID_IAPSR       = 1,
    ASRID_EAPSR       = 2,
    ASRID_XPSR        = 3,
    ASRID_IPSR        = 5,
    ASRID_EPSR        = 6,
    ASRID_IEPSR       = 7,
    ASRID_MSP         = 8,
    ASRID_PSP         = 9,
    ASRID_PRIMASK     = 16,
    ASRID_BASEPRI     = 17,
    ASRID_BASEPRI_MAX = 18,
    ASRID_FAULTMASK   = 19,
    ASRID_CONTROL     = 20
} armSysRegId;

//
// This specifies bits specified by mask field of MSR
//
typedef enum armPSRBitsE {
    ARM_PSRBITS_NA    = 0,  // no bits specified
    ARM_PSRBITS_GE    = 1,  // GE only, no flags
    ARM_PSRBITS_FLAGS = 2,  // Flags only, no GE
    ARM_PSRBITS_ALL   = 3,  // GE and flags
} armPSRBits;

//
// This specifies SIMD/VFP type
//
typedef enum armSDFPTypeE {
    ARM_SDFPT_NA,   // no SIMD/VFP type
    ARM_SDFPT_8,    // 8 bit value - type not specified
    ARM_SDFPT_16,   // 16 bit value - type not specified
    ARM_SDFPT_32,   // 32 bit value - type not specified
    ARM_SDFPT_64,   // 64 bit value - type not specified
    ARM_SDFPT_F16,  // floating point 16 bit value
    ARM_SDFPT_F32,  // floating point 32 bit value
    ARM_SDFPT_F64,  // floating point 64 bit value
    ARM_SDFPT_I8,   // integer 8 bit value
    ARM_SDFPT_I16,  // integer 16 bit value
    ARM_SDFPT_I32,  // integer 32 bit value
    ARM_SDFPT_I64,  // integer 64 bit value
    ARM_SDFPT_P8,   // polynomial 8 bit value
    ARM_SDFPT_S8,   // signed 8 bit value
    ARM_SDFPT_S16,  // signed 16 bit value
    ARM_SDFPT_S32,  // signed 32 bit value
    ARM_SDFPT_S64,  // signed 64 bit value
    ARM_SDFPT_U8,   // unsigned 8 bit value
    ARM_SDFPT_U16,  // unsigned 16 bit value
    ARM_SDFPT_U32,  // unsigned 32 bit value
    ARM_SDFPT_U64,  // unsigned 64 bit value
} armSDFPType;

//
// Type to hold a modified immediate constant value for SIMD and VFP instructions
//
typedef union armSdfpMItypeU {
    Uns64   u64;
    Flt64   f64;
    Flt32   f32;
    struct {
        Uns32 w0;
        Uns32 w1;
    } u32;
    struct {
        Uns16 h0;
        Uns16 h1;
        Uns16 h2;
        Uns16 h3;
    } u16;
    struct {
        Uns8 b0;
        Uns8 b1;
        Uns8 b2;
        Uns8 b3;
        Uns8 b4;
        Uns8 b5;
        Uns8 b6;
        Uns8 b7;
    } u8;
} armSdfpMItype;

//
// This specifies instruction support implied by ISAR registers
//
typedef enum armISARSupportE {
    ARM_ISAR_NA = 0,    // no ISAR restriction
    ARM_ISAR_DIV,       // SDIV/UDIV support
    ARM_ISAR_BKPT,      // BKPT support
    ARM_ISAR_CBZ,       // CBZ/CBNZ support
    ARM_ISAR_BFC,       // BFC/BFI/SBFX/UBFX support
    ARM_ISAR_CLZ,       // CLZ support
    ARM_ISAR_SWP,       // SWP/SWPB support
    ARM_ISAR_BXJ,       // BXJ support
    ARM_ISAR_BX,        // BX support
    ARM_ISAR_BLX,       // BX support
    ARM_ISAR_MOVT,      // MOVT/MOV(16)/ADD(12) etc support
    ARM_ISAR_IT,        // IT support
    ARM_ISAR_SXTB,      // SXTB/SXTH/UXTB/UXTH support
    ARM_ISAR_SXTAB,     // SXTAB/SXTAH/UXTAB/UXTAH support
    ARM_ISAR_SXTB16,    // SXTB16/SXTAB16/UXTB16/UXTAB16 support
    ARM_ISAR_SRS,       // SRS/RFE and A/R profile CPS
    ARM_ISAR_LDM_UR,    // user mode LDM/STM, exception return LDM
    ARM_ISAR_SETEND,    // SETEND support
    ARM_ISAR_REV,       // REV/REV16/REVSH support
    ARM_ISAR_RBIT,      // RBIT support
    ARM_ISAR_MRS_AR,    // A/R profile MRS/MSR and exception return support
    ARM_ISAR_UMULL,     // UMULL/UMLAL support
    ARM_ISAR_UMAAL,     // UMAAL support
    ARM_ISAR_SMULL,     // SMULL/SMLAL support
    ARM_ISAR_SMLABB,    // SMLABB/SMLABT ... SMULWB/SMULWT support
    ARM_ISAR_SMLAD,     // SMLAD/SMLADX ... SMUSD/SMUSDX support
    ARM_ISAR_MLA,       // MLA support
    ARM_ISAR_MLS,       // MLS support
    ARM_ISAR_PLD,       // PLD support
    ARM_ISAR_PLI,       // PLI support
    ARM_ISAR_LDRD,      // LDRD/STRD support
    ARM_ISAR_NOP,       // NOP support
    ARM_ISAR_MOVLL,     // Thumb MOV low->low support
    ARM_ISAR_TBB,       // TBB/TBH support
    ARM_ISAR_LDREX,     // LDREX/STREX support
    ARM_ISAR_CLREX,     // CLREX/LDREXB/LDREXH/STREXB/STREXH support
    ARM_ISAR_LDREXD,    // LDREXD/STREXD support
    ARM_ISAR_SVC,       // SVC support
    ARM_ISAR_SSAT,      // SSAT/USAT support
    ARM_ISAR_PKHBT,     // PKHBT/PKHTB ... USUB8/USAX support
    ARM_ISAR_QADD,      // QADD/QDADD/QDSUB/QSUB support
    ARM_ISAR_MRS_M,     // M profile CPS/MRS/MSR support
    ARM_ISAR_DMB,       // DMB/DSB/ISB support
    ARM_ISAR_LDRBT,     // LDRBT/LDRT/STRBT/STRT support
    ARM_ISAR_LDRHT,     // LDRHT/LDRSBT/LDRSHT/STRHT support
    ARM_ISAR_VMRS,      // load, store, cp moves to SIMD regs supported (VFP2,VFP3,SIMD)
    ARM_ISAR_VFPSQRT,   // VFP VSQRT support
    ARM_ISAR_VFPDIV,    // VFP VDIV support
    ARM_ISAR_VFPV3,     // VFP v3-only support (also check double/single precision support)
    ARM_ISAR_VFPV2,     // VFP v2 and later support (also check double/single precision support)
    ARM_ISAR_VFPFMAC,   // VFP fused multilply accumulate support
    ARM_ISAR_VFPCVT3,   // VFP v3-only conversion support
    ARM_ISAR_VFPCVT2,   // VFP v2 and later conversion support
    ARM_ISAR_VFPHP,     // VFP half precision convert support
} armISARSupport;

//
// This structure is filled with information extracted from the decoded
// instruction
//
typedef struct armInstructionInfoS {
    const char        *opcode;      // opcode name
    const char        *format;      // disassembly format string
    armArchitecture    support;     // variants on which instruction supported
    armISARSupport     isar;        // ISAR instruction support
    Uns32              thisPC;      // instruction address
    Uns32              instruction; // instruction word
    armInstructionType type;        // instruction type
    armCondition       cond;        // condition under which instruction executes
    armSetFlags        f;           // set flags?
    armShiftOp         so;          // shifter operation to apply to arg 2
    armIncDec          incDec;      // increment/decrement action
    Uns32              c;           // constant value
    Uns32              t;           // target address
    Uns32              rList;       // register list
    Uns8               crotate;     // constant rotation from instruction
    Uns8               bytes;       // instruction size in bytes (1, 2 or 4)
    Uns8               r1;          // register 1
    Uns8               r2;          // register 2
    Uns8               r3;          // register 3
    Uns8               r4;          // register 4
    Uns8               sz;          // load/store size
    Int8               w;           // bit operation width
    Uns8               cpNum;       // coprocessor number
    Uns8               cpOp1;       // coprocessor opcode1
    Uns8               cpOp2;       // coprocessor opcode2
    Bool               xs;          // sign extend?
    Bool               tl;          // translate?
    Bool               pi;          // post-indexed?
    Bool               wb;          // instruction specifies writeback?
    Bool               u;           // instruction U bit set?
    Bool               ll;          // instruction specifies long load?
    Bool               ea;          // load/store is exclusive access?
    armUnalignedAction ua;          // unaligned action
    Bool               ma;          // mode action (CPS)
    armFlagAction      fact;        // flag action (CPS)
    armFlagAffect      faff;        // flags affected (CPS)
    armPSRBits         psrbits;     // MSR instruction bits value
    Uns8               it;          // IT block specification
    Uns8               index;       // index for VFP scalars
    Uns8               nregs;       // number of registers for vfp register lists
    armSDFPType        dt1;         // VFP first data type
    armSDFPType        dt2;         // VFP second data type
    armSdfpMItype      sdfpMI;      // VFP modified immediate constant value
} armInstructionInfo;

#endif

