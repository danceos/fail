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

#ifndef ARM_SYS_REGISTERS_H
#define ARM_SYS_REGISTERS_H

// model header files
#include "armRegisters.h"


//
// Implementor codes
//
typedef enum armImplementorE {
    AI_ARM   = 0x41,
    AI_DEC   = 0x44,
    AI_INTEL = 0x69,
} armImplementor;

// -----------------------------------------------------------------------------
// SYSTEM REGISTERS
// -----------------------------------------------------------------------------

// construct enumeration member name from register name
#define SCS_ID(_R)                     SCS_ID_##_R

// morph-time macro to access a field in a system register
#define ARM_SCS_REG(_ID)               ARM_CPU_REG(scs.regs[_ID])

// access an entire system register as an Uns32
#define SCS_REG_UNS32(_P, _R)          ((_P)->scs.regs[SCS_ID(_R)])

// access an entire system register as a structure
#define SCS_REG_STRUCT(_P, _R)         ((_P)->scs.fields._R)

// access a field in a system register
#define SCS_FIELD(_P, _R, _F)          SCS_REG_STRUCT(_P, _R)._F

// get mask to use when writing a system register
#define SCS_MASK_UNS32(_P, _R)         ((_P)->configInfo.regMasks._R.value32)

// get mask to use when writing a system register
#define SCS_MASK_FIELD(_P, _R, _F)     ((_P)->configInfo.regMasks._R.fields._F)

// access default value for an entire system register as a structure
#define SCS_REG_STRUCT_DEFAULT(_P, _R) ((_P)->configInfo.regDefaults._R)

// access default value for a field in a system register
#define SCS_FIELD_DEFAULT(_P, _R, _F)  (SCS_REG_STRUCT_DEFAULT(_P, _R)._F)

// should CPUID registers be used to determine feature presence?
#define SCS_USE_CPUID(_P) (SCS_FIELD(_P, CPUID, ARCHITECTURE)==0xf)

// access to ID_ISAR field
#define ARM_ISAR(_N, _F) SCS_FIELD(arm, ID_ISAR##_N, _F)


//
// Identifiers for each implemented system register
//
typedef enum armSCSRegIdE {

    // this code defines an invalid system register specification
    SCS_ID(INVALID)=-1,

    // system control register entries (represented in processor structure)
    SCS_ID(ICTR),
    SCS_ID(ACTLR),
    SCS_ID(CPUID),
    SCS_ID(VTOR),
    SCS_ID(AIRCR),
    SCS_ID(SCR),
    SCS_ID(CCR),
    SCS_ID(CFSR),
    SCS_ID(HFSR),
    SCS_ID(AFSR),
    SCS_ID(MMAR),
    SCS_ID(BFAR),
    SCS_ID(CPACR),

    // timer control register entries (represented in processor structure)
    SCS_ID(SYST_CSR),
    SCS_ID(SYST_RVR),
    SCS_ID(SYST_CVR),
    SCS_ID(SYST_CALIB),

    // CPU Id register entries (represented in processor structure)
    SCS_ID(ID_PFR0),
    SCS_ID(ID_PFR1),
    SCS_ID(ID_DFR0),
    SCS_ID(ID_AFR0),
    SCS_ID(ID_MMFR0),
    SCS_ID(ID_MMFR1),
    SCS_ID(ID_MMFR2),
    SCS_ID(ID_MMFR3),
    SCS_ID(ID_ISAR0),
    SCS_ID(ID_ISAR1),
    SCS_ID(ID_ISAR2),
    SCS_ID(ID_ISAR3),
    SCS_ID(ID_ISAR4),
    SCS_ID(ID_ISAR5),

    // FP Registers (represented in processor structure)
    SCS_ID(FPCCR),
    SCS_ID(FPCAR),
    SCS_ID(FPDSCR),
    SCS_ID(MVFR0),
    SCS_ID(MVFR1),

    // MPU control register entries (represented in processor structure)
    SCS_ID(MPU_TYPE),
    SCS_ID(MPU_CONTROL),
    SCS_ID(MPU_RNR),

    // marker for pseudo-registers (not represented in processor structure)
    SCS_ID(FirstPseudoReg),

    // system control register entries
    SCS_ID(ICSR) = SCS_ID(FirstPseudoReg),
    SCS_ID(SHCSR),
    SCS_ID(SHPR1),
    SCS_ID(SHPR2),
    SCS_ID(SHPR3),
    SCS_ID(STIR),

    // NVIC operations
    SCS_ID(NVIC_ISERx16),
    SCS_ID(NVIC_ICERx16),
    SCS_ID(NVIC_ISPRx16),
    SCS_ID(NVIC_ICPRx16),
    SCS_ID(NVIC_IABRx16),
    SCS_ID(NVIC_IPRx255),

    // MPU operations
    SCS_ID(MPU_RBAR),
    SCS_ID(MPU_RASR),
    SCS_ID(MPU_RBAR_A1),
    SCS_ID(MPU_RASR_A1),
    SCS_ID(MPU_RBAR_A2),
    SCS_ID(MPU_RASR_A2),
    SCS_ID(MPU_RBAR_A3),
    SCS_ID(MPU_RASR_A3),

    // keep last (used to define size of the enumeration)
    SCS_ID(Size),

} armSCSRegId;

