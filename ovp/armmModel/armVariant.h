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

#ifndef ARM_VARIANT_H
#define ARM_VARIANT_H

// this defines the architecture options and variants
typedef enum armArchitectureE {

                            // ARCHITECTURE OPTIONS
    ARM_VM  = 0x0010,       // enable long multiply instructions
    ARM_VT  = 0x0020,       // enable thumb instructions
    ARM_VD  = 0x0040,       // enable basic DSP instructions
    ARM_VD2 = 0x0080,       // enable LDRD, MCRR, MRRC, PLD and STRD insns
    ARM_SS  = 0x0100,       // enable supersections, TEX and S bits
    ARM_BX  = 0x0200,       // enable BX instruction
    ARM_J   = 0x0400,       // enable trivial Jazelle extension
    ARM_K   = 0x0800,       // enable multi-processing instructions
    ARM_VT2 = 0x1000,       // enable Thumb-2 instructions

                            // ARCHITECTURE VERSIONS
    ARM_V4XM   = 4,
    ARM_V4     = 4          | ARM_VM,
    ARM_V4TXM  = 4 | ARM_BX |          ARM_VT,
    ARM_V4T    = 4 | ARM_BX | ARM_VM | ARM_VT,
    ARM_V5XM   = 5 | ARM_BX,
    ARM_V5     = 5 | ARM_BX | ARM_VM,
    ARM_V5TXM  = 5 | ARM_BX |          ARM_VT,
    ARM_V5T    = 5 | ARM_BX | ARM_VM | ARM_VT,
    ARM_V5TEXP = 5 | ARM_BX | ARM_VM | ARM_VT | ARM_VD,
    ARM_V5TE   = 5 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS,
    ARM_V5TEJ  = 5 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS | ARM_J,
    ARM_V6     = 6 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS | ARM_J,
    ARM_V6K    = 6 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS | ARM_J | ARM_K,
    ARM_V6T2   = 6 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS | ARM_J |         ARM_VT2,
    ARM_V7     = 7 | ARM_BX | ARM_VM | ARM_VT | ARM_VD | ARM_VD2 | ARM_SS | ARM_J | ARM_K | ARM_VT2

} armArchitecture;

// this defines thumb versions
typedef enum armThumbVersionE {
    ARM_THUMB_NONE,         // no thumb instructions
    ARM_THUMB_V1,           // version 1 thumb instructions
    ARM_THUMB_V2            // version 2 thumb instructions
} armThumbVersion;

// this selects armArchitecture bits specifying instruction version
#define ARM_MASK_VERSION 0xf

// this is used to restrict availability of features to certain variants
#define ARM_SUPPORT(_V, _M) (((_V) & (_M)) == (_M))

// get the main instruction set version
#define ARM_INSTRUCTION_VERSION(_V) ((_V) & ARM_MASK_VERSION)

// get the thumb instruction set version
#define ARM_THUMB_VERSION(_V) ( \
    !ARM_SUPPORT(_V, ARM_VT) ? ARM_THUMB_NONE : \
    (ARM_INSTRUCTION_VERSION(_V)==4) ? ARM_THUMB_V1 : ARM_THUMB_V2 \
)

// get architecture index for the passed variant
#define ARM_VARIANT_ARCH(_V) \
    ({                                              \
        Uns32 _IV = ARM_INSTRUCTION_VERSION(_V);    \
        (_IV>=7)                   ? 0xf :          \
        (_IV==6)                   ? 0x7 :          \
        (_IV==5) && ((_V)&ARM_J)   ? 0x6 :          \
        (_IV==5) && ((_V)&ARM_VD2) ? 0x5 :          \
        (_IV==5) && ((_V)&ARM_VT)  ? 0x4 :          \
        (_IV==5)                   ? 0x3 :          \
        (_IV==4) && ((_V)&ARM_VT)  ? 0x2 :          \
        (_IV==4)                   ? 0x1 : 0x0;     \
    })

#endif
