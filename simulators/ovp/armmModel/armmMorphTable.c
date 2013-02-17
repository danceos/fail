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
#include "armEmit.h"
#include "armMorph.h"
#include "armMorphEntries.h"
#include "armMorphFunctions.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armUtils.h"


////////////////////////////////////////////////////////////////////////////////
// FLAGS
////////////////////////////////////////////////////////////////////////////////

const static vmiFlags flagsZNCV = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = ARM_VF_CONST
    }
};

const static vmiFlags flagsZNBV = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = ARM_VF_CONST
    },
    vmi_FN_CF_IN|vmi_FN_CF_OUT
};

const static vmiFlags flagsZN = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

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

const static vmiFlags flagsBIn = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    },
    vmi_FN_CF_IN
};

// Macro accessors for flags
#define FLAGS_ZNCV &flagsZNCV
#define FLAGS_ZNBV &flagsZNBV
#define FLAGS_ZN   &flagsZN
#define FLAGS_CIN  &flagsCIn
#define FLAGS_BIN  &flagsBIn


////////////////////////////////////////////////////////////////////////////////
// MORPHER DISPATCH TABLE
////////////////////////////////////////////////////////////////////////////////

const armMorphAttr armMorphTable[ARM_IT_LAST+1] = {

    ////////////////////////////////////////////////////////////////////////////
    // ARM INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SET_ADC (ADC,  vmi_ADC,  FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_ADC (ADD,  vmi_ADD,  FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_ADC (AND,  vmi_AND,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (BIC,  vmi_ANDN, FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (EOR,  vmi_XOR,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (MOV,  vmi_MOV,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (MUL,  vmi_MUL,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (MVN,  vmi_NOT,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (NEG,  vmi_NEG,  FLAGS_ZNBV, 0,         False),
    MORPH_SET_ADC (ORN,  vmi_ORN,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (ORR,  vmi_OR,   FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (RSB,  vmi_RSUB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (RSC,  vmi_RSBB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (SBC,  vmi_SBB,  FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (SUB,  vmi_SUB,  FLAGS_ZNBV, FLAGS_BIN, False),

    // ARMv6T2 move instructions
    MORPH_SINGLE (MOVT),
    MORPH_SINGLE (MOVW),

    // multiply instructions
    MORPH_SET_MLA   (MLA,                   FLAGS_ZN),
    MORPH_SET_MLA   (MLS,                   FLAGS_ZN),
    MORPH_SET_MLA   (MUL,                   FLAGS_ZN),
    MORPH_SET_SMULL (SMLAL, MLAL, vmi_IMUL, FLAGS_ZN),
    MORPH_SET_SMULL (SMULL, MULL, vmi_IMUL, FLAGS_ZN),
    MORPH_SET_SMULL (UMAAL, MAAL, vmi_MUL,  FLAGS_ZN),
    MORPH_SET_SMULL (UMLAL, MLAL, vmi_MUL,  FLAGS_ZN),
    MORPH_SET_SMULL (UMULL, MULL, vmi_MUL,  FLAGS_ZN),

    // compare instructions
    MORPH_SET_CMN (CMN, vmi_ADD, FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_CMN (CMP, vmi_SUB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_CMN (TEQ, vmi_XOR, FLAGS_ZN,   0,         True ),
    MORPH_SET_CMN (TST, vmi_AND, FLAGS_ZN,   0,         True ),

    // branch instructions
    MORPH_SET_B    (B,    False),
    MORPH_SET_B    (BL,   True ),
    MORPH_SET_BLX2 (BLX2, True ),
    MORPH_SET_BLX2 (BX,   False),

    // miscellaneous instructions
    MORPH_SINGLE (BKPT),
    MORPH_SINGLE (CLZ ),
    MORPH_SINGLE (SWI ),

    // load and store instructions
    MORPH_SET_LDR  (LDR,   LDR ),
    MORPH_SET_LDR  (LDRB,  LDR ),
    MORPH_SET_LDR  (LDRBT, LDR ),
    MORPH_SET_LDRH (LDRH,  LDR ),
    MORPH_SET_LDRH (LDRSB, LDR ),
    MORPH_SET_LDRH (LDRSH, LDR ),
    MORPH_SET_LDR  (LDRT,  LDR ),
    MORPH_SET_LDR  (STR,   STR ),
    MORPH_SET_LDR  (STRB,  STR ),
    MORPH_SET_LDR  (STRBT, STR ),
    MORPH_SET_LDRH (STRH,  STR ),
    MORPH_SET_LDR  (STRT,  STR ),

    // load and store multiple instructions
    MORPH_SET_LDM1 (LDM1),
    MORPH_SINGLE   (STM1),

    // ARMv6T2 load and store instructions
    MORPH_SET_LDRH (LDRHT,  LDR),
    MORPH_SET_LDRH (LDRSBT, LDR),
    MORPH_SET_LDRH (LDRSHT, LDR),
    MORPH_SET_LDRH (STRHT,  STR),

    // synchronization primitives
    MORPH_SET_SWP (LDREX,  LDREX),
    MORPH_SET_SWP (LDREXB, LDREX),
    MORPH_SET_SWP (LDREXH, LDREX),
    MORPH_SET_SWP (STREX,  STREX),
    MORPH_SET_SWP (STREXB, STREX),
    MORPH_SET_SWP (STREXH, STREX),

    // coprocessor instructions (all UsageFault)
    MORPH_SET_SWP (CDP,  UsageFaultCP),
    MORPH_SET_SWP (CDP2, UsageFaultCP),
    MORPH_SET_LDC (LDC,  UsageFaultCP),
    MORPH_SET_LDC (LDC2, UsageFaultCP),
    MORPH_SET_SWP (MCR,  UsageFaultCP),
    MORPH_SET_SWP (MCR2, UsageFaultCP),
    MORPH_SET_SWP (MRC,  UsageFaultCP),
    MORPH_SET_SWP (MRC2, UsageFaultCP),
    MORPH_SET_LDC (STC,  UsageFaultCP),
    MORPH_SET_LDC (STC2, UsageFaultCP),

    // status register access instructions
    MORPH_SINGLE (MRS),
    MORPH_SINGLE (MSR),

    // hint instructions
    MORPH_SET_NOP (NOP  ),
    MORPH_SET_NOP (YIELD),
    MORPH_SINGLE  (WFE  ),
    MORPH_SINGLE  (WFI  ),
    MORPH_SINGLE  (SEV  ),
    MORPH_SET_NOP (DBG  ),

    // ARMv6 miscellaneous instructions
    MORPH_SINGLE  (CPS   ),
    MORPH_SINGLE  (CLREX ),
    MORPH_SET_NOP (DSB   ),
    MORPH_SET_NOP (ISB   ),

    // ARMv7 miscellaneous instructions
    MORPH_SET_PLD  (PLD),
    MORPH_SET_PLD (PLI),
    MORPH_SET_NOP (DMB),

    ////////////////////////////////////////////////////////////////////////////
    // DSP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SINGLE (QADD ),
    MORPH_SINGLE (QDADD),
    MORPH_SINGLE (QDSUB),
    MORPH_SINGLE (QSUB ),

    // multiply instructions
    MORPH_SET_SMLA_XY (SMLA ),
    MORPH_SET_SMLA_XY (SMLAL),
    MORPH_SET_SMLAW_Y (SMLAW),
    MORPH_SET_SMLA_XY (SMUL ),
    MORPH_SET_SMLAW_Y (SMULW),

    // load and store instructions
    MORPH_SET_LDRH (LDRD, LDR),
    MORPH_SET_LDRH (STRD, STR),

    // coprocessor instructions (all UsageFault)
    MORPH_SET_SWP (MCRR,  UsageFaultCP),
    MORPH_SET_SWP (MCRR2, UsageFaultCP),
    MORPH_SET_SWP (MRRC,  UsageFaultCP),
    MORPH_SET_SWP (MRRC2, UsageFaultCP),

    ////////////////////////////////////////////////////////////////////////////
    // MEDIA INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // basic instructions
    MORPH_SINGLE (USAD8 ),
    MORPH_SINGLE (USADA8),
    MORPH_SINGLE (SBFX  ),
    MORPH_SINGLE (BFC   ),
    MORPH_SINGLE (BFI   ),
    MORPH_SINGLE (UBFX  ),

    // parallel add/subtract instructions
    MORPH_SET_PAS (S,  vmi_ADD,   vmi_SUB,   True,  True,  False),
    MORPH_SET_PAS (Q,  vmi_ADDSQ, vmi_SUBSQ, False, False, False),
    MORPH_SET_PAS (SH, vmi_ADD,   vmi_SUB,   True,  False, True ),
    MORPH_SET_PAS (U,  vmi_ADD,   vmi_SUB,   False, True,  False),
    MORPH_SET_PAS (UQ, vmi_ADDUQ, vmi_SUBUQ, False, False, False),
    MORPH_SET_PAS (UH, vmi_ADD,   vmi_SUB,   False, False, True ),

    // packing, unpacking, saturation and reversal instructions
    MORPH_SINGLE (PKHBT  ),
    MORPH_SINGLE (PKHTB  ),
    MORPH_SINGLE (SSAT   ),
    MORPH_SINGLE (SSAT16 ),
    MORPH_SINGLE (USAT   ),
    MORPH_SINGLE (USAT16 ),
    MORPH_SINGLE (SXTAB  ),
    MORPH_SINGLE (UXTAB  ),
    MORPH_SINGLE (SXTAB16),
    MORPH_SINGLE (UXTAB16),
    MORPH_SINGLE (SXTAH  ),
    MORPH_SINGLE (UXTAH  ),
    MORPH_SINGLE (SXTB   ),
    MORPH_SINGLE (UXTB   ),
    MORPH_SINGLE (SXTB16 ),
    MORPH_SINGLE (UXTB16 ),
    MORPH_SINGLE (SXTH   ),
    MORPH_SINGLE (UXTH   ),
    MORPH_SINGLE (SEL    ),
    MORPH_SINGLE (REV    ),
    MORPH_SINGLE (REV16  ),
    MORPH_SINGLE (RBIT   ),
    MORPH_SINGLE (REVSH  ),

    // signed multiply instructions
    MORPH_SET_MEDIA_X (SMLAD,  SMLXD,  vmi_ADD),
    MORPH_SET_MEDIA_X (SMUAD,  SMUXD,  vmi_ADD),
    MORPH_SET_MEDIA_X (SMLSD,  SMLXD,  vmi_SUB),
    MORPH_SET_MEDIA_X (SMUSD,  SMUXD,  vmi_SUB),
    MORPH_SET_MEDIA_X (SMLALD, SMLXLD, vmi_ADD),
    MORPH_SET_MEDIA_X (SMLSLD, SMLXLD, vmi_SUB),
    MORPH_SET_MEDIA_R (SMMLA,  SMMLX,  vmi_ADD,  True ),
    MORPH_SET_MEDIA_R (SMMUL,  SMMLX,  vmi_ADD,  False),
    MORPH_SET_MEDIA_R (SMMLS,  SMMLX,  vmi_RSUB, True ),

    // VFP data processing instructions
    MORPH_SET_VFP_RRR_F   (VMLA_VFP,   VMulAcc_VFP, vmi_FADD, 0),
    MORPH_SET_VFP_RRR_F   (VMLS_VFP,   VMulAcc_VFP, vmi_FSUB, 0),
    MORPH_SET_VFP_RRR_F   (VNMLA_VFP,  VMulAcc_VFP, vmi_FSUB, 1),
    MORPH_SET_VFP_RRR_F   (VNMLS_VFP,  VMulAcc_VFP, vmi_FADD, 1),
    MORPH_SET_VFP_RRR_F   (VNMUL_VFP,  VFPBinop,    vmi_FMUL, 1),
    MORPH_SET_VFP_RRR_F   (VMUL_VFP,   VFPBinop,    vmi_FMUL, 0),
    MORPH_SET_VFP_RRR_F   (VADD_VFP,   VFPBinop,    vmi_FADD, 0),
    MORPH_SET_VFP_RRR_F   (VSUB_VFP,   VFPBinop,    vmi_FSUB, 0),
    MORPH_SET_VFP_RRR_F   (VDIV_VFP,   VFPBinop,    vmi_FDIV, 0),
    MORPH_SET_VFP_FMAC_F  (VFNMA_VFP,  1, 1),
    MORPH_SET_VFP_FMAC_F  (VFNMS_VFP,  0, 1),
    MORPH_SET_VFP_FMAC_F  (VFMA_VFP,   0, 0),
    MORPH_SET_VFP_FMAC_F  (VFMS_VFP,   1, 0),
    MORPH_SET_VFP_S       (VMOVI_VFP,  VMOVI_VFP),
    MORPH_SET_VFP_S       (VMOVR_VFP,  VMOVR_VFP),
    MORPH_SET_VFP_S       (VABS_VFP,   VABS_VFP),
    MORPH_SET_VFP_S       (VNEG_VFP,   VNEG_VFP),
    MORPH_SET_VFP_RR_F    (VSQRT_VFP,               vmi_FSQRT),
    MORPH_SET_VFP_VCMP    (VCMP_VFP,   VCMP_VFP,    1),
    MORPH_SET_VFP_VCMP    (VCMPE_VFP,  VCMP_VFP,    0),
    MORPH_SET_VFP_VCMP    (VCMP0_VFP,  VCMP0_VFP,   1),
    MORPH_SET_VFP_VCMP    (VCMPE0_VFP, VCMP0_VFP,   0),
    //    VCVT between half precision and single precision
    MORPH_SET_VFP_VCVT_H  (VCVTTFH_VFP,    VCVT_SH_VFP, 2, 1),
    MORPH_SET_VFP_VCVT_H  (VCVTBFH_VFP,    VCVT_SH_VFP, 2, 0),
    MORPH_SET_VFP_VCVT_H  (VCVTTHF_VFP,    VCVT_HS_VFP, 2, 1),
    MORPH_SET_VFP_VCVT_H  (VCVTBHF_VFP,    VCVT_HS_VFP, 2, 0),
    //    VCVT from single precision to Integer (signed or unsigned) with round option
    MORPH_SET_VFP_VCVT    (VCVTRUF_VFP,    VCVT_IF_VFP, 4, 0, 1),
    MORPH_SET_VFP_VCVT    (VCVTUF_VFP,     VCVT_IF_VFP, 4, 0, 0),
    MORPH_SET_VFP_VCVT    (VCVTRSF_VFP,    VCVT_IF_VFP, 4, 1, 1),
    MORPH_SET_VFP_VCVT    (VCVTSF_VFP,     VCVT_IF_VFP, 4, 1, 0),
    //    VCVT from single precision to Fixed (signed or unsigned, word or half)
    MORPH_SET_VFP_VCVT    (VCVTXFUW_VFP,   VCVT_XF_VFP, 4, 0, 0),
    MORPH_SET_VFP_VCVT    (VCVTXFSW_VFP,   VCVT_XF_VFP, 4, 1, 0),
    MORPH_SET_VFP_VCVT    (VCVTXFUH_VFP,   VCVT_XF_VFP, 2, 0, 0),
    MORPH_SET_VFP_VCVT    (VCVTXFSH_VFP,   VCVT_XF_VFP, 2, 1, 0),
    //    VCVT from Fixed (signed or unsigned, word or half) to single precision
    MORPH_SET_VFP_VCVT    (VCVTFXUW_VFP,   VCVT_FX_VFP, 4, 0, 0),
    MORPH_SET_VFP_VCVT    (VCVTFXSW_VFP,   VCVT_FX_VFP, 4, 1, 0),
    MORPH_SET_VFP_VCVT    (VCVTFXUH_VFP,   VCVT_FX_VFP, 2, 0, 0),
    MORPH_SET_VFP_VCVT    (VCVTFXSH_VFP,   VCVT_FX_VFP, 2, 1, 0),
    //    VCVT From Integer (signed or unsigned) to single precision
    MORPH_SET_VFP_VCVT    (VCVTFU_VFP,     VCVT_FI_VFP, 4, 0, 1),
    MORPH_SET_VFP_VCVT    (VCVTFS_VFP,     VCVT_FI_VFP, 4, 1, 1),

    // 8, 16 and 32-bit transfer instructions
    MORPH_SET_SINGLE_VFP (VMRS),
    MORPH_SET_SINGLE_VFP (VMSR),
    MORPH_SET_SINGLE_VFP (VMOVRS),
    MORPH_SET_SINGLE_VFP (VMOVSR),
    MORPH_SET_SINGLE_VFP (VMOVRZ),
    MORPH_SET_SINGLE_VFP (VMOVZR),

    // Extension register load/store instructions
    MORPH_SET_VFP_DS    (VLDR,    VLDR),
    MORPH_SET_VFP_DS    (VSTR,    VSTR),
    MORPH_SET_VFP_DS    (VLDR,    VLDR),
    MORPH_SET_VFP_DS    (VSTR,    VSTR),
    MORPH_SET_VFP_DS    (VLDMIA,  VLDM),
    MORPH_SET_VFP_DS    (VLDMIAW, VLDM),
    MORPH_SET_VFP_DS    (VLDMDBW, VLDM),
    MORPH_SET_VFP_DS    (VPOP,    VLDM),
    MORPH_SET_VFP_DS    (VSTMIA,  VSTM),
    MORPH_SET_VFP_DS    (VSTMIAW, VSTM),
    MORPH_SET_VFP_DS    (VSTMDBW, VSTM),
    MORPH_SET_VFP_DS    (VPUSH,   VSTM),

    // 64-bit transfer instructions
    MORPH_SET_SINGLE_VFP (VMOVRRD),
    MORPH_SET_SINGLE_VFP (VMOVDRR),
    MORPH_SET_SINGLE_VFP (VMOVRRSS),
    MORPH_SET_SINGLE_VFP (VMOVSSRR),

    ////////////////////////////////////////////////////////////////////////////
    // THUMB INSTRUCTIONS (WHEN DISTINCT FROM ARM INSTRUCTIONS)
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SET_ADD4 (ADD4, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD6 (ADD6, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD7 (ADD7, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD7 (SUB4, vmi_SUB, 0, 0, False),
    MORPH_SET_MOV3 (MOV3, vmi_MOV, 0, 0, False),

    // address instructions
    MORPH_SET_ADR (ADD_ADR, False),
    MORPH_SET_ADR (SUB_ADR, True ),

    // branch instructions
    MORPH_SET_CBZ (CBNZ, False),
    MORPH_SET_CBZ (CBZ,  True ),
    MORPH_SINGLE  (TB),

    // divide instructions
    MORPH_SET_SDIV (SDIV, vmi_IDIV),
    MORPH_SET_SDIV (UDIV, vmi_DIV ),
};