// use this to declare a register structure below
#define SCS_REG_STRUCT_DECL(_N) armSCSReg_##_N


// -----------------------------------------------------------------------------
// ICTR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 INTLINESNUM :  4;
    Uns32 _u1         : 28;
} SCS_REG_STRUCT_DECL(ICTR);

#define SCS_WRITE_MASK_ICTR 0x00000000
#define SCS_ADDRESS_ICTR    0x004

// -----------------------------------------------------------------------------
// ACTLR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} SCS_REG_STRUCT_DECL(ACTLR);

#define SCS_WRITE_MASK_ACTLR 0xffffffff
#define SCS_ADDRESS_ACTLR    0x008

// -----------------------------------------------------------------------------
// CPUID
// -----------------------------------------------------------------------------

typedef struct {
    Uns32          REVISION     :  4;
    Uns32          PARTNO       : 12;
    Uns32          ARCHITECTURE :  4;
    Uns32          VARIANT      :  4;
    armImplementor IMPLEMENTER  :  8;
} SCS_REG_STRUCT_DECL(CPUID);

#define SCS_WRITE_MASK_CPUID 0x00000000
#define SCS_ADDRESS_CPUID    0xd00

// -----------------------------------------------------------------------------
// ICSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 VECTACTIVE  : 9;
    Uns32 _u1         : 2;
    Uns32 RETTOBASE   : 1;
    Uns32 VECTPENDING : 9;
    Uns32 _u2         : 1;
    Uns32 ISRPENDING  : 1;
    Uns32 ISRPREEMPT  : 1;
    Uns32 _u3         : 1;
    Uns32 PENDSTCLR   : 1;
    Uns32 PENDSTSET   : 1;
    Uns32 PENDSVCLR   : 1;
    Uns32 PENDSVSET   : 1;
    Uns32 _u4         : 2;
    Uns32 NMIPENDSET  : 1;
} SCS_REG_STRUCT_DECL(ICSR);

#define SCS_WRITE_MASK_ICSR 0xffffffff
#define SCS_ADDRESS_ICSR    0xd04

// -----------------------------------------------------------------------------
// VTOR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1     :  7;
    Uns32 TBLOFF  : 22;
    Uns32 TBLBASE :  1;
    Uns32 _u2     :  2;
} SCS_REG_STRUCT_DECL(VTOR);

#define SCS_WRITE_MASK_VTOR 0x3fffff80
#define SCS_ADDRESS_VTOR    0xd08

// -----------------------------------------------------------------------------
// AIRCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 VECTRESET     :  1;   // TODO: not implemented, debug feature
    Uns32 VECTCLRACTIVE :  1;   // TODO: not implemented, debug feature
    Uns32 SYSRESETREQ   :  1;
    Uns32 _u1           :  5;
    Uns32 PRIGROUP      :  3;
    Uns32 _u2           :  4;
    Uns32 ENDIANNESS    :  1;
    Uns32 VECTKEY       : 16;
} SCS_REG_STRUCT_DECL(AIRCR);

#define SCS_WRITE_MASK_AIRCR 0xffff7fff
#define SCS_ADDRESS_AIRCR    0xd0c

