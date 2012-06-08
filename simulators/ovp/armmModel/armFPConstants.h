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

#ifndef ARM_FPCONSTANTS_H
#define ARM_FPCONSTANTS_H

#define ARM_QNAN_DEFAULT_16     0x7e00
#define ARM_QNAN_DEFAULT_32     0x7fc00000
#define ARM_QNAN_DEFAULT_64     0x7ff8000000000000ULL
#define ARM_QNAN_MASK_16        0x0200
#define ARM_QNAN_MASK_32        0x00400000
#define ARM_QNAN_MASK_64        0x0008000000000000ULL
#define ARM_INDETERMINATE_16    0x8000
#define ARM_INDETERMINATE_32    0x80000000
#define ARM_INDETERMINATE_64    0x8000000000000000ULL
#define ARM_MIN_INT16           0x8000
#define ARM_MIN_INT32           0x80000000
#define ARM_MIN_INT64           0x8000000000000000ULL
#define ARM_MAX_INT16           0x7fff
#define ARM_MAX_INT32           0x7fffffff
#define ARM_MAX_INT64           0x7fffffffffffffffULL
#define ARM_MIN_UNS16           0x0000
#define ARM_MIN_UNS32           0x00000000
#define ARM_MIN_UNS64           0x0000000000000000ULL
#define ARM_MAX_UNS16           0xffff
#define ARM_MAX_UNS32           0xffffffff
#define ARM_MAX_UNS64           0xffffffffffffffffULL

//
// Single precision access macros
//
#define ARM_FP32_EXP_BIAS       127
#define ARM_FP32_EXP_ONES       0xff
#define ARM_FP32_EXP_SHIFT      23
#define ARM_FP32_SIGN_SHIFT     31

#define ARM_FP32_SIGN(_F)       (((_F) & 0x80000000) != 0)
#define ARM_FP32_EXPONENT(_F)   (((_F) & 0x7f800000) >> ARM_FP32_EXP_SHIFT)
#define ARM_FP32_FRACTION(_F)   ((_F) & ((1<<ARM_FP32_EXP_SHIFT)-1))

#define ARM_FP32_PLUS_ZERO      (0)
#define ARM_FP32_MINUS_ZERO     (0x80000000)
#define ARM_FP32_ZERO(_S)       ((_S) ? ARM_FP32_MINUS_ZERO : ARM_FP32_PLUS_ZERO)

#define ARM_FP32_PLUS_INFINITY  (0x7f800000)
#define ARM_FP32_MINUS_INFINITY (0xff800000)
#define ARM_FP32_INFINITY(_S)   ((_S) ? ARM_FP32_MINUS_INFINITY : ARM_FP32_PLUS_INFINITY)

// Build a fp value from its components
#define ARM_FP32_EXP_OFFSET       (127)
#define ARM_FP32_VALUE(_S, _E, _F) (ARM_FP32_ZERO(_S) | (((_E) + ARM_FP32_EXP_OFFSET) << 23) | (_F))
//
// Half precision access macros
//
#define ARM_FP16_EXP_BITS       5
#define ARM_FP16_EXP_BIAS       15
#define ARM_FP16_EXP_ONES       0x1f
#define ARM_FP16_EXP_SHIFT      10
#define ARM_FP16_SIGN_SHIFT     15
#define ARM_FP16_EXP_MIN        -14
#define ARM_FP16_EXP_MAX        15

#define ARM_FP16_SIGN(_F)       (((_F) & 0x8000) != 0)
#define ARM_FP16_EXPONENT(_F)   (((_F) & 0x7c00) >> ARM_FP16_EXP_SHIFT)
#define ARM_FP16_FRACTION(_F)   ((_F) & ((1<<ARM_FP16_EXP_SHIFT)-1))

#define ARM_FP16_PLUS_ZERO      (0)
#define ARM_FP16_MINUS_ZERO     (0x8000)
#define ARM_FP16_ZERO(_S)       ((_S) ? ARM_FP16_MINUS_ZERO : ARM_FP16_PLUS_ZERO)
#define ARM_FP16_MAX_NORMAL     (0x7bff)
#define ARM_FP16_AHP_INFINITY   (0x7fff)

#define ARM_FP16_PLUS_INFINITY  (0x7c00)
#define ARM_FP16_MINUS_INFINITY (0xfc00)
#define ARM_FP16_INFINITY(_S)   ((_S) ? ARM_FP16_MINUS_INFINITY : ARM_FP16_PLUS_INFINITY)

#endif
