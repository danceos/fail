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

#ifndef ARM_DECODE_ENTRIES_THUMB32_H
#define ARM_DECODE_ENTRIES_THUMB32_H

// VMI header files
#include "vmi/vmiDecode.h"

//
// This macro adds a decode table entry for a 32-bit Thumb instruction class
//
#define DECODE_TT32(_PRIORITY, _NAME, _PATTERN) \
    {type:TT32_##_NAME, priority:_PRIORITY, name:#_NAME"_T", pattern:_PATTERN}

//
// This macro adds an undefined instruction decode table entry for a 32-bit Thumb instruction class
//
#define DECODE_LAST(_PRIORITY, _PATTERN) \
    {type:TT_LAST, priority:_PRIORITY, name:"LAST_T", pattern:_PATTERN}

//
// Decode entries for 32-bit Thumb instructions like AND
//
#define DECODE_SET_32_AND(_NAME, _OP) \
    DECODE_TT32(0, _NAME##_IMM,         "|111|10.0|" _OP "|.|....|0...|....|........"), \
    DECODE_TT32(0, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|.|....|....|....|........"), \
    DECODE_TT32(1, _NAME##_RM_RRX,      "|111|0101|" _OP "|.|....|.000|....|0011....")

//
// Decode entries for 32-bit Thumb instructions like TST
//
#define DECODE_SET_32_TST(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,         "|111|10.0|" _OP "|1|....|0...|1111|........"), \
    DECODE_TT32(2, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|1|....|....|1111|........"), \
    DECODE_TT32(3, _NAME##_RM_RRX,      "|111|0101|" _OP "|1|....|.000|1111|0011....")

//
// Decode entries for 32-bit Thumb instructions like MOV
//
#define DECODE_SET_32_MOV(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,         "|111|10.0|" _OP "|.|1111|0...|....|........"), \
    DECODE_TT32(2, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|.|1111|....|....|........"), \
    DECODE_TT32(3, _NAME##_RM_RRX,      "|111|0101|" _OP "|.|1111|.000|....|0011....")

//
// Decode entries for 32-bit Thumb instructions like PKHBT
//
#define DECODE_SET_32_PKHBT(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "|111|01|01|0110|0|....|....|....|.." _OP "0|....")

//
// Decode entries for 32-bit Thumb instructions like ADD (plain binary immediate)
//
#define DECODE_SET_32_ADD_PI(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "|111|10|.1|" _OP "|....|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like ADR (plain binary immediate)
//
#define DECODE_SET_32_ADR_PI(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|1111|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like SSAT16
//
#define DECODE_SET_32_SSAT16(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|....|0|000|....|00......")

//
// Decode entries for 32-bit Thumb instructions like LSL
//
#define DECODE_SET_32_LSL(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(0, _NAME, "|111|1101|0|" _OP1 "|" _OP3 "|1111|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb instructions like SXTH
//
#define DECODE_SET_32_SXTH(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(1, _NAME, "|111|1101|0|" _OP1 "|" _OP3 "|1111|....|" _OP2 "|....")

//
// Decode entries for parallel add/subtract instructions
//
#define DECODE_SET_32_PAS(_NAME, _OP) \
    DECODE_TT32(0, S##_NAME,  "111|1101|01|" _OP "|....|1111|....|0000|....|"), \
    DECODE_TT32(0, Q##_NAME,  "111|1101|01|" _OP "|....|1111|....|0001|....|"), \
    DECODE_TT32(0, SH##_NAME, "111|1101|01|" _OP "|....|1111|....|0010|....|"), \
    DECODE_TT32(0, U##_NAME,  "111|1101|01|" _OP "|....|1111|....|0100|....|"), \
    DECODE_TT32(0, UQ##_NAME, "111|1101|01|" _OP "|....|1111|....|0101|....|"), \
    DECODE_TT32(0, UH##_NAME, "111|1101|01|" _OP "|....|1111|....|0110|....|")

//
// Decode entries for 32-bit Thumb instructions like MLA
//
#define DECODE_SET_32_MLA(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb instructions like MUL
//
#define DECODE_SET_32_MUL(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLA<x><y>
//
#define DECODE_SET_32_SMLA_XY(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME##BB, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "00|...."), \
    DECODE_TT32(0, _NAME##BT, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "01|...."), \
    DECODE_TT32(0, _NAME##TB, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "10|...."), \
    DECODE_TT32(0, _NAME##TT, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "11|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMUL<x><y>
//
#define DECODE_SET_32_SMUL_XY(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME##BB, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "00|...."), \
    DECODE_TT32(1, _NAME##BT, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "01|...."), \
    DECODE_TT32(1, _NAME##TB, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "10|...."), \
    DECODE_TT32(1, _NAME##TT, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "11|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLAD<x>
//
#define DECODE_SET_32_SMLAD(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME,    "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##X, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMUAD<x>
//
#define DECODE_SET_32_SMUAD(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME,    "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##X, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLAW<y>
//
#define DECODE_SET_32_SMLAW(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME##B, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##T, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMULW<y>
//
#define DECODE_SET_32_SMULW(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME##B, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##T, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMMLA
//
#define DECODE_SET_32_SMMLA(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME,    "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##R, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMMUL
//
#define DECODE_SET_32_SMMUL(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME,    "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##R, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb instructions like BFC
//
#define DECODE_SET_32_BFC(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|1111|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like B (1)
//
#define DECODE_SET_32_B1(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME, "|111|10|...........|1|" _OP1 "|..........." _OP2)

//
// Decode entries for 32-bit Thumb undefined instructions
//
#define DECODE_SET_32_UNDEF(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|....|........")

//
// Decode entries for 32-bit Thumb instructions like MSR
//
#define DECODE_SET_32_MSR(_NAME, _OP1, _OP2) \
    DECODE_TT32(2, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|....|........")

//
// Decode entries for 32-bit Thumb hint instructions like NOP
//
#define DECODE_SET_32_HINT1(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(3, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|.000|" _OP3)

//
// Decode entries for 32-bit Thumb hint instructions like YIELD
//
#define DECODE_SET_32_HINT2(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(4, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|.000|" _OP3)

//
// Decode entries for 32-bit Thumb instructions like CLREX
//
#define DECODE_SET_32_CLREX(_NAME, _OP) \
    DECODE_TT32(2, _NAME, "|111|10|0111011|....|10.0|....|" _OP "|....")

//
// Decode entries for 32-bit Thumb instructions like SRS
//
#define DECODE_SET_32_SRS(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|0100|" _OP1 "|0|" _OP2 "|................")

//
// Decode entries for 32-bit Thumb instructions like POPM
//
#define DECODE_SET_32_POPM(_NAME, _OP1, _OP2) \
    DECODE_TT32(2, _NAME, "|111|0100|" _OP1 "|0|" _OP2 "|................")

//
// Decode entries for 32-bit Thumb instructions like LDRD_IMM
//
#define DECODE_SET_32_LDRD_IMM(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(0, _NAME, "|111|0100|" _OP1 "|1|" _OP2 "|....|........" _OP3 "....")

//
// Decode entries for 32-bit Thumb instructions like LDREX
//
#define DECODE_SET_32_LDREX(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(1, _NAME, "|111|0100|" _OP1 "|1|" _OP2 "|....|........" _OP3 "....")

//
// Decode entries for 32-bit Thumb instructions like LDR
//
#define DECODE_SET_32_LDR(_NAME, _SIGN, _SZ) \
    DECODE_TT32(0, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|1|....|....|......|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1..1..|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1100..|......"), \
    DECODE_TT32(3, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|1|1111|....|......|......"), \
    DECODE_TT32(2, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|1|....|....|000000|00...."), \
    DECODE_TT32(1, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|1|....|....|000000|......"), \
    DECODE_TT32(1, _NAME##T_IMM,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1110..|......")

//
// Decode entries for 32-bit Thumb instructions like STR
//
#define DECODE_SET_32_STR(_NAME, _SIGN, _SZ) \
    DECODE_TT32(0, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|0|....|....|......|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1..1..|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1100..|......"), \
    DECODE_TT32(3, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|0|1111|....|......|......"), \
    DECODE_TT32(2, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|0|....|....|000000|00...."), \
    DECODE_TT32(1, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|0|....|....|000000|......"), \
    DECODE_TT32(1, _NAME##T_IMM,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1110..|......")

//
// Decode entries for 32-bit Thumb instructions like PLD
//
#define DECODE_SET_32_PLD(_NAME, _SIGN, _SZ) \
    DECODE_TT32(5, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|1|....|1111|......|......"), \
    DECODE_TT32(6, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|1100..|......"), \
    DECODE_TT32(8, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|1|1111|1111|......|......"), \
    DECODE_TT32(7, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|000000|00...."), \
    DECODE_TT32(6, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|000000|......")

//
// Decode entries for 32-bit Thumb instructions like UHINTH
//
#define DECODE_SET_32_UHINTH(_NAME, _SZ) \
    DECODE_TT32(4, _NAME, "|111|1100|..|" _SZ "|1|....|1111|......|......")

//
// Decode entries for 32-bit Thumb instructions like CDP
//
#define DECODE_SET_32_CDP(_NAME) \
    DECODE_TT32(0, _NAME, "....|1110|....|....|....|....|...|0|....")

//
// Decode entries for 32-bit Thumb instructions like CDP2
//
#define DECODE_SET_32_CDP2(_NAME) \
    DECODE_TT32(1, _NAME, "1111|1110|....|....|....|....|...|0|....")

//
// Decode entries for 32-bit Thumb instructions like LDC
//
#define DECODE_SET_32_LDC(_NAME, _OP) \
    DECODE_TT32(0, _NAME##_IMM,       "....|110|...." _OP "|....|....|....|........"), \
    DECODE_TT32(1, _NAME##_UNINDEXED, "....|110|0..0" _OP "|....|....|....|........")

//
// Decode entries for 32-bit Thumb instructions like LDC2
//
#define DECODE_SET_32_LDC2(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,       "1111|110|...." _OP "|....|....|....|........"), \
    DECODE_TT32(3, _NAME##_UNINDEXED, "1111|110|0..0" _OP "|....|....|....|........")

//
// Decode entries for 32-bit Thumb instructions like MCR
//
#define DECODE_SET_32_MCR(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "....|1110|...|" _OP "|....|....|....|...|1|....")

//
// Decode entries for 32-bit Thumb instructions like MCR2
//
#define DECODE_SET_32_MCR2(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "1111|1110|...|" _OP "|....|....|....|...|1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like MCRR
//
#define DECODE_SET_32_MCRR(_NAME, _OP) \
    DECODE_TT32(4, _NAME, "....|1100010" _OP "|....|....|....|....|....")

//
// Decode entries for 32-bit Thumb DSP instructions like MCRR2
//
#define DECODE_SET_32_MCRR2(_NAME, _OP) \
    DECODE_TT32(5, _NAME, "1111|1100010" _OP "|....|....|....|....|....")

//
// Decode entries for VFP/SIMD instructions like VMRS
// Note - When bit 28=1 manual states this is an undefined instruction so extra
//        decode is added. Without it this decodes as MRC and gives NOCP fault.
//
#define DECODE_SET_32_VMRS(_NAME, _OPL, _OPC, _OPA, _OPB) \
    DECODE_TT32(2, _NAME, "1110|1110" _OPA _OPL "|....|....|101" _OPC "|." _OPB "1|...."), \
    DECODE_LAST(2,        "1111|1110" _OPA _OPL "|....|....|101" _OPC "|." _OPB "1|....")

//
// Decode entries for VFP instructions like VMOVRRD
//
#define DECODE_SET_32_VMOVRRD(_NAME, _OPL, _OPC, _OP) \
    DECODE_TT32(6, _NAME, "1110|1100|010" _OPL "........|101" _OPC _OP "|...."), \
    DECODE_LAST(6,        "1111|1100|010" _OPL "........|101" _OPC _OP "|....")

//
// Decode entries for SIMD/VFP load/store instructions
//
#define DECODE_SET_32_SDFP_LDST(_NAME, _OP) \
    DECODE_TT32(4, _NAME##_D, "1110|110" _OP "|....|....|1011|....|...."), \
    DECODE_TT32(4, _NAME##_S, "1110|110" _OP "|....|....|1010|....|...."), \
    DECODE_LAST(6,            "1111|110" _OP "|....|....|101.|....|....")

//
// Decode entries for SIMD/VFP PUSH/POP instructions
//
#define DECODE_SET_32_SDFP_PUSH_POP(_NAME, _OP) \
    DECODE_TT32(5, _NAME##_D, "1110|110" _OP "|1101|....|1011|....|...."), \
    DECODE_TT32(5, _NAME##_S, "1110|110" _OP "|1101|....|1010|....|....")

//
// Decode entries for VFP instructions S version only
//
#define DECODE_SET_32_VFP_S(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(2, _NAME, "1110|1110|" _OP1 _OP2 "|....|1010|" _OP3 ".0|...."), \
    DECODE_LAST(3,        "1111|1110|" _OP1 _OP2 "|....|1010|" _OP3 ".0|....")
#endif

