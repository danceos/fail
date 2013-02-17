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

#ifndef ARM_DECODE_ENTRIES_THUMB16_H
#define ARM_DECODE_ENTRIES_THUMB16_H

// VMI header files
#include "vmi/vmiDecode.h"

//
// This macro adds a decode table entry for a 16-bit Thumb instruction class
//
#define DECODE_TT16(_PRIORITY, _NAME, _PATTERN) \
    {type:TT16_##_NAME, priority:_PRIORITY, name:#_NAME"_T", pattern:_PATTERN}

//
// Decode entries for 16-bit Thumb instructions like ADC
//
#define DECODE_SET_16_ADC(_NAME, _OP) \
    DECODE_TT16(1, _NAME, "|"_OP"|...|...|")

//
// Decode entries for 16-bit Thumb instructions like ADD (1)
//
#define DECODE_SET_16_ADD1(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|...|...|...|")

//
// Decode entries for 16-bit Thumb instructions like ADD (2)
//
#define DECODE_SET_16_ADD2(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|...|........|")

//
// Decode entries for 16-bit Thumb instructions like ADD (4)
//
#define DECODE_SET_16_ADD4(_NAME, _OP) \
    DECODE_TT16(1, _NAME, "|"_OP"|...|...|")

//
// Decode entries for 16-bit Thumb instructions like ADD (7)
//
#define DECODE_SET_16_ADD7(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|.......|")

//
// Decode entries for 16-bit Thumb instructions like ASR (1)
//
#define DECODE_SET_16_ASR1(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|.....|...|...|")

//
// Decode entries for 16-bit Thumb instructions like B (1)
//
#define DECODE_SET_16_B1(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|1101|" _OP "|........|")

//
// Decode entries for 16-bit Thumb instructions like B (2)
//
#define DECODE_SET_16_B2(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|111|" _OP "...........|")
    
//
// Decode entries for 16-bit Thumb instructions like BLX (2)
//
#define DECODE_SET_16_BLX2(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|....|...|")

//
// Decode entries for undefined 16-bit Thumb instructions like SWI
//
#define DECODE_SET_16_SWI(_NAME, _OP) \
    DECODE_TT16(1, _NAME, "|1101|" _OP "|........|")

//
// Decode entries for 16-bit Thumb instructions like BKPT
//
#define DECODE_SET_16_BKPT(_NAME, _OP) \
    DECODE_TT16(1, _NAME, "1011|"_OP"|.....|")

//
// Decode entries for 16-bit Thumb instructions like POP
//
#define DECODE_SET_16_POP(_NAME, _OP) \
    DECODE_TT16(0, _NAME, "|"_OP"|.|........|")

//
// Decode entries for 16-bit Thumb instructions like IT
//
#define DECODE_SET_16_IT(_NAME, _OP1, _OP2) \
    DECODE_TT16(0, _NAME, "|1011|1111|" _OP1 "|" _OP2 "|")

//
// Decode entries for 16-bit Thumb hint instructions like NOP
//
#define DECODE_SET_16_HINT1(_NAME, _OP1, _OP2) \
    DECODE_TT16(1, _NAME, "|1011|1111|" _OP1 "|" _OP2 "|")

//
// Decode entries for 16-bit Thumb hint instructions like YIELD
//
#define DECODE_SET_16_HINT2(_NAME, _OP1, _OP2) \
    DECODE_TT16(2, _NAME, "|1011|1111|" _OP1 "|" _OP2 "|")

#endif

