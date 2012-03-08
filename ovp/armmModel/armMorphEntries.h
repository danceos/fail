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

#ifndef ARM_MORPH_ENTRIES_H
#define ARM_MORPH_ENTRIES_H

// model header files
#include "armVariant.h"


////////////////////////////////////////////////////////////////////////////////
// NORMAL INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morpher attributes for an instruction with a single variant
//
#define MORPH_SINGLE(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME}

//
// Morpher attributes for ARM instructions like ADC
//
#define MORPH_SET_ADC(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitBinopI,   interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitBinopR,   interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitBinopRSI, interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitBinopRX,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_IT]          = {morphCB:armEmitBinopIT,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RT]          = {morphCB:armEmitBinopRT,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like MOV
//
#define MORPH_SET_MOV(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitUnopI,    interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitUnopR,    interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitUnopRSI,  interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {morphCB:armEmitUnopRSR,  interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitUnopRX,   interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RST] = {morphCB:armEmitUnopRSRT, interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like MOVT
//
#define MORPH_SET_MOVT(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmitUnopIT}

//
// Morpher attributes for ARM instructions like MLA
//
#define MORPH_SET_MLA(_NAME, _FLAGS_RW) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, flagsRW:_FLAGS_RW}

//
// Morpher attributes for ARM instructions like SMULL
//
#define MORPH_SET_SMULL(_NAME, _OP1, _OP2, _FLAGS_RW) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_OP1, binop:_OP2, flagsRW:_FLAGS_RW}

//
// Morpher attributes for ARM instructions like CMN
//
#define MORPH_SET_CMN(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitCmpopI,   binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitCmpopR,   binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitCmpopRSI, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitCmpopRX,  binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like B
//
#define MORPH_SET_B(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBranchC, condJump:True, isLink:_IS_LINK}

//
// Morpher attributes for ARM instructions like BLX (2)
//
#define MORPH_SET_BLX2(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBranchR, isLink:_IS_LINK}

//
// Morpher attributes for ARM instructions like LDR
//
#define MORPH_SET_LDR(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmit##_OP##I,   interwork:ARM_IW_L4}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmit##_OP##R,   interwork:ARM_IW_L4}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmit##_OP##RSI, interwork:ARM_IW_L4}

//
// Morpher attributes for ARM instructions like LDM (1)
//
#define MORPH_SET_LDM1(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, interwork:ARM_IW_L4}

