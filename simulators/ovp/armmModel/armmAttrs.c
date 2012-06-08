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

// VMI header files
#include "vmi/vmiAttrs.h"

// model header files
#include "armStructure.h"
#include "armFunctions.h"

static const char *dictNames[] = {"PRIV", "USER", "PRIV_MPU", "USER_MPU", "ARM", 0};

//
// Configuration block for instruction-accurate modelling
//
const vmiIASAttr modelAttrs =  {

    ////////////////////////////////////////////////////////////////////////
    // VERSION & SIZE ATTRIBUTES
    ////////////////////////////////////////////////////////////////////////

    .versionString = VMI_VERSION,
    .modelType     = VMI_PROCESSOR_MODEL,
    .dictNames     = dictNames,
    .cpuSize       = sizeof(arm),

    ////////////////////////////////////////////////////////////////////////
    // CREATE/DELETE ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .constructorCB = armConstructor,
    .vmInitCB      = armVMInit,
    .destructorCB  = armDestructor,

    ////////////////////////////////////////////////////////////////////////
    // MORPHER CORE ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .morphCB     = armMorphInstruction,
    .fetchSnapCB = armFetchSnap,

    ////////////////////////////////////////////////////////////////////////
    // SIMULATION SUPPORT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .getEndianCB = armGetEndian,
    .nextPCCB    = armNextInstruction,
    .disCB       = armDisassemble,
    .switchCB    = armContextSwitchCB,

    ////////////////////////////////////////////////////////////////////////
    // EXCEPTION ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .rdPrivExceptCB  = armRdPrivExceptionCB,
    .wrPrivExceptCB  = armWrPrivExceptionCB,
    .rdAlignExceptCB = armRdAlignExceptionCB,
    .wrAlignExceptCB = armWrAlignExceptionCB,
    .rdAbortExceptCB = armRdAbortExceptionCB,
    .wrAbortExceptCB = armWrAbortExceptionCB,
    .arithExceptCB   = armArithExceptionCB,
    .ifetchExceptCB  = armIFetchExceptionCB,
    .icountExceptCB  = armICountPendingCB,

    ////////////////////////////////////////////////////////////////////////
    // DEBUGGER INTEGRATION SUPPORT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .regGroupCB      = armRegGroup,
    .regInfoCB       = armRegInfo,
    .exceptionInfoCB = armExceptionInfo,
    .modeInfoCB      = armModeInfo,
    .getExceptionCB  = armGetException,
    .getModeCB       = armGetMode,
    .debugCB         = armDumpRegisters,

    ////////////////////////////////////////////////////////////////////////
    // PARAMETER SUPPORT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .paramSpecsCB     = armGetParamSpec,
    .paramValueSizeCB = armParamValueSize,

    ////////////////////////////////////////////////////////////////////////
    // PORT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .busPortSpecsCB   = armGetBusPortSpec,
    .netPortSpecsCB   = armGetNetPortSpec,

    ////////////////////////////////////////////////////////////////////////
    // IMPERAS INTERCEPTED FUNCTION SUPPORT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    .intReturnCB = armIntReturnCB,
    .intResultCB = armIntResultCB,
    .intParCB    = armIntParCB,

    ////////////////////////////////////////////////////////////////////////
    // PROCESSOR INFO ROUTINE
    ////////////////////////////////////////////////////////////////////////

    .procInfoCB  = armProcInfo,
};