// -----------------------------------------------------------------------------
// SCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1         :  1;
    Uns32 SLEEPONEXIT :  1;
    Uns32 SLEEPDEEP   :  1;
    Uns32 _u2         :  1;
    Uns32 SEVONPEND   :  1;
    Uns32 _u3         : 27;
} SCS_REG_STRUCT_DECL(SCR);

#define SCS_WRITE_MASK_SCR 0x00000016
#define SCS_ADDRESS_SCR    0xd10

// -----------------------------------------------------------------------------
// CCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 NONBASETHRDENA :  1;
    Uns32 USERSETMPEND   :  1;
    Uns32 _u1            :  1;
    Uns32 UNALIGN_TRP    :  1;
    Uns32 DIV_0_TRP      :  1;
    Uns32 _u2            :  3;
    Uns32 BFHFNMIGN      :  1;
    Uns32 STKALIGN       :  1;
    Uns32 _u3            : 22;
} SCS_REG_STRUCT_DECL(CCR);

#define SCS_WRITE_MASK_CCR 0x0000031b
#define SCS_ADDRESS_CCR    0xd14

// -----------------------------------------------------------------------------
// SHPR1 ... SHPR3
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_SHPR1 0x00ffffff
#define SCS_ADDRESS_SHPR1    0xd18
#define SCS_WRITE_MASK_SHPR2 0xff000000
#define SCS_ADDRESS_SHPR2    0xd1c
#define SCS_WRITE_MASK_SHPR3 0xffff00ff
#define SCS_ADDRESS_SHPR3    0xd20

// -----------------------------------------------------------------------------
// STIR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 INTID : 10;
    Uns32 _u1   : 22;
} SCS_REG_STRUCT_DECL(STIR);

#define SCS_WRITE_MASK_STIR 0xffffffff
#define SCS_ADDRESS_STIR    0xf00

// -----------------------------------------------------------------------------
// NVIC_ISER0 ... NVIC_ISER15
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_ISER 0xffffffff
#define SCS_ADDRESS_NVIC_ISER    0x100

// -----------------------------------------------------------------------------
// NVIC_ICER0 ... NVIC_ICER15
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_ICER 0xffffffff
#define SCS_ADDRESS_NVIC_ICER    0x180

// -----------------------------------------------------------------------------
// NVIC_ISPR0 ... NVIC_ISPR15
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_ISPR 0xffffffff
#define SCS_ADDRESS_NVIC_ISPR    0x200

// -----------------------------------------------------------------------------
// NVIC_ICPR0 ... NVIC_ICPR15
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_ICPR 0xffffffff
#define SCS_ADDRESS_NVIC_ICPR    0x280

// -----------------------------------------------------------------------------
// NVIC_IABR0 ... NVIC_IABR15
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_IABR 0xffffffff
#define SCS_ADDRESS_NVIC_IABR    0x300

// -----------------------------------------------------------------------------
// NVIC_IPR0 ... NVIC_IPR255
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_NVIC_IPR 0xffffffff
#define SCS_ADDRESS_NVIC_IPR    0x400

// -----------------------------------------------------------------------------
// SHCSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 MEMFAULTACT    :  1;
    Uns32 BUSFAULTACT    :  1;
    Uns32 _u1            :  1;
    Uns32 USGFAULTACT    :  1;
    Uns32 _u2            :  3;
    Uns32 SVCALLACT      :  1;
    Uns32 MONITORACT     :  1;
    Uns32 _u3            :  1;
    Uns32 PENDSVACT      :  1;
    Uns32 SYSTICKACT     :  1;
    Uns32 USGFAULTPENDED :  1;
    Uns32 MEMFAULTPENDED :  1;
    Uns32 BUSFAULTPENDED :  1;
    Uns32 SVCALLPENDED   :  1;
    Uns32 MEMFAULTENA    :  1;
    Uns32 BUSFAULTENA    :  1;
    Uns32 USGFAULTENA    :  1;
    Uns32 _u4            : 13;
} SCS_REG_STRUCT_DECL(SHCSR);

#define SCS_WRITE_MASK_SHCSR 0xffffffff
#define SCS_ADDRESS_SHCSR    0xd24

// -----------------------------------------------------------------------------
// CFSR
// -----------------------------------------------------------------------------

