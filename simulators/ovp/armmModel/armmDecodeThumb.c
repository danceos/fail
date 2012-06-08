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
#include "vmi/vmiDecode.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

// model header files
#include "armAttributeEntriesThumb16.h"
#include "armAttributeEntriesThumb32.h"
#include "armBitMacros.h"
#include "armDecode.h"
#include "armDecodeEntriesThumb16.h"
#include "armDecodeEntriesThumb32.h"
#include "armDecodeThumb.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armSysRegisters.h"
#include "armVariant.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_DECODE_THUMB"


////////////////////////////////////////////////////////////////////////////////
// THUMB INSTRUCTION TYPES
////////////////////////////////////////////////////////////////////////////////

//
// Decode entries for 32-bit Thumb instructions like LDR
//
#define TT32_SET_LDR(_NAME) \
    TT32_##_NAME##_IMM1,        \
    TT32_##_NAME##_IMM2,        \
    TT32_##_NAME##_IMM3,        \
    TT32_##_NAME##_RM,          \
    TT32_##_NAME##_RM_SHFT_IMM, \
    TT32_##_NAME##T_IMM

//
// Decode entries for 32-bit Thumb instructions like PLD
//
#define TT32_SET_PLD(_NAME) \
    TT32_##_NAME##_IMM1,        \
    TT32_##_NAME##_IMM2,        \
    TT32_##_NAME##_IMM3,        \
    TT32_##_NAME##_RM,          \
    TT32_##_NAME##_RM_SHFT_IMM

//
// Decode entries for 32-bit Thumb instructions like ADD
//
#define TT32_SET_ADD(_NAME) \
    TT32_##_NAME##_IMM,         \
    TT32_##_NAME##_RM_SHFT_IMM, \
    TT32_##_NAME##_RM_RRX

//
// Instruction types for 32-bit Thumb parallel add/subtract Media instructions
//
#define TT32_SET_PAS(_NAME) \
    TT32_S##_NAME,              \
    TT32_Q##_NAME,              \
    TT32_SH##_NAME,             \
    TT32_U##_NAME,              \
    TT32_UQ##_NAME,             \
    TT32_UH##_NAME

//
// Instruction types for 32-bit Thumb DSP instructions like SMLA<x><y>
//
#define TT32_SET_SMLA_XY(_NAME) \
    TT32_##_NAME##BB,           \
    TT32_##_NAME##BT,           \
    TT32_##_NAME##TB,           \
    TT32_##_NAME##TT

//
// Instruction types for 32-bit Thumb DSP instructions like SMLAW<y>
//
#define TT32_SET_SMLAW_Y(_NAME) \
    TT32_##_NAME##B,            \
    TT32_##_NAME##T

//
// Instruction types for 32-bit Thumb instructions with optional argument exchange
//
#define TT32_SET_MEDIA_X(_NAME) \
    TT32_##_NAME,               \
    TT32_##_NAME##X

//
// Instruction types for 32-bit Thumb instructions with optional result rounding
//
#define TT32_SET_MEDIA_R(_NAME) \
    TT32_##_NAME,               \
    TT32_##_NAME##R

//
// Instruction types for 32-bit Thumb  instructions like LDC
//
#define TT32_SET_LDC(_NAME) \
    TT32_##_NAME##_IMM,         \
    TT32_##_NAME##_UNINDEXED

//
// Instruction types for VFP instructions with D and S variants
//
#define TT32_VFP_DS(_NAME)   \
    TT32_##_NAME##_D,           \
    TT32_##_NAME##_S

//
// Instruction type enumeration
//
typedef enum armThumbTypeE {

    ////////////////////////////////////////////////////////////////////////////
    // 16-bit instructions
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    TT16_ADC,
    TT16_ADD1,
    TT16_ADD2,
    TT16_ADD3,
    TT16_ADD4LL,
    TT16_ADD4LH,
    TT16_ADD4H,
    TT16_ADD5,
    TT16_ADD6,
    TT16_ADD7,
    TT16_AND,
    TT16_ASR1,
    TT16_ASR2,
    TT16_BIC,
    TT16_EOR,
    TT16_LSL1,
    TT16_LSL2,
    TT16_LSR1,
    TT16_LSR2,
    TT16_MOV1,
    TT16_MOV2,
    TT16_MOV3LL,
    TT16_MOV3LH,
    TT16_MOV3H,
    TT16_MUL,
    TT16_MVN,
    TT16_NEG,
    TT16_ORR,
    TT16_ROR,
    TT16_SBC,
    TT16_SUB1,
    TT16_SUB2,
    TT16_SUB3,
    TT16_SUB4,

    // compare instructions
    TT16_CMN,
    TT16_CMP1,
    TT16_CMP2,
    TT16_CMP3LH,
    TT16_CMP3H,
    TT16_TST,

    // branch instructions
    TT16_B1,
    TT16_B2,
    TT16_BLX2,
    TT16_BX,
    TT16_SWI,
    TT16_BU,

    // miscellaneous instructions
    TT16_CPS,
    TT16_CBNZ,
    TT16_CBZ,
    TT16_SXTH,
    TT16_SXTB,
    TT16_UXTH,
    TT16_UXTB,
    TT16_REV,
    TT16_REV16,
    TT16_REVSH,
    TT16_BKPT,

    // load and store instructions
    TT16_LDR1,
    TT16_LDR2,
    TT16_LDR3,
    TT16_LDR4,
    TT16_LDRB1,
    TT16_LDRB2,
    TT16_LDRH1,
    TT16_LDRH2,
    TT16_LDRSB,
    TT16_LDRSH,
    TT16_STR1,
    TT16_STR2,
    TT16_STR3,
    TT16_STRB1,
    TT16_STRB2,
    TT16_STRH1,
    TT16_STRH2,

    // load and store multiple instructions
    TT16_LDMIA,
    TT16_POP,
    TT16_PUSH,
    TT16_STMIA,

    // if-then and hints
    TT16_IT,
    TT16_NOP,
    TT16_YIELD,
    TT16_WFE,
    TT16_WFI,
    TT16_SEV,

    ////////////////////////////////////////////////////////////////////////////
    // 32-bit instructions
    ////////////////////////////////////////////////////////////////////////////

    // data processing
    TT32_SET_ADD (AND),
    TT32_SET_ADD (TST),
    TT32_SET_ADD (BIC),
    TT32_SET_ADD (ORR),
    TT32_SET_ADD (MOV),
    TT32_SET_ADD (ORN),
    TT32_SET_ADD (MVN),
    TT32_SET_ADD (EOR),
    TT32_SET_ADD (TEQ),
    TT32_SET_ADD (ADD),
    TT32_SET_ADD (CMN),
    TT32_SET_ADD (ADC),
    TT32_SET_ADD (SBC),
    TT32_SET_ADD (SUB),
    TT32_SET_ADD (CMP),
    TT32_SET_ADD (RSB),

    // pack halfword
    TT32_PKHBT,
    TT32_PKHTB,

    // data processing (plain binary immediate)
    TT32_ADD_PI,
    TT32_ADD_ADR_PI,
    TT32_SUB_PI,
    TT32_SUB_ADR_PI,
    TT32_MOV_PI,
    TT32_MOVT_PI,
    TT32_SSAT,
    TT32_SSAT16,
    TT32_SBFX,
    TT32_BFI,
    TT32_BFC,
    TT32_USAT,
    TT32_USAT16,
    TT32_UBFX,

    // data processing (register)
    TT32_LSL,
    TT32_LSR,
    TT32_ASR,
    TT32_ROR,
    TT32_SXTAH,
    TT32_SXTH,
    TT32_UXTAH,
    TT32_UXTH,
    TT32_SXTAB16,
    TT32_SXTB16,
    TT32_UXTAB16,
    TT32_UXTB16,
    TT32_SXTAB,
    TT32_SXTB,
    TT32_UXTAB,
    TT32_UXTB,

    // parallel add/subtract instructions
    TT32_SET_PAS (ADD16),
    TT32_SET_PAS (ASX  ),
    TT32_SET_PAS (SAX  ),
    TT32_SET_PAS (SUB16),
    TT32_SET_PAS (ADD8 ),
    TT32_SET_PAS (SUB8 ),

    // miscellaneous operation instructions
    TT32_QADD,
    TT32_QDADD,
    TT32_QSUB,
    TT32_QDSUB,
    TT32_REV,
    TT32_REV16,
    TT32_RBIT,
    TT32_REVSH,
    TT32_SEL,
    TT32_CLZ,

    // multiply, divide, multiply accumulate and absolute difference instructions
    TT32_MLA,
    TT32_MUL,
    TT32_MLS,
    TT32_SDIV,
    TT32_UDIV,
    TT32_SET_SMLA_XY (SMLA),
    TT32_SET_SMLA_XY (SMUL),
    TT32_SET_MEDIA_X (SMLAD),
    TT32_SET_MEDIA_X (SMUAD),
    TT32_SET_SMLAW_Y (SMLAW),
    TT32_SET_SMLAW_Y (SMULW),
    TT32_SET_MEDIA_X (SMLSD),
    TT32_SET_MEDIA_X (SMUSD),
    TT32_SET_MEDIA_R (SMMLA),
    TT32_SET_MEDIA_R (SMMUL),
    TT32_SET_MEDIA_R (SMMLS),
    TT32_USAD8,
    TT32_USADA8,
    TT32_SMLAL,
    TT32_SMULL,
    TT32_UMAAL,
    TT32_UMLAL,
    TT32_UMULL,
    TT32_SET_SMLA_XY (SMLAL),
    TT32_SET_MEDIA_X (SMLALD),
    TT32_SET_MEDIA_X (SMLSLD),

    // branch and miscellaneous control instructions
    TT32_B1,
    TT32_B2,
    TT32_BL,
    TT32_MSR,
    TT32_NOP,
    TT32_YIELD,
    TT32_WFE,
    TT32_WFI,
    TT32_SEV,
    TT32_DBG,
    TT32_MRS,
    TT32_UNDEF,
    TT32_CLREX,
    TT32_DSB,
    TT32_DMB,
    TT32_ISB,

    // load and store multiple instructions
    TT32_STMDB,
    TT32_STMIA,
    TT32_LDMDB,
    TT32_LDMIA,
    TT32_POPM,
    TT32_PUSHM,

    // dual and exclusive instructions
    TT32_LDRD_IMM,
    TT32_STRD_IMM,
    TT32_LDREX,
    TT32_LDREXB,
    TT32_LDREXH,
    TT32_STREX,
    TT32_STREXB,
    TT32_STREXH,
    TT32_TBB,
    TT32_TBH,

    // load and store instructions
    TT32_SET_LDR (LDR  ),
    TT32_SET_LDR (LDRH ),
    TT32_SET_LDR (LDRB ),
    TT32_SET_LDR (LDRSH),
    TT32_SET_LDR (LDRSB),
    TT32_SET_LDR (STR  ),
    TT32_SET_LDR (STRH ),
    TT32_SET_LDR (STRB ),
    TT32_SET_PLD (PLD  ),
    TT32_SET_PLD (PLI  ),
    TT32_UHINTH,
    TT32_UHINTB,

    // coprocessor instructions
    TT32_CDP,
    TT32_CDP2,
    TT32_SET_LDC (LDC ),
    TT32_SET_LDC (LDC2),
    TT32_MCR,
    TT32_MCR2,
    TT32_MRC,
    TT32_MRC2,
    TT32_SET_LDC (STC ),
    TT32_SET_LDC (STC2),
    TT32_MCRR,
    TT32_MCRR2,
    TT32_MRRC,
    TT32_MRRC2,

    // VFP data processing instructions
    TT32_VMLA_VFP,
    TT32_VMLS_VFP,
    TT32_VNMLS_VFP,
    TT32_VNMLA_VFP,
    TT32_VMUL_VFP,
    TT32_VNMUL_VFP,
    TT32_VADD_VFP,
    TT32_VSUB_VFP,
    TT32_VDIV_VFP,
    TT32_VFNMA_VFP,
    TT32_VFNMS_VFP,
    TT32_VFMA_VFP,
    TT32_VFMS_VFP,
    TT32_VMOVI_VFP,
    TT32_VMOVR_VFP,
    TT32_VABS_VFP,
    TT32_VNEG_VFP,
    TT32_VSQRT_VFP,
    TT32_VCVTBFH_VFP,
    TT32_VCVTTFH_VFP,
    TT32_VCVTBHF_VFP,
    TT32_VCVTTHF_VFP,
    TT32_VCMP_VFP,
    TT32_VCMPE_VFP,
    TT32_VCMP0_VFP,
    TT32_VCMPE0_VFP,
    TT32_VCVTFU_VFP,
    TT32_VCVTFS_VFP,
    TT32_VCVTFXUH_VFP,
    TT32_VCVTFXUW_VFP,
    TT32_VCVTFXSH_VFP,
    TT32_VCVTFXSW_VFP,
    TT32_VCVTUF_VFP,
    TT32_VCVTRUF_VFP,
    TT32_VCVTSF_VFP,
    TT32_VCVTRSF_VFP,
    TT32_VCVTXFSH_VFP,
    TT32_VCVTXFSW_VFP,
    TT32_VCVTXFUH_VFP,
    TT32_VCVTXFUW_VFP,

    // Extension register load/store instructions
    TT32_VFP_DS (VSTMIA),
    TT32_VFP_DS (VSTMIAW),
    TT32_VFP_DS (VSTR),
    TT32_VFP_DS (VSTMDBW),
    TT32_VFP_DS (VPUSH),
    TT32_VFP_DS (VLDMIA),
    TT32_VFP_DS (VLDMIAW),
    TT32_VFP_DS (VPOP),
    TT32_VFP_DS (VLDR),
    TT32_VFP_DS (VLDMDBW),

    // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
    TT32_VMRS,
    TT32_VMSR,
    TT32_VMOVRS,
    TT32_VMOVSR,
    TT32_VMOVZR,
    TT32_VMOVRZ,

    // 64-bit transfer instructions between ARM core regs and extension regs
    TT32_VMOVRRD,
    TT32_VMOVDRR,
    TT32_VMOVRRSS,
    TT32_VMOVSSRR,

    // KEEP LAST
    TT_LAST

} armThumbType;


////////////////////////////////////////////////////////////////////////////////
// FIELD EXTRACTION MACROS
////////////////////////////////////////////////////////////////////////////////

