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

#ifndef ARM_STRUCTURE_H
#define ARM_STRUCTURE_H

// VMI header files
#include "vmi/vmiPorts.h"
#include "vmi/vmiTypes.h"

// model header files
#include "armConfig.h"
#include "armExceptionTypes.h"
#include "armSysRegisters.h"
#include "armMode.h"
#include "armTypeRefs.h"
#include "armVariant.h"

// processor debug flags
#define ARM_DISASSEMBLE_MASK    0x00000001
#define ARM_THUMB_MASK          0x00000002
#define ARM_DEBUG_MMU_MASK      0x00000004
#define ARM_DEBUG_EXCEPT_MASK   0x00000008
#define ARM_DUMP_SDFP_REG_MASK  0x00000010

#define ARM_DISASSEMBLE(_P)     ((_P)->flags & ARM_DISASSEMBLE_MASK)
#define ARM_THUMB(_P)           ((_P)->flags & ARM_THUMB_MASK)
#define ARM_DEBUG_MMU(_P)       ((_P)->flags & ARM_DEBUG_MMU_MASK)
#define ARM_DEBUG_EXCEPT(_P)    ((_P)->flags & ARM_DEBUG_EXCEPT_MASK)
#define ARM_DUMP_SDFP_REG(_P)   ((_P)->flags & ARM_DUMP_SDFP_REG_MASK)

#define ARM_GPR_BITS            32
#define ARM_GPR_BYTES           (ARM_GPR_BITS/8)
#define ARM_GPR_NUM             16
#define ARM_TEMP_NUM            12

#define ARM_VFP_REG_BITS        64
#define ARM_VFP_REG_BYTES       8
#define ARM_VFP16_REG_NUM       16
#define ARM_VFP32_REG_NUM       32

#define ARM_EXCEPT_NUM          512
#define ARM_EXCEPT_MASK_NUM     ((ARM_EXCEPT_NUM+31)/32)
#define ARM_INTERRUPT_NUM       496

#define ARM_NO_TAG              -1

// simulator compatibility modes
typedef enum armCompatModeE {
    COMPAT_ISA,             // conform to the documented ISA
    COMPAT_GDB,             // conform to gdb simulator
    COMPAT_CODE_SOURCERY    // conform to ARM CodeSourcery toolchain output (ignore BKPT)
} armCompatMode;

// arithmetic flags
typedef struct armArithFlagsS {
    Uns8 ZF;        // zero flag
    Uns8 NF;        // sign flag
    Uns8 CF;        // carry flag
    Uns8 VF;        // overflow flag
} armArithFlags;

// other flags
typedef struct armOtherFlagsS {
    Uns8 HI;        // hi flag (created on demand)
    Uns8 LT;        // lt flag (created on demand)
    Uns8 LE;        // le flag (created on demand)
    Uns8 QF;        // saturation flag
} armOtherFlags;

// ARM PSR (combines APSR, IPSR and EPSR)
typedef union armPSRU {
    struct {
        Uns32 exceptNum : 9;    // exception number
        Uns32 align4    : 1;    // 4 byte alignment (in stack)
        Uns32 IT72      : 6;    // if-then state, bits 7:2
        Uns32 GE        : 4;    // GE bits (unused when no DSP)
        Uns32 _u1       : 4;    // unused bits
        Uns32 T         : 1;    // Thumb mode bit
        Uns32 IT10      : 2;    // if-then state, bits 1:0
        Uns32 Q         : 1;    // DSP overflow/saturate flag
        Uns32 V         : 1;    // overflow flag
        Uns32 C         : 1;    // carry flag
        Uns32 Z         : 1;    // zero flag
        Uns32 N         : 1;    // sign flag
    } fields;
    Uns32 reg;
} armPSR;

// PSR Access Macros
#define PSR_FIELD(_A, _F)       ((_A)->sregs.PSR.fields._F)
#define IN_USER_MODE(_A)        (!PSR_FIELD(_A, exceptNum) && CONTROL_FIELD(_A, threadUnpriv))
#define IN_THUMB_MODE(_A)       PSR_FIELD(_A, T)
#define IN_HANDLER_MODE(_A)     (PSR_FIELD(_A, exceptNum) && True)
#define IN_BASE_MODE(_A)        IN_USER_MODE(_A)
#define IN_PRIV_MPU_MODE(_A)    (!IN_USER_MODE(_A) && MPU_ENABLED(_A))