typedef struct {
                            // MMFSR
    Uns32 IACCVIOL    : 1;
    Uns32 DACCVIOL    : 1;
    Uns32 _u1         : 1;
    Uns32 MUNSTKERR   : 1;
    Uns32 MSTKERR     : 1;
    Uns32 _u2         : 2;
    Uns32 MMARVALID   : 1;
                            // BFSR
    Uns32 IBUSERR     : 1;
    Uns32 PRECISERR   : 1;
    Uns32 IMPRECISERR : 1;
    Uns32 UNSTKERR    : 1;
    Uns32 STKERR      : 1;
    Uns32 _u3         : 2;
    Uns32 BFARVALID   : 1;
                            // UFSR
    Uns32 UNDEFINSTR  : 1;
    Uns32 INVSTATE    : 1;
    Uns32 INVPC       : 1;
    Uns32 NOCP        : 1;
    Uns32 _u4         : 4;
    Uns32 UNALIGNED   : 1;
    Uns32 DIVBYZERO   : 1;
    Uns32 _u5         : 6;
} SCS_REG_STRUCT_DECL(CFSR);

#define SCS_WRITE_MASK_CFSR 0xffffffff
#define SCS_ADDRESS_CFSR    0xd28

// -----------------------------------------------------------------------------
// HFSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1      :  1;
    Uns32 VECTTBL  :  1;
    Uns32 _u2      : 28;
    Uns32 FORCED   :  1;
    Uns32 DEBUGEVT :  1;
} SCS_REG_STRUCT_DECL(HFSR);

#define SCS_WRITE_MASK_HFSR 0xffffffff
#define SCS_ADDRESS_HFSR    0xd2c

// -----------------------------------------------------------------------------
// AFSR
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_AFSR 0xffffffff
#define SCS_ADDRESS_AFSR    0xd3c

// -----------------------------------------------------------------------------
// MMAR
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_MMAR 0xffffffff
#define SCS_ADDRESS_MMAR    0xd34

// -----------------------------------------------------------------------------
// BFAR
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_BFAR 0xffffffff
#define SCS_ADDRESS_BFAR    0xd38

// -----------------------------------------------------------------------------
// CPACR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 cp0    :  2;
    Uns32 cp1    :  2;
    Uns32 cp2    :  2;
    Uns32 cp3    :  2;
    Uns32 cp4    :  2;
    Uns32 cp5    :  2;
    Uns32 cp6    :  2;
    Uns32 cp7    :  2;
    Uns32 _u1    :  4;
    Uns32 cp10   :  2;
    Uns32 cp11   :  2;
    Uns32 _u2    :  8;
} SCS_REG_STRUCT_DECL(CPACR);

#define SCS_WRITE_MASK_CPACR 0xffffffff    /* Actual mask used is from config data */
#define SCS_ADDRESS_CPACR    0xd88

// -----------------------------------------------------------------------------
// SYST_CSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ENABLE    :  1;
    Uns32 TICKINT   :  1;
    Uns32 CLKSOURCE :  1;
    Uns32 _u1       : 13;
    Uns32 COUNTFLAG :  1;
    Uns32 _u2       : 15;
} SCS_REG_STRUCT_DECL(SYST_CSR);

#define SCS_WRITE_MASK_SYST_CSR 0x00000007
#define SCS_ADDRESS_SYST_CSR    0x010

// -----------------------------------------------------------------------------
// SYST_RVR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 RELOAD : 24;
    Uns32 _u1    :  8;
} SCS_REG_STRUCT_DECL(SYST_RVR);

#define SCS_WRITE_MASK_SYST_RVR 0x00ffffff
#define SCS_ADDRESS_SYST_RVR    0x014

// -----------------------------------------------------------------------------
// SYST_CVR
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_SYST_CVR 0xffffffff
#define SCS_ADDRESS_SYST_CVR    0x018

// -----------------------------------------------------------------------------
// SYST_CALIB
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 TENMS : 24;
    Uns32 _u1   :  6;
    Uns32 SKEW  :  1;
    Uns32 NOREF :  1;
} SCS_REG_STRUCT_DECL(SYST_CALIB);

#define SCS_WRITE_MASK_SYST_CALIB 0x00000000
#define SCS_ADDRESS_SYST_CALIB    0x01c

// -----------------------------------------------------------------------------
// ID_PFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 State0 :  4;
    Uns32 State1 :  4;
    Uns32 State2 :  4;
    Uns32 State3 :  4;
    Uns32 _u1    : 16;
} SCS_REG_STRUCT_DECL(ID_PFR0);

