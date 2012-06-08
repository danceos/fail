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

#ifndef ARM_UTILS_H
#define ARM_UTILS_H

// model header files
#include "armTypeRefs.h"

//
// Set the initial endianness for the model
//
void armSetInitialEndian(armP arm, Bool isBigEndian);

//
// Return the name of a GPR
//
const char *armGetGPRName(armP arm, Uns32 index);

//
// Return the name of a CPR
//
const char *armGetCPRName(armP arm, Uns32 index);

//
// Update processor block mask
//
void armSetBlockMask(armP arm);

//
// Switch banked registers on switch to the passed mode
//
void armSwitchRegs(armP arm, Bool oldUseSPProcess, Bool newUseSPProcess);

//
// Write CONTROL.SP_PROCESS field
//
void armWriteSPProcess(armP arm, Bool newSPProcess);

//
// Read CPSR register
//
Uns32 armReadCPSR(armP arm);

//
// Write CPSR register
//
void armWriteCPSR(armP arm, Uns32 value, Uns32 mask);

//
// Write PRIMASK register
//
void armWritePRIMASK(armP arm, Uns32 value);

//
// Write BASEPRI register
//
void armWriteBASEPRI(armP arm, Uns32 value);

//
// Write BASEPRI register (using BASEPRI_MAX semantics)
//
void armWriteBASEPRI_MAX(armP arm, Uns32 value);

//
// Write FAULTMASK register
//
void armWriteFAULTMASK(armP arm, Uns32 value);

//
// read CONTROL register
//
Uns32 armReadCONTROL(armP arm);

//
// Write CONTROL register
//
void armWriteCONTROL(armP arm, Uns32 value);

//
// update the CONTROL.FPCA bit to the value. If it changes then set the block mask
//
void armUpdateFPCA(armP arm, Bool value);

//
// update the FPCCR.LSPACT bit to the value. If it changes then set the block mask
//
void armUpdateLSPACT(armP arm, Bool value);

//
// Write SP register
//
void armWriteSP(armP arm, Uns32 value);

//
// Switch processor mode if required
//
void armSwitchMode(armP arm);

//
// Abort any active exclusive access
//
void armAbortExclusiveAccess(armP arm);

#endif