//
// Morpher attributes for ARM instructions like LDRH
//
#define MORPH_SET_LDRH(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM] = {morphCB:armEmit##_OP##I}, \
    [ARM_IT_##_NAME##_RM]  = {morphCB:armEmit##_OP##R}

//
// Morpher attributes for ARM instructions like SWP
//
#define MORPH_SET_SWP(_NAME, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_OP}

//
// Morpher attributes for ARM instructions like LDC
//
#define MORPH_SET_LDC(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM]       = {morphCB:armEmit##_OP}, \
    [ARM_IT_##_NAME##_UNINDEXED] = {morphCB:armEmit##_OP}

//
// Morpher attributes for ARM instructions like NOP
//
#define MORPH_SET_NOP(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmitNOP}

//
// Morpher attributes for ARM instructions like SMLA<x><y>
//
#define MORPH_SET_SMLA_XY(_NAME) \
    [ARM_IT_##_NAME##BB] = {morphCB:armEmit##_NAME##BB}, \
    [ARM_IT_##_NAME##BT] = {morphCB:armEmit##_NAME##BT}, \
    [ARM_IT_##_NAME##TB] = {morphCB:armEmit##_NAME##TB}, \
    [ARM_IT_##_NAME##TT] = {morphCB:armEmit##_NAME##TT}

//
// Morpher attributes for ARM instructions like SMLAW<y>
//
#define MORPH_SET_SMLAW_Y(_NAME) \
    [ARM_IT_##_NAME##B] = {morphCB:armEmit##_NAME##B}, \
    [ARM_IT_##_NAME##T] = {morphCB:armEmit##_NAME##T}

//
// Morpher attributes for parallel add/subtract instructions
//
#define MORPH_SET_PAS(_NAME, _OP1, _OP2, _SEXTEND, _SETGE, _HALVE) \
    [ARM_IT_##_NAME##ADD16] = {morphCB:armEmitParallelBinop16, binop:_OP1, binop2:_OP1, exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##ASX]   = {morphCB:armEmitParallelBinop16, binop:_OP2, binop2:_OP1, exchange:1, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SAX]   = {morphCB:armEmitParallelBinop16, binop:_OP1, binop2:_OP2, exchange:1, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SUB16] = {morphCB:armEmitParallelBinop16, binop:_OP2, binop2:_OP2, exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##ADD8]  = {morphCB:armEmitParallelBinop8,  binop:_OP1,              exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SUB8]  = {morphCB:armEmitParallelBinop8,  binop:_OP2,              exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}

//
// Morpher attributes for signed multiply instructions with optional argument exchange
//
#define MORPH_SET_MEDIA_X(_NAME, _OP1, _OP2) \
    [ARM_IT_##_NAME]    = {morphCB:armEmit##_OP1, binop:_OP2, exchange:0}, \
    [ARM_IT_##_NAME##X] = {morphCB:armEmit##_OP1, binop:_OP2, exchange:1}

//
// Morpher attributes for signed multiply instructions with optional rounding
//
#define MORPH_SET_MEDIA_R(_NAME, _OP1, _OP2, _ACC) \
    [ARM_IT_##_NAME]    = {morphCB:armEmit##_OP1, binop:_OP2, round:0, accumulate:_ACC}, \
    [ARM_IT_##_NAME##R] = {morphCB:armEmit##_OP1, binop:_OP2, round:1, accumulate:_ACC}

//
// Morpher attributes for ARM instructions like PLD
//
#define MORPH_SET_PLD(_NAME) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitNOP}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitNOP}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitNOP}


////////////////////////////////////////////////////////////////////////////////
// THUMB INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morpher attributes for Thumb instructions like ADD (4)
//
#define MORPH_SET_ADD4(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopRT, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADD (6)
//
#define MORPH_SET_ADD6(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopI, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADD (7)
//
#define MORPH_SET_ADD7(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopIT, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like MOV (3)
//
#define MORPH_SET_MOV3(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitUnopR, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADR
//
#define MORPH_SET_ADR(_NAME, _NEGATE) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopADR, binop:vmi_ADD, negate:_NEGATE}

//
// Morpher attributes for Thumb instructions like CBZ
//
#define MORPH_SET_CBZ(_NAME, _JUMP_IF_TRUE) \
    [ARM_IT_##_NAME] = {morphCB:armEmitCBZ, jumpIfTrue:_JUMP_IF_TRUE}

//
// Morpher attributes for Thumb instructions like SDIV
//
#define MORPH_SET_SDIV(_NAME, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmitDIV, binop:_OP}

//
// Morpher attributes for Single entry VFP instructions
//
#define MORPH_SET_SINGLE_VFP(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, iType:ARM_TY_VFP}

//
// Morpher attributes for VFP instructions with D and S versions
//
#define MORPH_SET_VFP_DS(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_S] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:4}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:8}

//
// Morpher attributes for VFP instructions with S version only
//
#define MORPH_SET_VFP_S(_NAME, _FUNC) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:4}

//
// Morpher attributes for VFP instructions doing single precision binary floating point op
//
#define MORPH_SET_VFP_RRR_F(_NAME, _FUNC, _OP, _NEGATE) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, fbinop:_OP, ebytes:4, negate:_NEGATE}

//
// Morpher attributes for VFP instructions doing single precision unary floating point op
//
#define MORPH_SET_VFP_RR_F(_NAME, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmitVFPUnop, iType:ARM_TY_VFP, funop:_OP, ebytes:4}

//
// Morpher attributes for VFP VCMP instructions
//
#define MORPH_SET_VFP_VCMP(_NAME, _FUNC, _QNAN) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:4, allowQNaN:_QNAN}

//
// Morpher attributes for VFP VCVT instructions
//
#define MORPH_SET_VFP_VCVT(_NAME, _FUNC, _BYTES, _SIGN, _RND) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:_BYTES, sextend:_SIGN, roundFPSCR:_RND}

//
// Morpher attributes for VFP VCVT half precision instructions which may specify top or bottom half
//
#define MORPH_SET_VFP_VCVT_H(_NAME, _FUNC, _BYTES, _TOP) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:_BYTES, highhalf:_TOP}

//
// Morpher attributes for single precision VFP Fused Multiply instructions
//
#define MORPH_SET_VFP_FMAC_F(_NAME, _SUB, _NEGATE) \
    [ARM_IT_##_NAME] = {morphCB:armEmitVFusedMAC, iType:ARM_TY_VFP, subtract:_SUB, negate:_NEGATE}

#endif
