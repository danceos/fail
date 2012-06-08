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

#ifndef ARM_ATTRIBUTE_ENTRIES_THUMB32_H
#define ARM_ATTRIBUTE_ENTRIES_THUMB32_H

#include "armDisassembleFormats.h"

//
// Attribute entries for 32-bit Thumb instructions like AND
//
#define ATTR_SET_32_AND(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_R2_UIMM,          type:ARM_IT_##_NAME##_IMM,         support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_16, r3:0,    cs:CS_MI,  ss:0     }, \
    [TT32_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, type:ARM_IT_##_NAME##_RM_SHFT_IMM, support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_16, r3:R4_0, cs:CS_PI5, ss:SS2_4 }, \
    [TT32_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT,      type:ARM_IT_##_NAME##_RM_RRX,      support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_16, r3:R4_0, cs:CS_NA,  ss:SS_RRX}

//
// Attribute entries for 32-bit Thumb instructions like TST
//
#define ATTR_SET_32_TST(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_UIMM,          type:ARM_IT_##_NAME##_IMM,         support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R4_16, r2:0,    cs:CS_MI,  ss:0     }, \
    [TT32_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, type:ARM_IT_##_NAME##_RM_SHFT_IMM, support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R4_16, r2:R4_0, cs:CS_PI5, ss:SS2_4 }, \
    [TT32_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT,      type:ARM_IT_##_NAME##_RM_RRX,      support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R4_16, r2:R4_0, cs:CS_NA,  ss:SS_RRX}

//
// Attribute entries for 32-bit Thumb instructions like MOV
//
#define ATTR_SET_32_MOV(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_UIMM,          type:ARM_IT_##_NAME##_IMM,         support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:0,    cs:CS_MI,  ss:0     }, \
    [TT32_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, type:ARM_IT_##_NAME##_RM_SHFT_IMM, support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_0, cs:CS_PI5, ss:SS2_4 }, \
    [TT32_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT,      type:ARM_IT_##_NAME##_RM_RRX,      support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_0, cs:CS_NA,  ss:SS_RRX}

//
// Attribute entries for 32-bit Thumb instructions like PKHBT
//
#define ATTR_SET_32_PKHBT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, cs:CS_PI5, ss:SS2_4}

//
// Attribute entries for 32-bit Thumb instructions like ADD (plain binary immediate)
//
#define ATTR_SET_32_ADD_PI(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_UIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, cs:CS_PI12}

//
// Attribute entries for 32-bit Thumb instructions like MOV (plain binary immediate)
//
#define ATTR_SET_32_MOV_PI(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_UIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, cs:CS_PI16}

//
// Attribute entries for 32-bit Thumb instructions like SSAT
//
#define ATTR_SET_32_SSAT(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2_SHIFT_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, cs:CS_PI5, ss:SS2_20, w:WS_WIDTH5M1}

//
// Attribute entries for 32-bit Thumb instructions like SSAT16
//
#define ATTR_SET_32_SSAT16(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, w:WS_WIDTH4M1}

//
// Attribute entries for 32-bit Thumb instructions like USAT
//
#define ATTR_SET_32_USAT(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2_SHIFT_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, cs:CS_PI5, ss:SS2_20, w:WS_WIDTH5}

//
// Attribute entries for 32-bit Thumb instructions like USAT16
//
#define ATTR_SET_32_USAT16(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, w:WS_WIDTH4}

//
// Attribute entries for 32-bit Thumb instructions like SBFX
//
#define ATTR_SET_32_SBFX(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_LSB_WIDTH, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, cs:CS_PI5, w:WS_WIDTH5M1}

//
// Attribute entries for 32-bit Thumb instructions like BFI
//
#define ATTR_SET_32_BFI(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_LSB_WIDTH, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, cs:CS_PI5, w:WS_MSB}

//
// Attribute entries for 32-bit Thumb instructions like BFC
//
#define ATTR_SET_32_BFC(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_LSB_WIDTH, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_8, cs:CS_PI5, w:WS_MSB}

//
// Attribute entries for 32-bit Thumb instructions like LSL
//
#define ATTR_SET_32_LSL(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_20_V, r1:R4_8, r2:R4_16, r3:R4_0, ss:SS2_21}

//
// Attribute entries for 32-bit Thumb instructions like SXTAH
//
#define ATTR_SET_32_SXTAH(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, cs:CS_U_2_4x8, ss:SS_ROR}

//
// Attribute entries for 32-bit Thumb instructions like SXTH
//
#define ATTR_SET_32_SXTH(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_0, cs:CS_U_2_4x8, ss:SS_ROR}

//
// Attribute entries for 32-bit Thumb parallel add/subtract instructions
//
#define ATTR_SET_32_PAS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_S##_NAME]  = {opcode:"s" _OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_S##_NAME,  support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_Q##_NAME]  = {opcode:"q" _OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_Q##_NAME,  support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_SH##_NAME] = {opcode:"sh"_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_SH##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_U##_NAME]  = {opcode:"u" _OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_U##_NAME,  support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_UQ##_NAME] = {opcode:"uq"_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_UQ##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_UH##_NAME] = {opcode:"uh"_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_UH##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like QADD
//
#define ATTR_SET_32_QADD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_0, r3:R4_16}

//
// Attribute entries for 32-bit Thumb instructions like CLZ
//
#define ATTR_SET_32_CLZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like SEL
//
#define ATTR_SET_32_SEL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like MLA
//
#define ATTR_SET_32_MLA(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}

//
// Attribute entries for 32-bit Thumb instructions like SMLAL
//
#define ATTR_SET_32_SMLAL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like MUL
//
#define ATTR_SET_32_MUL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like SMLA<x><y>
//
#define ATTR_SET_32_SMLA_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##BB, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##BT, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##TB, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##TT, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}

//
// Attribute entries for 32-bit Thumb instructions like SMUL<x><y>
//
#define ATTR_SET_32_SMUL_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##BB, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##BT, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##TB, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##TT, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like SMLAL<x><y>
//
#define ATTR_SET_32_SMLAL_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##BB, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}, \
    [TT32_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##BT, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}, \
    [TT32_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##TB, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}, \
    [TT32_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##TT, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like SMLAD<x>
//
#define ATTR_SET_32_SMLAD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME,    support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##X, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}

//
// Attribute entries for 32-bit Thumb instructions like SMUAD<x>
//
#define ATTR_SET_32_SMUAD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3, type:ARM_IT_##_NAME,    support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##X, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for DSP instructions like SMLAW<y>
//
#define ATTR_SET_32_SMLAW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##B] = {opcode:_OPCODE"b", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##B, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##T] = {opcode:_OPCODE"t", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##T, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}

//
// Attribute entries for DSP instructions like SMULW<y>
//
#define ATTR_SET_32_SMULW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##B] = {opcode:_OPCODE"b", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##B, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##T] = {opcode:_OPCODE"t", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##T, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for instructions like SMMLA
//
#define ATTR_SET_32_SMMLA(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME,    support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}, \
    [TT32_##_NAME##R] = {opcode:_OPCODE"r", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##R, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0, r4:R4_12}

//
// Attribute entries for instructions like SMMUL
//
#define ATTR_SET_32_SMMUL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3, type:ARM_IT_##_NAME,    support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}, \
    [TT32_##_NAME##R] = {opcode:_OPCODE"r", format:FMT_R1_R2_R3, type:ARM_IT_##_NAME##R, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_16, r3:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like SMLALD<x>
//
#define ATTR_SET_32_SMLALD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME,    support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}, \
    [TT32_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3_R4, type:ARM_IT_##_NAME##X, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_8, r3:R4_16, r4:R4_0}

//
// Attribute entries for 32-bit Thumb instructions like B (1)
//
#define ATTR_SET_32_B1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, cond:CO_22, ts:TC_S20_T2}

//
// Attribute entries for 32-bit Thumb instructions like BL
//
#define ATTR_SET_32_BL(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, ts:TC_S24_T2}

//
// Attribute entries for 32-bit Thumb instructions like BLX
//
#define ATTR_SET_32_BLX(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, ts:TC_S24_T4}

//
// Attribute entries for 32-bit Thumb instructions like BXJ
//
#define ATTR_SET_32_BXJ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_16}

//
// Attribute entries for 32-bit Thumb instructions like MSR
//
#define ATTR_SET_32_MSR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_SR_R1, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_16, cs:CS_U_8_0, mask:MSRMASK_10}

//
// Attribute entries for 32-bit Thumb instructions like MRS
//
#define ATTR_SET_32_MRS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_SR, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, cs:CS_U_8_0}

//
// Attribute entries for 32-bit Thumb instructions like DSB
//
#define ATTR_SET_32_DSB(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_LIM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_4_0}

//
// Attribute entries for 32-bit Thumb instructions like SUBS_PC_LR
//
#define ATTR_SET_32_SUBS_PC_LR(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_V, r1:R_PC, r2:R_LR, cs:CS_U_8_0}

//
// Attribute entries for 32-bit Thumb instructions like CPS
//
#define ATTR_SET_32_CPS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_FLAGS_OPT_MODE, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_5_0, imod:IS_9, m:1, aif:AIF_5}

//
// Attribute entries for 32-bit Thumb instructions like NOP
//
#define ATTR_SET_32_NOP(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for 32-bit Thumb instructions like DBG
//
#define ATTR_SET_32_DBG(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_UIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_4_0}

//
// Attribute entries for 32-bit Thumb instructions like SRS
//
#define ATTR_SET_32_SRS(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _INCDEC, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WB_UIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R_SP, cs:CS_U_5_0, wb:WB_21, incDec:_INCDEC, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 32-bit Thumb instructions like RFE
//
#define ATTR_SET_32_RFE(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _INCDEC, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WB, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_16, wb:WB_21, incDec:_INCDEC, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 32-bit Thumb instructions like LDM
//
#define ATTR_SET_32_LDM(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _INCDEC, _RLIST, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_RLIST, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_16, wb:WB_21, rList:_RLIST, incDec:_INCDEC, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 32-bit Thumb instructions like POPM
//
#define ATTR_SET_32_POPM(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _INCDEC, _RLIST, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_RLIST, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_16, wb:WB_21, rList:_RLIST, incDec:_INCDEC, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 32-bit Thumb instructions like LDRD_IMM
//
#define ATTR_SET_32_LDRD_IMM(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R4_ADDR_R2_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r4:R4_8, cs:CS_U_8_0x4_U, sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67, u:US_23}

//
// Attribute entries for 32-bit Thumb instructions like LDREX
//
#define ATTR_SET_32_LDREX(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, cs:CS_U_8_0_SZ, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like LDREXB
//
#define ATTR_SET_32_LDREXB(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like LDREXD
//
#define ATTR_SET_32_LDREXD(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R4_ADDR_R2_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r4:R4_8, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like STREX
//
#define ATTR_SET_32_STREX(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_ADDR_R3_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_8, r2:R4_12, r3:R4_16, cs:CS_U_8_0_SZ, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like STREXB
//
#define ATTR_SET_32_STREXB(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_ADDR_R3_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_0, r2:R4_12, r3:R4_16, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like STREXD
//
#define ATTR_SET_32_STREXD(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R4_ADDR_R3_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_0, r2:R4_12, r3:R4_16, r4:R4_8, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for 32-bit Thumb instructions like TBB
//
#define ATTR_SET_32_TBB(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:ADDR_R1_R2_SZSHIFT, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:R4_0, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 32-bit Thumb instructions like LDR
//
#define ATTR_SET_32_LDR(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME##_IMM1]        = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:ARM_ISAR_NA, r1:R4_12, r2:R4_16, r3:0,    cs:CS_U_12_0,   ss:0,      sz:_SZ, xs:_XS, tl:0, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }, \
    [TT32_##_NAME##_IMM2]        = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:ARM_ISAR_NA, r1:R4_12, r2:R4_16, r3:0,    cs:CS_U_8_0_U,  ss:0,      sz:_SZ, xs:_XS, tl:0, pi:PI_10, wb:WB_8, ua45:_UA45, ua67:_UA67, u:US_9 }, \
    [TT32_##_NAME##_IMM3]        = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:ARM_ISAR_NA, r1:R4_12, r2:R4_16, r3:0,    cs:CS_U_12_0_U, ss:0,      sz:_SZ, xs:_XS, tl:0, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:US_23}, \
    [TT32_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3,            type:ARM_IT_##_TYPE##_RM,          support:_SUPPORT, isar:ARM_ISAR_NA, r1:R4_12, r2:R4_16, r3:R4_0, cs:0,           ss:0,      sz:_SZ, xs:_XS, tl:0, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }, \
    [TT32_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3_SHIFT_SIMM, type:ARM_IT_##_TYPE##_RM_SHFT_IMM, support:_SUPPORT, isar:ARM_ISAR_NA, r1:R4_12, r2:R4_16, r3:R4_0, cs:CS_U_2_4,    ss:SS_LSL, sz:_SZ, xs:_XS, tl:0, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }, \
    [TT32_##_NAME##T_IMM]        = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          type:ARM_IT_##_TYPE##T_IMM,        support:_SUPPORT, isar:_ISAR,       r1:R4_12, r2:R4_16, r3:0,    cs:CS_U_8_0_U,  ss:0,      sz:_SZ, xs:_XS, tl:1, pi:PI_10, wb:WB_8, ua45:_UA45, ua67:_UA67, u:US_9 }

//
// Attribute entries for 32-bit Thumb instructions like PLD
//
#define ATTR_SET_32_PLD(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT32_##_NAME##_IMM1]        = {opcode:_OPCODE, format:FMT_ADDR_R1_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:0,    cs:CS_U_12_0,   ss:0,      sz:_SZ, xs:_XS, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }, \
    [TT32_##_NAME##_IMM2]        = {opcode:_OPCODE, format:FMT_ADDR_R1_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:0,    cs:CS_U_8_0_U,  ss:0,      sz:_SZ, xs:_XS, pi:PI_10, wb:WB_8, ua45:_UA45, ua67:_UA67, u:US_9 }, \
    [TT32_##_NAME##_IMM3]        = {opcode:_OPCODE, format:FMT_ADDR_R1_SIMM,          type:ARM_IT_##_TYPE##_IMM,         support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:0,    cs:CS_U_12_0_U, ss:0,      sz:_SZ, xs:_XS, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:US_23}, \
    [TT32_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_ADDR_R1_R2,            type:ARM_IT_##_TYPE##_RM,          support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:R4_0, cs:0,           ss:0,      sz:_SZ, xs:_XS, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }, \
    [TT32_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_ADDR_R1_R2_SHIFT_SIMM, type:ARM_IT_##_TYPE##_RM_SHFT_IMM, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:R4_0, cs:CS_U_2_4,    ss:SS_LSL, sz:_SZ, xs:_XS, pi:0,     wb:0,    ua45:_UA45, ua67:_UA67, u:0    }

//
// Attribute entries for 32-bit Thumb instructions like UHINTH
//
#define ATTR_SET_32_UHINTH(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, type:ARM_IT_NOP, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for 32-bit Thumb undefined instructions
//
#define ATTR_SET_32_UND(_NAME, _SUPPORT, _ISAR) \
    [TT32_##_NAME] = {type:ARM_IT_LAST, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for 32-bit Thumb instructions like CDP
//
#define ATTR_SET_32_CDP(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_CR1_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_4_20, cpOp2:1}

//
// Attribute entries for 32-bit Thumb instructions like CDP2
//
#define ATTR_SET_32_CDP2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_CR1_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_4_20, cpOp2:1}

//
// Attribute entries for 32-bit Thumb instructions like LDC
//
#define ATTR_SET_32_LDC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_IMM]       = {opcode:_OPCODE, format:FMT_CPNUM_CR1_SIMM,      type:ARM_IT_##_NAME##_IMM,       support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, cs:CS_U_8_0x4_U, pi:PI_24, wb:WB_21, ll:1, cpNum:1, u:US_23}, \
    [TT32_##_NAME##_UNINDEXED] = {opcode:_OPCODE, format:FMT_CPNUM_CR1_UNINDEXED, type:ARM_IT_##_NAME##_UNINDEXED, support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, cs:CS_U_8_0,     pi:PI_24, wb:WB_21, ll:1, cpNum:1, u:0    }

//
// Attribute entries for 32-bit Thumb instructions like LDC2
//
#define ATTR_SET_32_LDC2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_IMM]       = {opcode:_OPCODE, format:FMT_CPNUM_CR1_SIMM,      type:ARM_IT_##_NAME##_IMM,       support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, cs:CS_U_8_0x4_U, pi:PI_24, wb:WB_21, ll:1, cpNum:1, u:US_23}, \
    [TT32_##_NAME##_UNINDEXED] = {opcode:_OPCODE, format:FMT_CPNUM_CR1_UNINDEXED, type:ARM_IT_##_NAME##_UNINDEXED, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, cs:CS_U_8_0,     pi:PI_24, wb:WB_21, ll:1, cpNum:1, u:0    }

//
// Attribute entries for 32-bit Thumb instructions like MCR
//
#define ATTR_SET_32_MCR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for 32-bit Thumb instructions like MCR2
//
#define ATTR_SET_32_MCR2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for 32-bit Thumb instructions like MRC
//
#define ATTR_SET_32_MRC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1F_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for 32-bit Thumb instructions like MRC2
//
#define ATTR_SET_32_MRC2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1F_CR2_CR3_COP2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for instructions like MCRR
//
#define ATTR_SET_32_MCRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_R2_CR3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cond:CO_28, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_4_4}

//
// Attribute entries for instructions like MCRR2
//
#define ATTR_SET_32_MCRR2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_R2_CR3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:R4_0, cpNum:1, cpOp1:COP_4_4}

//
// Attribute entries for instructions like VMRS
//
#define ATTR_SET_32_VMRS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_FPSCR, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12}

//
// Attribute entries for instructions like VMSR
//
#define ATTR_SET_32_VMSR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_FPSCR_R1, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12}

//
// Attribute entries for instructions like VMOVRS
//
#define ATTR_SET_32_VMOVRS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_S2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:V_16_7}

//
// Attribute entries for instructions like VMOVSR
//
#define ATTR_SET_32_VMOVSR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_R2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:V_16_7, r2:R4_12}

//
// Attribute entries for instructions VMOVZR
//
#define ATTR_SET_32_VMOVZR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_Z1_R2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:V_7_16, r2:R4_12, index:IDX_21}

//
// Attribute entries for instructions VMOVRZ
//
#define ATTR_SET_32_VMOVRZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME ] = {opcode:_OPCODE, format:FMT_R1_Z2, type:ARM_IT_##_NAME,  support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:V_7_16, index:IDX_21}

//
// Attribute entries for instructions like VMOVRRD
//
#define ATTR_SET_32_VMOVRRD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_D3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:V_5_0}

//
// Attribute entries for instructions like VMOVDRR
//
#define ATTR_SET_32_VMOVDRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_D1_R2_R3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:V_5_0, r2:R4_12, r3:R4_16}

//
// Attribute entries for instructions like VMOVRRSS
//
#define ATTR_SET_32_VMOVRRSS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SS3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_12, r2:R4_16, r3:V_0_5}

//
// Attribute entries for instructions like VMOVSSRR
//
#define ATTR_SET_32_VMOVSSRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_SS1_R2_R3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:V_0_5, r2:R4_12, r3:R4_16}

//
// Attribute entries for SIMD/VFP load/store multiple instructions w/ 1 arm reg and a reg list (initial reg + number of consecutive regs)
//   D/S versions
//
#define ATTR_SET_32_SDFP_LDSTM(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_R1_SIMD_RL, type:ARM_IT_##_NAME##_D, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:V_22_12, nregs:NREG_7_1, incDec:ID_U_P, wb:WB_21, ua67:ARM_UA_DABORT}, \
    [TT32_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_R1_VFP_RL,  type:ARM_IT_##_NAME##_S, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:V_12_22, nregs:NREG_8_0, incDec:ID_U_P, wb:WB_21, ua67:ARM_UA_DABORT}

//
// Attribute entries for SIMD/VFP load/store instructions w/ 1 reg and 1 arm reg and an immediate
//   D/S versions
//
#define ATTR_SET_32_SDFP_LDST(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_D1_ADDR_R2_SIMM, type:ARM_IT_##_NAME##_D, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R4_16, cs:CS_U_8_0x4_U, u:US_23, ua67:ARM_UA_DABORT}, \
    [TT32_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_S1_ADDR_R2_SIMM, type:ARM_IT_##_NAME##_S, support:_SUPPORT, isar:_ISAR, r1:V_12_22, r2:R4_16, cs:CS_U_8_0x4_U, u:US_23, ua67:ARM_UA_DABORT}

//
// Attribute entries for SIMD/VFP push/pop instructions w/ arm reg SP and a reg list (initial reg + number of consecutive regs)
//   D/S versions
//
#define ATTR_SET_32_SDFP_PUSH(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_SIMD_RL, type:ARM_IT_##_NAME##_D, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:V_22_12, nregs:NREG_7_1, incDec:ID_U_P_IMP, wb:WB_21, ua67:ARM_UA_DABORT}, \
    [TT32_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_VFP_RL,  type:ARM_IT_##_NAME##_S, support:_SUPPORT, isar:_ISAR, r1:R4_16, r2:V_12_22, nregs:NREG_8_0, incDec:ID_U_P_IMP, wb:WB_21, ua67:ARM_UA_DABORT}

//
// Attribute entries for VFP data processing instructions w/ 3 regs same length:
//
#define ATTR_SET_32_VFP_RRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2_S3, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_16_7, r3:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs same length:
//
#define ATTR_SET_32_VFP_RR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and modified immediate:
// Note: immediate value 'abcdefgh' is in c (for disassembly purposes). Must still be converted to modified imm value at morph time
//
#define ATTR_SET_32_VFP_RI(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_SDFP_MI, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F32, r1:V_12_22, sdfpMI:SDFP_MI_VFP_S}

//
// Attribute entries for VFP data processing instructions w/ 2 regs same length:
//   S version only   2 dt1 sizes specified
//
#define ATTR_SET_32_VFP_RR_S_S2(_NAME, _SUPPORT, _ISAR, _OPCODE, _S1, _S2) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F##_S1, dt2:ARM_SDFPT_F##_S2, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and implied immediate 0.0 value
//
#define ATTR_SET_32_VFP_R0(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_F0, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs SS: Type for dt2  specified
//
#define ATTR_SET_32_VFP_LS_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F32, dt2:ARM_SDFPT##_T##32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs SS: Type for dt1 specified
//
#define ATTR_SET_32_VFP_NS_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, dt2:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and an immediate:
//   2 ftypes with sizes specified      A modifier for the constant specified
//
#define ATTR_SET_32_VFP_RI_T2C(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS1, _TS2, _C) \
    [TT32_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2_UIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS1, dt2:ARM_SDFPT##_TS2, r1:V_12_22, r2:V_12_22, cs:CS_U_5_0_5M##_C}


#endif

