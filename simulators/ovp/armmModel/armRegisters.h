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

#ifndef ARM_REGISTERS_H
#define ARM_REGISTERS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"


////////////////////////////////////////////////////////////////////////////////
// REGISTER ACCESS MACROS
////////////////////////////////////////////////////////////////////////////////

// aliases for specific GPRs
#define ARM_REG_SP              13
#define ARM_REG_LR              14
#define ARM_REG_PC              15

// morph-time macros to calculate offsets to registers in an arm structure
#define ARM_CPU_OFFSET(_F)      VMI_CPU_OFFSET(armP, _F)
#define ARM_CPU_REG(_F)         VMI_CPU_REG(armP, _F)
#define ARM_CPU_REG_CONST(_F)   VMI_CPU_REG_CONST(armP, _F)
#define ARM_CPU_TEMP(_F)        VMI_CPU_TEMP(armP, _F)

// morph-time macros to calculate constant offsets to flags in an arm structure
#define ARM_ZF_CONST            ARM_CPU_REG_CONST(aflags.ZF)
#define ARM_NF_CONST            ARM_CPU_REG_CONST(aflags.NF)
#define ARM_CF_CONST            ARM_CPU_REG_CONST(aflags.CF)
#define ARM_VF_CONST            ARM_CPU_REG_CONST(aflags.VF)
#define ARM_HI_CONST            ARM_CPU_REG_CONST(oflags.HI)
#define ARM_LT_CONST            ARM_CPU_REG_CONST(oflags.LT)
#define ARM_LE_CONST            ARM_CPU_REG_CONST(oflags.LE)

// morph-time macros to calculate variable offsets to flags in an arm structure
#define ARM_AFLAGS              ARM_CPU_REG(aflags)
#define ARM_ZF                  ARM_CPU_REG(aflags.ZF)
#define ARM_NF                  ARM_CPU_REG(aflags.NF)
#define ARM_CF                  ARM_CPU_REG(aflags.CF)
#define ARM_VF                  ARM_CPU_REG(aflags.VF)
#define ARM_QF                  ARM_CPU_REG(oflags.QF)

// morph-time macros to calculate offsets to fields in an arm structure
#define ARM_REG(_R)             ARM_CPU_REG(regs[_R])
#define ARM_TREG(_R)            ARM_CPU_TEMP(regs[_R])
#define ARM_TEMP(_R)            ARM_CPU_TEMP(temps[_R])
#define ARM_SP                  ARM_REG(ARM_REG_SP)
#define ARM_LR                  ARM_REG(ARM_REG_LR)
#define ARM_PC                  ARM_TREG(ARM_REG_PC)

// morph-time macros to calculate offsets to control registers
#define ARM_PSR                 ARM_CPU_REG(sregs.PSR)
#define ARM_CONTROL             ARM_CPU_REG(sregs.CONTROL)
#define ARM_PRIMASK             ARM_CPU_REG(sregs.PRIMASK)
#define ARM_FAULTMASK           ARM_CPU_REG(sregs.FAULTMASK)
#define ARM_BASEPRI             ARM_CPU_REG(sregs.BASEPRI)
#define ARM_FPSCR               ARM_CPU_REG(sregs.FPSCR)

// morph-time macros to calculate offsets to banked registers in an arm structure
#define ARM_BANK_REG(_N, _SET)  ARM_CPU_REG(bank.R##_N##_##_SET)
#define ARM_BANK_SP             ARM_BANK_REG(13, process)

// morph-time macro to calculate offset to EA tag in an arm structure
#define ARM_EA_TAG              ARM_CPU_REG(exclusiveTag)

// morph-time macro to calculate offset to ITSTATE in an arm structure
#define ARM_IT_STATE            ARM_CPU_REG(itStateRT)

// morph-time macro to calculate offset to divide target index in an arm structure
#define ARM_DIVIDE_TARGET       ARM_CPU_REG(divideTarget)

// morph-time macro to calculate offset to disable reason, event register and
// pending interrupt state in an arm structure
#define ARM_DISABLE_REASON      ARM_CPU_REG(disableReason)
#define ARM_EVENT               ARM_CPU_REG(eventRegister)
#define ARM_PENDING             ARM_CPU_REG(pendingInterrupt)

// morph-time macros to calculate offsets to floating point registers in an arm
// structure
#define ARM_FP_FLAGS            ARM_CPU_REG(sdfpFlags)
#define ARM_FP_STICKY           ARM_CPU_REG(sdfpSticky)
#define ARM_BREG(_R)            ARM_CPU_REG(vregs.b[_R])
#define ARM_HREG(_R)            ARM_CPU_REG(vregs.h[_R])
#define ARM_WREG(_R)            ARM_CPU_REG(vregs.w[_R])
#define ARM_DREG(_R)            ARM_CPU_REG(vregs.d[_R])

#endif

