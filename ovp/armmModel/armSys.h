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

#ifndef ARM_SYS_H
#define ARM_SYS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armSysRegisters.h"
#include "armTypeRefs.h"

//
// These are the bounds of the Peripheral region
//
#define PERIPH_LOW  0x40000000
#define PERIPH_HIGH 0x4fffffff

//
// These are the bounds of the System Control Space
//
#define DEVICE_LOW  0xa0000000
#define DEVICE_HIGH 0xdfffffff

//
// These are the bounds of the System Control Space
//
#define SYSTEM_LOW  0xe0000000
#define SYSTEM_HIGH 0xffffffff

//
// These are the bounds of the Private Peripheral Bus
//
#define PPB_LOW  0xe0000000
#define PPB_HIGH 0xe00fffff

//
// Call on initialization
//
void armSysInitialize(armP arm);

//
// Call on reset
//
void armSysReset(armP arm);

//
// Is the indicated system register supported on this processor?
//
Bool armGetSysRegSupported(armSCSRegId id, armP arm);

//
// Perform a privileged-mode read of the system register
//
Bool armReadSysRegPriv(armSCSRegId id, armP arm, Uns32 *result);

//
// Perform a privileged-mode write of the system register
//
Bool armWriteSysRegPriv(armSCSRegId id, armP arm, Uns32 value);

//
// Add programmer's view of system register
//
void armAddSysRegisterView(
    armSCSRegId    id,
    armP           arm,
    vmiViewObjectP baseObject,
    const char    *name
);

//
// Structure filled with system register description by
// armGetSysRegisterDesc
//
typedef struct armSysRegDescS {
    const char *name;
    armSCSRegId id;
    Uns32       address;
    const char  *privRW;
    const char  *userRW;
} armSysRegDesc, *armSysRegDescP;

//
// Iterator filling 'desc' with the next system register description -
// 'desc.name' should be initialized to NULL prior to the first call
//
Bool armGetSysRegisterDesc(armSysRegDescP desc);

//
// Insert SCS region into the passed domain at the standard location
//
void armSysCreateSCSRegion(armP arm, memDomainP domain);

#endif