#define OP_R0(_I)         WIDTH(4,(_I)>> 0)
#define OP_R12(_I)        WIDTH(4,(_I)>>12)
#define OP_R16(_I)        WIDTH(4,(_I)>>16)
#define OP_F20(_I)        WIDTH(1,(_I)>>20)
#define OP_COND_8(_I)     WIDTH(4,(_I)>> 8)
#define OP_COND_22(_I)    WIDTH(4,(_I)>>22)
#define OP_COND_28(_I)    WIDTH(4,(_I)>>28)
#define OP_IT(_I)         WIDTH(8,(_I)>> 0)
#define OP_R3_0(_I)       WIDTH(3,(_I)>> 0)
#define OP_R3_3(_I)       WIDTH(3,(_I)>> 3)
#define OP_R3_6(_I)       WIDTH(3,(_I)>> 6)
#define OP_R3_8(_I)       WIDTH(3,(_I)>> 8)
#define OP_R4_0(_I)       WIDTH(4,(_I)>> 0)
#define OP_R4_8(_I)       WIDTH(4,(_I)>> 8)
#define OP_R4_12(_I)      WIDTH(4,(_I)>>12)
#define OP_R4_16(_I)      WIDTH(4,(_I)>>16)
#define OP_U_1_5(_I)      WIDTH(1,(_I)>> 5)
#define OP_U_1_7(_I)      WIDTH(1,(_I)>> 7)
#define OP_U_1_19(_I)     WIDTH(1,(_I)>>19)
#define OP_U_1_21(_I)     WIDTH(1,(_I)>>21)
#define OP_U_2_4(_I)      WIDTH(2,(_I)>> 4)
#define OP_U_2_10(_I)     WIDTH(2,(_I)>>10)
#define OP_U_2_20(_I)     WIDTH(2,(_I)>>20)
#define OP_U_2_21(_I)     WIDTH(2,(_I)>>21)
#define OP_U_3_0(_I)      WIDTH(3,(_I)>> 0)
#define OP_U_3_6(_I)      WIDTH(3,(_I)>> 6)
#define OP_U_4_0(_I)      WIDTH(4,(_I)>> 0)
#define OP_U_4_4(_I)      WIDTH(4,(_I)>> 4)
#define OP_U_5_0(_I)      WIDTH(5,(_I)>> 0)
#define OP_U_5_0_5(_I)    (((WIDTH(4,(_I)>>0))<<1)|WIDTH(1,(_I)>>5))
#define OP_U_5_6(_I)      WIDTH(5,(_I)>> 6)
#define OP_U_7_0(_I)      WIDTH(7,(_I)>> 0)
#define OP_U_7_1(_I)      WIDTH(7,(_I)>> 1)
#define OP_U_8_0(_I)      WIDTH(8,(_I)>> 0)
#define OP_U_8_16_0(_I)   (((WIDTH(4,(_I)>>16))<<4) | WIDTH(4,(_I)>>0))
#define OP_U_9(_I)        WIDTH(1,(_I)>> 9)
#define OP_U_12_0(_I)     WIDTH(12,(_I)>>0)
#define OP_U_23(_I)       WIDTH(1,(_I)>>23)
#define OP_R4_0_H7(_I)    (((WIDTH(1,(_I)>>7))<<3)|OP_R3_0(_I))
#define OP_R4_3_H6(_I)    (((WIDTH(1,(_I)>>6))<<3)|OP_R3_3(_I))
#define OP_TS8(_I)        (((Int32)((_I)<<24))>>23)
#define OP_TS11(_I)       (((Int32)((_I)<<21))>>20)
#define OP_TU9_7_3(_I)    ((WIDTH(1,(_I)>>9)<<6) | (WIDTH(5,(_I)>>3)<<1))
#define OP_RL_16(_I)      WIDTH(8,(_I)>>0)
#define OP_RL_16_LR(_I)   (OP_RL_16(_I) | (((_I)&0x100) ? (1<<ARM_REG_LR) : 0))
#define OP_RL_16_PC(_I)   (OP_RL_16(_I) | (((_I)&0x100) ? (1<<ARM_REG_PC) : 0))
#define OP_RL_32(_I)      WIDTH(16,(_I)>>0)
#define OP_RL_32_SP(_I)   (OP_RL_32(_I) & ~(1<<ARM_REG_SP))
#define OP_RL_32_PCSP(_I) (OP_RL_32(_I) & ~((1<<ARM_REG_SP)|(1<<ARM_REG_PC)))
#define OP_IS_4(_I)       WIDTH(1,(_I)>> 4)
#define OP_AIF_0(_I)      WIDTH(3,(_I)>> 0)
#define OP_MA(_I)         WIDTH(1,(_I)>> 8)
#define OP_PI_10(_I)      WIDTH(1,(_I)>>10)
#define OP_PI_24(_I)      WIDTH(1,(_I)>>24)
#define OP_WB_8(_I)       WIDTH(1,(_I)>> 8)
#define OP_WB_21(_I)      WIDTH(1,(_I)>>21)
#define OP_CPNUM(_I)      WIDTH(4,(_I)>> 8)
#define OP_CPOP1_4_4(_I)  WIDTH(4,(_I)>> 4)
#define OP_CPOP1_4_20(_I) WIDTH(4,(_I)>>20)
#define OP_CPOP1_3_21(_I) WIDTH(3,(_I)>>21)
#define OP_CPOP2(_I)      WIDTH(3,(_I)>> 5)
#define OP_LL(_I)         WIDTH(1,(_I)>>22)
#define OP_V0_5(_I)       ((OP_R0(_I) <<1) | WIDTH(1,(_I)>> 5))
#define OP_V16_7(_I)      ((OP_R16(_I)<<1) | WIDTH(1,(_I)>> 7))
#define OP_V12_22(_I)     ((OP_R12(_I)<<1) | WIDTH(1,(_I)>>22))
#define OP_V5_0(_I)       ((WIDTH(1,(_I)>>5) <<4) | OP_R0(_I))
#define OP_V22_12(_I)     ((WIDTH(1,(_I)>>22)<<4) | OP_R12(_I))
#define OP_V7_16(_I)      ((WIDTH(1,(_I)>>7) <<4) | OP_R16(_I))
#define OP_U(_I)          WIDTH(1,(_I)>>23)
#define OP_PI(_I)         WIDTH(1,(_I)>>24)


////////////////////////////////////////////////////////////////////////////////
// INSTRUCTION ATTRIBUTE TABLE
////////////////////////////////////////////////////////////////////////////////

//
// This defines whether the instruction sets flags
//
typedef enum setFlagsE {
    SF_0,           // don't set flags
    SF_V,           // set flags, show in disassembly using "s" suffix
    SF_I,           // set flags, not shown in instruction disassembly
    SF_20_V,        // set flags if field 20 set, show in disassembly
    SF_IT,          // only when not in if-then block
} setFlags;

//
// This defines whether the instruction sets flags
//
typedef enum condSpecE {
    CO_NA,          // no condition
    CO_8,           // condition at 11:8
    CO_22,          // condition at 25:22
    CO_28,          // condition at 31:28
} condSpec;

//
// Define the location of register in an instruction
//
typedef enum rSpecE {
    R_NA,           // no register
    R3_0,           // 3-bit register specification at 2:0
    R3_3,           // 3-bit register specification at 5:3
    R3_6,           // 3-bit register specification at 8:6
    R3_8,           // 3-bit register specification at 10:8
    R4_0,           // 4-bit register specification at 3:0
    R4_8,           // 4-bit register specification at 11:8
    R4_12,          // 4-bit register specification at 15:12
    R4_16,          // 4-bit register specification at 19:16
    R4_0H7,         // 4-bit register specification at 7,2:0
    R4_3H6,         // 4-bit register specification at 6,5:3
    R_PC,           // register PC
    R_SP,           // register SP
    R_LR,           // register LR
    V_0_5,          // register at 3:0,5
    V_12_22,        // register at 15:12,22
    V_16_7,         // register at 19:16,7
    V_5_0,          // register at 5,3:0
    V_22_12,        // register at 22,15:12
    V_7_16,         // register at 7,19:16
    V3_0,           // register is 3bits wide at 2:0
} rSpec;

//
// Define the location of a constant in an instruction
//
typedef enum constSpecE {
    CS_NA,          // instruction has no constant
    CS_U_2_4,       // 2-bit unsigned constant at 5:4
    CS_U_2_4x8,     // 2-bit unsigned constant at 5:4*8
    CS_U_2_10,      // 2-bit unsigned constant at 11:10
    CS_U_3_6,       // 3-bit unsigned constant at 8:6
    CS_U_4_4,       // 4-bit unsigned constant at 8:5
    CS_U_4_0,       // 4-bit unsigned constant at 4:0
    CS_U_5_6,       // 5-bit unsigned constant at 10:6
    CS_U_5_6_SZ,    // 5-bit unsigned constant at 10:6, scaled by size
    CS_U_5_0_5M16,  // 5-bit unsigned constant at 3:0, 5. Subtract from 16 to get value
    CS_U_5_0_5M32,  // 5-bit unsigned constant at 3:0, 5. Subtract from 32 to get value
    CS_U_7_0x4,     // 7-bit unsigned constant at 6:0*4
    CS_U_8_0,       // 8-bit unsigned constant at 7:0
    CS_U_8_0_U,     // 8-bit unsigned constant at 7:0, negated if U=0
    CS_U_8_0_SZ,    // 8-bit unsigned constant at 7:0, scaled by size
    CS_U_8_0x4_U,   // 8-bit unsigned constant at 7:0*4, negated if U=0
    CS_U_8_0x4,     // 8-bit unsigned constant at 7:0*4
    CS_U_12_0,      // 12-bit unsigned constant at 11:0
    CS_U_12_0_U,    // 12-bit unsigned constant at 11:0, negated if U=0
    CS_PI5,         // 5-bit plain immediate
    CS_PI12,        // 12-bit plain immediate
    CS_PI16,        // 16-bit plain immediate
    CS_MI           // modified immediate
} constSpec;

//
// This defines target address field in the instruction
//
typedef enum targetSpecE {
    TC_NA,      // no target
    TC_S8,      // target PC + s8 field (2-byte aligned)
    TC_S11,     // target PC + s11 field (2-byte aligned)
    TC_S20_T2,  // target PC + s20 field (2-byte aligned)
    TC_S24_T2,  // target PC + s24 field (2-byte aligned)
    TC_U9_7_3   // target PC + unsigned constant at 9,7:3*2
} targetSpec;

//
// This defines shift in the instruction
//
typedef enum shiftSpecE {
    SS_NA,      // no shift operation
    SS_ASR,     // ASR
    SS_LSL,     // LSL
    SS_LSR,     // LSR
    SS_ROR,     // ROR
    SS_RRX,     // RRX
    SS2_4,      // shift spec at offset 5:4
    SS2_20,     // shift spec at offset 21:20
    SS2_21,     // shift spec at offset 22:21
} shiftSpec;

//
// This defines whether the instruction specifies post-indexed addressing
//
typedef enum postIndexSpecE {
    PI_0,       // not post-indexed
    PI_1,       // post-indexed
    PI_10,      // post-indexed at position 10
    PI_24,      // post-indexed at position 24
} postIndexSpec;

//
// This defines whether the instruction specifies writeback
//
typedef enum writebackSpecE {
    WB_0,       // no writeback
    WB_1,       // writeback
    WB_1_NB,    // writeback unless base in register list
    WB_8,       // writeback at position 8
    WB_21       // writeback at position 21
} writebackSpec;

//
// This defines coprocessor opcode field in the instruction
//
typedef enum cpOp1SpecE {
    COP_NA,     // no opcode1 field
    COP_4_4,    // 4-bit constant at offset 7:4
    COP_4_20,   // 4-bit constant at offset 23:20
    COP_3_21    // 3-bit constant at offset 23:21
} cpOp1Spec;

//
// This defines register list field in the instruction
//
typedef enum rListSpecE {
    RL_NA,      // no register list
    RL_16,      // register list in 16-bit Thumb opcode
    RL_16_LR,   // register list in 16-bit Thumb opcode, possibly including LR
    RL_16_PC,   // register list in 16-bit Thumb opcode, possibly including PC
    RL_32_SP,   // register list in 32-bit Thumb opcode, excluding SP
    RL_32_PCSP, // register list in 32-bit Thumb opcode, excluding PC & SP
} rListSpec;

//
// This defines increment/decrement specification in the instruction
//
typedef enum incDecSpecE {
    ID_NA,      // no increment/decrement specification
    ID_U_P,     // increment/decrement defined by U and P bits
    ID_DB,      // increment/decrement specification is Decrement Before
    ID_IA,      // increment/decrement specification is Increment After
    ID_U_P_IAI, // increment/decrement defined by U and P bits, IA is implicit in disassembly (UAL only)
    ID_U_P_IMP, // increment/decrement defined by U and P bits, IA is always implicit in disassembly
    ID_DB_I,    // increment/decrement specification is Decrement Before, Implicit in disassembly
    ID_IA_I,    // increment/decrement specification is Increment After, Implicit in disassembly
} incDecSpec;

//
// This defines interrupt enable/disable fields in the instruction
//
typedef enum imodSpecE {
    IS_NA,      // no imod spec
    IS_4        // imod spec at bit 4
} imodSpec;

//
// This defines interrupt disable bits in the instruction
//
typedef enum aifSpecE {
    AIF_NA,     // no aif spec
    AIF_0       // aif field at 2:0
} aifSpec;

//
// This defines width field in the instruction
//
typedef enum widthSpecE {
    WS_NA,       // no width specification
    WS_WIDTH4,   // width in field 3:0
    WS_WIDTH4M1, // width in field 3:0+1
    WS_WIDTH5,   // width in field 4:0
    WS_WIDTH5M1, // width in field 4:0+1
    WS_MSB       // width in field 4:0 - {14:12,7:6} + 1;
} widthSpec;

//
// This defines u field in the instruction
//
typedef enum uSpecE {
    US_1,       // u=1
    US_9,       // u in field at position 9
    US_23,      // u in field at position 23
} uSpec;

//
// This defines mask field in MSR instruction
//
typedef enum maskSpecE {
    MSRMASK_NA,     // no MSR mask spec
    MSRMASK_10      // mask field in bits 11:10
} maskSpec;

//
// Define a SIMD/VFP modified immediate constant type
//
typedef enum sdfpMISpecE {
    SDFP_MI_NA,       // instruction has no SIMD/VFP modified immediate constant
    SDFP_MI_VFP_S,    // single precision VFP modified immediate value
} sdfpMISpec;

//
// This defines the SIMD scalar index field in the instruction
//
typedef enum indexSpecE {
    IDX_NA,       // no index specification
    IDX_21,       // index is 1 bit  wide, in bit  21
    IDX_5,        // index is 1 bit  wide, in bit  5
    IDX_7,        // index is 1 bit  wide, in bit  7
    IDX_19,       // index is 1 bit  wide, in bit  19
} indexSpec;

//
// This defines the number of regs in a VFP register list
//
typedef enum nregSpecE {
    NREG_NA,       // no alignment specification
    NREG_7_1,      // Nregs is 7 bits wide in bits 7:1
    NREG_8_0,      // Nregs is 8 bits wide in bits 7:0
} nregSpec;

