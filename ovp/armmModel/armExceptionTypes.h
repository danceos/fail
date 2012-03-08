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

#ifndef ARM_EXCEPTION_TYPES_H
#define ARM_EXCEPTION_TYPES_H

//
// This enumerates exceptions
//
typedef enum armExceptNumE {
    AEN_None         =   0,
    AEN_Reset        =   1,
    AEN_NMI          =   2,
    AEN_HardFault    =   3,
    AEN_MemManage    =   4,
    AEN_BusFault     =   5,
    AEN_UsageFault   =   6,
    AEN_SVCall       =  11,
    AEN_DebugMonitor =  12,
    AEN_PendSV       =  14,
    AEN_SysTick      =  15,
    AEN_ExternalInt0 =  16,
    AEN_LAST         = 512  // KEEP LAST: for sizing
} armExceptNum;

//
// This identifies whether the passed exception is an interrupt (and whether
// SCR.SLEEPONEXIT applies to this exception type)
//
#define EX_IS_INTERRUPT(_E) ((_E)>=AEN_PendSV)

//
// This enumerates the context of a load/store exception
//
typedef enum armExceptCxtE {
    AEC_None = 0,
    AEC_PushStack,
    AEC_PopStack,
    AEC_ReadVector,
    AEC_PreserveFPState
} armExceptCxt;

//
// This defines permanently-enabled exceptions
//
#define ARM_EXCEPT_PERMANENT_ENABLE ( \
    (1 << AEN_Reset)     |  \
    (1 << AEN_NMI)       |  \
    (1 << AEN_HardFault) |  \
    (1 << AEN_SVCall)    |  \
    (1 << AEN_PendSV)    |  \
    (1 << AEN_SysTick)      \
)

//
// Utility macros for accessing exception bitmasks
//
#define EX_MASK_INDEX(_N)     ((_N)/32)
#define EX_MASK_BIT(_N)       ((_N)&31)
#define EX_MASK_MASK(_N)      (1<<EX_MASK_BIT(_N))
#define EX_MASK_GET(_M, _N)   ((_M[EX_MASK_INDEX(_N)] >> EX_MASK_BIT(_N)) & 1)
#define EX_MASK_SET_1(_M, _N) _M[EX_MASK_INDEX(_N)] |=  EX_MASK_MASK(_N)
#define EX_MASK_SET_0(_M, _N) _M[EX_MASK_INDEX(_N)] &= ~EX_MASK_MASK(_N)

#define EX_MASK_SET_V(_M, _N, _V) { \
    EX_MASK_SET_0(_M, _N);                              \
    _M[EX_MASK_INDEX(_N)] |= ((_V)<<EX_MASK_BIT(_N));   \
}

//
// Masks for the low and high halves of interrupt registers
//
#define EX_INT_MASK_LO ((1<<AEN_ExternalInt0)-1)
#define EX_INT_MASK_HI (~EX_INT_MASK_LO)

//
// Masks for extraction of fields from targetPC possibly specifying exception
// return
//
#define EXC_RETURN_MAGIC 0xf0000000
#define EXC_RETURN_TYPE  0x0000000f

//
// Masks for undefined instruction reasons
//
#define EXC_UNDEF_NOCP       0x00080000
#define EXC_UNDEF_UNDEFINSTR 0x00010000

#endif
