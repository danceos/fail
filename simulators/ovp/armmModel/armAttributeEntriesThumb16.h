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

#ifndef ARM_ATTRIBUTE_ENTRIES_THUMB16_H
#define ARM_ATTRIBUTE_ENTRIES_THUMB16_H

#include "armDisassembleFormats.h"

//
// Attribute entries for 16-bit Thumb instructions like ADC
//
#define ATTR_SET_16_ADC(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3}

//
// Attribute entries for 16-bit Thumb instructions like ADD (1)
//
#define ATTR_SET_16_ADD1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3, cs:CS_U_3_6}

//
// Attribute entries for 16-bit Thumb instructions like ADD (2)
//
#define ATTR_SET_16_ADD2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_8, cs:CS_U_8_0}

//
// Attribute entries for 16-bit Thumb instructions like ADD (3)
//
#define ATTR_SET_16_ADD3(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3, r3:R3_6}

//
// Attribute entries for 16-bit Thumb instructions like ADD (4)
//
#define ATTR_SET_16_ADD4(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R4_0H7, r2:R4_3H6}

//
// Attribute entries for 16-bit Thumb instructions like ADD (5)
//
#define ATTR_SET_16_ADD5(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, r2:R_PC, cs:CS_U_8_0x4}

//
// Attribute entries for 16-bit Thumb instructions like ADD (6)
//
#define ATTR_SET_16_ADD6(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, r2:R_SP, cs:CS_U_8_0x4}

//
// Attribute entries for 16-bit Thumb instructions like ADD (7)
//
#define ATTR_SET_16_ADD7(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R_SP, cs:CS_U_7_0x4}

//
// Attribute entries for 16-bit Thumb instructions like ASR (1)
//
#define ATTR_SET_16_ASR1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SS) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3, cs:CS_U_5_6, ss:_SS}

//
// Attribute entries for 16-bit Thumb instructions like ASR (2)
//
#define ATTR_SET_16_ASR2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SS) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3, ss:_SS}

//
// Attribute entries for 16-bit Thumb instructions like MOV (2)
//
#define ATTR_SET_16_MOV2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_IT, r1:R3_0, r2:R3_3, cs:CS_U_5_6, ss:SS_LSL}

//
// Attribute entries for 16-bit Thumb instructions like CMP (1)
//
#define ATTR_SET_16_CMP1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R3_8, cs:CS_U_8_0}

//
// Attribute entries for 16-bit Thumb instructions like CMP (2)
//
#define ATTR_SET_16_CMP2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R3_0, r2:R3_3}

//
// Attribute entries for 16-bit Thumb instructions like CMP (3)
//
#define ATTR_SET_16_CMP3(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, f:SF_I, r1:R4_0H7, r2:R4_3H6}

//
// Attribute entries for 16-bit Thumb instructions like B (1)
//
#define ATTR_SET_16_B1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, cond:CO_8, ts:TC_S8}

//
// Attribute entries for 16-bit Thumb instructions like B (2)
//
#define ATTR_SET_16_B2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, ts:TC_S11}

//
// Attribute entries for 16-bit Thumb instructions like BLX (2)
//
#define ATTR_SET_16_BLX2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R4_3H6}

//
// Attribute entries for 16-bit Thumb instructions like SXTH
//
#define ATTR_SET_16_SXTH(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R3_0, r2:R3_3}

//
// Attribute entries for 16-bit Thumb instructions like BKPT
//
#define ATTR_SET_16_BKPT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_XIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_8_0}

//
// Attribute entries for 16-bit Thumb instructions like SETEND
//
#define ATTR_SET_16_SETEND(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_ENDIAN, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_1_3}

//
// Attribute entries for 16-bit Thumb instructions like CPS
//
#define ATTR_SET_16_CPS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_FLAGS_OPT_MODE, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, imod:IS_4, aif:AIF_0}

//
// Attribute entries for 16-bit Thumb instructions like CBNZ
//
#define ATTR_SET_16_CBNZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_T, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, r1:R3_0, ts:TC_U9_7_3}

//
// Attribute entries for 16-bit Thumb instructions like LDMIA
//
#define ATTR_SET_16_LDMIA(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _RLIST, _INC, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_RLIST, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, wb:WB_1_NB, rList:_RLIST, incDec:_INC, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like LDR (1)
//
#define ATTR_SET_16_LDR1(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_0, r2:R3_3, cs:CS_U_5_6_SZ, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like LDR (2)
//
#define ATTR_SET_16_LDR2(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_0, r2:R3_3, r3:R3_6, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like LDR (3)
//
#define ATTR_SET_16_LDR3(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, r2:R_PC, cs:CS_U_8_0_SZ, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like LDR (4)
//
#define ATTR_SET_16_LDR4(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, r2:R_SP, cs:CS_U_8_0_SZ, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like POP
//
#define ATTR_SET_16_POP(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _RLIST, _INC, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_RLIST, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R_SP, pi:PI_1, wb:WB_1, rList:_RLIST, incDec:_INC,  ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like STMIA
//
#define ATTR_SET_16_STMIA(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE, _RLIST, _INC, _UA45, _UA67) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_R1_RLIST_T, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, r1:R3_8, pi:PI_1, wb:WB_1, rList:_RLIST, incDec:_INC,  ua45:_UA45, ua67:_UA67}

//
// Attribute entries for 16-bit Thumb instructions like IT
//
#define ATTR_SET_16_IT(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_ITC, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR, cs:CS_U_4_4, it:1}

//
// Attribute entries for 16-bit Thumb instructions like NOP
//
#define ATTR_SET_16_NOP(_NAME, _TYPE, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, type:ARM_IT_##_TYPE, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for 16-bit Thumb instructions like BL_H10
//
#define ATTR_SET_16_BL_H10(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_SIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_S_11_0S12}

//
// Attribute entries for 16-bit Thumb instructions like BL_H11
//
#define ATTR_SET_16_BL_H11(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [TT16_##_NAME] = {opcode:_OPCODE, format:FMT_UIMM, type:ARM_IT_##_NAME, support:_SUPPORT, isar:_ISAR, cs:CS_U_11_0S1}

//
// Attribute entry for undecoded 16-bit Thumb instruction
//
#define ATTR_SET_16_UNDECODED(_NAME) \
    [TT16_##_NAME] = {type:ARM_IT_LAST}

#endif