// Masks for fields in PSR
#define PSR_FLAGS      0xf8000000
#define PSR_NZCV       0xf0000000
#define PSR_EXCEPT_NUM 0x000001ff
#define PSR_THUMB      0x01000000
#define PSR_IT10       0x06000000
#define PSR_IT72       0x0000fc00
#define PSR_GE         0x000f0000
#define PSR_GE3        0x00080000
#define PSR_GE2        0x00040000
#define PSR_GE1        0x00020000
#define PSR_GE0        0x00010000
#define PSR_GE32       (PSR_GE3|PSR_GE2)
#define PSR_GE10       (PSR_GE1|PSR_GE0)
#define PSR_GE30       (PSR_GE32|PSR_GE10)
#define PSR_IT         (PSR_IT72 | PSR_IT10)
#define PSR_ALL        (PSR_FLAGS | PSR_IT | PSR_THUMB | PSR_GE | PSR_EXCEPT_NUM)
#define PSR_NOT_FLAGS  (PSR_ALL & ~(PSR_FLAGS | PSR_GE))

// ARM CONTROL
typedef union armCONTROLU {
    struct {
        Uns32 threadUnpriv  :  1;    // is thread mode unprivileged?
        Uns32 useSP_Process :  1;    // use SP_process stack
        Uns32 FPCA          :  1;    // Is FP active in current context?
    } fields;
    Uns32 reg;
} armCONTROL;

// CONTROL Access Macros
#define CONTROL_FIELD(_A, _F)   ((_A)->sregs.CONTROL.fields._F)
#define USE_SP_PROCESS(_A)      CONTROL_FIELD(_A, useSP_Process)

// Masks for fields in CONTROL
#define CONTROL_FPCA    0x00000004

// ARM FPSCR
typedef union armFPSCRU {
    struct {
        Uns32 IOC    : 1;
        Uns32 DZC    : 1;
        Uns32 OFC    : 1;
        Uns32 UFC    : 1;
        Uns32 IXC    : 1;
        Uns32 _u1    : 2;
        Uns32 IDC    : 1;
        Uns32 _u2    : 14;
        Uns32 RMode  : 2;
        Uns32 FZ     : 1;
        Uns32 DN     : 1;
        Uns32 AHP    : 1;
        Uns32 _u3    : 1;
        Uns32 V      : 1;
        Uns32 C      : 1;
        Uns32 Z      : 1;
        Uns32 N      : 1;
    } fields;
    Uns32 reg;
} armFPSCR;

// FPSCR Access Macros
#define FPSCR_MASK   0xf7c0009f
#define FPSCR_REG(_A)           ((_A)->sregs.FPSCR.reg)
#define FPSCR_FIELD(_A, _F)     ((_A)->sregs.FPSCR.fields._F)

// Write Masks for fields in other control registers (with no structs)
#define PRIMASK_MASK   0x00000001
#define FAULTMASK_MASK 0x00000001
#define BASEPRI_MASK   0x000000ff

// Special register definitions
typedef struct armSpecialRegsS {
    armPSR     PSR;
    armCONTROL CONTROL;
    Uns32      PRIMASK;
    Uns32      FAULTMASK;
    Uns32      BASEPRI;
    armFPSCR   FPSCR;
} armSpecialRegs;

// Banked registers
typedef struct armBankRegsS {

    // process stack pointer
    Uns32 R13_process;

} armBankRegs;

typedef struct armDomainSetS {
    memDomainP external;        // external memory domain
    memDomainP vmPriv;          // virtual code domain, privileged mode
    memDomainP vmUser;          // virtual code domain, user mode
    memDomainP system;          // system domain
} armDomainSet, *armDomainSetP;

typedef enum armPIDSetE {
    APS_PHYS,                   // physical domains (privileged or user mode)
    APS_VM_P,                   // MMU/MPU-managed privileged mode domains
    APS_VM_U,                   // MMU/MPU-managed user mode domains
    APS_LAST                    // KEEP LAST: for sizing
} armPIDSet;

// decoder callback function to decode instruction at the passed address
#define ARM_DECODER_FN(_NAME) void _NAME( \
    armP                arm,     \
    Uns32               thisPC,  \
    armInstructionInfoP info     \
)
typedef ARM_DECODER_FN((*armDecoderFn));

// Callback function to return size of instruction at the passed for the specified mode
#define ARM_ISIZE_FN(_NAME) Uns32 _NAME( \
    armP  arm,     \
    Uns32 thisPC,  \
    Bool  isThumb  \
)
typedef ARM_ISIZE_FN((*armIsizeFn));

// opaque type for MPU protection region
typedef struct protRegionS *protRegionP;

// VFP register bank
typedef union armVFPRS {
    Uns8  b[ARM_VFP16_REG_NUM*8];  // when viewed as bytes
    Uns16 h[ARM_VFP16_REG_NUM*4];  // When viewed as 16 bit halfwords
    Uns32 w[ARM_VFP16_REG_NUM*2];  // when viewed as 32-bit words
    Uns64 d[ARM_VFP16_REG_NUM];    // when viewed as 64-bit double words
} armVFPR;

#define FP_REG(_A, _I)  ((_A)->vregs.w[(_I)])