//
// Structure defining characteristics of each opcode type
//
typedef struct opAttrsS {
    const char        *opcode;      // opcode name
    const char        *format;      // format string
    armInstructionType type;        // equivalent ARM instruction
    armArchitecture    support:16;  // variants on which instruction supported
    armISARSupport     isar   : 8;  // ISAR instruction support
    setFlags           f      : 4;  // does this opcode set flags?
    condSpec           cond   : 4;  // condition field specification
    rSpec              r1     : 8;  // does instruction have r1?
    rSpec              r2     : 8;  // does instruction have r2?
    rSpec              r3     : 8;  // does instruction have r3?
    rSpec              r4     : 8;  // does instruction have r4?
    constSpec          cs     : 8;  // location of constant
    targetSpec         ts     : 4;  // target specification
    shiftSpec          ss     : 4;  // shifter specification
    Uns8               sz     : 4;  // load/store size
    Uns8               xs     : 4;  // sign extend?
    Uns8               tl     : 4;  // translate?
    postIndexSpec      pi     : 4;  // instruction specifies post-indexed address?
    writebackSpec      wb     : 4;  // instruction specifies writeback?
    Uns8               ll     : 4;  // instruction specifies long load?
    Uns8               cpNum  : 4;  // does instruction have coprocessor number?
    cpOp1Spec          cpOp1  : 4;  // does instruction have coprocessor op 1?
    Uns8               cpOp2  : 4;  // does instruction have coprocessor op 2?
    rListSpec          rList  : 4;  // does instruction have register list?
    incDecSpec         incDec : 4;  // does instruction have increment/decrement?
    armUnalignedAction ua45   : 4;  // action if unaligned (Control.U=0)
    armUnalignedAction ua67   : 4;  // action if unaligned (Control.U=1)
    Bool               ea     : 1;  // exclusive access?
    imodSpec           imod   : 4;  // imod field specification
    Bool               m      : 1;  // M field present?
    aifSpec            aif    : 4;  // A/I/F fields specification
    Bool               it     : 1;  // IT specification present?
    widthSpec          w      : 4;  // width specification
    uSpec              u      : 4;  // U bit specification
    maskSpec           mask   : 4;  // mask specification (MSR instruction)
    indexSpec          index  : 4;  // VFP scalar index specification?
    nregSpec           nregs  : 4;  // number of regs in VFP register list specification?
    sdfpMISpec         sdfpMI : 4;  // SIMD/floating point modified immediate constant?
    armSDFPType        dt1    : 8;  // SIMD/VFP first data type?
    armSDFPType        dt2    : 8;  // SIMD/VFP second data type?
} opAttrs;

typedef const struct opAttrsS *opAttrsCP;