#define SCS_WRITE_MASK_ID_PFR0 0x00000000
#define SCS_ADDRESS_ID_PFR0    0xd40

// -----------------------------------------------------------------------------
// ID_PFR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ProgrammersModel      :  4;
    Uns32 SecurityExtension     :  4;
    Uns32 MicroProgrammersModel :  4;
    Uns32 _u1                   : 20;
} SCS_REG_STRUCT_DECL(ID_PFR1);

#define SCS_WRITE_MASK_ID_PFR1 0x00000000
#define SCS_ADDRESS_ID_PFR1    0xd44

// -----------------------------------------------------------------------------
// ID_DFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CoreDebug     : 4;
    Uns32 SecureDebug   : 4;
    Uns32 EmbeddedDebug : 4;
    Uns32 TraceDebugCP  : 4;
    Uns32 TraceDebugMM  : 4;
    Uns32 MicroDebug    : 4;
    Uns32 _u1           : 8;
} SCS_REG_STRUCT_DECL(ID_DFR0);

#define SCS_WRITE_MASK_ID_DFR0 0x00000000
#define SCS_ADDRESS_ID_DFR0    0xd48

// -----------------------------------------------------------------------------
// ID_AFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} SCS_REG_STRUCT_DECL(ID_AFR0);

#define SCS_WRITE_MASK_ID_AFR0 0x00000000
#define SCS_ADDRESS_ID_AFR0    0xd4c

// -----------------------------------------------------------------------------
// ID_MMFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 VMSA        : 4;
    Uns32 PMSA        : 4;
    Uns32 Cache_Agent : 4;
    Uns32 Cache_DMA   : 4;
    Uns32 TCM_DMA     : 4;
    Uns32 AuxControl  : 4;
    Uns32 FCSE        : 4;
    Uns32 _u1         : 4;
} SCS_REG_STRUCT_DECL(ID_MMFR0);

#define SCS_WRITE_MASK_ID_MMFR0 0x00000000
#define SCS_ADDRESS_ID_MMFR0    0xd50

// -----------------------------------------------------------------------------
// ID_MMFR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 L1VAHarvard : 4;
    Uns32 L1VAUnified : 4;
    Uns32 L1SWHarvard : 4;
    Uns32 L1SWUnified : 4;
    Uns32 L1Harvard   : 4;
    Uns32 L1Unified   : 4;
    Uns32 L1TestClean : 4;
    Uns32 BTB         : 4;
} SCS_REG_STRUCT_DECL(ID_MMFR1);

#define SCS_WRITE_MASK_ID_MMFR1 0x00000000
#define SCS_ADDRESS_ID_MMFR1    0xd54

// -----------------------------------------------------------------------------
// ID_MMFR2
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 L1FgndPrefetchHarvard : 4;
    Uns32 L1BgndPrefetchHarvard : 4;
    Uns32 L1MaintRangeHarvard   : 4;
    Uns32 TLBMaintHarvard       : 4;
    Uns32 TLBMaintUnified       : 4;
    Uns32 MemoryBarrierCP15     : 4;
    Uns32 WaitForInterruptStall : 4;
    Uns32 HWAccessFlag          : 4;
} SCS_REG_STRUCT_DECL(ID_MMFR2);

#define SCS_WRITE_MASK_ID_MMFR2 0x00000000
#define SCS_ADDRESS_ID_MMFR2    0xd58

// -----------------------------------------------------------------------------
// ID_MMFR3
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 HierMaintSW  :  4;
    Uns32 HierMaintMVA :  4;
    Uns32 BPMaint      :  4;
    Uns32 _u1          : 20;
} SCS_REG_STRUCT_DECL(ID_MMFR3);

#define SCS_WRITE_MASK_ID_MMFR3 0x00000000
#define SCS_ADDRESS_ID_MMFR3    0xd5c

// -----------------------------------------------------------------------------
// ID_ISAR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Swap_instrs      : 4;
    Uns32 BitCount_instrs  : 4;
    Uns32 BitField_instrs  : 4;
    Uns32 CmpBranch_instrs : 4;
    Uns32 Coproc_instrs    : 4;
    Uns32 Debug_instrs     : 4;
    Uns32 Divide_instrs    : 4;
    Uns32 _u1              : 4;
} SCS_REG_STRUCT_DECL(ID_ISAR0);

