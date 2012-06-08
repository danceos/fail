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

#ifndef ARM_MODE_H
#define ARM_MODE_H

//
// Dictionary modes
//
typedef enum armModeE {

    // BITMASKS
    ARM_MODE_U        = 0x1, // user (unprivileged) mode bitmask
    ARM_MODE_MPU      = 0x2, // MPU enabled mode bitmask

    // VALID MODES
    ARM_MODE_PRIV     = (0 | 0          | 0           ),
    ARM_MODE_USER     = (0 | ARM_MODE_U | 0           ),
    ARM_MODE_PRIV_MPU = (0 | 0          | ARM_MODE_MPU),
    ARM_MODE_USER_MPU = (0 | ARM_MODE_U | ARM_MODE_MPU),

    // CATCHER FOR ARM MODE EXECUTION
    ARM_MODE_ARM,

    // KEEP LAST: for array sizing
    ARM_MODE_LAST

} armMode;

//
// Wait reasons (bitmask)
//
typedef enum armDisableE {
    AD_WFE    = 0x01,   // wait for event
    AD_WFI    = 0x02,   // wait for interrupt
    AD_LOCKUP = 0x04    // lockup state
} armDisable;

//
// Block mask entries
//
typedef enum armBlockMaskE {
    ARM_BM_USE_SP_PROCESS = 0x0001,
    ARM_BM_BIG_ENDIAN     = 0x0002,
    ARM_BM_UNALIGNED      = 0x0004,
    ARM_BM_CP10           = 0x0008,
    ARM_BM_FPCA           = 0x0010,
    ARM_BM_ASPEN          = 0x0020,
    ARM_BM_LSPACT         = 0x0040,
} armBlockMask;

#endif