// floating point control word type
typedef union armFPCWU {
    Uns32            u32;   // when viewed as composed value
    vmiFPControlWord cw;    // when viewed as fields
} armFPCW;

// processor structure
typedef struct armS {

    // TRUE PROCESSOR REGISTERS
    armArithFlags  aflags;              // arithmetic flags
    armOtherFlags  oflags;              // other flags
    Uns32          regs[ARM_GPR_NUM];   // current mode GPRs
    armSpecialRegs sregs;               // special purpose registers

    // SIMULATOR SUPPORT
    Uns32          temps[ARM_TEMP_NUM]; // temporary registers
    Uns8           itStateRT;           // if-then state (run time)
    Uns8           itStateMT;           // if-then state (morph time)
    Uns8           divideTarget;        // target of divide instruction
    Uns8           disableReason;       // reason why processor disabled
    Bool           eventRegister;       // event register
    Bool           pendingInterrupt;    // is interrupt pending?
    armMode        mode             :8; // current processor mode
    Bool           validHI          :1; // is hi flag valid?
    Bool           validLT          :1; // is lt flag valid?
    Bool           validLE          :1; // is le flag valid?
    Bool           checkEndian      :1; // check endian using blockMask?
    Bool           checkL4          :1; // check interwork using blockMask?
    Bool           checkUnaligned   :1; // check alignment mode using blockMask?
    Bool           disableTimerRVR0 :1; // disable timer (SYST_RVR==0)?
    Bool           sleepOnExit      :1; // sleeping at exception exit?
    Bool           denormalInput    :1; // input denormal sticky flag
    memEndian      instructionEndian:1; // instruction endianness
    armExceptCxt   exceptionContext :3; // exception context
    Uns32          exclusiveTag;        // tag for active exclusive access
    Uns32          exclusiveTagMask;    // mask to select exclusive tag bits
    Uns32          priorityMask;        // priority mask
    Uns32          exceptNum;           // total number of exceptions
    Uns32          exceptMaskNum;       // number of words in exception masks
    Uns32          timerModulus;        // modulus value for counter
    Uns64          timerBase;           // nominal counter base value

    // SYSTEM AND BANKED REGISTERS (INFREQUENTLY USED AT RUN TIME)
    armBankRegs    bank;                // banked registers
    armSCSRegs     scs;                 // SCS registers

    // VFP REGISTERS
    armFPCW        currentCW;           // current control word
    armArithFlags  sdfpAFlags;          // FPU comparison flags
    Uns8           sdfpFlags;           // FPU operation flags
    Uns8           sdfpSticky;          // FPU sticky flags
    armVFPR        vregs;               // VFP data registers

    // VARIANT CONFIGURATION
    Uns32          flags;               // configuration flags
    armConfig      configInfo;          // configuration register defaults
    armCompatMode  compatMode     :2;   // compatibility mode
    Bool           simEx          :1;   // simulate exceptions?
    Bool           verbose        :1;   // verbose messages enabled?
    Bool           showHiddenRegs :1;   // show hidden registers in reg dump
    Bool           UAL            :1;   // disassemble using UAL syntax
    Bool           disableBitBand :1;   // bit banding disabled

    // MEMORY SUBSYSTEM SUPPORT
    Bool           restoreDomain  :1;   // whether to resore domain (LDRT, STRT)
    protRegionP    impu;                // instruction/unified MPU
    protRegionP    dmpu;                // data MPU (if not unified)
    armDomainSet   ids;                 // instruction domain set
    armDomainSet   dds;                 // data domain set

    // NET HANDLES (used for NVIC)
    Uns32          sysResetReq;         // system reset requested
    Uns32          intISS;              // interrupt service started
    Uns32          eventOut;            // event output signal
    Uns32          lockup;              // lockup output signal

    // EXCEPTION SUPPORT
    Int32          unboostedPriority;           // unboosted execution priority
    Int32          executionPriority;           // current execution priority
    armExceptNum   enabledException;            // current enabled exception
    armExceptNum   pendingException;            // current pending exception
    armExceptNum   derivedException;            // pushStack/popStack exception
    Uns32          nestedActivation;            // used for handler->thread check
    Uns32          xPend[ARM_EXCEPT_MASK_NUM];  // pending exceptions
    Uns32          xActive[ARM_EXCEPT_MASK_NUM];// active exceptions
    Uns32          xEnable[ARM_EXCEPT_MASK_NUM];// exception enables
    Uns8           xPriority[ARM_EXCEPT_NUM];   // exception priorities
    vmiNetPortP    netPorts;                    // net ports on this variant
    memDomainP     FPCARdomain;                 // memory domain associated with the FPCAR

    // INTERCEPT LIBRARY SUPPORT
    armDecoderFn   decoderCB;           // generic instruction decoder
    armIsizeFn     isizeCB;             // instruction size callback

} arm;


#endif