#define SCS_WRITE_MASK_ID_ISAR0 0x00000000
#define SCS_ADDRESS_ID_ISAR0    0xd60

// -----------------------------------------------------------------------------
// ID_ISAR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Endian_instrs    : 4;
    Uns32 Except_instrs    : 4;
    Uns32 Except_AR_instrs : 4;
    Uns32 Extend_instrs    : 4;
    Uns32 IfThen_instrs    : 4;
    Uns32 Immediate_instrs : 4;
    Uns32 Interwork_instrs : 4;
    Uns32 Jazelle_instrs   : 4;
} SCS_REG_STRUCT_DECL(ID_ISAR1);

#define SCS_WRITE_MASK_ID_ISAR1 0x00000000
#define SCS_ADDRESS_ID_ISAR1    0xd64

// -----------------------------------------------------------------------------
// ID_ISAR2
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 LoadStore_instrs      : 4;
    Uns32 MemHint_instrs        : 4;
    Uns32 MultiAccessInt_instrs : 4;
    Uns32 Mult_instrs           : 4;
    Uns32 MultS_instrs          : 4;
    Uns32 MultU_instrs          : 4;
    Uns32 PSR_AR_instrs         : 4;
    Uns32 Reversal_instrs       : 4;
} SCS_REG_STRUCT_DECL(ID_ISAR2);

#define SCS_WRITE_MASK_ID_ISAR2 0x00000000
#define SCS_ADDRESS_ID_ISAR2    0xd68

// -----------------------------------------------------------------------------
// ID_ISAR3
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Saturate_instrs     : 4;
    Uns32 SIMD_instrs         : 4;
    Uns32 SVC_instrs          : 4;
    Uns32 SynchPrim_instrs    : 4;
    Uns32 TabBranch_instrs    : 4;
    Uns32 ThumbCopy_instrs    : 4;
    Uns32 TrueNOP_instrs      : 4;
    Uns32 T2ExeEnvExtn_instrs : 4;
} SCS_REG_STRUCT_DECL(ID_ISAR3);

#define SCS_WRITE_MASK_ID_ISAR3 0x00000000
#define SCS_ADDRESS_ID_ISAR3    0xd6c

// -----------------------------------------------------------------------------
// ID_ISAR4
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Unpriv_instrs         : 4;
    Uns32 WithShifts_instrs     : 4;
    Uns32 Writeback_instrs      : 4;
    Uns32 SMI_instrs            : 4;
    Uns32 Barrier_instrs        : 4;
    Uns32 SynchPrim_instrs_frac : 4;
    Uns32 PSR_M_instrs          : 4;
    Uns32 SWP_frac              : 4;
} SCS_REG_STRUCT_DECL(ID_ISAR4);

#define SCS_WRITE_MASK_ID_ISAR4 0x00000000
#define SCS_ADDRESS_ID_ISAR4    0xd70

// -----------------------------------------------------------------------------
// ID_ISAR5
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} SCS_REG_STRUCT_DECL(ID_ISAR5);

#define SCS_WRITE_MASK_ID_ISAR5 0x00000000
#define SCS_ADDRESS_ID_ISAR5    0xd74

// -----------------------------------------------------------------------------
// FPCCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 LSPACT     :  1;  // Blockmask bit
    Uns32 USER       :  1;
    Uns32 _u1        :  1;
    Uns32 THREAD     :  1;
    Uns32 HFRDY      :  1;
    Uns32 MMRDY      :  1;
    Uns32 BFRDY      :  1;
    Uns32 _u2        :  1;
    Uns32 MONRDY     :  1;
    Uns32 _u3        : 21;
    Uns32 LSPEN      :  1;
    Uns32 ASPEN      :  1;  // Blockmask bit
} SCS_REG_STRUCT_DECL(FPCCR);

#define SCS_WRITE_MASK_FPCCR 0xc000017b
#define SCS_ADDRESS_FPCCR    0xf34

// Bits in FPCCR that are part of the block mask
#define SCS_BLOCKMASK_FPCCR  0x80000001

