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

#ifndef ARM_EXCEPTIONS_H
#define ARM_EXCEPTIONS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armExceptionTypes.h"
#include "armTypeRefs.h"

//
// Return the current execution priority
//
Int32 armGetExecutionPriority(armP arm);

//
// Refresh execution priority on any state change which affects it
//
void armRefreshExecutionPriority(armP arm);

//
// Refresh pending exception state on any state change which affects it
//
void armRefreshPendingException(armP arm);

//
// Refresh execution priority and pending exception state on any state change
// which affects them
//
void armRefreshExecutionPriorityPendingException(armP arm);

//
// Derive value of ICSR.RETTOBASE
//
Bool armGetRetToBase(armP arm);

//
// Handle exception return
//
void armExceptionReturn(armP arm, Uns32 targetPC);

//
// Connect up processor interrupts
//
void armConnectInterrupts(armP arm);

//
// Do breakpoint exception
//
void armBKPT(armP arm, Uns32 thisPC);

//
// Do software exception
//
void armSWI(armP arm, Uns32 thisPC);

//
// Do UsageFault exception
//
void armUsageFault(armP arm, Uns32 thisPC);

//
// Do BusFault exception
//
void armBusFault(armP arm, Uns32 faultAddress, memPriv priv);

//
// Do data/prefetch abort exception
//
void armMemoryAbort(armP arm, Uns32 faultAddress, memPriv priv);

//
// Raise an exception
//
void armRaise(armP arm, armExceptNum num);

//
// Perform SEV instruction actions
//
void armSEV(armP arm);

//
// Read SYST_CVR register
//
Uns32 armReadSYST_CVR(armP arm);

//
// Write SYST_CVR register
//
void armWriteSYST_CVR(armP arm, Uns32 newValue);

//
// Read SYST_CSR register
//
Uns32 armReadSYST_CSR(armP arm);

//
// Write SYST_CSR register
//
void armWriteSYST_CSR(armP arm, Uns32 newValue);

//
// Write SYST_RVR register
//
void armWriteSYST_RVR(armP arm, Uns32 newValue);

//
// This is the ARM PreserveFPState primitive
//
void armPreserveFPState(armP arm);

//
// Create port specifications
//
void armNewPortSpecs(armP arm);

//
// Free port specifications
//
void armFreePortSpecs(armP arm);

#endif
