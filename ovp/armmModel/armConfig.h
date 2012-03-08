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

#ifndef ARM_CONFIG_H
#define ARM_CONFIG_H

// basic number types
#include "hostapi/impTypes.h"

// model header files
#include "armSysRegisters.h"
#include "armTypeRefs.h"
#include "armVariant.h"

//
// Use this to define a write mask entry in the structure below
//
#define SCS_MASK_DECL(_N) union {      \
    Uns32                   value32;   \
    SCS_REG_STRUCT_DECL(_N) fields;    \
} _N

//
// This structure hold configuration information about an ARM variant
//
typedef struct armConfigS {

    // name of configuration
    const char *name;

    // configuration not held in system registers
    armArchitecture arch           :16; // specific ISA supported
    Uns32           numInterrupts  :16; // number of external interrupt lines
    Uns32           ERG            : 4; // exclusives reservation granule
    Bool            rotateUnaligned: 1; // rotate unaligned LDR/LDRT/SWP?
    Bool            align64as32    : 1; // align 64-bit load/store on 32-bit
    Bool            STRoffsetPC12  : 1; // STR/STM store PC with offset 12?
    Uns32           priorityBitsM1 : 3; // number of priority bits, minus 1

    // default values for system registers
    struct {
        SCS_REG_DECL(ICTR);
        SCS_REG_DECL(ACTLR);
        SCS_REG_DECL(CPUID);
        SCS_REG_DECL(CPACR);
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
        SCS_REG_DECL(MVFR0);
        SCS_REG_DECL(MVFR1);
        SCS_REG_DECL(MPU_TYPE);
    } regDefaults;

    // write masks for system registers
    struct {
        SCS_MASK_DECL(CPACR);
    } regMasks;

} armConfig;

DEFINE_CS(armConfig);

//
// This specifies configuration information for each supported variant
//
extern const struct armConfigS armConfigTable[];

//
// Predicates for system features
//

// is MPU enabled?
#define MPU_ENABLED(_A)     SCS_FIELD(_A, MPU_CONTROL, ENABLE)

// is MPU unified?
#define MPU_UNIFIED(_A)     (!SCS_FIELD(_A, MPU_TYPE, SEPARATE))

// is MPU present?
#define MPU_PRESENT(_A)     SCS_FIELD(_A, MPU_TYPE, DREGION)
#define MPUS_PRESENT(_A)    (MPU_PRESENT(_A) && !MPU_UNIFIED(_A))


// is alignment checking enabled?
#define ALIGN_ENABLED(_A)   SCS_FIELD(_A, CCR, UNALIGN_TRP)
#define DO_UNALIGNED(_A)    !ALIGN_ENABLED(_A)

// is Jazelle present?
#define JAZELLE_PRESENT(_A) ARM_SUPPORT((_A)->configInfo.arch, ARM_J)

// get number of priority bits supported
#define PRIORITY_BITS(_A)   ((_A)->configInfo.priorityBitsM1+1)

// get number of interrupt lines
#define NUM_INTERRUPTS(_A)  ((_A)->configInfo.numInterrupts)

// is FPU present?
#define FPU_PRESENT(_A)     SCS_FIELD((_A), MVFR0, A_SIMD_Registers)

// is DSP present?
#define DSP_PRESENT(_A)     (SCS_FIELD((_A), ID_ISAR3, SIMD_instrs)>2)

#endif