// -----------------------------------------------------------------------------
// FPCAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1        :  3;
    Uns32 ADDRESS    :  29;
} SCS_REG_STRUCT_DECL(FPCAR);

#define SCS_WRITE_MASK_FPCAR 0xfffffff8
#define SCS_ADDRESS_FPCAR    0xf38

// -----------------------------------------------------------------------------
// FPDSCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1        :  22;
    Uns32 RMode      :  2;
    Uns32 FZ         :  1;
    Uns32 DN         :  1;
    Uns32 AHP        :  1;
    Uns32 _u2        :  5;
} SCS_REG_STRUCT_DECL(FPDSCR);

#define SCS_WRITE_MASK_FPDSCR 0x07c00000
#define SCS_ADDRESS_FPDSCR    0xf3c

// -----------------------------------------------------------------------------
// MVFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 A_SIMD_Registers  : 4;
    Uns32 SinglePrecision   : 4;
    Uns32 DoublePrecision   : 4;
    Uns32 VFP_ExceptionTrap : 4;
    Uns32 Divide            : 4;
    Uns32 SquareRoot        : 4;
    Uns32 ShortVectors      : 4;
    Uns32 VFP_RoundingModes : 4;
} SCS_REG_STRUCT_DECL(MVFR0);

#define SCS_WRITE_MASK_MVFR0 0x00000000
#define SCS_ADDRESS_MVFR0    0xf40

// -----------------------------------------------------------------------------
// MVFR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 FlushToZeroMode        : 4;
    Uns32 DefaultNaNMode         : 4;
    Uns32 _u1                    : 16;
    Uns32 VFP_HalfPrecision      : 4;
    Uns32 VFP_FusedMAC           : 4;
} SCS_REG_STRUCT_DECL(MVFR1);

#define SCS_WRITE_MASK_MVFR1 0x00000000
#define SCS_ADDRESS_MVFR1    0xf44

// -----------------------------------------------------------------------------
// MPU_TYPE
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 SEPARATE : 1;
    Uns32 _u1      : 7;
    Uns32 DREGION  : 8;
    Uns32 IREGION  : 8;
    Uns32 _u2      : 8;
} SCS_REG_STRUCT_DECL(MPU_TYPE);

#define SCS_WRITE_MASK_MPU_TYPE 0x00000000
#define SCS_ADDRESS_MPU_TYPE    0xd90

// -----------------------------------------------------------------------------
// MPU_CONTROL
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ENABLE     :  1;
    Uns32 HFNMIENA   :  1;
    Uns32 PRIVDEFENA :  1;
    Uns32 _u1        : 29;
} SCS_REG_STRUCT_DECL(MPU_CONTROL);

#define SCS_WRITE_MASK_MPU_CONTROL 0x00000007
#define SCS_ADDRESS_MPU_CONTROL    0xd94

// -----------------------------------------------------------------------------
// MPU_RNR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 REGION :  8;
    Uns32 _u1    : 24;
} SCS_REG_STRUCT_DECL(MPU_RNR);

#define SCS_WRITE_MASK_MPU_RNR 0x000000ff
#define SCS_ADDRESS_MPU_RNR    0xd98

// -----------------------------------------------------------------------------
// MPU_RBAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 REGION :  4;
    Uns32 VALID  :  1;
    Uns32 ADDR   : 27;
} SCS_REG_STRUCT_DECL(MPU_RBAR),
  SCS_REG_STRUCT_DECL(MPU_RBAR_A1),
  SCS_REG_STRUCT_DECL(MPU_RBAR_A2),
  SCS_REG_STRUCT_DECL(MPU_RBAR_A3);

#define SCS_MASK_MPU_RBAR_ADDR     0xffffffe0
#define SCS_WRITE_MASK_MPU_RBAR    0xffffffff
#define SCS_WRITE_MASK_MPU_RBAR_A1 0xffffffff
#define SCS_WRITE_MASK_MPU_RBAR_A2 0xffffffff
#define SCS_WRITE_MASK_MPU_RBAR_A3 0xffffffff
#define SCS_ADDRESS_MPU_RBAR       0xd9c
#define SCS_ADDRESS_MPU_RBAR_A1    0xda4
#define SCS_ADDRESS_MPU_RBAR_A2    0xdac
#define SCS_ADDRESS_MPU_RBAR_A3    0xdb4

