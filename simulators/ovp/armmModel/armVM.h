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

#ifndef ARM_VM_H
#define ARM_VM_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"


//
// This is the minimum page size (1Kb)
//
#define MIN_PAGE_SIZE 1024

//
// Enumation describing actions performed by armVMMiss
//
typedef enum armVMActionE {
    MA_OK,          // memory was mapped, access can proceed
    MA_EXCEPTION    // memory was not mapped, exception was triggered
} armVMAction;

//
// Try mapping memory at the passed address for the specified access type and
// return a status code indicating whether the mapping succeeded
//
armVMAction armVMMiss(
    armP           arm,
    memPriv        requiredPriv,
    Uns32          address,
    Uns32          bytes,
    memAccessAttrs attrs
);

//
// Set the privileged mode data domain to the user domain (for LDRT, STRT)
//
void armVMSetUserPrivilegedModeDataDomain(armP arm);

//
// Restore the normal data domain for the current mode (for LDRT, STRT)
//
void armVMRestoreNormalDataDomain(armP arm);

//
// Write the indexed MPU RBAR register value
//
void armVMWriteRBAR(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed MPU RBAR register value
//
Uns32 armVMReadRBAR(armP arm, Uns32 index, Bool isData);

//
// Write the indexed MPU RASR register value
//
void armVMWriteRASR(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed MPU RASR register value
//
Uns32 armVMReadRASR(armP arm, Uns32 index, Bool isData);

//
// Flush the privileged mode MPU
//
void armVMFlushMPUPriv(armP arm);

//
// Reset VM structures
//
void armVMReset(armP arm);

//
// Free structures used for virtual memory management
//
void armVMFree(armP arm);

#endif

