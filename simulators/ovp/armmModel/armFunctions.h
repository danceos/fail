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

#ifndef ARM_FUNCTIONS_H
#define ARM_FUNCTIONS_H

// VMI header files
#include "vmi/vmiAttrs.h"

// constructor & destructor
VMI_CONSTRUCTOR_FN(armConstructor);
VMI_VMINIT_FN(armVMInit);
VMI_DESTRUCTOR_FN(armDestructor);

// morph function
VMI_MORPH_FN(armMorphInstruction);
VMI_FETCH_SNAP_FN(armFetchSnap);

// simulation support functions
VMI_ENDIAN_FN(armGetEndian);
VMI_NEXT_PC_FN(armNextInstruction);
VMI_DISASSEMBLE_FN(armDisassemble);
VMI_IASSWITCH_FN(armContextSwitchCB);

// debugger integration support routines
VMI_REG_GROUP_FN(armRegGroup);
VMI_REG_INFO_FN(armRegInfo);
VMI_EXCEPTION_INFO_FN(armExceptionInfo);
VMI_MODE_INFO_FN(armModeInfo);
VMI_GET_EXCEPTION_FN(armGetException);
VMI_GET_MODE_FN(armGetMode);
VMI_DEBUG_FN(armDumpRegisters);

// parameter support functions
VMI_PROC_PARAM_SPECS_FN(armGetParamSpec);
VMI_PROC_PARAM_TABLE_SIZE_FN(armParamValueSize);

// port functions
VMI_BUS_PORT_SPECS_FN(armGetBusPortSpec);
VMI_NET_PORT_SPECS_FN(armGetNetPortSpec);

// exception functions
VMI_RD_PRIV_EXCEPT_FN(armRdPrivExceptionCB);
VMI_WR_PRIV_EXCEPT_FN(armWrPrivExceptionCB);
VMI_RD_ALIGN_EXCEPT_FN(armRdAlignExceptionCB);
VMI_WR_ALIGN_EXCEPT_FN(armWrAlignExceptionCB);
VMI_RD_ABORT_EXCEPT_FN(armRdAbortExceptionCB);
VMI_WR_ABORT_EXCEPT_FN(armWrAbortExceptionCB);
VMI_IFETCH_FN(armIFetchExceptionCB);
VMI_ARITH_EXCEPT_FN(armArithExceptionCB);
VMI_ICOUNT_FN(armICountPendingCB);

// Imperas intercepted function support
VMI_INT_RETURN_FN(armIntReturnCB);
VMI_INT_RESULT_FN(armIntResultCB);
VMI_INT_PAR_FN(armIntParCB);

// Processor information support
VMI_PROC_INFO_FN(armProcInfo);

#endif