// -----------------------------------------------------------------------------
// MPU_RASR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ENABLE : 1;
    Uns32 SIZE   : 5;
    Uns32 _u1    : 2;
    Uns32 SRD    : 8;
    Uns32 B      : 1;
    Uns32 C      : 1;
    Uns32 S      : 1;
    Uns32 TEX    : 3;
    Uns32 _u2    : 2;
    Uns32 AP     : 3;
    Uns32 _u3    : 1;
    Uns32 XN     : 1;
    Uns32 _u4    : 3;
} SCS_REG_STRUCT_DECL(MPU_RASR),
  SCS_REG_STRUCT_DECL(MPU_RASR_A1),
  SCS_REG_STRUCT_DECL(MPU_RASR_A2),
  SCS_REG_STRUCT_DECL(MPU_RASR_A3);

#define SCS_WRITE_MASK_MPU_RASR    0xffffffff
#define SCS_WRITE_MASK_MPU_RASR_A1 0xffffffff
#define SCS_WRITE_MASK_MPU_RASR_A2 0xffffffff
#define SCS_WRITE_MASK_MPU_RASR_A3 0xffffffff
#define SCS_ADDRESS_MPU_RASR       0xda0
#define SCS_ADDRESS_MPU_RASR_A1    0xda8
#define SCS_ADDRESS_MPU_RASR_A2    0xdb0
#define SCS_ADDRESS_MPU_RASR_A3    0xdb8


// -----------------------------------------------------------------------------
// IGNORED OPERATION WRITE MASK
// -----------------------------------------------------------------------------

#define SCS_WRITE_MASK_Ignored 0x00000000


// -----------------------------------------------------------------------------
// CONTAINER
// -----------------------------------------------------------------------------

// use this to define a field-based register entry in armSCSRegsU below
#define SCS_REG_DECL(_N) SCS_REG_STRUCT_DECL(_N) _N

// use this to define a plain register entry in armSCSRegsU below
#define SCS_UNS32_DECL(_N) Uns32 _N

//
// This type defines the entire implemented system register set
//
typedef union armSCSRegsU {

    Uns32 regs[SCS_ID(FirstPseudoReg)];     // use this for by-register access

    struct {                                // use this for by-field access
        // system register entries
        SCS_REG_DECL(ICTR);
        SCS_UNS32_DECL(ACTLR);
        SCS_REG_DECL(CPUID);
        SCS_REG_DECL(VTOR);
        SCS_REG_DECL(AIRCR);
        SCS_REG_DECL(SCR);
        SCS_REG_DECL(CCR);
        SCS_REG_DECL(CFSR);
        SCS_REG_DECL(HFSR);
        SCS_UNS32_DECL(AFSR);
        SCS_UNS32_DECL(MMAR);
        SCS_UNS32_DECL(BFAR);
        SCS_REG_DECL(CPACR);
        SCS_REG_DECL(SYST_CSR);
        SCS_REG_DECL(SYST_RVR);
        SCS_UNS32_DECL(SYST_CVR);
        SCS_REG_DECL(SYST_CALIB);
        SCS_REG_DECL(ID_PFR0);
        SCS_REG_DECL(ID_PFR1);
        SCS_REG_DECL(ID_DFR0);
        SCS_REG_DECL(ID_AFR0);
        SCS_REG_DECL(ID_MMFR0);
        SCS_REG_DECL(ID_MMFR1);
        SCS_REG_DECL(ID_MMFR2);
        SCS_REG_DECL(ID_MMFR3);
        SCS_REG_DECL(ID_ISAR0);
        SCS_REG_DECL(ID_ISAR1);
        SCS_REG_DECL(ID_ISAR2);
        SCS_REG_DECL(ID_ISAR3);
        SCS_REG_DECL(ID_ISAR4);
        SCS_REG_DECL(ID_ISAR5);
        SCS_REG_DECL(FPCCR);
        SCS_REG_DECL(FPCAR);
        SCS_REG_DECL(FPDSCR);
        SCS_REG_DECL(MVFR0);
        SCS_REG_DECL(MVFR1);
        SCS_REG_DECL(MPU_TYPE);
        SCS_REG_DECL(MPU_CONTROL);
        SCS_REG_DECL(MPU_RNR);
    } fields;

} armSCSRegs, *armSCSRegsP;

#endif