//
// This specifies attributes for each opcode
//
const static opAttrs attrsArray[TT_LAST+1] = {

    ////////////////////////////////////////////////////////////////////////////
    // 16-bit instructions
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ATTR_SET_16_ADC  (ADC,    ADC_RT,            ARM_VT,  ARM_ISAR_NA,    "adc"        ),
    ATTR_SET_16_ADD1 (ADD1,   ADD_IMM,           ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD2 (ADD2,   ADD_IT,            ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD3 (ADD3,   ADD_RM,            ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD4 (ADD4LL, ADD4,              ARM_VT2, ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD4 (ADD4LH, ADD4,              ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD4 (ADD4H,  ADD4,              ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD5 (ADD5,   ADD_ADR,           ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD6 (ADD6,   ADD6,              ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ADD7 (ADD7,   ADD7,              ARM_VT,  ARM_ISAR_NA,    "add"        ),
    ATTR_SET_16_ASR1 (ASR1,   MOV_RM_SHFT_IMM,   ARM_VT,  ARM_ISAR_NA,    "asr", SS_ASR),
    ATTR_SET_16_ASR2 (ASR2,   MOV_RM_SHFT_RST,   ARM_VT,  ARM_ISAR_NA,    "asr", SS_ASR),
    ATTR_SET_16_ADC  (AND,    AND_RT,            ARM_VT,  ARM_ISAR_NA,    "and"        ),
    ATTR_SET_16_ADC  (BIC,    BIC_RT,            ARM_VT,  ARM_ISAR_NA,    "bic"        ),
    ATTR_SET_16_ADC  (EOR,    EOR_RT,            ARM_VT,  ARM_ISAR_NA,    "eor"        ),
    ATTR_SET_16_ASR1 (LSL1,   MOV_RM_SHFT_IMM,   ARM_VT,  ARM_ISAR_NA,    "lsl", SS_LSL),
    ATTR_SET_16_ASR2 (LSL2,   MOV_RM_SHFT_RST,   ARM_VT,  ARM_ISAR_NA,    "lsl", SS_LSL),
    ATTR_SET_16_ASR1 (LSR1,   MOV_RM_SHFT_IMM,   ARM_VT,  ARM_ISAR_NA,    "lsr", SS_LSR),
    ATTR_SET_16_ASR2 (LSR2,   MOV_RM_SHFT_RST,   ARM_VT,  ARM_ISAR_NA,    "lsr", SS_LSR),
    ATTR_SET_16_ADD2 (MOV1,   MOV_IMM,           ARM_VT,  ARM_ISAR_NA,    "mov"        ),
    ATTR_SET_16_MOV2 (MOV2,   MOV_RM_SHFT_IMM,   ARM_VT,  ARM_ISAR_NA,    "mov"        ),
    ATTR_SET_16_ADD4 (MOV3LL, MOV3,            6|ARM_VT,  ARM_ISAR_MOVLL, "mov"        ),
    ATTR_SET_16_ADD4 (MOV3LH, MOV3,              ARM_VT,  ARM_ISAR_NA,    "mov"        ),
    ATTR_SET_16_ADD4 (MOV3H,  MOV3,              ARM_VT,  ARM_ISAR_NA,    "mov"        ),
    ATTR_SET_16_ADC  (MUL,    MUL_RT,            ARM_VT,  ARM_ISAR_NA,    "mul"        ),
    ATTR_SET_16_ADC  (MVN,    MVN_RM,            ARM_VT,  ARM_ISAR_NA,    "mvn"        ),
    ATTR_SET_16_ADC  (NEG,    NEG_RM,            ARM_VT,  ARM_ISAR_NA,    "neg"        ),
    ATTR_SET_16_ADC  (ORR,    ORR_RT,            ARM_VT,  ARM_ISAR_NA,    "orr"        ),
    ATTR_SET_16_ASR2 (ROR,    MOV_RM_SHFT_RST,   ARM_VT,  ARM_ISAR_NA,    "ror", SS_ROR),
    ATTR_SET_16_ADC  (SBC,    SBC_RT,            ARM_VT,  ARM_ISAR_NA,    "sbc"        ),
    ATTR_SET_16_ADD1 (SUB1,   SUB_IMM,           ARM_VT,  ARM_ISAR_NA,    "sub"        ),
    ATTR_SET_16_ADD2 (SUB2,   SUB_IT,            ARM_VT,  ARM_ISAR_NA,    "sub"        ),
    ATTR_SET_16_ADD3 (SUB3,   SUB_RM,            ARM_VT,  ARM_ISAR_NA,    "sub"        ),
    ATTR_SET_16_ADD7 (SUB4,   SUB4,              ARM_VT,  ARM_ISAR_NA,    "sub"        ),

    // compare instructions
    ATTR_SET_16_CMP2 (CMN,    CMN_RM,  ARM_VT, ARM_ISAR_NA, "cmn"),
    ATTR_SET_16_CMP1 (CMP1,   CMP_IMM, ARM_VT, ARM_ISAR_NA, "cmp"),
    ATTR_SET_16_CMP2 (CMP2,   CMP_RM,  ARM_VT, ARM_ISAR_NA, "cmp"),
    ATTR_SET_16_CMP3 (CMP3LH, CMP_RM,  ARM_VT, ARM_ISAR_NA, "cmp"),
    ATTR_SET_16_CMP3 (CMP3H,  CMP_RM,  ARM_VT, ARM_ISAR_NA, "cmp"),
    ATTR_SET_16_CMP2 (TST,    TST_RM,  ARM_VT, ARM_ISAR_NA, "tst"),

    // branch instructions
    ATTR_SET_16_B1        (B1,   B,   ARM_VT, ARM_ISAR_NA,  "b"  ),
    ATTR_SET_16_B2        (B2,   B,   ARM_VT, ARM_ISAR_NA,  "b"  ),
    ATTR_SET_16_BLX2      (BLX2,    5|ARM_VT, ARM_ISAR_BLX, "blx"),
    ATTR_SET_16_BLX2      (BX,        ARM_VT, ARM_ISAR_BX,  "bx" ),
    ATTR_SET_16_BKPT      (SWI,       ARM_VT, ARM_ISAR_SVC, "svc"),
    ATTR_SET_16_UNDECODED (BU                                    ),

    // miscellaneous instructions
    ATTR_SET_16_CPS  (CPS,    6|ARM_VT,  ARM_ISAR_MRS_M, "cps"  ),
    ATTR_SET_16_CBNZ (CBNZ,     ARM_VT2, ARM_ISAR_CBZ,   "cbnz" ),
    ATTR_SET_16_CBNZ (CBZ,      ARM_VT2, ARM_ISAR_CBZ,   "cbz"  ),
    ATTR_SET_16_SXTH (SXTH,   6|ARM_VT,  ARM_ISAR_SXTB,  "sxth" ),
    ATTR_SET_16_SXTH (SXTB,   6|ARM_VT,  ARM_ISAR_SXTB,  "sxtb" ),
    ATTR_SET_16_SXTH (UXTH,   6|ARM_VT,  ARM_ISAR_SXTB,  "uxth" ),
    ATTR_SET_16_SXTH (UXTB,   6|ARM_VT,  ARM_ISAR_SXTB,  "uxtb" ),
    ATTR_SET_16_SXTH (REV,    6|ARM_VT,  ARM_ISAR_REV,   "rev"  ),
    ATTR_SET_16_SXTH (REV16,  6|ARM_VT,  ARM_ISAR_REV,   "rev16"),
    ATTR_SET_16_SXTH (REVSH,  6|ARM_VT,  ARM_ISAR_REV,   "revsh"),
    ATTR_SET_16_BKPT (BKPT,   5|ARM_VT,  ARM_ISAR_BKPT,  "bkpt" ),

    // load and store instructions
    ATTR_SET_16_LDR1  (LDR1,  LDR_IMM, ARM_VT, ARM_ISAR_NA, "ldr",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (LDR2,  LDR_RM,  ARM_VT, ARM_ISAR_NA, "ldr",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR3  (LDR3,  LDR_IMM, ARM_VT, ARM_ISAR_NA, "ldr",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR4  (LDR4,  LDR_IMM, ARM_VT, ARM_ISAR_NA, "ldr",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR1  (LDRB1, LDR_IMM, ARM_VT, ARM_ISAR_NA, "ldr",  1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (LDRB2, LDR_RM,  ARM_VT, ARM_ISAR_NA, "ldr",  1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR1  (LDRH1, LDR_IMM, ARM_VT, ARM_ISAR_NA, "ldr",  2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (LDRH2, LDR_RM,  ARM_VT, ARM_ISAR_NA, "ldr",  2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (LDRSB, LDR_RM,  ARM_VT, ARM_ISAR_NA, "ldr",  1, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (LDRSH, LDR_RM,  ARM_VT, ARM_ISAR_NA, "ldr",  2, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR1  (STR1,  STR_IMM, ARM_VT, ARM_ISAR_NA, "str",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (STR2,  STR_RM,  ARM_VT, ARM_ISAR_NA, "str",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR4  (STR3,  STR_IMM, ARM_VT, ARM_ISAR_NA, "str",  4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR1  (STRB1, STR_IMM, ARM_VT, ARM_ISAR_NA, "str",  1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (STRB2, STR_RM,  ARM_VT, ARM_ISAR_NA, "str",  1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR1  (STRH1, STR_IMM, ARM_VT, ARM_ISAR_NA, "str",  2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_16_LDR2  (STRH2, STR_RM,  ARM_VT, ARM_ISAR_NA, "str",  2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),

    // load and store multiple instructions
    ATTR_SET_16_LDMIA (LDMIA, LDM1, ARM_VT, ARM_ISAR_NA, "ldm",  RL_16,    ID_IA,   ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_16_POP   (POP,   LDM1, ARM_VT, ARM_ISAR_NA, "pop",  RL_16_PC, ID_IA_I, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_16_POP   (PUSH,  STM1, ARM_VT, ARM_ISAR_NA, "push", RL_16_LR, ID_DB_I, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_16_STMIA (STMIA, STM1, ARM_VT, ARM_ISAR_NA, "stm",  RL_16,    ID_IA,   ARM_UA_ALIGN, ARM_UA_DABORT),

    // if-then and hints
    ATTR_SET_16_IT  (IT,    NOP,   ARM_VT2, ARM_ISAR_IT,  "it"   ),
    ATTR_SET_16_NOP (NOP,   NOP,   ARM_VT2, ARM_ISAR_NOP, "nop"  ),
    ATTR_SET_16_NOP (YIELD, YIELD, ARM_VT2, ARM_ISAR_NOP, "yield"),
    ATTR_SET_16_NOP (WFE,   WFE,   ARM_VT2, ARM_ISAR_NOP, "wfe"  ),
    ATTR_SET_16_NOP (WFI,   WFI,   ARM_VT2, ARM_ISAR_NOP, "wfi"  ),
    ATTR_SET_16_NOP (SEV,   SEV,   ARM_VT2, ARM_ISAR_NOP, "sev"  ),

    ////////////////////////////////////////////////////////////////////////////
    // 32-bit instructions
    ////////////////////////////////////////////////////////////////////////////

    // data processing
    ATTR_SET_32_AND (AND, ARM_VT2, ARM_ISAR_NA, "and"),
    ATTR_SET_32_TST (TST, ARM_VT2, ARM_ISAR_NA, "tst"),
    ATTR_SET_32_AND (BIC, ARM_VT2, ARM_ISAR_NA, "bic"),
    ATTR_SET_32_AND (ORR, ARM_VT2, ARM_ISAR_NA, "orr"),
    ATTR_SET_32_MOV (MOV, ARM_VT2, ARM_ISAR_NA, "mov"),
    ATTR_SET_32_AND (ORN, ARM_VT2, ARM_ISAR_NA, "orn"),
    ATTR_SET_32_MOV (MVN, ARM_VT2, ARM_ISAR_NA, "mvn"),
    ATTR_SET_32_AND (EOR, ARM_VT2, ARM_ISAR_NA, "eor"),
    ATTR_SET_32_TST (TEQ, ARM_VT2, ARM_ISAR_NA, "teq"),
    ATTR_SET_32_AND (ADD, ARM_VT2, ARM_ISAR_NA, "add"),
    ATTR_SET_32_TST (CMN, ARM_VT2, ARM_ISAR_NA, "cmn"),
    ATTR_SET_32_AND (ADC, ARM_VT2, ARM_ISAR_NA, "adc"),
    ATTR_SET_32_AND (SBC, ARM_VT2, ARM_ISAR_NA, "sbc"),
    ATTR_SET_32_AND (SUB, ARM_VT2, ARM_ISAR_NA, "sub"),
    ATTR_SET_32_TST (CMP, ARM_VT2, ARM_ISAR_NA, "cmp"),
    ATTR_SET_32_AND (RSB, ARM_VT2, ARM_ISAR_NA, "rsb"),

    // pack halfword
    ATTR_SET_32_PKHBT (PKHBT, ARM_VT2, ARM_ISAR_PKHBT, "pkhbt"),
    ATTR_SET_32_PKHBT (PKHTB, ARM_VT2, ARM_ISAR_PKHBT, "pkhtb"),

    // data processing (plain binary immediate)
    ATTR_SET_32_ADD_PI (ADD_PI,     ADD_IMM, ARM_VT2, ARM_ISAR_MOVT,  "addw"),
    ATTR_SET_32_ADD_PI (ADD_ADR_PI, ADD_ADR, ARM_VT2, ARM_ISAR_MOVT,  "addw"),
    ATTR_SET_32_ADD_PI (SUB_PI,     SUB_IMM, ARM_VT2, ARM_ISAR_MOVT,  "subw"),
    ATTR_SET_32_ADD_PI (SUB_ADR_PI, SUB_ADR, ARM_VT2, ARM_ISAR_MOVT,  "subw"),
    ATTR_SET_32_MOV_PI (MOV_PI,     MOVW,    ARM_VT2, ARM_ISAR_MOVT,  "movw"),
    ATTR_SET_32_MOV_PI (MOVT_PI,    MOVT,    ARM_VT2, ARM_ISAR_MOVT,  "movt"),
    ATTR_SET_32_SSAT   (SSAT,       SSAT,    ARM_VT2, ARM_ISAR_SSAT,  "ssat"),
    ATTR_SET_32_SSAT16 (SSAT16,     SSAT16,  ARM_VT2, ARM_ISAR_PKHBT, "ssat16"),
    ATTR_SET_32_SBFX   (SBFX,       SBFX,    ARM_VT2, ARM_ISAR_BFC,   "sbfx"),
    ATTR_SET_32_BFI    (BFI,        BFI,     ARM_VT2, ARM_ISAR_BFC,   "bfi" ),
    ATTR_SET_32_BFC    (BFC,        BFC,     ARM_VT2, ARM_ISAR_BFC,   "bfc" ),
    ATTR_SET_32_USAT   (USAT,       USAT,    ARM_VT2, ARM_ISAR_SSAT,  "usat"),
    ATTR_SET_32_USAT16 (USAT16,     USAT16,  ARM_VT2, ARM_ISAR_PKHBT, "usat16"),
    ATTR_SET_32_SBFX   (UBFX,       UBFX,    ARM_VT2, ARM_ISAR_BFC,   "ubfx"),

    // data processing (register)
    ATTR_SET_32_LSL   (LSL,     MOV_RM_SHFT_RS, ARM_VT2, ARM_ISAR_NA,     "lsl" ),
    ATTR_SET_32_LSL   (LSR,     MOV_RM_SHFT_RS, ARM_VT2, ARM_ISAR_NA,     "lsr" ),
    ATTR_SET_32_LSL   (ASR,     MOV_RM_SHFT_RS, ARM_VT2, ARM_ISAR_NA,     "asr" ),
    ATTR_SET_32_LSL   (ROR,     MOV_RM_SHFT_RS, ARM_VT2, ARM_ISAR_NA,     "ror" ),
    ATTR_SET_32_SXTAH (SXTAH,                   ARM_VT2, ARM_ISAR_SXTAB,  "sxtah"  ),
    ATTR_SET_32_SXTH  (SXTH,                    ARM_VT2, ARM_ISAR_SXTB,   "sxth"   ),
    ATTR_SET_32_SXTAH (UXTAH,                   ARM_VT2, ARM_ISAR_SXTAB,  "uxtah"  ),
    ATTR_SET_32_SXTH  (UXTH,                    ARM_VT2, ARM_ISAR_SXTB,   "uxth"   ),
    ATTR_SET_32_SXTAH (SXTAB16,                 ARM_VT2, ARM_ISAR_SXTB16, "sxtab16"),
    ATTR_SET_32_SXTH  (SXTB16,                  ARM_VT2, ARM_ISAR_SXTB16, "sxtb16" ),
    ATTR_SET_32_SXTAH (UXTAB16,                 ARM_VT2, ARM_ISAR_SXTB16, "uxtab16"),
    ATTR_SET_32_SXTH  (UXTB16,                  ARM_VT2, ARM_ISAR_SXTB16, "uxtb16" ),
    ATTR_SET_32_SXTAH (SXTAB,                   ARM_VT2, ARM_ISAR_SXTAB,  "sxtab"  ),
    ATTR_SET_32_SXTH  (SXTB,                    ARM_VT2, ARM_ISAR_SXTB,   "sxtb"   ),
    ATTR_SET_32_SXTAH (UXTAB,                   ARM_VT2, ARM_ISAR_SXTAB,  "uxtab"  ),
    ATTR_SET_32_SXTH  (UXTB,                    ARM_VT2, ARM_ISAR_SXTB,   "uxtb"   ),

    // parallel add/subtract instructions
    ATTR_SET_32_PAS (ADD16, ARM_VT2, ARM_ISAR_PKHBT, "add16"),
    ATTR_SET_32_PAS (ASX,   ARM_VT2, ARM_ISAR_PKHBT, "asx"  ),
    ATTR_SET_32_PAS (SAX,   ARM_VT2, ARM_ISAR_PKHBT, "sax"  ),
    ATTR_SET_32_PAS (SUB16, ARM_VT2, ARM_ISAR_PKHBT, "sub16"),
    ATTR_SET_32_PAS (ADD8,  ARM_VT2, ARM_ISAR_PKHBT, "add8" ),
    ATTR_SET_32_PAS (SUB8,  ARM_VT2, ARM_ISAR_PKHBT, "sub8" ),

    // miscellaneous operation instructions
    ATTR_SET_32_QADD (QADD,  ARM_VT2, ARM_ISAR_QADD,  "qadd" ),
    ATTR_SET_32_QADD (QDADD, ARM_VT2, ARM_ISAR_QADD,  "qdadd"),
    ATTR_SET_32_QADD (QSUB,  ARM_VT2, ARM_ISAR_QADD,  "qsub" ),
    ATTR_SET_32_QADD (QDSUB, ARM_VT2, ARM_ISAR_QADD,  "qdsub"),
    ATTR_SET_32_CLZ  (REV,   ARM_VT2, ARM_ISAR_REV,   "rev"  ),
    ATTR_SET_32_CLZ  (REV16, ARM_VT2, ARM_ISAR_REV,   "rev16"),
    ATTR_SET_32_CLZ  (RBIT,  ARM_VT2, ARM_ISAR_RBIT,  "rbit" ),
    ATTR_SET_32_CLZ  (REVSH, ARM_VT2, ARM_ISAR_REV,   "revsh"),
    ATTR_SET_32_SEL  (SEL,   ARM_VT2, ARM_ISAR_PKHBT, "sel"  ),
    ATTR_SET_32_CLZ  (CLZ,   ARM_VT2, ARM_ISAR_CLZ,   "clz"  ),

    // multiply, multiply accumulate and absolute difference instructions
    ATTR_SET_32_MLA      (MLA,    ARM_VT2, ARM_ISAR_MLA,   "mla"  ),
    ATTR_SET_32_MUL      (MUL,    ARM_VT2, ARM_ISAR_NA,    "mul"  ),
    ATTR_SET_32_MLA      (MLS,    ARM_VT2, ARM_ISAR_MLS,   "mls"  ),
    ATTR_SET_32_MUL      (SDIV,         7, ARM_ISAR_DIV,   "sdiv" ),
    ATTR_SET_32_MUL      (UDIV,         7, ARM_ISAR_DIV,   "udiv" ),
    ATTR_SET_32_SMLA_XY  (SMLA,   ARM_VT2, ARM_ISAR_SMLABB, "smla"  ),
    ATTR_SET_32_SMUL_XY  (SMUL,   ARM_VT2, ARM_ISAR_SMLABB, "smul"  ),
    ATTR_SET_32_SMLAD    (SMLAD,  ARM_VT2, ARM_ISAR_SMLAD,  "smlad" ),
    ATTR_SET_32_SMUAD    (SMUAD,  ARM_VT2, ARM_ISAR_SMLAD,  "smuad" ),
    ATTR_SET_32_SMLAW    (SMLAW,  ARM_VT2, ARM_ISAR_SMLABB, "smlaw" ),
    ATTR_SET_32_SMULW    (SMULW,  ARM_VT2, ARM_ISAR_SMLABB, "smulw" ),
    ATTR_SET_32_SMLAD    (SMLSD,  ARM_VT2, ARM_ISAR_SMLAD,  "smlsd" ),
    ATTR_SET_32_SMUAD    (SMUSD,  ARM_VT2, ARM_ISAR_SMLAD,  "smusd" ),
    ATTR_SET_32_SMMLA    (SMMLA,  ARM_VT2, ARM_ISAR_SMLAD,  "smmla" ),
    ATTR_SET_32_SMMUL    (SMMUL,  ARM_VT2, ARM_ISAR_SMLAD,  "smmul" ),
    ATTR_SET_32_SMMLA    (SMMLS,  ARM_VT2, ARM_ISAR_SMLAD,  "smmls" ),
    ATTR_SET_32_MUL      (USAD8,  ARM_VT2, ARM_ISAR_PKHBT,  "usad8" ),
    ATTR_SET_32_MLA      (USADA8, ARM_VT2, ARM_ISAR_PKHBT,  "usada8"),
    ATTR_SET_32_SMLAL    (SMLAL,  ARM_VT2, ARM_ISAR_SMULL,  "smlal" ),
    ATTR_SET_32_SMLAL    (SMULL,  ARM_VT2, ARM_ISAR_SMULL,  "smull" ),
    ATTR_SET_32_SMLAL    (UMAAL,  ARM_VT2, ARM_ISAR_UMAAL,  "umaal" ),
    ATTR_SET_32_SMLAL    (UMLAL,  ARM_VT2, ARM_ISAR_UMULL,  "umlal" ),
    ATTR_SET_32_SMLAL    (UMULL,  ARM_VT2, ARM_ISAR_UMULL,  "umull" ),
    ATTR_SET_32_SMLAL_XY (SMLAL,  ARM_VT2, ARM_ISAR_SMLABB, "smlal" ),
    ATTR_SET_32_SMLALD   (SMLALD, ARM_VT2, ARM_ISAR_SMLAD,  "smlald"),
    ATTR_SET_32_SMLALD   (SMLSLD, ARM_VT2, ARM_ISAR_SMLAD,  "smlsld"),

    // branch and miscellaneous control instructions
    ATTR_SET_32_B1  (B1,    B,  ARM_VT2, ARM_ISAR_NA,    "b"    ),
    ATTR_SET_32_BL  (B2,    B,  ARM_VT2, ARM_ISAR_NA,    "b"    ),
    ATTR_SET_32_BL  (BL,    BL,  ARM_VT, ARM_ISAR_NA,    "bl"   ),
    ATTR_SET_32_MSR (MSR,             7, ARM_ISAR_MRS_M, "msr"  ),
    ATTR_SET_32_NOP (NOP,       ARM_VT2, ARM_ISAR_NOP,   "nop"  ),
    ATTR_SET_32_NOP (YIELD,     ARM_VT2, ARM_ISAR_NOP,   "yield"),
    ATTR_SET_32_NOP (WFE,       ARM_VT2, ARM_ISAR_NOP,   "wfe"  ),
    ATTR_SET_32_NOP (WFI,       ARM_VT2, ARM_ISAR_NOP,   "wfi"  ),
    ATTR_SET_32_NOP (SEV,       ARM_VT2, ARM_ISAR_NOP,   "sev"  ),
    ATTR_SET_32_DBG (DBG,       ARM_VT2, ARM_ISAR_NOP,   "dbg"  ),
    ATTR_SET_32_MRS (MRS,             7, ARM_ISAR_MRS_M, "mrs"  ),
    ATTR_SET_32_UND (UNDEF,     ARM_VT2, ARM_ISAR_NA            ),
    ATTR_SET_32_NOP (CLREX,           7, ARM_ISAR_CLREX, "clrex"),
    ATTR_SET_32_DSB (DSB,             7, ARM_ISAR_DMB,   "dsb"  ),
    ATTR_SET_32_DSB (DMB,             7, ARM_ISAR_DMB,   "dmb"  ),
    ATTR_SET_32_DSB (ISB,             7, ARM_ISAR_DMB,   "isb"  ),

    // load and store multiple instructions
    ATTR_SET_32_LDM  (STMDB, STM1, ARM_VT2, ARM_ISAR_NA, "stm",  ID_DB,   RL_32_PCSP, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDM  (STMIA, STM1, ARM_VT2, ARM_ISAR_NA, "stm",  ID_IA,   RL_32_PCSP, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDM  (LDMDB, LDM1, ARM_VT2, ARM_ISAR_NA, "ldm",  ID_DB,   RL_32_SP,   ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDM  (LDMIA, LDM1, ARM_VT2, ARM_ISAR_NA, "ldm",  ID_IA,   RL_32_SP,   ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_POPM (POPM,  LDM1, ARM_VT2, ARM_ISAR_NA, "pop",  ID_IA_I, RL_32_SP,   ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_POPM (PUSHM, STM1, ARM_VT2, ARM_ISAR_NA, "push", ID_DB_I, RL_32_PCSP, ARM_UA_ALIGN, ARM_UA_DABORT),

    // dual and exclusive instructions
    ATTR_SET_32_LDRD_IMM (LDRD_IMM, ARM_VT2, ARM_ISAR_LDRD,  "ldr", 8, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDRD_IMM (STRD_IMM, ARM_VT2, ARM_ISAR_LDRD,  "str", 8, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDREX    (LDREX,    ARM_VT2, ARM_ISAR_LDREX, "ldr", 4, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDREXB   (LDREXB,         7, ARM_ISAR_CLREX, "ldr", 1, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_LDREXB   (LDREXH,         7, ARM_ISAR_CLREX, "ldr", 2, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_STREX    (STREX,    ARM_VT2, ARM_ISAR_LDREX, "str", 4, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_STREXB   (STREXB,         7, ARM_ISAR_CLREX, "str", 1, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_STREXB   (STREXH,         7, ARM_ISAR_CLREX, "str", 2, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_TBB      (TBB, TB,  ARM_VT2, ARM_ISAR_TBB,   "tb",  1, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_32_TBB      (TBH, TB,  ARM_VT2, ARM_ISAR_TBB,   "tb",  2, False, ARM_UA_ALIGN, ARM_UA_DABORT),

    // load instructions
    ATTR_SET_32_LDR    (LDR,    LDR, ARM_VT2, ARM_ISAR_LDRBT, "ldr", 4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (LDRH,   LDR, ARM_VT2, ARM_ISAR_LDRHT, "ldr", 2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (LDRB,   LDR, ARM_VT2, ARM_ISAR_LDRBT, "ldr", 1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (LDRSH,  LDR, ARM_VT2, ARM_ISAR_LDRHT, "ldr", 2, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (LDRSB,  LDR, ARM_VT2, ARM_ISAR_LDRHT, "ldr", 1, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (STR,    STR, ARM_VT2, ARM_ISAR_LDRBT, "str", 4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (STRH,   STR, ARM_VT2, ARM_ISAR_LDRHT, "str", 2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_LDR    (STRB,   STR, ARM_VT2, ARM_ISAR_LDRBT, "str", 1, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_PLD    (PLD,    PLD, ARM_VT2, ARM_ISAR_PLD,   "pld", 4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_PLD    (PLI,    PLI, ARM_VT2, ARM_ISAR_PLI,   "pli", 4, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_32_UHINTH (UHINTH, NOP, ARM_VT2, ARM_ISAR_NA,    "nop"                                          ),
    ATTR_SET_32_UHINTH (UHINTB, NOP, ARM_VT2, ARM_ISAR_NA,    "nop"                                          ),

    // coprocessor instructions
    ATTR_SET_32_CDP   (CDP,   ARM_VT2, ARM_ISAR_NA, "cdp"  ),
    ATTR_SET_32_CDP2  (CDP2,  ARM_VT2, ARM_ISAR_NA, "cdp2" ),
    ATTR_SET_32_LDC   (LDC,   ARM_VT2, ARM_ISAR_NA, "ldc"  ),
    ATTR_SET_32_LDC2  (LDC2,  ARM_VT2, ARM_ISAR_NA, "ldc2" ),
    ATTR_SET_32_MCR   (MCR,   ARM_VT2, ARM_ISAR_NA, "mcr"  ),
    ATTR_SET_32_MCR2  (MCR2,  ARM_VT2, ARM_ISAR_NA, "mcr2" ),
    ATTR_SET_32_MRC   (MRC,   ARM_VT2, ARM_ISAR_NA, "mrc"  ),
    ATTR_SET_32_MRC2  (MRC2,  ARM_VT2, ARM_ISAR_NA, "mrc2" ),
    ATTR_SET_32_LDC   (STC,   ARM_VT2, ARM_ISAR_NA, "stc"  ),
    ATTR_SET_32_LDC2  (STC2,  ARM_VT2, ARM_ISAR_NA, "stc2" ),
    ATTR_SET_32_MCRR  (MCRR,  ARM_VT2, ARM_ISAR_NA, "mcrr" ),
    ATTR_SET_32_MCRR2 (MCRR2, ARM_VT2, ARM_ISAR_NA, "mcrr2"),
    ATTR_SET_32_MCRR  (MRRC,  ARM_VT2, ARM_ISAR_NA, "mrrc" ),
    ATTR_SET_32_MCRR2 (MRRC2, ARM_VT2, ARM_ISAR_NA, "mrrc2"),

    ////////////////////////////////////////////////////////////////////////////
    // VFP Instructions (single precision only)
    ////////////////////////////////////////////////////////////////////////////

    // VFP data processing instructions
    ATTR_SET_32_VFP_RRR     (VMLA_VFP,     7, ARM_ISAR_VFPV2,  "vmla"),
    ATTR_SET_32_VFP_RRR     (VMLS_VFP,     7, ARM_ISAR_VFPV2,  "vmls"),
    ATTR_SET_32_VFP_RRR     (VNMLS_VFP,    7, ARM_ISAR_VFPV2,  "vnmls"),
    ATTR_SET_32_VFP_RRR     (VNMLA_VFP,    7, ARM_ISAR_VFPV2,  "vnmla"),
    ATTR_SET_32_VFP_RRR     (VMUL_VFP,     7, ARM_ISAR_VFPV2,  "vmul"),
    ATTR_SET_32_VFP_RRR     (VNMUL_VFP,    7, ARM_ISAR_VFPV2,  "vnmul"),
    ATTR_SET_32_VFP_RRR     (VADD_VFP,     7, ARM_ISAR_VFPV2,   "vadd"),
    ATTR_SET_32_VFP_RRR     (VSUB_VFP,     7, ARM_ISAR_VFPV2,   "vsub"),
    ATTR_SET_32_VFP_RRR     (VDIV_VFP,     7, ARM_ISAR_VFPDIV,  "vdiv"),
    ATTR_SET_32_VFP_RRR     (VFNMA_VFP,    7, ARM_ISAR_VFPFMAC, "vfnma"),
    ATTR_SET_32_VFP_RRR     (VFNMS_VFP,    7, ARM_ISAR_VFPFMAC, "vfnms"),
    ATTR_SET_32_VFP_RRR     (VFMA_VFP,     7, ARM_ISAR_VFPFMAC, "vfma"),
    ATTR_SET_32_VFP_RRR     (VFMS_VFP,     7, ARM_ISAR_VFPFMAC, "vfms"),
    ATTR_SET_32_VFP_RI      (VMOVI_VFP,    7, ARM_ISAR_VFPV3,   "vmov"),
    ATTR_SET_32_VFP_RR      (VMOVR_VFP,    7, ARM_ISAR_VFPV2,   "vmov"),
    ATTR_SET_32_VFP_RR      (VABS_VFP,     7, ARM_ISAR_VFPV2,   "vabs"),
    ATTR_SET_32_VFP_RR      (VNEG_VFP,     7, ARM_ISAR_VFPV2,   "vneg"),
    ATTR_SET_32_VFP_RR      (VSQRT_VFP,    7, ARM_ISAR_VFPSQRT, "vsqrt"),
    ATTR_SET_32_VFP_RR_S_S2 (VCVTBFH_VFP,  7, ARM_ISAR_VFPHP,   "vcvtb", 32, 16),
    ATTR_SET_32_VFP_RR_S_S2 (VCVTTFH_VFP,  7, ARM_ISAR_VFPHP,   "vcvtt", 32, 16),
    ATTR_SET_32_VFP_RR_S_S2 (VCVTBHF_VFP,  7, ARM_ISAR_VFPHP,   "vcvtb", 16, 32),
    ATTR_SET_32_VFP_RR_S_S2 (VCVTTHF_VFP,  7, ARM_ISAR_VFPHP,   "vcvtt", 16, 32),
    ATTR_SET_32_VFP_RR      (VCMP_VFP,     7, ARM_ISAR_VFPV2,   "vcmp"),
    ATTR_SET_32_VFP_RR      (VCMPE_VFP,    7, ARM_ISAR_VFPV2,   "vcmpe"),
    ATTR_SET_32_VFP_R0      (VCMP0_VFP,    7, ARM_ISAR_VFPV2,   "vcmp"),
    ATTR_SET_32_VFP_R0      (VCMPE0_VFP,   7, ARM_ISAR_VFPV2,   "vcmpe"),
    ATTR_SET_32_VFP_LS_T    (VCVTFU_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _U),
    ATTR_SET_32_VFP_LS_T    (VCVTFS_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _S),
    ATTR_SET_32_VFP_RI_T2C  (VCVTFXUH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _U16, 16),
    ATTR_SET_32_VFP_RI_T2C  (VCVTFXUW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _U32, 32),
    ATTR_SET_32_VFP_RI_T2C  (VCVTFXSH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _S16, 16),
    ATTR_SET_32_VFP_RI_T2C  (VCVTFXSW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _S32, 32),
    ATTR_SET_32_VFP_NS_T    (VCVTRUF_VFP,  7, ARM_ISAR_VFPCVT2, "vcvtr", _U),
    ATTR_SET_32_VFP_NS_T    (VCVTUF_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _U),
    ATTR_SET_32_VFP_NS_T    (VCVTRSF_VFP,  7, ARM_ISAR_VFPCVT2, "vcvtr", _S),
    ATTR_SET_32_VFP_NS_T    (VCVTSF_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _S),
    ATTR_SET_32_VFP_RI_T2C  (VCVTXFSH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _S16, _F32, 16),
    ATTR_SET_32_VFP_RI_T2C  (VCVTXFSW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _S32, _F32, 32),
    ATTR_SET_32_VFP_RI_T2C  (VCVTXFUH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _U16, _F32, 16),
    ATTR_SET_32_VFP_RI_T2C  (VCVTXFUW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _U32, _F32, 32),

    // Extension register load/store instructions
    ATTR_SET_32_SDFP_LDSTM    (VSTMIA,  7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_32_SDFP_LDSTM    (VSTMIAW, 7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_32_SDFP_LDST     (VSTR,    7, ARM_ISAR_VMRS, "vstr"),
    ATTR_SET_32_SDFP_LDSTM    (VSTMDBW, 7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_32_SDFP_PUSH     (VPUSH,   7, ARM_ISAR_VMRS, "vpush"),
    ATTR_SET_32_SDFP_LDSTM    (VLDMIA,  7, ARM_ISAR_VMRS, "vldm"),
    ATTR_SET_32_SDFP_LDSTM    (VLDMIAW, 7, ARM_ISAR_VMRS, "vldm"),
    ATTR_SET_32_SDFP_PUSH     (VPOP,    7, ARM_ISAR_VMRS, "vpop"),
    ATTR_SET_32_SDFP_LDST     (VLDR,    7, ARM_ISAR_VMRS, "vldr"),
    ATTR_SET_32_SDFP_LDSTM    (VLDMDBW, 7, ARM_ISAR_VMRS, "vldm"),

    // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
    ATTR_SET_32_VMRS   (VMRS,   7, ARM_ISAR_VMRS, "vmrs"),
    ATTR_SET_32_VMSR   (VMSR,   7, ARM_ISAR_VMRS, "vmsr"),
    ATTR_SET_32_VMOVRS (VMOVRS, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVSR (VMOVSR, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVZR (VMOVZR, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVRZ (VMOVRZ, 7, ARM_ISAR_VMRS, "vmov"),

    // 64-bit transfer instructions between ARM core regs and extension regs
    ATTR_SET_32_VMOVRRD  (VMOVRRD,  7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVDRR  (VMOVDRR,  7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVRRSS (VMOVRRSS, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_32_VMOVSSRR (VMOVSSRR, 7, ARM_ISAR_VMRS, "vmov"),

    ////////////////////////////////////////////////////////////////////////////
    // terminator
    ////////////////////////////////////////////////////////////////////////////

    // dummy entry for undecoded instruction
    [TT_LAST] = {type:ARM_IT_LAST}
};


////////////////////////////////////////////////////////////////////////////////
// GENERIC DECODE TYPES
////////////////////////////////////////////////////////////////////////////////

//
// This type specifies the pattern for a decode table entry
//
typedef struct decodeEntryS {
    armThumbType type    :16;
    Uns32        priority:16;
    const char  *name;
    const char  *pattern;
} decodeEntry;


////////////////////////////////////////////////////////////////////////////////
// 16-BIT INSTRUCTION DECODE TABLE
////////////////////////////////////////////////////////////////////////////////

//
// Create the 16-bit Thumb instruction decode table
//
static vmidDecodeTableP createDecodeTableThumb16(void) {

    const static decodeEntry entries[] = {

        // data processing instructions
        DECODE_SET_16_ADC  (ADC,    "010000|0101"),
        DECODE_SET_16_ADD1 (ADD1,   "0001110"),
        DECODE_SET_16_ADD2 (ADD2,   "00110"),
        DECODE_SET_16_ADD1 (ADD3,   "0001100"),
        DECODE_SET_16_ADD4 (ADD4LL, "01000100|0|0"),
        DECODE_SET_16_ADD4 (ADD4LH, "01000100|0|1"),
        DECODE_SET_16_ADD4 (ADD4H,  "01000100|1|."),
        DECODE_SET_16_ADD2 (ADD5,   "10100"),
        DECODE_SET_16_ADD2 (ADD6,   "10101"),
        DECODE_SET_16_ADD7 (ADD7,   "101100000"),
        DECODE_SET_16_ADC  (AND,    "010000|0000"),
        DECODE_SET_16_ASR1 (ASR1,   "00010"),
        DECODE_SET_16_ADC  (ASR2,   "010000|0100"),
        DECODE_SET_16_ADC  (BIC,    "010000|1110"),
        DECODE_SET_16_ADC  (EOR,    "010000|0001"),
        DECODE_SET_16_ASR1 (LSL1,   "00000"),
        DECODE_SET_16_ADC  (LSL2,   "010000|0010"),
        DECODE_SET_16_ASR1 (LSR1,   "00001"),
        DECODE_SET_16_ADC  (LSR2,   "010000|0011"),
        DECODE_SET_16_ADD2 (MOV1,   "00100"),
        DECODE_SET_16_ADC  (MOV2,   "000000|0000"),
        DECODE_SET_16_ADD4 (MOV3LL, "01000110|0|0"),
        DECODE_SET_16_ADD4 (MOV3LH, "01000110|0|1"),
        DECODE_SET_16_ADD4 (MOV3H,  "01000110|1|."),
        DECODE_SET_16_ADC  (MVN,    "010000|1111"),
        DECODE_SET_16_ADC  (NEG,    "010000|1001"),
        DECODE_SET_16_ADC  (ORR,    "010000|1100"),
        DECODE_SET_16_ADC  (ROR,    "010000|0111"),
        DECODE_SET_16_ADC  (SBC,    "010000|0110"),
        DECODE_SET_16_ADD1 (SUB1,   "0001111"),
        DECODE_SET_16_ADD2 (SUB2,   "00111"),
        DECODE_SET_16_ADD1 (SUB3,   "0001101"),
        DECODE_SET_16_ADD7 (SUB4,   "101100001"),

        // multiply instructions
        DECODE_SET_16_ADC (MUL,  "010000|1101"),

        // compare instructions
        DECODE_SET_16_ADC  (CMN,    "010000|1011"),
        DECODE_SET_16_ADD2 (CMP1,   "00101"),
        DECODE_SET_16_ADC  (CMP2,   "010000|1010"),
        DECODE_SET_16_ADD4 (CMP3LH, "01000101|0|1"),
        DECODE_SET_16_ADD4 (CMP3H,  "01000101|1|."),
        DECODE_SET_16_ADC  (TST,    "010000|1000"),

        // branch instructions
        DECODE_SET_16_B1   (B1,   "...."),
        DECODE_SET_16_B2   (B2,   "00"),
        DECODE_SET_16_BLX2 (BLX2, "010001111"),
        DECODE_SET_16_BLX2 (BX,   "010001110"),
        DECODE_SET_16_SWI  (SWI,  "1111"),
        DECODE_SET_16_SWI  (BU,   "1110"),

        // miscellaneous instructions
        DECODE_SET_16_BKPT (CPS,    "0110011"),
        DECODE_SET_16_BKPT (CBNZ,   "10.1..."),
        DECODE_SET_16_BKPT (CBZ,    "00.1..."),
        DECODE_SET_16_BKPT (SXTH,   "001000."),
        DECODE_SET_16_BKPT (SXTB,   "001001."),
        DECODE_SET_16_BKPT (UXTH,   "001010."),
        DECODE_SET_16_BKPT (UXTB,   "001011."),
        DECODE_SET_16_BKPT (REV,    "101000."),
        DECODE_SET_16_BKPT (REV16,  "101001."),
        DECODE_SET_16_BKPT (REVSH,  "101011."),
        DECODE_SET_16_BKPT (BKPT,   "1110..."),

        // load and store instructions
        DECODE_SET_16_ASR1 (LDR1,  "01101"),
        DECODE_SET_16_ADD1 (LDR2,  "0101100"),
        DECODE_SET_16_ADD2 (LDR3,  "01001"),
        DECODE_SET_16_ADD2 (LDR4,  "10011"),
        DECODE_SET_16_ASR1 (LDRB1, "01111"),
        DECODE_SET_16_ADD1 (LDRB2, "0101110"),
        DECODE_SET_16_ASR1 (LDRH1, "10001"),
        DECODE_SET_16_ADD1 (LDRH2, "0101101"),
        DECODE_SET_16_ADD1 (LDRSB, "0101011"),
        DECODE_SET_16_ADD1 (LDRSH, "0101111"),
        DECODE_SET_16_ASR1 (STR1,  "01100"),
        DECODE_SET_16_ADD1 (STR2,  "0101000"),
        DECODE_SET_16_ADD2 (STR3,  "10010"),
        DECODE_SET_16_ASR1 (STRB1, "01110"),
        DECODE_SET_16_ADD1 (STRB2, "0101010"),
        DECODE_SET_16_ASR1 (STRH1, "10000"),
        DECODE_SET_16_ADD1 (STRH2, "0101001"),

        // load and store multiple instructions
        DECODE_SET_16_ADD2 (LDMIA, "11001"),
        DECODE_SET_16_POP  (POP,   "1011110"),
        DECODE_SET_16_POP  (PUSH,  "1011010"),
        DECODE_SET_16_ADD2 (STMIA, "11000"),

        // if-then and hints
        DECODE_SET_16_IT    (IT,    "....", "...."),
        DECODE_SET_16_HINT1 (NOP,   "....", "0000"),
        DECODE_SET_16_HINT2 (YIELD, "0001", "0000"),
        DECODE_SET_16_HINT2 (WFE,   "0010", "0000"),
        DECODE_SET_16_HINT2 (WFI,   "0011", "0000"),
        DECODE_SET_16_HINT2 (SEV,   "0100", "0000"),

        // terminator
        {0}
    };

    // create the table
    vmidDecodeTableP   table = vmidNewDecodeTable(16, TT_LAST);
    const decodeEntry *entry;

    // add all entries to the decode table
    for(entry=entries; entry->pattern; entry++) {
        vmidNewEntryFmtBin(
            table,
            entry->name,
            entry->type,
            entry->pattern,
            entry->priority
        );
    }

    return table;
}

//
// Create the 32-bit Thumb instruction decode table
//
static vmidDecodeTableP createDecodeTableThumb32(void) {

    const static decodeEntry entries[] = {

        // data processing
        DECODE_SET_32_AND (AND, "0000"),
        DECODE_SET_32_TST (TST, "0000"),
        DECODE_SET_32_AND (BIC, "0001"),
        DECODE_SET_32_AND (ORR, "0010"),
        DECODE_SET_32_MOV (MOV, "0010"),
        DECODE_SET_32_AND (ORN, "0011"),
        DECODE_SET_32_MOV (MVN, "0011"),
        DECODE_SET_32_AND (EOR, "0100"),
        DECODE_SET_32_TST (TEQ, "0100"),
        DECODE_SET_32_AND (ADD, "1000"),
        DECODE_SET_32_TST (CMN, "1000"),
        DECODE_SET_32_AND (ADC, "1010"),
        DECODE_SET_32_AND (SBC, "1011"),
        DECODE_SET_32_AND (SUB, "1101"),
        DECODE_SET_32_TST (CMP, "1101"),
        DECODE_SET_32_AND (RSB, "1110"),

        // pack halfword
        DECODE_SET_32_PKHBT (PKHBT, "0"),
        DECODE_SET_32_PKHBT (PKHTB, "1"),

        // data processing (plain binary immediate)
        DECODE_SET_32_ADD_PI (ADD_PI,     "00000"),
        DECODE_SET_32_ADR_PI (ADD_ADR_PI, "00000"),
        DECODE_SET_32_ADD_PI (SUB_PI,     "01010"),
        DECODE_SET_32_ADR_PI (SUB_ADR_PI, "01010"),
        DECODE_SET_32_ADD_PI (MOV_PI,     "00100"),
        DECODE_SET_32_ADD_PI (MOVT_PI,    "01100"),
        DECODE_SET_32_ADD_PI (SSAT,       "100.0"),
        DECODE_SET_32_SSAT16 (SSAT16,     "10010"),
        DECODE_SET_32_ADD_PI (SBFX,       "10100"),
        DECODE_SET_32_ADD_PI (BFI,        "10110"),
        DECODE_SET_32_BFC    (BFC,        "10110"),
        DECODE_SET_32_ADD_PI (UBFX,       "11100"),
        DECODE_SET_32_ADD_PI (USAT,       "110.0"),
        DECODE_SET_32_SSAT16 (USAT16,     "11010"),

        // data processing (register)
        DECODE_SET_32_LSL  (LSL,     "000.", "0000", "...."),
        DECODE_SET_32_LSL  (LSR,     "001.", "0000", "...."),
        DECODE_SET_32_LSL  (ASR,     "010.", "0000", "...."),
        DECODE_SET_32_LSL  (ROR,     "011.", "0000", "...."),
        DECODE_SET_32_LSL  (SXTAH,   "0000", "1...", "...."),
        DECODE_SET_32_SXTH (SXTH,    "0000", "1...", "1111"),
        DECODE_SET_32_LSL  (UXTAH,   "0001", "1...", "...."),
        DECODE_SET_32_SXTH (UXTH,    "0001", "1...", "1111"),
        DECODE_SET_32_LSL  (SXTAB16, "0010", "1...", "...."),
        DECODE_SET_32_SXTH (SXTB16,  "0010", "1...", "1111"),
        DECODE_SET_32_LSL  (UXTAB16, "0011", "1...", "...."),
        DECODE_SET_32_SXTH (UXTB16,  "0011", "1...", "1111"),
        DECODE_SET_32_LSL  (SXTAB,   "0100", "1...", "...."),
        DECODE_SET_32_SXTH (SXTB,    "0100", "1...", "1111"),
        DECODE_SET_32_LSL  (UXTAB,   "0101", "1...", "...."),
        DECODE_SET_32_SXTH (UXTB,    "0101", "1...", "1111"),

        // parallel add/subtract instructions
        DECODE_SET_32_PAS (ADD16, "001"),
        DECODE_SET_32_PAS (ASX,   "010"),
        DECODE_SET_32_PAS (SAX,   "110"),
        DECODE_SET_32_PAS (SUB16, "101"),
        DECODE_SET_32_PAS (ADD8,  "000"),
        DECODE_SET_32_PAS (SUB8,  "100"),

        // miscellaneous operation instructions
        DECODE_SET_32_LSL (QADD,  "1000", "1000", "...."),
        DECODE_SET_32_LSL (QDADD, "1000", "1001", "...."),
        DECODE_SET_32_LSL (QSUB,  "1000", "1010", "...."),
        DECODE_SET_32_LSL (QDSUB, "1000", "1011", "...."),
        DECODE_SET_32_LSL (REV,   "1001", "1000", "...."),
        DECODE_SET_32_LSL (REV16, "1001", "1001", "...."),
        DECODE_SET_32_LSL (RBIT,  "1001", "1010", "...."),
        DECODE_SET_32_LSL (REVSH, "1001", "1011", "...."),
        DECODE_SET_32_LSL (SEL,   "1010", "1000", "...."),
        DECODE_SET_32_LSL (CLZ,   "1011", "1000", "...."),

        // multiply, multiply accumulate and absolute difference instructions
        DECODE_SET_32_MLA     (MLA,    "0000", "0000"),
        DECODE_SET_32_MUL     (MUL,    "0000", "0000"),
        DECODE_SET_32_MLA     (MLS,    "0000", "0001"),
        DECODE_SET_32_MLA     (SDIV,   "1001", "1111"),
        DECODE_SET_32_MLA     (UDIV,   "1011", "1111"),
        DECODE_SET_32_SMLA_XY (SMLA,   "0001", "00"  ),
        DECODE_SET_32_SMUL_XY (SMUL,   "0001", "00"  ),
        DECODE_SET_32_SMLAD   (SMLAD,  "0010", "000" ),
        DECODE_SET_32_SMUAD   (SMUAD,  "0010", "000" ),
        DECODE_SET_32_SMLAW   (SMLAW,  "0011", "000" ),
        DECODE_SET_32_SMULW   (SMULW,  "0011", "000" ),
        DECODE_SET_32_SMLAD   (SMLSD,  "0100", "000" ),
        DECODE_SET_32_SMUAD   (SMUSD,  "0100", "000" ),
        DECODE_SET_32_SMMLA   (SMMLA,  "0101", "000" ),
        DECODE_SET_32_SMMUL   (SMMUL,  "0101", "000" ),
        DECODE_SET_32_SMMLA   (SMMLS,  "0110", "000" ),
        DECODE_SET_32_MUL     (USAD8,  "0111", "0000"),
        DECODE_SET_32_MLA     (USADA8, "0111", "0000"),
        DECODE_SET_32_MLA     (SMLAL,  "1100", "0000"),
        DECODE_SET_32_MLA     (SMULL,  "1000", "0000"),
        DECODE_SET_32_MLA     (UMAAL,  "1110", "0110"),
        DECODE_SET_32_MLA     (UMLAL,  "1110", "0000"),
        DECODE_SET_32_MLA     (UMULL,  "1010", "0000"),
        DECODE_SET_32_SMLA_XY (SMLAL,  "1100", "10"  ),
        DECODE_SET_32_SMLAD   (SMLALD, "1100", "110" ),
        DECODE_SET_32_SMLAD   (SMLSLD, "1101", "110" ),

        // branch and miscellaneous control instructions
        DECODE_SET_32_B1    (B1,         "0.0", "."),
        DECODE_SET_32_B1    (B2,         "0.1", "."),
        DECODE_SET_32_B1    (BL,         "1.1", "."),
        DECODE_SET_32_MSR   (MSR,        "0.0", "011100."),
        DECODE_SET_32_HINT1 (NOP,        "0.0", "0111010", "........"),
        DECODE_SET_32_HINT2 (YIELD,      "0.0", "0111010", "00000001"),
        DECODE_SET_32_HINT2 (WFE,        "0.0", "0111010", "00000010"),
        DECODE_SET_32_HINT2 (WFI,        "0.0", "0111010", "00000011"),
        DECODE_SET_32_HINT2 (SEV,        "0.0", "0111010", "00000100"),
        DECODE_SET_32_HINT2 (DBG,        "0.0", "0111010", "1111...."),
        DECODE_SET_32_MSR   (MRS,        "0.0", "011111."),
        DECODE_SET_32_UNDEF (UNDEF,      "0.0", ".111..."),
        DECODE_SET_32_CLREX (CLREX,      "0010"),
        DECODE_SET_32_CLREX (DSB,        "0100"),
        DECODE_SET_32_CLREX (DMB,        "0101"),
        DECODE_SET_32_CLREX (ISB,        "0110"),

        // load and store multiple instructions
        DECODE_SET_32_SRS  (STMDB, "10", ".0...."),
        DECODE_SET_32_SRS  (STMIA, "01", ".0...."),
        DECODE_SET_32_SRS  (LDMDB, "10", ".1...."),
        DECODE_SET_32_SRS  (LDMIA, "01", ".1...."),
        DECODE_SET_32_POPM (POPM,  "01", "111101"),
        DECODE_SET_32_POPM (PUSHM, "10", "101101"),

        // dual and exclusive instructions
        DECODE_SET_32_LDRD_IMM (LDRD_IMM, "0.", "11", "...."),
        DECODE_SET_32_LDRD_IMM (LDRD_IMM, "1.", ".1", "...."),
        DECODE_SET_32_LDRD_IMM (STRD_IMM, "0.", "10", "...."),
        DECODE_SET_32_LDRD_IMM (STRD_IMM, "1.", ".0", "...."),
        DECODE_SET_32_LDREX    (LDREX,    "00", "01", "...."),
        DECODE_SET_32_LDREX    (LDREXB,   "01", "01", "0100"),
        DECODE_SET_32_LDREX    (LDREXH,   "01", "01", "0101"),
        DECODE_SET_32_LDREX    (STREX,    "00", "00", "...."),
        DECODE_SET_32_LDREX    (STREXB,   "01", "00", "0100"),
        DECODE_SET_32_LDREX    (STREXH,   "01", "00", "0101"),
        DECODE_SET_32_LDREX    (TBB,      "01", "01", "0000"),
        DECODE_SET_32_LDREX    (TBH,      "01", "01", "0001"),

        // load instructions
        DECODE_SET_32_LDR    (LDR,   "0", "10"),
        DECODE_SET_32_LDR    (LDRH,  "0", "01"),
        DECODE_SET_32_LDR    (LDRB,  "0", "00"),
        DECODE_SET_32_LDR    (LDRSH, "1", "01"),
        DECODE_SET_32_LDR    (LDRSB, "1", "00"),
        DECODE_SET_32_STR    (STR,   "0", "10"),
        DECODE_SET_32_STR    (STRH,  "0", "01"),
        DECODE_SET_32_STR    (STRB,  "0", "00"),
        DECODE_SET_32_PLD    (PLD,   "0", "00"),
        DECODE_SET_32_PLD    (PLI,   "1", "00"),
        DECODE_SET_32_UHINTH (UHINTH,     "01"),
        DECODE_SET_32_UHINTH (UHINTB,     "00"),

        // coprocessor instructions
        DECODE_SET_32_CDP   (CDP       ),
        DECODE_SET_32_CDP2  (CDP2      ),
        DECODE_SET_32_LDC   (LDC,   "1"),
        DECODE_SET_32_LDC2  (LDC2,  "1"),
        DECODE_SET_32_MCR   (MCR,   "0"),
        DECODE_SET_32_MCR2  (MCR2,  "0"),
        DECODE_SET_32_MCR   (MRC,   "1"),
        DECODE_SET_32_MCR2  (MRC2,  "1"),
        DECODE_SET_32_LDC   (STC,   "0"),
        DECODE_SET_32_LDC2  (STC2,  "0"),
        DECODE_SET_32_MCRR  (MCRR,  "0"),
        DECODE_SET_32_MCRR2 (MCRR2, "0"),
        DECODE_SET_32_MCRR  (MRRC,  "1"),
        DECODE_SET_32_MCRR2 (MRRC2, "1"),

        // VFP data processing instructions
        DECODE_SET_32_VFP_S (VMLA_VFP,     "0.00", "....", ".0"),
        DECODE_SET_32_VFP_S (VMLS_VFP,     "0.00", "....", ".1"),
        DECODE_SET_32_VFP_S (VNMLS_VFP,    "0.01", "....", ".0"),
        DECODE_SET_32_VFP_S (VNMLA_VFP,    "0.01", "....", ".1"),
        DECODE_SET_32_VFP_S (VMUL_VFP,     "0.10", "....", ".0"),
        DECODE_SET_32_VFP_S (VNMUL_VFP,    "0.10", "....", ".1"),
        DECODE_SET_32_VFP_S (VADD_VFP,     "0.11", "....", ".0"),
        DECODE_SET_32_VFP_S (VSUB_VFP,     "0.11", "....", ".1"),
        DECODE_SET_32_VFP_S (VDIV_VFP,     "1.00", "....", ".0"),
        // Note: Arm docs wrong: op=1 is VFNMA, not VFNMS
        DECODE_SET_32_VFP_S (VFNMA_VFP,    "1.01", "....", ".1"),
        DECODE_SET_32_VFP_S (VFNMS_VFP,    "1.01", "....", ".0"),
        DECODE_SET_32_VFP_S (VFMA_VFP,     "1.10", "....", ".0"),
        DECODE_SET_32_VFP_S (VFMS_VFP,     "1.10", "....", ".1"),
        DECODE_SET_32_VFP_S (VMOVI_VFP,    "1.11", "....", ".0"),
        DECODE_SET_32_VFP_S (VMOVR_VFP,    "1.11", "0000", "01"),
        DECODE_SET_32_VFP_S (VABS_VFP,     "1.11", "0000", "11"),
        DECODE_SET_32_VFP_S (VNEG_VFP,     "1.11", "0001", "01"),
        DECODE_SET_32_VFP_S (VSQRT_VFP,    "1.11", "0001", "11"),
        DECODE_SET_32_VFP_S (VCVTBFH_VFP,  "1.11", "0010", "01"),
        DECODE_SET_32_VFP_S (VCVTTFH_VFP,  "1.11", "0010", "11"),
        DECODE_SET_32_VFP_S (VCVTBHF_VFP,  "1.11", "0011", "01"),
        DECODE_SET_32_VFP_S (VCVTTHF_VFP,  "1.11", "0011", "11"),
        DECODE_SET_32_VFP_S (VCMP_VFP,     "1.11", "0100", "01"),
        DECODE_SET_32_VFP_S (VCMPE_VFP,    "1.11", "0100", "11"),
        DECODE_SET_32_VFP_S (VCMP0_VFP,    "1.11", "0101", "01"),
        DECODE_SET_32_VFP_S (VCMPE0_VFP,   "1.11", "0101", "11"),
        DECODE_SET_32_VFP_S (VCVTFU_VFP,   "1.11", "1000", "01"),
        DECODE_SET_32_VFP_S (VCVTFS_VFP,   "1.11", "1000", "11"),
        DECODE_SET_32_VFP_S (VCVTFXSH_VFP, "1.11", "1010", "01"),
        DECODE_SET_32_VFP_S (VCVTFXSW_VFP, "1.11", "1010", "11"),
        DECODE_SET_32_VFP_S (VCVTFXUH_VFP, "1.11", "1011", "01"),
        DECODE_SET_32_VFP_S (VCVTFXUW_VFP, "1.11", "1011", "11"),
        DECODE_SET_32_VFP_S (VCVTRUF_VFP,  "1.11", "1100", "01"),
        DECODE_SET_32_VFP_S (VCVTUF_VFP,   "1.11", "1100", "11"),
        DECODE_SET_32_VFP_S (VCVTRSF_VFP,  "1.11", "1101", "01"),
        DECODE_SET_32_VFP_S (VCVTSF_VFP,   "1.11", "1101", "11"),
        DECODE_SET_32_VFP_S (VCVTXFSH_VFP, "1.11", "1110", "01"),
        DECODE_SET_32_VFP_S (VCVTXFSW_VFP, "1.11", "1110", "11"),
        DECODE_SET_32_VFP_S (VCVTXFUH_VFP, "1.11", "1111", "01"),
        DECODE_SET_32_VFP_S (VCVTXFUW_VFP, "1.11", "1111", "11"),

        // Extension register load/store instructions
        DECODE_SET_32_SDFP_LDST     (VSTMIA,  "01.00"),
        DECODE_SET_32_SDFP_LDST     (VSTMIAW, "01.10"),
        DECODE_SET_32_SDFP_LDST     (VSTR,    "1..00"),
        DECODE_SET_32_SDFP_LDST     (VSTMDBW, "10.10"),
        DECODE_SET_32_SDFP_PUSH_POP (VPUSH,   "10.10"),
        DECODE_SET_32_SDFP_LDST     (VLDMIA,  "01.01"),
        DECODE_SET_32_SDFP_LDST     (VLDMIAW, "01.11"),
        DECODE_SET_32_SDFP_PUSH_POP (VPOP,    "01.11"),
        DECODE_SET_32_SDFP_LDST     (VLDR,    "1..01"),
        DECODE_SET_32_SDFP_LDST     (VLDMDBW, "10.11"),

        // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
        DECODE_SET_32_VMRS   (VMRS,   "1", "0", "111", ".."),
        DECODE_SET_32_VMRS   (VMSR,   "0", "0", "111", ".."),
        DECODE_SET_32_VMRS   (VMOVRS, "1", "0", "000", ".."),
        DECODE_SET_32_VMRS   (VMOVSR, "0", "0", "000", ".."),
        DECODE_SET_32_VMRS   (VMOVRZ, "1", "1", "00.", "00"),
        DECODE_SET_32_VMRS   (VMOVZR, "0", "1", "00.", "00"),

        // 64-bit transfer instructions between ARM core regs and extension regs
        DECODE_SET_32_VMOVRRD (VMOVRRD,  "1", "1", "00.1"),
        DECODE_SET_32_VMOVRRD (VMOVDRR,  "0", "1", "00.1"),
        DECODE_SET_32_VMOVRRD (VMOVRRSS, "1", "0", "00.1"),
        DECODE_SET_32_VMOVRRD (VMOVSSRR, "0", "0", "00.1"),

        // terminator
        {0}
    };

    // create the table
    vmidDecodeTableP   table = vmidNewDecodeTable(32, TT_LAST);
    const decodeEntry *entry;

    // add all entries to the decode table
    for(entry=entries; entry->pattern; entry++) {
        vmidNewEntryFmtBin(
            table,
            entry->name,
            entry->type,
            entry->pattern,
            entry->priority
        );
    }

    return table;
}

//
// Get the 16-bit Thumb instruction decode table
//
static vmidDecodeTableP getDecodeTableThumb16(void) {

    static vmidDecodeTableP table;

    if(!table) {
        table = createDecodeTableThumb16();
    }

    return table;
}

//
// Get the 32-bit Thumb instruction decode table
//
static vmidDecodeTableP getDecodeTableThumb32(void) {

    static vmidDecodeTableP table;

    if(!table) {
        table = createDecodeTableThumb32();
    }

    return table;
}

//
// Return effect that the instruction has on the flags (note that this may
// depend on whether the instruction is in an if-then block)
//
static armSetFlags getSetFlagsThumb(armP arm, Uns32 instr, armSetFlags sf) {

    armSetFlags result;

    switch(sf) {

        case SF_0:
            result = ARM_SF_0;
            break;

        case SF_V:
            result = ARM_SF_V;
            break;

        case SF_I:
            result = ARM_SF_I;
            break;

        case SF_20_V:
            result = OP_F20(instr) ? ARM_SF_V : ARM_SF_0;
            break;

        case SF_IT:
            if(arm->itStateMT) {
                result = ARM_SF_0;
            } else if(arm->UAL) {
                result = ARM_SF_V;
            } else {
                result = ARM_SF_I;
            }
            break;

        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return condition associated with the instruction (note that this may depend
// on whether the instruction is in an if-then block)
//
static armCondition getConditionThumb(armP arm, Uns32 instr, condSpec cond) {

    armCondition result = ARM_C_AL;

    if(arm->itStateMT) {
        result = arm->itStateMT>>4;
    } else switch(cond) {
        case CO_NA: break;
        case CO_8:  result = OP_COND_8(instr);  break;
        case CO_22: result = OP_COND_22(instr); break;
        case CO_28: result = OP_COND_28(instr); break;
        default:    VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return register index encoded in the Thumb instruction
//
static Uns8 getRegisterThumb(Uns32 instr, rSpec r) {

    Uns8 result = 0;

    switch(r) {
        case R_NA:   break;
        case R3_0:   result = OP_R3_0(instr);    break;
        case R3_3:   result = OP_R3_3(instr);    break;
        case R3_6:   result = OP_R3_6(instr);    break;
        case R3_8:   result = OP_R3_8(instr);    break;
        case R4_0:   result = OP_R4_0(instr);    break;
        case R4_8:   result = OP_R4_8(instr);    break;
        case R4_12:  result = OP_R4_12(instr);   break;
        case R4_16:  result = OP_R4_16(instr);   break;
        case R4_0H7: result = OP_R4_0_H7(instr); break;
        case R4_3H6: result = OP_R4_3_H6(instr); break;
        case R_PC:   result = ARM_REG_PC;        break;
        case R_SP:   result = ARM_REG_SP;        break;
        case R_LR:   result = ARM_REG_LR;        break;
        case V_0_5:   result = OP_V0_5(instr);   break;
        case V_16_7:  result = OP_V16_7(instr);  break;
        case V_12_22: result = OP_V12_22(instr); break;
        case V_5_0:   result = OP_V5_0(instr);   break;
        case V_22_12: result = OP_V22_12(instr); break;
        case V_7_16:  result = OP_V7_16(instr);  break;
        case V3_0:    result = OP_U_3_0(instr);  break;
        default:     VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

///
// Return shift operation encoded in the Thumb instruction
//
static armShiftOp getShiftOpThumb(Uns32 instr, shiftSpec ss) {

    Uns32 result = ARM_SO_NA;

    const static armShiftOp shiftMap[] = {
        ARM_SO_LSL, ARM_SO_LSR, ARM_SO_ASR, ARM_SO_ROR
    };

    switch(ss) {
        case SS_NA:
            break;
        case SS_ASR:
            result = ARM_SO_ASR;
            break;
        case SS_LSL:
            result = ARM_SO_LSL;
            break;
        case SS_LSR:
            result = ARM_SO_LSR;
            break;
        case SS_ROR:
            result = ARM_SO_ROR;
            break;
        case SS_RRX:
            result = ARM_SO_RRX;
            break;
        case SS2_4:
            result = shiftMap[OP_U_2_4(instr)];
            break;
        case SS2_20:
            result = shiftMap[OP_U_2_20(instr)];
            break;
        case SS2_21:
            result = shiftMap[OP_U_2_21(instr)];
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Allow for cases where constant value of zero is interpreted as 32
//
static Uns32 adjustShift(Uns32 result, constSpec c, armShiftOp so) {

    if((result==0) && (c!=CS_NA) && ((so==ARM_SO_LSR) || (so==ARM_SO_ASR))) {
        result = 32;
    }

    return result;
}

//
// Return a 5-bit plain immediate constant encoded within the Thumb instruction
//
static Uns32 plainImmediateThumb5(Uns32 instr) {

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 _u1  :  6;
            Uns32 imm2 :  2;
            Uns32 _u2  :  4;
            Uns32 imm3 :  3;
            Uns32 _u3  : 17;
        } f;
    } u1 = {instr};

    // compose result
    return ((u1.f.imm3)<<2) | u1.f.imm2;
}

//
// Return a 12-bit plain immediate constant encoded within the Thumb instruction
//
static Uns32 plainImmediateThumb12(Uns32 instr) {

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 imm8 :  8;
            Uns32 _u1  :  4;
            Uns32 imm3 :  3;
            Uns32 _u2  : 11;
            Uns32 i    :  1;
            Uns32 _u3  :  5;
        } f;
    } u1 = {instr};

    // compose result
    return ((u1.f.i)<<11) | ((u1.f.imm3)<<8) | u1.f.imm8;
}

//
// Return a 16-bit plain immediate constant encoded within the Thumb instruction
//
static Uns32 plainImmediateThumb16(Uns32 instr) {

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 imm8 : 8;
            Uns32 _u1  : 4;
            Uns32 imm3 : 3;
            Uns32 _u2  : 1;
            Uns32 imm4 : 4;
            Uns32 _u3  : 6;
            Uns32 i    : 1;
            Uns32 _u4  : 5;
        } f;
    } u1 = {instr};

    // compose result
    return ((u1.f.imm4)<<12) | ((u1.f.i)<<11) | ((u1.f.imm3)<<8) | u1.f.imm8;
}

//
// Return a modified immediate constant encoded within the Thumb instruction
// and set byref 'crotate' to any constant rotation
//
static Uns32 modifiedImmediateThumb(Uns32 instr, Uns8 *crotate) {

    Uns32 result;

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 bcdefgh :  7;
            Uns32 a       :  1;
            Uns32 _u1     :  4;
            Uns32 imm3    :  3;
            Uns32 _u2     : 11;
            Uns32 i       :  1;
            Uns32 _u3     :  5;
        } f;
    } u1 = {instr};

    // compose code
    Uns32 code     = ((u1.f.i)<<4) | ((u1.f.imm3)<<1) | u1.f.a;
    Uns32 abcdefgh = (u1.f.a<<7) | u1.f.bcdefgh;

    // derive result from code
    switch(code) {

        case 0x00: case 0x01:
            result = abcdefgh;
            break;

        case 0x02: case 0x03:
            result = (abcdefgh<<16) | abcdefgh;
            break;

        case 0x04: case 0x05:
            result = (abcdefgh<<24) | (abcdefgh<<8);
            break;

        case 0x06: case 0x07:
            result = (abcdefgh<<24) | (abcdefgh<<16) | (abcdefgh<<8) | abcdefgh;
            break;

        default: {
            Uns32 shift = 32-code;
            result = (1<<7) | u1.f.bcdefgh;
            result <<= shift;
            *crotate = code;
            break;
        }
    }

    return result;
}

//
// Negate the argument if U bit of Thumb instruction is zero
//
inline static Uns32 negateIfU(armInstructionInfoP info, Uns32 result) {
    return info->u ? result : -result;
}

//
// Return a constant encoded within the Thumb instruction and set byref
// 'crotate' to any constant rotation
//
static Uns32 getConstantThumb(
    armP                arm,
    armInstructionInfoP info,
    Uns32               instr,
    constSpec           c,
    armShiftOp          so,
    Uns8               *crotate
) {
    Uns32 result = 0;

    // assume constant rotation is zero
    *crotate = 0;

    switch(c) {
        case CS_NA:
            break;
        case CS_U_2_4:
            result = OP_U_2_4(instr);
            break;
        case CS_U_2_4x8:
            result = OP_U_2_4(instr)*8;
            break;
        case CS_U_2_10:
            result = OP_U_2_10(instr);
            break;
        case CS_U_3_6:
            result = OP_U_3_6(instr);
            break;
        case CS_U_4_4:
            result = OP_U_4_4(instr);
            break;
        case CS_U_4_0:
            result = OP_U_4_0(instr);
            break;
        case CS_U_5_6:
            result = OP_U_5_6(instr);
            break;
        case CS_U_5_6_SZ:
            result = OP_U_5_6(instr)*info->sz;
            break;
        case CS_U_5_0_5M16:
            result = OP_U_5_0_5(instr);
            result = 16 - (result > 16 ? 0 : result);
            break;
        case CS_U_5_0_5M32:
            result = 32 - OP_U_5_0_5(instr);
            break;
        case CS_U_7_0x4:
            result = OP_U_7_0(instr)*4;
            break;
        case CS_U_8_0:
            result = OP_U_8_0(instr);
            break;
        case CS_U_8_0_U:
            result = negateIfU(info, OP_U_8_0(instr));
            break;
        case CS_U_8_0_SZ:
            result = OP_U_8_0(instr)*info->sz;
            break;
        case CS_U_8_0x4_U:
            result = negateIfU(info, OP_U_8_0(instr)*4);
            break;
        case CS_U_8_0x4:
            result = OP_U_8_0(instr)*4;
            break;
        case CS_U_12_0:
            result = OP_U_12_0(instr);
            break;
        case CS_U_12_0_U:
            result = negateIfU(info, OP_U_12_0(instr));
            break;
        case CS_PI5:
            result = plainImmediateThumb5(instr);
            break;
        case CS_PI12:
            result = plainImmediateThumb12(instr);
            break;
        case CS_PI16:
            result = plainImmediateThumb16(instr);
            break;
        case CS_MI:
            result = modifiedImmediateThumb(instr, crotate);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    // allow for cases where constant value of zero is interpreted as 32
    return adjustShift(result, c, so);
}

//
// Return nregs, fixed or encoded within the Thumb instruction
//
static Uns8 getNRegsThumb(Uns32 instr, nregSpec nRegs) {

    Uns8 result = 0;

    switch(nRegs) {
        case NREG_NA:
            break;
        case NREG_7_1:
            result = OP_U_7_1(instr);
            break;
        case NREG_8_0:
            result = OP_U_8_0(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return bit operation width encoded in the ARM instruction
//
static Int8 getWidthThumb(Uns32 instr, widthSpec w) {

    Int8 result = 0;

    switch(w) {
        case WS_NA:
            break;
        case WS_WIDTH4:
            result = OP_U_4_0(instr);
            break;
        case WS_WIDTH4M1:
            result = OP_U_4_0(instr)+1;
            break;
        case WS_WIDTH5:
            result = OP_U_5_0(instr);
            break;
        case WS_WIDTH5M1:
            result = OP_U_5_0(instr)+1;
            break;
        case WS_MSB:
            result = OP_U_5_0(instr)-plainImmediateThumb5(instr)+1;
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Get post-indexed attributes for the Thumb instruction
//
static Bool getPostIndexedThumb(Uns32 instr, postIndexSpec pi) {

    Bool result = False;

    switch(pi) {
        case PI_0:
            result = False;
            break;
        case PI_1:
            result = True;
            break;
        case PI_10:
            result = !OP_PI_10(instr);
            break;
        case PI_24:
            result = !OP_PI_24(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Get writeback attributes for the Thumb instruction
//
static Bool getWritebackThumb(
    Uns32         instr,
    writebackSpec wb,
    Uns32         base,
    Uns32         rList
) {
    Bool result = False;

    switch(wb) {
        case WB_0:
            result = False;
            break;
        case WB_1:
            result = True;
            break;
        case WB_1_NB:
            result = !(rList & (1<<base));
            break;
        case WB_8:
            result = OP_WB_8(instr);
            break;
        case WB_21:
            result = OP_WB_21(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return target address calculated by adding the current PC and the displacement
//
static Uns32 getTarget(armInstructionInfoP info, Int32 displacement) {
    return info->thisPC + displacement;
}

//
// Return a 20-bit target address encoded within the Thumb instruction
//
static Uns32 getTarget20(armInstructionInfoP info, Uns32 instr) {

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 imm11 : 11;
            Uns32 j2    :  1;
            Uns32 _u1   :  1;
            Uns32 j1    :  1;
            Uns32 _u2   :  2;
            Uns32 imm6  :  6;
            Uns32 _u3   :  4;
            Uns32 s     :  1;
            Uns32 _u4   :  5;
        } f;
    } u1 = {instr};

    // bitfield to compose target
    union {
        Uns32 offset;
        struct {
            Uns32 zero  :  1;
            Uns32 imm11 : 11;
            Uns32 imm6  :  6;
            Uns32 j2    :  1;
            Uns32 j1    :  1;
            Uns32 s     : 12;
        } f;
    } u2;

    // fill target fields;
    u2.f.zero  = 0;
    u2.f.imm11 = u1.f.imm11;
    u2.f.imm6  = u1.f.imm6;
    u2.f.j2    = u1.f.j2;
    u2.f.j1    = u1.f.j1;
    u2.f.s     = -u1.f.s;

    // return target address
    return getTarget(info, u2.offset+4);
}

//
// Return a 24-bit target address encoded within the Thumb instruction
//
static Uns32 getTarget24(armInstructionInfoP info, Uns32 instr) {

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 imm11 : 11;
            Uns32 j2    :  1;
            Uns32 _u1   :  1;
            Uns32 j1    :  1;
            Uns32 _u2   :  2;
            Uns32 imm10 : 10;
            Uns32 s     :  1;
            Uns32 _u3   :  5;
        } f;
    } u1 = {instr};

    // bitfield to compose target
    union {
        Uns32 offset;
        struct {
            Uns32 zero  :  1;
            Uns32 imm11 : 11;
            Uns32 imm10 : 10;
            Uns32 i2    :  1;
            Uns32 i1    :  1;
            Uns32 s     :  8;
        } f;
    } u2;

    // compose i1 and i2 fields;
    Uns32 s  = u1.f.s;
    Uns32 i1 = !(u1.f.j1 ^ s);
    Uns32 i2 = !(u1.f.j2 ^ s);

    // fill target fields;
    u2.f.zero  = 0;
    u2.f.imm11 = u1.f.imm11;
    u2.f.imm10 = u1.f.imm10;
    u2.f.i2    = i2;
    u2.f.i1    = i1;
    u2.f.s     = -s;

    // return target address
    return getTarget(info, u2.offset+4);
}

//
// Return a target address encoded within the Thumb instruction
//
static Uns32 getTargetThumb(
    armInstructionInfoP info,
    Uns32               instr,
    targetSpec          t
) {
    Uns32 result = 0;

    switch(t) {
        case TC_NA:
            break;
        case TC_S8:
            result = getTarget(info, OP_TS8(instr)+4);
            break;
        case TC_S11:
            result = getTarget(info, OP_TS11(instr)+4);
            break;
        case TC_S20_T2:
            result = getTarget20(info, instr);
            break;
        case TC_S24_T2:
            result = getTarget24(info, instr);
            break;
        case TC_U9_7_3:
            result = getTarget(info, OP_TU9_7_3(instr)+4);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return coprocessor opcode1 encoded within the Thumb instruction
//
static Uns32 getCpOp1Thumb(
    armP                arm,
    armInstructionInfoP info,
    Uns32               instr,
    cpOp1Spec           cpOp1
) {
    Uns32 result = 0;

    switch(cpOp1) {
        case COP_NA:   break;
        case COP_4_4:  result = OP_CPOP1_4_4(instr);  break;
        case COP_4_20: result = OP_CPOP1_4_20(instr); break;
        case COP_3_21: result = OP_CPOP1_3_21(instr); break;
        default:       VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return register list encoded within the Thumb instruction
//
static Uns32 getRListThumb(Uns32 instr, rListSpec rList) {

    Uns32 result = 0;

    switch(rList) {
        case RL_NA:      break;
        case RL_16:      result = OP_RL_16(instr); break;
        case RL_16_LR:   result = OP_RL_16_LR(instr); break;
        case RL_16_PC:   result = OP_RL_16_PC(instr); break;
        case RL_32_SP:   result = OP_RL_32_SP(instr); break;
        case RL_32_PCSP: result = OP_RL_32_PCSP(instr); break;
        default:         VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return increment/decrement action encoded within the Thumb instruction
//
static armIncDec getIncDecThumb(
    armInstructionInfoP info,
    armP                arm,
    Uns32               instr,
    incDecSpec          incDec
) {
    armIncDec result = ARM_ID_NA;

    switch (incDec) {
        case ID_NA:   break;
        case ID_DB:   result = ARM_ID_DB;  break;
        case ID_IA:   result = ARM_ID_IA;  break;
        case ID_DB_I: result = ARM_ID_DBI; break;
        case ID_IA_I: result = ARM_ID_IAI; break;
        case ID_U_P:
        case ID_U_P_IMP:
        case ID_U_P_IAI: {
            armIncDec USpec = OP_U (instr) ? ARM_ID_I : ARM_ID_D;
            armIncDec PSpec = OP_PI(instr) ? ARM_ID_B : ARM_ID_A;

            result = (ARM_ID_P | USpec | PSpec);

            if (incDec==ID_U_P_IMP) {
                // inc/dec spec is always implicit (VPUSH/VPOP)
                result |= ARM_ID_NS;
            } else if(arm->UAL && (result==ARM_ID_IA) && (incDec==ID_U_P_IAI)) {
                // inc/dec spec is implicit in UAL mode when it is IA
                result |= ARM_ID_NS;
            }
            break;
        }
        default: VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return indication of whether Thumb instruction enables or disables interrupts
// (CPS)
//
static armFlagAction getFlagActionThumb(
    armInstructionInfoP info,
    imodSpec            imod,
    Uns32               instr
) {
    armFlagAction result = ARM_FACT_NA;

    switch(imod) {
        case IS_NA: break;
        case IS_4:  result = OP_IS_4(instr) ? ARM_FACT_ID : ARM_FACT_IE; break;
        default:    VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return interrupt flag list in Thumb instruction
//
static armFlagAffect getFlagAffectThumb(
    armInstructionInfoP info,
    aifSpec             aif,
    Uns32               instr
) {
    armFlagAffect result = ARM_FAFF_NA;

    if((info->fact==ARM_FACT_IE) || (info->fact==ARM_FACT_ID)) {
        switch(aif) {
            case AIF_0: result = OP_AIF_0(instr); break;
            default:    VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
        }
    }

    return result;
}

//
// Get the value of the U bit in the instruction
//
static Bool getUThumb(Uns32 instr, uSpec u) {

    Bool result = False;

    switch(u) {
        case US_1:
            result = True;
            break;
        case US_9:
            result = OP_U_9(instr);
            break;
        case US_23:
            result = OP_U_23(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return indication of PSR bits specified by the instruction, if any
//
static armPSRBits getPSRBitsThumb(
    armInstructionInfoP info,
    maskSpec            mask,
    Uns32               instr
) {
    armPSRBits result = ARM_PSRBITS_NA;

    switch(mask) {
        case MSRMASK_NA: break;
        case MSRMASK_10: result = OP_U_2_10(instr); break;
        default:         VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return a modified immediate constant for the given immediate value
//  If single is true then form a single precision value,
//  otherwise form the high order 32 bits of a double precision value (low order 32 bits are always all 0's)
//
static armSdfpMItype modifiedImmediateVFPthumb(Uns32 imm, Bool single) {

    armSdfpMItype result;

    if (single) {

        // Set the sign bit to be a
        result.u32.w0 = (imm & 0x80) ? 0x80000000 : 0;

        // Set the exponent sign to be Bbbbbb  - where B = Not b
        result.u32.w0 |= (imm & 0x40) ? 0x3e000000 : 0x40000000;

        // set the mantissa to cdefgh
        result.u32.w0 |= (imm & 0x3f) << 19;

        // CLear the high order bits just to be safe
        result.u32.w1 = 0;

    } else {
        // Set the sign bit to be a
        result.u64 = (imm & 0x80) ? 0x8000000000000000ULL : 0ULL;

        // Set the exponent sign to be Bbbbbbbbb  - where B = Not b
        result.u64 |= (imm & 0x40) ? 0x3fc0000000000000ULL : 0x4000000000000000ULL;

        // set the mantissa to cdefgh
        result.u64 |= ((Uns64) (imm & 0x3f)) << 48;

    }

    return result;
}

//
// Decode and return a SIMD/VFP modified immediate constant encoded within the ARM instruction
//
static armSdfpMItype getSdfpMIthumb(
    Uns32       instr,
    sdfpMISpec  s,
    armSDFPType dt
) {

    armSdfpMItype result;

    result.u64 = 0;

    switch (s) {

        case SDFP_MI_NA:
            break;
        case SDFP_MI_VFP_S:
            VMI_ASSERT(dt == ARM_SDFPT_F32, "VFP Modified immediate constant type does not match dt");
            result = modifiedImmediateVFPthumb(OP_U_8_16_0(instr), True);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return the VFP scalar index encoded in the Thumb instruction
//
static Int8 getIndexThumb(Uns32 instr, indexSpec index) {

    Int8 result = 0;

    switch(index) {
        case IDX_NA:
            break;
        case IDX_21:
            result = OP_U_1_21(instr);
            break;
        case IDX_5:
            result = OP_U_1_5(instr);
            break;
        case IDX_7:
            result = OP_U_1_7(instr);
            break;
        case IDX_19:
            result = OP_U_1_19(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Bitmasks for identification of 32-bit Thumb instructions
//
#define THUMB32_MASK   0xf800
#define THUMB32_BL_H01 0xe800
#define THUMB32_BL_H10 0xf000
#define THUMB32_BL_H11 0xf800

//
// Return size in bytes of Thumb instruction at the passed address
//
Uns32 armGetThumbInstructionSize(armP arm, Uns32 thisPC) {

    Uns16 instr1   = vmicxtFetch2Byte((vmiProcessorP)arm, thisPC);
    Uns16 hw1      = instr1 & THUMB32_MASK;
    Bool  isBL_H01 = (hw1==THUMB32_BL_H01);
    Bool  isBL_H10 = (hw1==THUMB32_BL_H10);
    Bool  isBL_H11 = (hw1==THUMB32_BL_H11);

    return (isBL_H01 || isBL_H10 || isBL_H11) ? 4 : 2;
}

//
// Decode the Thumb instruction at the passed address. The 'info' structure is
// filled with details of the instruction.
//
void armDecodeThumb(armP arm, Uns32 thisPC, armInstructionInfoP info) {

    vmiProcessorP processor = (vmiProcessorP)arm;
    Uns32         bytes     = armGetThumbInstructionSize(arm, thisPC);
    Uns32         instr     = vmicxtFetch2Byte(processor, thisPC);
    armThumbType  type;

    // fetch instruction and decode it based on size
    if(bytes==2) {
        type = vmidDecode(getDecodeTableThumb16(), instr);
    } else {
        instr = (instr<<16) | vmicxtFetch2Byte(processor, thisPC+2);
        type  = vmidDecode(getDecodeTableThumb32(), instr);
    }

    // save instruction
    info->instruction = instr;

    // get instruction attributes based on type
    opAttrsCP attrs = &attrsArray[type];

    // get the equivalent main series type
    info->type = attrs->type;

    // specify the name and format for the opcode
    info->opcode = attrs->opcode;
    info->format = attrs->format;

    // specify required architecture for the instruction
    info->support = attrs->support;
    info->isar    = attrs->isar;

    // save instruction bytes
    info->bytes = bytes;

    // get any flags set in the instruction
    info->f = getSetFlagsThumb(arm, instr, attrs->f);

    // indicate whether the instruction is conditional
    info->cond = getConditionThumb(arm, instr, attrs->cond);

    // get registers used by this instruction
    info->r1 = getRegisterThumb(instr, attrs->r1);
    info->r2 = getRegisterThumb(instr, attrs->r2);
    info->r3 = getRegisterThumb(instr, attrs->r3);
    info->r4 = getRegisterThumb(instr, attrs->r4);

    // get shiftop for the instruction
    info->so = getShiftOpThumb(instr, attrs->ss);

    // get load/store size, sign extension, translate and exclusive access
    // settings
    info->sz = attrs->sz;
    info->w  = getWidthThumb(instr, attrs->w);
    info->xs = attrs->xs;
    info->tl = attrs->tl;
    info->ea = attrs->ea;

    // does the opcode have a long load field?
    info->ll = attrs->ll ? OP_LL(instr) : 0;

    // get U bit (must be valid *before* getConstantThumb which uses it)
    info->u = getUThumb(instr, attrs->u);

    // get any constant value and constant rotate associated with the instruction
    info->c = getConstantThumb(
        arm, info, instr, attrs->cs, info->so, &info->crotate
    );

    // get any constant target address associated with the instruction
    info->t = getTargetThumb(info, instr, attrs->ts);

    // get any coprocessor fields associated with the instruction
    info->cpNum = attrs->cpNum ? OP_CPNUM(instr) : 0;
    info->cpOp1 = getCpOp1Thumb(arm, info, instr, attrs->cpOp1);
    info->cpOp2 = attrs->cpOp2 ? OP_CPOP2(instr) : 0;

    // get any _bits specification for MSR instructions
    info->psrbits = getPSRBitsThumb(info, attrs->mask, instr);

    // get any register list and increment/decrement specification
    info->rList  = getRListThumb(instr, attrs->rList);
    info->incDec = getIncDecThumb(info, arm, instr, attrs->incDec);

    // get post-indexed and writeback attributes for the instruction
    info->pi = getPostIndexedThumb(instr, attrs->pi);
    info->wb = info->pi || getWritebackThumb(instr, attrs->wb, info->r1, info->rList);

    // specify action on unaligned access
    info->ua = attrs->ua67;

    // get flag and mode effect fields (CPS instruction)
    info->ma   = attrs->m ? OP_MA(instr) : 0;
    info->fact = getFlagActionThumb(info, attrs->imod, instr);
    info->faff = getFlagAffectThumb(info, attrs->aif, instr);

    // get if-then specification
    info->it = attrs->it ? OP_IT(instr) : 0;

    // Get VFP specifications
    info->index = getIndexThumb(instr, attrs->index);
    info->nregs = getNRegsThumb(instr, attrs->nregs);

    // get any SIMD modified immediate constant value (64 bits long)
    info->sdfpMI = getSdfpMIthumb (instr, attrs->sdfpMI, attrs->dt1);

    // get floating point type specification
    info->dt1 = attrs->dt1;
    info->dt2 = attrs->dt2;
}
