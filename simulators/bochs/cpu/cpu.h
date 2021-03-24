/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001-2011  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
/////////////////////////////////////////////////////////////////////////

#ifndef BX_CPU_H
#  define BX_CPU_H 1

#include <setjmp.h>

// <TAG-DEFINES-DECODE-START>
// segment register encoding
#define BX_SEG_REG_ES    0
#define BX_SEG_REG_CS    1
#define BX_SEG_REG_SS    2
#define BX_SEG_REG_DS    3
#define BX_SEG_REG_FS    4
#define BX_SEG_REG_GS    5
// NULL now has to fit in 3 bits.
#define BX_SEG_REG_NULL  7
#define BX_NULL_SEG_REG(seg) ((seg) == BX_SEG_REG_NULL)
// <TAG-DEFINES-DECODE-END>

#define BX_16BIT_REG_AX 0
#define BX_16BIT_REG_CX 1
#define BX_16BIT_REG_DX 2
#define BX_16BIT_REG_BX 3
#define BX_16BIT_REG_SP 4
#define BX_16BIT_REG_BP 5
#define BX_16BIT_REG_SI 6
#define BX_16BIT_REG_DI 7

#define BX_32BIT_REG_EAX 0
#define BX_32BIT_REG_ECX 1
#define BX_32BIT_REG_EDX 2
#define BX_32BIT_REG_EBX 3
#define BX_32BIT_REG_ESP 4
#define BX_32BIT_REG_EBP 5
#define BX_32BIT_REG_ESI 6
#define BX_32BIT_REG_EDI 7

#define BX_64BIT_REG_RAX 0
#define BX_64BIT_REG_RCX 1
#define BX_64BIT_REG_RDX 2
#define BX_64BIT_REG_RBX 3
#define BX_64BIT_REG_RSP 4
#define BX_64BIT_REG_RBP 5
#define BX_64BIT_REG_RSI 6
#define BX_64BIT_REG_RDI 7

#define BX_64BIT_REG_R8  8
#define BX_64BIT_REG_R9  9
#define BX_64BIT_REG_R10 10
#define BX_64BIT_REG_R11 11
#define BX_64BIT_REG_R12 12
#define BX_64BIT_REG_R13 13
#define BX_64BIT_REG_R14 14
#define BX_64BIT_REG_R15 15

#if BX_SUPPORT_X86_64
# define BX_GENERAL_REGISTERS 16
#else
# define BX_GENERAL_REGISTERS 8
#endif

#define BX_TMP_REGISTER (BX_GENERAL_REGISTERS)

#define BX_16BIT_REG_IP  (BX_GENERAL_REGISTERS+1)
#define BX_32BIT_REG_EIP (BX_GENERAL_REGISTERS+1)
#define BX_64BIT_REG_RIP (BX_GENERAL_REGISTERS+1)

#define BX_NIL_REGISTER  (BX_GENERAL_REGISTERS+2)

#if defined(NEED_CPU_REG_SHORTCUTS)

/* WARNING:
   Only BX_CPU_C member functions can use these shortcuts safely!
   Functions that use the shortcuts outside of BX_CPU_C might work
   when BX_USE_CPU_SMF=1 but will fail when BX_USE_CPU_SMF=0
   (for example in SMP mode).
*/

// access to 8 bit general registers
#define AL (BX_CPU_THIS_PTR gen_reg[0].word.byte.rl)
#define CL (BX_CPU_THIS_PTR gen_reg[1].word.byte.rl)
#define DL (BX_CPU_THIS_PTR gen_reg[2].word.byte.rl)
#define BL (BX_CPU_THIS_PTR gen_reg[3].word.byte.rl)
#define AH (BX_CPU_THIS_PTR gen_reg[0].word.byte.rh)
#define CH (BX_CPU_THIS_PTR gen_reg[1].word.byte.rh)
#define DH (BX_CPU_THIS_PTR gen_reg[2].word.byte.rh)
#define BH (BX_CPU_THIS_PTR gen_reg[3].word.byte.rh)

#define TMP8L (BX_CPU_THIS_PTR gen_reg[BX_TMP_REGISTER].word.byte.rl)

// access to 16 bit general registers
#define AX (BX_CPU_THIS_PTR gen_reg[0].word.rx)
#define CX (BX_CPU_THIS_PTR gen_reg[1].word.rx)
#define DX (BX_CPU_THIS_PTR gen_reg[2].word.rx)
#define BX (BX_CPU_THIS_PTR gen_reg[3].word.rx)
#define SP (BX_CPU_THIS_PTR gen_reg[4].word.rx)
#define BP (BX_CPU_THIS_PTR gen_reg[5].word.rx)
#define SI (BX_CPU_THIS_PTR gen_reg[6].word.rx)
#define DI (BX_CPU_THIS_PTR gen_reg[7].word.rx)

// access to 16 bit instruction pointer
#define IP (BX_CPU_THIS_PTR gen_reg[BX_16BIT_REG_IP].word.rx)

#define TMP16 (BX_CPU_THIS_PTR gen_reg[BX_TMP_REGISTER].word.rx)

// accesss to 32 bit general registers
#define EAX (BX_CPU_THIS_PTR gen_reg[0].dword.erx)
#define ECX (BX_CPU_THIS_PTR gen_reg[1].dword.erx)
#define EDX (BX_CPU_THIS_PTR gen_reg[2].dword.erx)
#define EBX (BX_CPU_THIS_PTR gen_reg[3].dword.erx)
#define ESP (BX_CPU_THIS_PTR gen_reg[4].dword.erx)
#define EBP (BX_CPU_THIS_PTR gen_reg[5].dword.erx)
#define ESI (BX_CPU_THIS_PTR gen_reg[6].dword.erx)
#define EDI (BX_CPU_THIS_PTR gen_reg[7].dword.erx)

// access to 32 bit instruction pointer
#define EIP (BX_CPU_THIS_PTR gen_reg[BX_32BIT_REG_EIP].dword.erx)

#define TMP32 (BX_CPU_THIS_PTR gen_reg[BX_TMP_REGISTER].dword.erx)

#if BX_SUPPORT_X86_64
// accesss to 64 bit general registers
#define RAX (BX_CPU_THIS_PTR gen_reg[0].rrx)
#define RCX (BX_CPU_THIS_PTR gen_reg[1].rrx)
#define RDX (BX_CPU_THIS_PTR gen_reg[2].rrx)
#define RBX (BX_CPU_THIS_PTR gen_reg[3].rrx)
#define RSP (BX_CPU_THIS_PTR gen_reg[4].rrx)
#define RBP (BX_CPU_THIS_PTR gen_reg[5].rrx)
#define RSI (BX_CPU_THIS_PTR gen_reg[6].rrx)
#define RDI (BX_CPU_THIS_PTR gen_reg[7].rrx)
#define R8  (BX_CPU_THIS_PTR gen_reg[8].rrx)
#define R9  (BX_CPU_THIS_PTR gen_reg[9].rrx)
#define R10 (BX_CPU_THIS_PTR gen_reg[10].rrx)
#define R11 (BX_CPU_THIS_PTR gen_reg[11].rrx)
#define R12 (BX_CPU_THIS_PTR gen_reg[12].rrx)
#define R13 (BX_CPU_THIS_PTR gen_reg[13].rrx)
#define R14 (BX_CPU_THIS_PTR gen_reg[14].rrx)
#define R15 (BX_CPU_THIS_PTR gen_reg[15].rrx)

// access to 64 bit instruction pointer
#define RIP (BX_CPU_THIS_PTR gen_reg[BX_64BIT_REG_RIP].rrx)

#define TMP64 (BX_CPU_THIS_PTR gen_reg[BX_TMP_REGISTER].rrx)

// access to 64 bit MSR registers
#define MSR_FSBASE  (BX_CPU_THIS_PTR sregs[BX_SEG_REG_FS].cache.u.segment.base)
#define MSR_GSBASE  (BX_CPU_THIS_PTR sregs[BX_SEG_REG_GS].cache.u.segment.base)
#define MSR_STAR    (BX_CPU_THIS_PTR msr.star)
#define MSR_LSTAR   (BX_CPU_THIS_PTR msr.lstar)
#define MSR_CSTAR   (BX_CPU_THIS_PTR msr.cstar)
#define MSR_FMASK   (BX_CPU_THIS_PTR msr.fmask)
#define MSR_KERNELGSBASE   (BX_CPU_THIS_PTR msr.kernelgsbase)
#define MSR_TSC_AUX (BX_CPU_THIS_PTR msr.tsc_aux)
#endif

#if BX_SUPPORT_X86_64
#define BX_READ_8BIT_REGx(index,extended)  ((((index) & 4) == 0 || (extended)) ? \
  (BX_CPU_THIS_PTR gen_reg[index].word.byte.rl) : \
  (BX_CPU_THIS_PTR gen_reg[(index)-4].word.byte.rh))
#define BX_READ_64BIT_REG(index) (BX_CPU_THIS_PTR gen_reg[index].rrx)
#define BX_READ_64BIT_REG_HIGH(index) (BX_CPU_THIS_PTR gen_reg[index].dword.hrx)
#else
#define BX_READ_8BIT_REG(index)  (((index) & 4) ? \
  (BX_CPU_THIS_PTR gen_reg[(index)-4].word.byte.rh) : \
  (BX_CPU_THIS_PTR gen_reg[index].word.byte.rl))
#define BX_READ_8BIT_REGx(index,ext) BX_READ_8BIT_REG(index)
#endif

#define BX_READ_8BIT_REGH(index) (BX_CPU_THIS_PTR gen_reg[index].word.byte.rh)
#define BX_READ_16BIT_REG(index) (BX_CPU_THIS_PTR gen_reg[index].word.rx)
#define BX_READ_32BIT_REG(index) (BX_CPU_THIS_PTR gen_reg[index].dword.erx)

#define BX_WRITE_8BIT_REGH(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].word.byte.rh = val; \
}

#define BX_WRITE_16BIT_REG(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].word.rx = val; \
}

/*
#define BX_WRITE_32BIT_REG(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].dword.erx = val; \
}
*/

#if BX_SUPPORT_X86_64

#define BX_WRITE_8BIT_REGx(index, extended, val) {\
  if (((index) & 4) == 0 || (extended)) \
    BX_CPU_THIS_PTR gen_reg[index].word.byte.rl = val; \
  else \
    BX_CPU_THIS_PTR gen_reg[(index)-4].word.byte.rh = val; \
}

#define BX_WRITE_32BIT_REGZ(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].rrx = (Bit32u) val; \
}

#define BX_WRITE_64BIT_REG(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].rrx = val; \
}
#define BX_CLEAR_64BIT_HIGH(index) {\
  BX_CPU_THIS_PTR gen_reg[index].dword.hrx = 0; \
}

#else

#define BX_WRITE_8BIT_REG(index, val) {\
  if ((index) & 4) \
    BX_CPU_THIS_PTR gen_reg[(index)-4].word.byte.rh = val; \
  else \
    BX_CPU_THIS_PTR gen_reg[index].word.byte.rl = val; \
}
#define BX_WRITE_8BIT_REGx(index, ext, val) BX_WRITE_8BIT_REG(index, val)

// For x86-32, I just pretend this one is like the macro above,
// so common code can be used.
#define BX_WRITE_32BIT_REGZ(index, val) {\
  BX_CPU_THIS_PTR gen_reg[index].dword.erx = (Bit32u) val; \
}

#define BX_CLEAR_64BIT_HIGH(index)

#endif

#define CPL       (BX_CPU_THIS_PTR sregs[BX_SEG_REG_CS].selector.rpl)

#define USER_PL   (BX_CPU_THIS_PTR user_pl) /* CPL == 3 */

#if BX_SUPPORT_SMP
#define BX_CPU_ID (BX_CPU_THIS_PTR bx_cpuid)
#else
#define BX_CPU_ID (0)
#endif

#endif  // defined(NEED_CPU_REG_SHORTCUTS)

struct BxExceptionInfo {
  unsigned exception_type;
  unsigned exception_class;
  bx_bool push_error;
};

#define BX_DE_EXCEPTION   0 // Divide Error (fault)
#define BX_DB_EXCEPTION   1 // Debug (fault/trap)
#define BX_BP_EXCEPTION   3 // Breakpoint (trap)
#define BX_OF_EXCEPTION   4 // Overflow (trap)
#define BX_BR_EXCEPTION   5 // BOUND (fault)
#define BX_UD_EXCEPTION   6
#define BX_NM_EXCEPTION   7
#define BX_DF_EXCEPTION   8
#define BX_TS_EXCEPTION  10
#define BX_NP_EXCEPTION  11
#define BX_SS_EXCEPTION  12
#define BX_GP_EXCEPTION  13
#define BX_PF_EXCEPTION  14
#define BX_MF_EXCEPTION  16
#define BX_AC_EXCEPTION  17
#define BX_MC_EXCEPTION  18
#define BX_XM_EXCEPTION  19

#define BX_CPU_HANDLED_EXCEPTIONS  20

/* MSR registers */
#define BX_MSR_TSC                 0x010
#define BX_MSR_APICBASE            0x01b

#if BX_CPU_LEVEL >= 6
  #define BX_MSR_SYSENTER_CS       0x174
  #define BX_MSR_SYSENTER_ESP      0x175
  #define BX_MSR_SYSENTER_EIP      0x176
#endif

#define BX_MSR_DEBUGCTLMSR         0x1d9
#define BX_MSR_LASTBRANCHFROMIP    0x1db
#define BX_MSR_LASTBRANCHTOIP      0x1dc
#define BX_MSR_LASTINTOIP          0x1dd

#if BX_CPU_LEVEL >= 6
  #define BX_MSR_MTRRCAP           0x0fe
  #define BX_MSR_MTRRPHYSBASE0     0x200
  #define BX_MSR_MTRRPHYSMASK0     0x201
  #define BX_MSR_MTRRPHYSBASE1     0x202
  #define BX_MSR_MTRRPHYSMASK1     0x203
  #define BX_MSR_MTRRPHYSBASE2     0x204
  #define BX_MSR_MTRRPHYSMASK2     0x205
  #define BX_MSR_MTRRPHYSBASE3     0x206
  #define BX_MSR_MTRRPHYSMASK3     0x207
  #define BX_MSR_MTRRPHYSBASE4     0x208
  #define BX_MSR_MTRRPHYSMASK4     0x209
  #define BX_MSR_MTRRPHYSBASE5     0x20a
  #define BX_MSR_MTRRPHYSMASK5     0x20b
  #define BX_MSR_MTRRPHYSBASE6     0x20c
  #define BX_MSR_MTRRPHYSMASK6     0x20d
  #define BX_MSR_MTRRPHYSBASE7     0x20e
  #define BX_MSR_MTRRPHYSMASK7     0x20f
  #define BX_MSR_MTRRFIX64K_00000  0x250
  #define BX_MSR_MTRRFIX16K_80000  0x258
  #define BX_MSR_MTRRFIX16K_A0000  0x259
  #define BX_MSR_MTRRFIX4K_C0000   0x268
  #define BX_MSR_MTRRFIX4K_C8000   0x269
  #define BX_MSR_MTRRFIX4K_D0000   0x26a
  #define BX_MSR_MTRRFIX4K_D8000   0x26b
  #define BX_MSR_MTRRFIX4K_E0000   0x26c
  #define BX_MSR_MTRRFIX4K_E8000   0x26d
  #define BX_MSR_MTRRFIX4K_F0000   0x26e
  #define BX_MSR_MTRRFIX4K_F8000   0x26f
  #define BX_MSR_PAT               0x277
  #define BX_MSR_MTRR_DEFTYPE      0x2ff
#endif

#define BX_MSR_MAX_INDEX          0x1000

enum {
  BX_MEMTYPE_UC = 0,
  BX_MEMTYPE_WC,
  BX_MEMTYPE_RESERVED2,
  BX_MEMTYPE_RESERVED3,
  BX_MEMTYPE_WT,
  BX_MEMTYPE_WP,
  BX_MEMTYPE_WB
};

#if BX_SUPPORT_VMX
  #define BX_MSR_VMX_BASIC                0x480
  #define BX_MSR_VMX_PINBASED_CTRLS       0x481
  #define BX_MSR_VMX_PROCBASED_CTRLS      0x482
  #define BX_MSR_VMX_VMEXIT_CTRLS         0x483
  #define BX_MSR_VMX_VMENTRY_CTRLS        0x484
  #define BX_MSR_VMX_MISC                 0x485
  #define BX_MSR_VMX_CR0_FIXED0           0x486
  #define BX_MSR_VMX_CR0_FIXED1           0x487
  #define BX_MSR_VMX_CR4_FIXED0           0x488
  #define BX_MSR_VMX_CR4_FIXED1           0x489
  #define BX_MSR_VMX_VMCS_ENUM            0x48a
  #define BX_MSR_VMX_PROCBASED_CTRLS2     0x48b
  #define BX_MSR_VMX_MSR_VMX_EPT_VPID_CAP 0x48c
  #define BX_MSR_VMX_TRUE_PINBASED_CTRLS  0x48d
  #define BX_MSR_VMX_TRUE_PROCBASED_CTRLS 0x48e
  #define BX_MSR_VMX_TRUE_VMEXIT_CTRLS    0x48f
  #define BX_MSR_VMX_TRUE_VMENTRY_CTRLS   0x490
  #define BX_MSR_IA32_FEATURE_CONTROL     0x03A
  #define BX_MSR_IA32_SMM_MONITOR_CTL     0x09B
#endif

#if BX_SUPPORT_X86_64
#define BX_MSR_EFER             0xc0000080
#define BX_MSR_STAR             0xc0000081
#define BX_MSR_LSTAR            0xc0000082
#define BX_MSR_CSTAR            0xc0000083
#define BX_MSR_FMASK            0xc0000084
#define BX_MSR_FSBASE           0xc0000100
#define BX_MSR_GSBASE           0xc0000101
#define BX_MSR_KERNELGSBASE     0xc0000102
#define BX_MSR_TSC_AUX          0xc0000103
#endif

#define BX_MODE_IA32_REAL       0x0   // CR0.PE=0                |
#define BX_MODE_IA32_V8086      0x1   // CR0.PE=1, EFLAGS.VM=1   | EFER.LMA=0
#define BX_MODE_IA32_PROTECTED  0x2   // CR0.PE=1, EFLAGS.VM=0   |
#define BX_MODE_LONG_COMPAT     0x3   // EFER.LMA = 1, CR0.PE=1, CS.L=0
#define BX_MODE_LONG_64         0x4   // EFER.LMA = 1, CR0.PE=1, CS.L=1

extern const char* cpu_mode_string(unsigned cpu_mode);

#if BX_SUPPORT_X86_64
#define IsCanonical(offset) ((Bit64u)((((Bit64s)(offset)) >> (BX_LIN_ADDRESS_WIDTH-1)) + 1) < 2)
#endif

#define IsValidPhyAddr(addr) ((addr & BX_PHY_ADDRESS_RESERVED_BITS) == 0)

#if BX_SUPPORT_APIC
  #define BX_CPU_INTR  (BX_CPU_THIS_PTR INTR || BX_CPU_THIS_PTR lapic.INTR)
#else
  #define BX_CPU_INTR  (BX_CPU_THIS_PTR INTR)
#endif

#define CACHE_LINE_SIZE 64

class BX_CPU_C;
class BX_MEM_C;

#if BX_USE_CPU_SMF == 0
// normal member functions.  This can ONLY be used within BX_CPU_C classes.
// Anyone on the outside should use the BX_CPU macro (defined in bochs.h)
// instead.
#  define BX_CPU_THIS_PTR  this->
#  define BX_CPU_THIS      this
#  define BX_SMF
#  define BX_CPU_C_PREFIX  BX_CPU_C::
// with normal member functions, calling a member fn pointer looks like
// object->*(fnptr)(arg, ...);
// Since this is different from when SMF=1, encapsulate it in a macro.
#  define BX_CPU_CALL_METHOD(func, args) \
            (this->*((BxExecutePtr_tR) (func))) args
#  define BX_CPU_CALL_METHODR(func, args) \
            (this->*((BxResolvePtr_tR) (func))) args
#else
// static member functions.  With SMF, there is only one CPU by definition.
#  define BX_CPU_THIS_PTR  BX_CPU(0)->
#  define BX_CPU_THIS      BX_CPU(0)
#  define BX_SMF           static
#  define BX_CPU_C_PREFIX
#  define BX_CPU_CALL_METHOD(func, args) \
            ((BxExecutePtr_tR) (func)) args
#  define BX_CPU_CALL_METHODR(func, args) \
            ((BxResolvePtr_tR) (func)) args
#endif

#if BX_SUPPORT_SMP
// multiprocessor simulation, we need an array of cpus and memories
BOCHSAPI extern BX_CPU_C **bx_cpu_array;
#else
// single processor simulation, so there's one of everything
BOCHSAPI extern BX_CPU_C   bx_cpu;
#endif

// accessors for all eflags in bx_flags_reg_t
// The macro is used once for each flag bit
// Do not use for arithmetic flags !
#define DECLARE_EFLAG_ACCESSOR(name,bitnum)                     \
  BX_SMF BX_CPP_INLINE Bit32u  get_##name ();                   \
  BX_SMF BX_CPP_INLINE bx_bool getB_##name ();                  \
  BX_SMF BX_CPP_INLINE void assert_##name ();                   \
  BX_SMF BX_CPP_INLINE void clear_##name ();                    \
  BX_SMF BX_CPP_INLINE void set_##name (bx_bool val);

#define IMPLEMENT_EFLAG_ACCESSOR(name,bitnum)                   \
  BX_CPP_INLINE bx_bool BX_CPU_C::getB_##name () {              \
    return 1 & (BX_CPU_THIS_PTR eflags >> bitnum);              \
  }                                                             \
  BX_CPP_INLINE Bit32u  BX_CPU_C::get_##name () {               \
    return BX_CPU_THIS_PTR eflags & (1 << bitnum);              \
  }

#define IMPLEMENT_EFLAG_SET_ACCESSOR(name,bitnum)               \
  BX_CPP_INLINE void BX_CPU_C::assert_##name () {               \
    BX_CPU_THIS_PTR eflags |= (1<<bitnum);                      \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::clear_##name () {                \
    BX_CPU_THIS_PTR eflags &= ~(1<<bitnum);                     \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::set_##name (bx_bool val) {       \
    BX_CPU_THIS_PTR eflags =                                    \
      (BX_CPU_THIS_PTR eflags&~(1<<bitnum))|((val)<<bitnum);    \
  }

#if BX_SUPPORT_ALIGNMENT_CHECK && BX_CPU_LEVEL >= 4

#define IMPLEMENT_EFLAG_SET_ACCESSOR_AC(bitnum)                 \
  BX_CPP_INLINE void BX_CPU_C::assert_AC () {                   \
    BX_CPU_THIS_PTR eflags |= (1<<bitnum);                      \
    handleAlignmentCheck();                                     \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::clear_AC() {                     \
    BX_CPU_THIS_PTR eflags &= ~(1<<bitnum);                     \
    BX_CPU_THIS_PTR alignment_check_mask = 0;                   \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::set_AC(bx_bool val) {            \
    BX_CPU_THIS_PTR eflags =                                    \
      (BX_CPU_THIS_PTR eflags&~(1<<bitnum))|((val)<<bitnum);    \
    handleAlignmentCheck();                                     \
  }

#endif

#define IMPLEMENT_EFLAG_SET_ACCESSOR_VM(bitnum)                 \
  BX_CPP_INLINE void BX_CPU_C::assert_VM() {                    \
    set_VM(1);                                                  \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::clear_VM() {                     \
    set_VM(0);                                                  \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::set_VM(bx_bool val) {            \
    if (!long_mode()) {                                         \
      BX_CPU_THIS_PTR eflags =                                  \
        (BX_CPU_THIS_PTR eflags&~(1<<bitnum))|((val)<<bitnum);  \
      handleCpuModeChange();                                    \
    }                                                           \
  }

// assert async_event when RF, IF or TF is set
#define IMPLEMENT_EFLAG_SET_ACCESSOR_IF_RF_TF(name,bitnum)      \
  BX_CPP_INLINE void BX_CPU_C::assert_##name() {                \
    BX_CPU_THIS_PTR async_event = 1;                            \
    BX_CPU_THIS_PTR eflags |= (1<<bitnum);                      \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::clear_##name() {                 \
    BX_CPU_THIS_PTR eflags &= ~(1<<bitnum);                     \
  }                                                             \
  BX_CPP_INLINE void BX_CPU_C::set_##name(bx_bool val) {        \
    if (val) BX_CPU_THIS_PTR async_event = 1;                   \
    BX_CPU_THIS_PTR eflags =                                    \
      (BX_CPU_THIS_PTR eflags&~(1<<bitnum))|((val)<<bitnum);    \
  }

#define DECLARE_EFLAG_ACCESSOR_IOPL(bitnum)                     \
  BX_SMF BX_CPP_INLINE void set_IOPL(Bit32u val);               \
  BX_SMF BX_CPP_INLINE Bit32u  get_IOPL(void);                  

#define IMPLEMENT_EFLAG_ACCESSOR_IOPL(bitnum)                   \
  BX_CPP_INLINE void BX_CPU_C::set_IOPL(Bit32u val) {           \
    BX_CPU_THIS_PTR eflags &= ~(3<<12);                         \
    BX_CPU_THIS_PTR eflags |= ((3&val) << 12);                  \
  }                                                             \
  BX_CPP_INLINE Bit32u BX_CPU_C::get_IOPL() {                   \
    return 3 & (BX_CPU_THIS_PTR eflags >> 12);                  \
  }

#define EFlagsCFMask   (1 <<  0)
#define EFlagsPFMask   (1 <<  2)
#define EFlagsAFMask   (1 <<  4)
#define EFlagsZFMask   (1 <<  6)
#define EFlagsSFMask   (1 <<  7)
#define EFlagsTFMask   (1 <<  8)
#define EFlagsIFMask   (1 <<  9)
#define EFlagsDFMask   (1 << 10)
#define EFlagsOFMask   (1 << 11)
#define EFlagsIOPLMask (3 << 12)
#define EFlagsNTMask   (1 << 14)
#define EFlagsRFMask   (1 << 16)
#define EFlagsVMMask   (1 << 17)
#define EFlagsACMask   (1 << 18)
#define EFlagsVIFMask  (1 << 19)
#define EFlagsVIPMask  (1 << 20)
#define EFlagsIDMask   (1 << 21)

#define EFlagsOSZAPCMask \
    (EFlagsCFMask | EFlagsPFMask | EFlagsAFMask | EFlagsZFMask | EFlagsSFMask | EFlagsOFMask)

#define EFlagsOSZAPMask  \
    (EFlagsPFMask | EFlagsAFMask | EFlagsZFMask | EFlagsSFMask | EFlagsOFMask)

#define EFlagsValidMask  0x003f7fd5	// only supported bits for EFLAGS

#if BX_CPU_LEVEL >= 5
typedef struct
{
#if BX_SUPPORT_APIC
  bx_phy_address apicbase;
#endif

#if BX_SUPPORT_X86_64
  Bit64u star;
  Bit64u lstar;
  Bit64u cstar;
  Bit32u fmask;
  Bit64u kernelgsbase;
  Bit32u tsc_aux;
#endif

  // TSC: Time Stamp Counter
  // Instead of storing a counter and incrementing it every instruction, we
  // remember the time in ticks that it was reset to zero.  With a little
  // algebra, we can also support setting it to something other than zero.
  // Don't read this directly; use get_TSC and set_TSC to access the TSC.
  Bit64u tsc_last_reset;

  // SYSENTER/SYSEXIT instruction msr's
#if BX_CPU_LEVEL >= 6
  Bit32u sysenter_cs_msr;
  bx_address sysenter_esp_msr;
  bx_address sysenter_eip_msr;
#endif

#if BX_CPU_LEVEL >= 6
  Bit64u mtrrphys[16];
  Bit64u mtrrfix64k_00000;
  Bit64u mtrrfix16k[2];
  Bit64u mtrrfix4k[8];
  Bit16u mtrr_deftype;
  Bit64u pat;
#endif

#if BX_SUPPORT_VMX
  Bit32u ia32_feature_ctrl;
#endif

  /* TODO finish of the others */
} bx_regs_msr_t;
#endif

#define MAX_STD_CPUID_FUNCTION 14
#define MAX_EXT_CPUID_FUNCTION 9

// cpuid features (duplicated in disasm.h)
#define BX_CPU_X87              0x00000001        /* FPU (X87) instruction */
#define BX_CPU_486              0x00000002        /* 486 new instruction */
#define BX_CPU_PENTIUM          0x00000004        /* Pentium new instruction */
#define BX_CPU_P6               0x00000008        /* P6 new instruction */
#define BX_CPU_MMX              0x00000010        /* MMX instruction */
#define BX_CPU_3DNOW            0x00000020        /* 3DNow! instruction */
#define BX_CPU_FXSAVE_FXRSTOR   0x00000040        /* FXSAVE/FXRSTOR instruction */
#define BX_CPU_SYSENTER_SYSEXIT 0x00000080        /* SYSENTER/SYSEXIT instruction */
#define BX_CPU_CLFLUSH          0x00000100        /* CLFLUSH instruction */
#define BX_CPU_SSE              0x00000200        /* SSE  instruction */
#define BX_CPU_SSE2             0x00000400        /* SSE2 instruction */
#define BX_CPU_SSE3             0x00000800        /* SSE3 instruction */
#define BX_CPU_SSSE3            0x00001000        /* SSSE3 instruction */
#define BX_CPU_SSE4_1           0x00002000        /* SSE4_1 instruction */
#define BX_CPU_SSE4_2           0x00004000        /* SSE4_2 instruction */
#define BX_CPU_SSE4A            0x00008000        /* SSE4A instruction */
#define BX_CPU_MONITOR_MWAIT    0x00010000        /* MONITOR/MWAIT instruction */
#define BX_CPU_VMX              0x00020000        /* VMX instruction */
#define BX_CPU_SMX              0x00040000        /* SMX instruction */
#define BX_CPU_SVM              0x00080000        /* SVM instruction */
#define BX_CPU_XSAVE            0x00100000        /* XSAVE/XRSTOR extensions instruction */
#define BX_CPU_XSAVEOPT         0x00200000        /* XSAVEOPT instruction */
#define BX_CPU_AES_PCLMULQDQ    0x00400000        /* AES+PCLMULQDQ instructions */
#define BX_CPU_MOVBE            0x00800000        /* MOVBE Intel Atom(R) instruction */
#define BX_CPU_FSGSBASE         0x01000000        /* FS/GS BASE access instructions */
#define BX_CPU_AVX              0x02000000        /* AVX instruction */
#define BX_CPU_AVX_FMA          0x04000000        /* AVX FMA instruction */
#define BX_CPU_X86_64           0x08000000        /* x86-64 instruction */

struct cpuid_function_t {
  Bit32u eax;
  Bit32u ebx;
  Bit32u ecx;
  Bit32u edx;
};

#include "model_specific.h"
#include "crregs.h"
#include "descriptor.h"
#include "instr.h"
#include "lazy_flags.h"

// BX_TLB_SIZE: Number of entries in TLB
// BX_TLB_INDEX_OF(lpf): This macro is passed the linear page frame
//   (top 20 bits of the linear address.  It must map these bits to
//   one of the TLB cache slots, given the size of BX_TLB_SIZE.
//   There will be a many-to-one mapping to each TLB cache slot.
//   When there are collisions, the old entry is overwritten with
//   one for the newest access.

#define BX_TLB_SIZE 1024
#define BX_TLB_MASK ((BX_TLB_SIZE-1) << 12)
#define BX_TLB_INDEX_OF(lpf, len) ((((unsigned)(lpf) + (len)) & BX_TLB_MASK) >> 12)

typedef bx_ptr_equiv_t bx_hostpageaddr_t;

typedef struct {
  bx_address lpf;       // linear page frame
  bx_phy_address ppf;   // physical page frame
  bx_hostpageaddr_t hostPageAddr;
  Bit32u accessBits;
  Bit32u lpf_mask;      // linear address mask of the page size
} bx_TLB_entry;

#if BX_SUPPORT_X86_64
  #define LPF_MASK BX_CONST64(0xfffffffffffff000)
#else
  #define LPF_MASK (0xfffff000)
#endif

#if BX_PHY_ADDRESS_LONG
  #define PPF_MASK BX_CONST64(0xfffffffffffff000)
#else
  #define PPF_MASK (0xfffff000)
#endif

#define LPFOf(laddr)               ((laddr) & LPF_MASK)
#define PPFOf(laddr)               ((laddr) & PPF_MASK)

#define AlignedAccessLPFOf(laddr, alignment_mask) \
                  ((laddr) & (LPF_MASK | (alignment_mask)))

#define PAGE_OFFSET(laddr) ((Bit32u)(laddr) & 0xfff)

#include "icache.h"

// general purpose register
#if BX_SUPPORT_X86_64

#ifdef BX_BIG_ENDIAN
typedef struct {
  union {
    struct {
      Bit32u dword_filler;
      Bit16u  word_filler;
      union {
        Bit16u rx;
        struct {
          Bit8u rh;
          Bit8u rl;
        } byte;
      };
    } word;
    Bit64u rrx;
    struct {
      Bit32u hrx;  // hi 32 bits
      Bit32u erx;  // lo 32 bits
    } dword;
  };
} bx_gen_reg_t;
#else
typedef struct {
  union {
    struct {
      union {
        Bit16u rx;
        struct {
          Bit8u rl;
          Bit8u rh;
        } byte;
      };
      Bit16u  word_filler;
      Bit32u dword_filler;
    } word;
    Bit64u rrx;
    struct {
      Bit32u erx;  // lo 32 bits
      Bit32u hrx;  // hi 32 bits
    } dword;
  };
} bx_gen_reg_t;

#endif

#else  // #if BX_SUPPORT_X86_64

#ifdef BX_BIG_ENDIAN
typedef struct {
  union {
    struct {
      Bit32u erx;
    } dword;
    struct {
      Bit16u word_filler;
      union {
        Bit16u rx;
        struct {
          Bit8u rh;
          Bit8u rl;
        } byte;
      };
    } word;
  };
} bx_gen_reg_t;
#else
typedef struct {
  union {
    struct {
      Bit32u erx;
    } dword;
    struct {
      union {
        Bit16u rx;
        struct {
          Bit8u rl;
          Bit8u rh;
        } byte;
      };
      Bit16u word_filler;
    } word;
  };
} bx_gen_reg_t;
#endif

#endif  // #if BX_SUPPORT_X86_64

#if BX_SUPPORT_APIC
#include "apic.h"
#endif

#if BX_SUPPORT_FPU
#include "cpu/i387.h"
#include "cpu/xmm.h"
#endif

#if BX_SUPPORT_VMX
#include "vmx.h"
#endif

#if BX_SUPPORT_MONITOR_MWAIT
struct monitor_addr_t {

    bx_phy_address monitor_begin;
    bx_phy_address monitor_end;
    bx_bool armed;

    monitor_addr_t():
      monitor_begin(0xffffffff), monitor_end(0xffffffff), armed(0) {}

    BX_CPP_INLINE void arm(bx_phy_address addr) {
      monitor_begin = addr;
      monitor_end = addr + CACHE_LINE_SIZE - 1;
      armed = 1;
    }

    BX_CPP_INLINE void reset_monitor(void) { armed = 0; }
};
#endif

class BOCHSAPI BX_CPU_C : public logfunctions {

public: // for now...

  unsigned bx_cpuid;

  // cpuid
  cpuid_function_t cpuid_std_function[MAX_STD_CPUID_FUNCTION];
  cpuid_function_t cpuid_ext_function[MAX_EXT_CPUID_FUNCTION];

  Bit32u isa_extensions_bitmask;

#define BX_CPU_SUPPORT_ISA_EXTENSION(feature) \
   (BX_CPU_THIS_PTR isa_extensions_bitmask & (feature))

  // General register set
  // rax: accumulator
  // rbx: base
  // rcx: count
  // rdx: data
  // rbp: base pointer
  // rsi: source index
  // rdi: destination index
  // esp: stack pointer
  // r8..r15 x86-64 extended registers
  // rip: instruction pointer
  // nil: null register
  // tmp: temp register
  bx_gen_reg_t gen_reg[BX_GENERAL_REGISTERS+3];

  /* 31|30|29|28| 27|26|25|24| 23|22|21|20| 19|18|17|16
   * ==|==|=====| ==|==|==|==| ==|==|==|==| ==|==|==|==
   *  0| 0| 0| 0|  0| 0| 0| 0|  0| 0|ID|VP| VF|AC|VM|RF
   *
   * 15|14|13|12| 11|10| 9| 8|  7| 6| 5| 4|  3| 2| 1| 0
   * ==|==|=====| ==|==|==|==| ==|==|==|==| ==|==|==|==
   *  0|NT| IOPL| OF|DF|IF|TF| SF|ZF| 0|AF|  0|PF| 1|CF
   */
  Bit32u eflags; // Raw 32-bit value in x86 bit position.

  // status and control flags register set
  Bit32u lf_flags_status;
  bx_lf_flags_entry oszapc;

  // so that we can back up when handling faults, exceptions, etc.
  // we need to store the value of the instruction pointer, before
  // each fetch/execute cycle.
  bx_address prev_rip;
  bx_address prev_rsp;
  bx_bool    speculative_rsp;

#define BX_INHIBIT_INTERRUPTS        0x01
#define BX_INHIBIT_DEBUG             0x02
#define BX_INHIBIT_INTERRUPTS_SHADOW 0x04
#define BX_INHIBIT_DEBUG_SHADOW      0x08

#define BX_INHIBIT_INTERRUPTS_BY_MOVSS        \
    (BX_INHIBIT_INTERRUPTS | BX_INHIBIT_DEBUG)
#define BX_INHIBIT_INTERRUPTS_BY_MOVSS_SHADOW \
    (BX_INHIBIT_INTERRUPTS_SHADOW | BX_INHIBIT_DEBUG_SHADOW)

  // What events to inhibit at any given time.  Certain instructions
  // inhibit interrupts, some debug exceptions and single-step traps.
  unsigned inhibit_mask;

  /* user segment register set */
  bx_segment_reg_t  sregs[6];

  /* system segment registers */
  bx_global_segment_reg_t gdtr; /* global descriptor table register */
  bx_global_segment_reg_t idtr; /* interrupt descriptor table register */
  bx_segment_reg_t        ldtr; /* local descriptor table register */
  bx_segment_reg_t        tr;   /* task register */

  /* debug registers DR0-DR7 */
#if BX_CPU_LEVEL >= 3
  bx_address dr[4]; /* DR0-DR3 */
  Bit32u     dr6;
  Bit32u     dr7;
#endif

  /* TR3 - TR7 (Test Register 3-7), unimplemented */

  /* Control registers */
  bx_cr0_t       cr0;
  bx_address     cr2;
  bx_address     cr3;
#if BX_CPU_LEVEL >= 4
  bx_cr4_t       cr4;
#endif

#if BX_SUPPORT_X86_64
  bx_efer_t efer;
#endif

#if BX_CPU_LEVEL >= 6
  xcr0_t xcr0;
#endif

  /* SMM base register */
  Bit32u smbase;

#if BX_CPU_LEVEL >= 5
  bx_regs_msr_t msr;
#endif

#if BX_CONFIGURE_MSRS
  MSR *msrs[BX_MSR_MAX_INDEX];
#endif

#if BX_SUPPORT_FPU
  i387_t the_i387;
#endif

#if BX_CPU_LEVEL >= 6
  bx_xmm_reg_t xmm[BX_XMM_REGISTERS+1];
  bx_mxcsr_t mxcsr;
  Bit32u mxcsr_mask;
#endif

#if BX_SUPPORT_MONITOR_MWAIT
  monitor_addr_t monitor;
#endif

#if BX_SUPPORT_APIC
  bx_local_apic_c lapic;
#endif

#if BX_SUPPORT_VMX
  bx_bool in_event;
  bx_bool in_vmx;
  bx_bool in_vmx_guest;
  bx_bool in_smm_vmx; // save in_vmx and in_vmx_guest flags when in SMM mode
  bx_bool in_smm_vmx_guest;
  bx_bool vmx_interrupt_window;
  Bit64u  vmcsptr;
  bx_hostpageaddr_t vmcshostptr;
  Bit64u  vmxonptr;
  
  VMCS_CACHE vmcs;
#endif

  bx_bool EXT; /* 1 if processing external interrupt or exception
                * or if not related to current instruction,
                * 0 if current CS:IP caused exception */
  unsigned errorno;   /* signal exception during instruction emulation */

#define BX_ACTIVITY_STATE_ACTIVE        (0)
#define BX_ACTIVITY_STATE_HLT           (1)
#define BX_ACTIVITY_STATE_SHUTDOWN      (2)
#define BX_ACTIVITY_STATE_WAIT_FOR_SIPI (3)
#define BX_ACTIVITY_STATE_MWAIT         (4)
#define BX_ACTIVITY_STATE_MWAIT_IF      (5)
  unsigned activity_state;

#define BX_DEBUG_DR_ACCESS_BIT          (1 << 13)
#define BX_DEBUG_SINGLE_STEP_BIT        (1 << 14)
#define BX_DEBUG_TRAP_TASK_SWITCH_BIT   (1 << 15)
  Bit32u   debug_trap; // holds DR6 value (16bit) to be set as well

  volatile Bit32u  async_event;

#if BX_SUPPORT_TRACE_CACHE
  #define BX_ASYNC_EVENT_STOP_TRACE (0x80000000)
#endif

#if BX_X86_DEBUGGER
  bx_bool  in_repeat;
#endif
  bx_bool  in_smm;
  unsigned cpu_mode;
  bx_bool  user_pl;
  volatile bx_bool INTR;
  volatile bx_bool pending_SMI;
  volatile bx_bool pending_NMI;
  volatile bx_bool pending_INIT;
  bx_bool  disable_SMI;
  bx_bool  disable_NMI;
  bx_bool  disable_INIT;
#if BX_CPU_LEVEL >= 5
  bx_bool  ignore_bad_msrs;
#endif
#if BX_CPU_LEVEL >= 6
  unsigned sse_ok;
#endif

  // for exceptions
  jmp_buf jmp_buf_env;
  Bit8u curr_exception;

  // Boundaries of current page, based on EIP
  bx_address eipPageBias;
  Bit32u     eipPageWindowSize;
  const Bit8u *eipFetchPtr;
  bx_phy_address pAddrPage; // Guest physical address of current instruction page

#if BX_CPU_LEVEL >= 4 && BX_SUPPORT_ALIGNMENT_CHECK
  unsigned alignment_check_mask;
#endif

#if BX_DEBUGGER
  bx_phy_address watchpoint;
  Bit8u break_point;
  Bit8u magic_break;
  Bit8u stop_reason;
  bx_bool trace_reg;
  bx_bool trace_mem;
  bx_bool mode_break;
  unsigned show_flag;
  bx_guard_found_t guard_found;
#endif
  Bit8u trace;

  // for paging
  struct {
    bx_TLB_entry entry[BX_TLB_SIZE] BX_CPP_AlignN(16);
#if BX_CPU_LEVEL >= 5
    bx_bool split_large;
#endif
  } TLB;

#if BX_CPU_LEVEL >= 6
  struct {
    bx_bool valid;
    Bit64u entry[4];
  } PDPTR_CACHE;
#endif

  // An instruction cache.  Each entry should be exactly 32 bytes, and
  // this structure should be aligned on a 32-byte boundary to be friendly
  // with the host cache lines.
  bxICache_c iCache BX_CPP_AlignN(32);
  Bit32u fetchModeMask;

  struct {
    bx_address rm_addr;       // The address offset after resolution
    bx_phy_address paddress1; // physical address after translation of 1st len1 bytes of data
    bx_phy_address paddress2; // physical address after translation of 2nd len2 bytes of data
    Bit32u len1;              // Number of bytes in page 1
    Bit32u len2;              // Number of bytes in page 2
    bx_ptr_equiv_t pages;     // Number of pages access spans (1 or 2).  Also used
                              // for the case when a native host pointer is
                              // available for the R-M-W instructions.  The host
                              // pointer is stuffed here.  Since this field has
                              // to be checked anyways (and thus cached), if it
                              // is greated than 2 (the maximum possible for
                              // normal cases) it is a native pointer and is used
                              // for a direct write access.
  } address_xlation;

  BX_SMF void setEFlags(Bit32u val) BX_CPP_AttrRegparmN(1);

#define ArithmeticalFlag(flag, lfMask, eflagsBitShift) \
  BX_SMF bx_bool get_##flag##Lazy(void); \
  BX_SMF bx_bool getB_##flag(void) { \
    if ((BX_CPU_THIS_PTR lf_flags_status & (lfMask)) == 0) \
      return (BX_CPU_THIS_PTR eflags >> eflagsBitShift) & 1; \
    else \
      return !!get_##flag##Lazy(); \
  } \
  BX_SMF bx_bool get_##flag(void) { \
    if ((BX_CPU_THIS_PTR lf_flags_status & (lfMask)) == 0) \
      return BX_CPU_THIS_PTR eflags & (lfMask); \
    else \
      return get_##flag##Lazy(); \
  } \
  BX_SMF void set_##flag(bx_bool val) { \
    BX_CPU_THIS_PTR lf_flags_status &= ~(lfMask); \
    BX_CPU_THIS_PTR eflags &= ~(lfMask); \
    BX_CPU_THIS_PTR eflags |= ((val)<<eflagsBitShift); \
  } \
  BX_SMF void clear_##flag(void) { \
    BX_CPU_THIS_PTR lf_flags_status &= ~(lfMask); \
    BX_CPU_THIS_PTR eflags &= ~(lfMask); \
  } \
  BX_SMF void assert_##flag(void) { \
    BX_CPU_THIS_PTR lf_flags_status &= ~(lfMask); \
    BX_CPU_THIS_PTR eflags |= (lfMask); \
  } \
  BX_SMF void force_##flag(void) { \
    if ((BX_CPU_THIS_PTR lf_flags_status & (lfMask)) != 0) { \
      set_##flag(!!get_##flag##Lazy()); \
    } \
  }

  ArithmeticalFlag(OF, EFlagsOFMask, 11);
  ArithmeticalFlag(SF, EFlagsSFMask,  7);
  ArithmeticalFlag(ZF, EFlagsZFMask,  6);
  ArithmeticalFlag(AF, EFlagsAFMask,  4);
  ArithmeticalFlag(PF, EFlagsPFMask,  2);
  ArithmeticalFlag(CF, EFlagsCFMask,  0);

  BX_SMF BX_CPP_INLINE void set_PF_base(Bit8u val);

  // constructors & destructors...
  BX_CPU_C(unsigned id = 0);
 ~BX_CPU_C();

  void initialize(void);
  void after_restore_state(void);
  void register_state(void);
#if BX_WITH_WX
  void register_wx_state(void);
#endif
  static Bit64s param_save_handler(void *devptr, bx_param_c *param);
  static void param_restore_handler(void *devptr, bx_param_c *param, Bit64s val);
#if !BX_USE_CPU_SMF
  Bit64s param_save(bx_param_c *param);
  void param_restore(bx_param_c *param, Bit64s val);
#endif

// <TAG-CLASS-CPU-START>
  // prototypes for CPU instructions...
  BX_SMF void ADD_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PUSH16_CS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH16_DS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP16_DS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH16_ES(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP16_ES(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH16_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP16_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH16_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP16_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH16_SS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP16_SS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PUSH32_CS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH32_DS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP32_DS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH32_ES(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP32_ES(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH32_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP32_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH32_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP32_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH32_SS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP32_SS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void DAA(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DAS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AAA(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AAS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AAM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AAD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PUSHAD32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSHAD16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPAD32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPAD16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ARPL_EwGw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_Id(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INSB32_YbDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSB16_YbDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSW32_YwDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSW16_YwDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSD32_YdDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSD16_YdDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSB32_DXXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSB16_DXXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSW32_DXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSW16_DXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSD32_DXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSD16_DXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void REP_INSB_YbDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_INSW_YwDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_INSD_YdDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_OUTSB_DXXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_OUTSW_DXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_OUTSD_DXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BOUND_GwMa(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BOUND_GdMa(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void TEST_EbGbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XCHG_EbGbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XCHG_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GbEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GwEwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV32_GdEdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV32_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_EwSwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EwSwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_SwEw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LEA_GdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LEA_GwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CBW(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CWD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL32_Ap(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL16_Ap(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSHF_Fw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPF_Fw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSHF_Fd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPF_Fd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAHF(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LAHF(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_ALOd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EAXOd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_AXOd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OdAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OdEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OdAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_EAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_AXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // repeatable instructions
  BX_SMF void REP_MOVSB_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_MOVSW_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_MOVSD_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_CMPSB_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_CMPSW_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_CMPSD_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_STOSB_YbAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_LODSB_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_SCASB_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_STOSW_YwAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_LODSW_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_SCASW_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_STOSD_YdEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_LODSD_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_SCASD_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // qualified by address size
  BX_SMF void CMPSB16_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSW16_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSD16_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSB32_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSW32_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSD32_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SCASB16_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASW16_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASD16_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASB32_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASW32_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASD32_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LODSB16_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSW16_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSD16_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSB32_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSW32_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSD32_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void STOSB16_YbAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSW16_YwAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSD16_YdEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSB32_YbAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSW32_YwAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSD32_YdEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVSB16_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSW16_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSD16_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSB32_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSW32_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSD32_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ENTER16_IwIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ENTER32_IwIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LEAVE16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LEAVE32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INT1(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INT3(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INT_Ib(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INTO(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IRET32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IRET16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SALC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XLAT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LOOPNE16_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOPE16_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOP16_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOPNE32_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOPE32_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOP32_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JCXZ_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JECXZ_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_ALIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_AXIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_EAXIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_IbAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_IbAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_IbEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_Ap(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_ALDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_AXDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IN_EAXDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_DXAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_DXAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUT_DXEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void HLT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CLC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CLI(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STI(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CLD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LAR_GvEw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LSL_GvEw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CLTS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INVD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void WBINVD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CLFLUSH(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_CR0Rd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR2Rd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR3Rd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR4Rd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdCR0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdCR2(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdCR3(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdCR4(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_DdRd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdDd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_TdRd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RdTd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void JO_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNO_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JB_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNB_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JZ_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNZ_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JBE_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNBE_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JS_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNS_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JP_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNP_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JL_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNL_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JLE_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNLE_Jw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void JO_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNO_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JB_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNB_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JZ_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNZ_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JBE_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNBE_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JS_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNS_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JP_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNP_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JL_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNL_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JLE_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNLE_Jd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SETO_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNO_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETB_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNB_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETZ_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNZ_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETBE_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNBE_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETS_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNS_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETP_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNP_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETL_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNL_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETLE_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNLE_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SETO_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNO_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETB_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNB_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETZ_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNZ_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETBE_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNBE_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETS_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNS_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETP_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNP_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETL_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNL_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETLE_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SETNLE_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CPUID(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SHRD_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHRD_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHLD_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHLD_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHRD_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHRD_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHLD_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHLD_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BSF_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BSF_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BSR_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BSR_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BT_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BT_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EwIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BT_EdIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EwIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EdIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EwIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EdIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EwIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EdIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EwIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BT_EdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EwIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EwIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EwIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LES_GwMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LDS_GwMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LSS_GwMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LFS_GwMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LGS_GwMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LES_GdMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LDS_GdMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LSS_GdMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LFS_GdMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LGS_GdMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVZX_GwEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GdEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GdEwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GwEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GdEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GdEwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVZX_GwEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GdEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GdEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GwEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GdEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GdEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BSWAP_RX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BSWAP_ERX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_GbEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EbIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EwIwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EdIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NOT_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NOT_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NOT_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NOT_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NOT_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NOT_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NEG_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NEG_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void TEST_EbIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_EwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_EdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void IMUL_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_GdEdIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MUL_ALEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_ALEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIV_ALEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IDIV_ALEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MUL_EAXEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_EAXEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIV_EAXEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IDIV_EAXEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INC_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INC_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INC_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INC_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CALL_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CALL32_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL16_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP32_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP16_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void JMP_EdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SLDT_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STR_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LLDT_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LTR_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VERR_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VERW_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SGDT_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SIDT_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LGDT_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LIDT_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SMSW_EwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SMSW_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LMSW_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // LOAD methods
  BX_SMF void LOAD_Eb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Ew(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Ed(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void LOAD_Eq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void LOADU_Wdq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Wdq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Wss(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Wsd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOAD_Ww(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

#if BX_SUPPORT_FPU == 0	// if FPU is disabled
  BX_SMF void FPU_ESC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif

  BX_SMF void FWAIT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

#if BX_SUPPORT_FPU
  // load/store
  BX_SMF void FLD_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLD_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLD_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLD_EXTENDED_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FILD_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FILD_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FILD_QWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FBLD_PACKED_BCD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FST_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FST_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FST_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSTP_EXTENDED_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIST_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIST_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FISTP_QWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FBSTP_PACKED_BCD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FISTTP16(bxInstruction_c *) BX_CPP_AttrRegparmN(1); // SSE3
  BX_SMF void FISTTP32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FISTTP64(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // control
  BX_SMF void FNINIT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNCLEX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FRSTOR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNSAVE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDENV(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNSTENV(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FLDCW(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNSTCW(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNSTSW(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNSTSW_AX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // const
  BX_SMF void FLD1(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDL2T(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDL2E(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDPI(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDLG2(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDLN2(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FLDZ(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // add
  BX_SMF void FADD_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FADD_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FADD_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FADD_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIADD_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIADD_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // mul
  BX_SMF void FMUL_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FMUL_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FMUL_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FMUL_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIMUL_WORD_INTEGER (bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIMUL_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // sub
  BX_SMF void FSUB_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUBR_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUB_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUBR_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUB_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUBR_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUB_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSUBR_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FISUB_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FISUBR_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FISUB_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FISUBR_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // div
  BX_SMF void FDIV_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIVR_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIV_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIVR_STi_ST0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIV_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIVR_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIV_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDIVR_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FIDIV_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIDIVR_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIDIV_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FIDIVR_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // compare
  BX_SMF void FCOM_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FUCOM_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCOMI_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FUCOMI_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCOM_SINGLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCOM_DOUBLE_REAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FICOM_WORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FICOM_DWORD_INTEGER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCMOV_ST0_STj(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void FCOMPP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FUCOMPP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // misc
  BX_SMF void FXCH_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FNOP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FPLEGACY(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCHS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FABS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FTST(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FXAM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FDECSTP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FINCSTP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FFREE_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FFREEP_STi(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void F2XM1(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FYL2X(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FPTAN(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FPATAN(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FXTRACT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FPREM1(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FPREM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FYL2XP1(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSQRT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSINCOS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FRNDINT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#undef FSCALE            // <sys/param.h> is #included on Mac OS X from bochs.h
  BX_SMF void FSCALE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FSIN(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FCOS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif

  /* MMX */
  BX_SMF void PUNPCKLBW_PqQd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKLWD_PqQd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKLDQ_PqQd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKSSWB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKUSWB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHBW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHWD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHDQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKSSDW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_PqEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_PqEdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_PqQqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_PqQqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void EMMS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_EdPdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_EdPdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_QqPqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_QqPqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULLW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBUSB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBUSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAND_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDUSB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDUSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PANDN_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBSB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POR_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDSB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PXOR_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMADDWD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLW_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAW_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLW_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLD_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAD_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLD_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLQ_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLQ_PqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* MMX */

#if BX_SUPPORT_3DNOW
  BX_SMF void PFPNACC_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PI2FW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PI2FD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PF2IW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PF2ID_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFNACC_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFCMPGE_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFMIN_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFRCP_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFRSQRT_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFSUB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFADD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFCMPGT_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFMAX_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFRCPIT1_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFRSQIT1_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFSUBR_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFACC_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFCMPEQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFMUL_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PFRCPIT2_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHRW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSWAPD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif

  /* SSE */
  BX_SMF void FXSAVE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void FXRSTOR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LDMXCSR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STMXCSR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PREFETCH(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE */

  /* SSE */
  BX_SMF void ANDPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ORPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XORPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ANDNPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVUPS_VpsWpsM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVUPS_WpsVpsM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSS_VssWssM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSS_WssVssM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVHLPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVLPS_VpsMq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVLPS_MqVps(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVLHPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVHPS_VpsMq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVHPS_MqVps(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVAPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVAPS_VpsWpsM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVAPS_WpsVpsM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPI2PS_VpsQqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPI2PS_VpsQqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSI2SS_VssEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTPS2PI_PqWps(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTSS2SI_GdWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPS2PI_PqWps(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSS2SI_GdWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void UCOMISS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void COMISS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVMSKPS_GdVRps(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SQRTPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SQRTSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RSQRTPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RSQRTSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCPPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCPSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MULPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MULSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUBPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUBSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MINPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MINSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIVPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIVSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MAXPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MAXSS_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSHUFW_PqQqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSHUFLW_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPPS_VpsWpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSS_VssWssIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PINSRW_PqEwIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRW_GdPqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHUFPS_VpsWpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVMSKB_GdPRq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINUB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXUB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAVGB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAVGW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHUW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVNTQ_MqPq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSADBW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MASKMOVQ_PqPRq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE */

  /* SSE2 */
  BX_SMF void MOVSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPI2PD_VpdQqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPI2PD_VpdQqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSI2SD_VsdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTPD2PI_PqWpd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTSD2SI_GdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPD2PI_PqWpd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSD2SI_GdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void UCOMISD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void COMISD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVMSKPD_GdVRpd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SQRTPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SQRTSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MULPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MULSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUBPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUBSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPS2PD_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPD2PS_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSD2SS_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSS2SD_VssWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTDQ2PS_VpsWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPS2DQ_VdqWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTPS2DQ_VdqWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MINPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MINSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIVPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIVSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MAXPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MAXSD_VsdWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKLBW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKLWD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void UNPCKLPS_VpsWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKSSWB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPGTD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKUSWB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHBW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHWD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void UNPCKHPS_VpsWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKSSDW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKLQDQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUNPCKHQDQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_VdqEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSHUFD_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSHUFHW_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVD_EdVdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_VqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_VqWqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPPD_VpdWpdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSD_VsdWsdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVNTI_MdGd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PINSRW_VdqEwIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRW_GdUdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHUFPD_VpdWpdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULLW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVDQ2Q_PqVRq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ2DQ_VdqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVMSKB_GdUdq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBUSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBUSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINUB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDUSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDUSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXUB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAVGB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAVGW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHUW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTPD2DQ_VqWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTPD2DQ_VqWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTDQ2PD_VpdWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULUDQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULUDQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMADDWD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSADBW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MASKMOVDQU_VdqUdq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBQ_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSUBQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PADDD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLW_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAW_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLW_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLD_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRAD_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLD_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLQ_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSRLDQ_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLQ_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSLLDQ_UdqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE2 */

  /* SSE3 */
  BX_SMF void MOVDDUP_VpdWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSLDUP_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSHDUP_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void HADDPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void HADDPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void HSUBPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void HSUBPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDSUBPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADDSUBPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE3 */

  /* SSSE3 */
  BX_SMF void PSHUFB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMADDUBSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGNB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGNW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGND_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHRSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSB_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSW_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSD_PqQq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PALIGNR_PqQqIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PSHUFB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHADDSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMADDUBSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHSUBD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGNB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGNW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PSIGND_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULHRSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PABSD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PALIGNR_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSSE3 */

  /* SSE4.1 */
  BX_SMF void PBLENDVB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BLENDVPS_VpsWpsR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BLENDVPD_VpdWpdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PTEST_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULDQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPEQQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PACKUSDW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXBW_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXBD_VdqWdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXBQ_VdqWwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXWD_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXWQ_VdqWdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVSXDQ_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXBW_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXBD_VdqWdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXBQ_VdqWwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXWD_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXWQ_VdqWdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMOVZXDQ_VdqWqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINSD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINUW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMINUD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXSB_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXSD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXUW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMAXUD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PMULLD_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PHMINPOSUW_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROUNDPS_VpsWpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROUNDPD_VpdWpdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROUNDSS_VssWssIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROUNDSD_VsdWsdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BLENDPS_VpsWpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BLENDPD_VpdWpdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PBLENDW_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRB_EbdVdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRB_EbdVdqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRW_EwdVdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRW_EwdVdqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRD_EdVdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PEXTRD_EdVdqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void EXTRACTPS_EdVpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void EXTRACTPS_EdVpsIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PINSRB_VdqEbIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSERTPS_VpsWssIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PINSRD_VdqEdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PINSRD_VdqEdIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DPPS_VpsWpsIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DPPD_VpdWpdIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MPSADBW_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE4.1 */

  /* SSE4.2 */
  BX_SMF void CRC32_GdEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CRC32_GdEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CRC32_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void CRC32_GdEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void PCMPGTQ_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPESTRM_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPESTRI_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPISTRM_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCMPISTRI_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* SSE4.2 */

  /* MOVBE Intel Atom(R) instruction */
  BX_SMF void MOVBE_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVBE_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVBE_EwGw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVBE_EdGd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void MOVBE_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVBE_EqGq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
  /* MOVBE Intel Atom(R) instruction */

  /* XSAVE/XRSTOR extensions */
  BX_SMF void XSAVE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XRSTOR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XGETBV(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XSETBV(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* XSAVE/XRSTOR extensions */

  /* AES instructions */
  BX_SMF void AESIMC_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AESENC_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AESENCLAST_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AESDEC_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AESDECLAST_VdqWdqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AESKEYGENASSIST_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PCLMULQDQ_VdqWdqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* AES instructions */

  /* VMX instructions */
  BX_SMF void VMXON(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMXOFF(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMCALL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMLAUNCH(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMCLEAR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMPTRLD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMPTRST(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMREAD(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMWRITE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* VMX instructions */

  /* VMXx2 */
  BX_SMF void INVEPT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INVVPID(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  /* VMXx2 */

  BX_SMF void CMPXCHG_XBTS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_IBTS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG8B(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETnear32_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETnear32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETnear16_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETnear16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETfar32_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETfar32(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETfar16_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETfar16(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XADD_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XADD_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XADD_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XADD_EbGbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XADD_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XADD_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMOVO_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNO_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVB_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNB_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVZ_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNZ_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVBE_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNBE_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVS_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNS_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVP_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNP_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVL_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNL_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVLE_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNLE_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMOVO_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNO_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVB_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNB_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVZ_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNZ_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVBE_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNBE_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVS_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNS_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVP_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNP_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVL_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNL_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVLE_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNLE_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CWDE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CDQ(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMPXCHG_EbGbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_EwGwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMPXCHG_EbGbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_EwGwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_EdGdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MUL_AXEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_AXEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIV_AXEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IDIV_AXEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_GwEwIwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NOP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PAUSE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RLIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RHIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RXIw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_ERXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INC_RX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_RX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INC_ERX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_ERX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_RXAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_ERXEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PUSH_RX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_ERX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void POP_RX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP_EwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP_ERX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP_EdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void POPCNT_GwEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPCNT_GdEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void POPCNT_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif

#if BX_SUPPORT_X86_64
  // 64 bit extensions
  BX_SMF void ADD_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ADD_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OR_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ADC_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SBB_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void AND_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SUB_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XOR_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMP_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void TEST_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void TEST_RAXId(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XCHG_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LEA_GqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_RAXOq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OqRAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EAXOq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OqEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_AXOq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OqAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_ALOq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_OqAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_GqEqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_EqIdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // repeatable instructions
  BX_SMF void REP_MOVSQ_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_CMPSQ_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_STOSQ_YqRAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_LODSQ_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void REP_SCASQ_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  // qualified by address size
  BX_SMF void CMPSB64_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSW64_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSD64_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASB64_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASW64_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASD64_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSB64_ALXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSW64_AXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSD64_EAXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSB64_YbAL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSW64_YwAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSD64_YdEAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSB64_XbYb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSW64_XwYw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSD64_XdYd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMPSQ32_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPSQ64_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASQ32_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SCASQ64_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSQ32_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LODSQ64_RAXXq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSQ32_YqRAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void STOSQ64_YqRAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSQ32_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSQ64_XqYq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INSB64_YbDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSW64_YwDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INSD64_YdDX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void OUTSB64_DXXb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSW64_DXXw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void OUTSD64_DXXd(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CALL_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void JO_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNO_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JB_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNB_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JZ_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNZ_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JBE_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNBE_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JS_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNS_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JP_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNP_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JL_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNL_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JLE_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JNLE_Jq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ENTER64_IwIb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LEAVE64(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IRET64(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_CR0Rq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR2Rq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR3Rq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_CR4Rq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RqCR0(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RqCR2(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RqCR3(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RqCR4(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_DqRq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV_RqDq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SHLD_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHLD_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHRD_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHRD_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV64_GdEdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOV64_EdGdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVZX_GqEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GqEwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEwM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEdM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVZX_GqEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVZX_GqEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEwR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVSX_GqEdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BSF_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BSR_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EqIbM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BT_EqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTS_EqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTR_EqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BTC_EqIbR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void BSWAP_RRX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void ROL_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void ROR_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCL_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RCR_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHL_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SHR_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SAR_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void NOT_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NOT_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void NEG_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void TEST_EqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MUL_RAXEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_RAXEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DIV_RAXEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IDIV_RAXEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void IMUL_GqEqIdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INC_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void INC_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void DEC_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CALL64_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP_EqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JMP64_Ep(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSHF_Fq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POPF_Fq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMPXCHG_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CDQE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CQO(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void XADD_EqGqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XADD_EqGqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void RETnear64_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETnear64(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RETfar64_Iw(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CMOVO_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNO_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVB_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNB_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVZ_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNZ_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVBE_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNBE_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVS_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNS_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVP_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNP_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVL_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNL_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVLE_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMOVNLE_GqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOV_RRXIq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH_RRX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP_EqM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP_RRX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void XCHG_RRXRAX(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void PUSH64_Id(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH64_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP64_FS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void PUSH64_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void POP64_GS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LSS_GqMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LFS_GqMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LGS_GqMp(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SGDT64_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SIDT64_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LGDT64_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LIDT64_Ms(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SYSCALL(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SYSRET(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CMPXCHG16B(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void SWAPGS(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RDFSBASE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RDGSBASE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void WRFSBASE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void WRGSBASE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void LOOPNE64_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOPE64_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void LOOP64_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void JRCXZ_Jb(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVQ_EqPqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_EqVqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_PqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MOVQ_VdqEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void CVTSI2SS_VssEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSI2SD_VsdEqR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTSD2SI_GqWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTTSS2SI_GqWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSD2SI_GqWsdR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void CVTSS2SI_GqWssR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MOVNTI_MqGq(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif  // #if BX_SUPPORT_X86_64

  BX_SMF void RDTSCP(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void INVLPG(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RSM(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void WRMSR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RDTSC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RDPMC(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void RDMSR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SYSENTER(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void SYSEXIT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void MONITOR(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void MWAIT(bxInstruction_c *) BX_CPP_AttrRegparmN(1);

  BX_SMF void UndefinedOpcode(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF void BxError(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_CPU_LEVEL >= 6
  BX_SMF void BxNoSSE(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif

  BX_SMF bx_address BxResolve16BaseIndex(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_address BxResolve32Base(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_address BxResolve32BaseIndex(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF bx_address BxResolve64Base(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_address BxResolve64BaseIndex(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
// <TAG-CLASS-CPU-END>

#if BX_DEBUGGER
  BX_SMF void       dbg_take_irq(void);
  BX_SMF void       dbg_force_interrupt(unsigned vector);
  BX_SMF void       dbg_take_dma(void);
  BX_SMF bx_bool    dbg_set_reg(unsigned reg, Bit32u val);
  BX_SMF bx_bool    dbg_get_sreg(bx_dbg_sreg_t *sreg, unsigned sreg_no);
  BX_SMF bx_bool    dbg_set_sreg(unsigned sreg_no, bx_segment_reg_t *sreg);
  BX_SMF void       dbg_get_tr(bx_dbg_sreg_t *sreg);
  BX_SMF void       dbg_get_ldtr(bx_dbg_sreg_t *sreg);
  BX_SMF void       dbg_get_gdtr(bx_dbg_global_sreg_t *sreg);
  BX_SMF void       dbg_get_idtr(bx_dbg_global_sreg_t *sreg);
  BX_SMF unsigned   dbg_query_pending(void);
#endif
#if BX_DEBUGGER || BX_GDBSTUB
  BX_SMF bx_bool  dbg_instruction_epilog(void);
#endif
#if BX_DEBUGGER || BX_DISASM || BX_INSTRUMENTATION || BX_GDBSTUB
  BX_SMF bx_bool  dbg_xlate_linear2phy(bx_address linear, bx_phy_address *phy, bx_bool verbose = 0);
#if BX_SUPPORT_VMX >= 2
  BX_SMF bx_bool dbg_translate_guest_physical(bx_phy_address guest_paddr, bx_phy_address *phy, bx_bool verbose = 0);
#endif
#endif
  BX_SMF void     atexit(void);

  // now for some ancillary functions...
  BX_SMF void cpu_loop(Bit32u max_instr_count);
  BX_SMF unsigned handleAsyncEvent(void);

  BX_SMF int fetchDecode32(const Bit8u *fetchPtr, bxInstruction_c *i, unsigned remainingInPage) BX_CPP_AttrRegparmN(3);
#if BX_SUPPORT_X86_64
  BX_SMF int fetchDecode64(const Bit8u *fetchPtr, bxInstruction_c *i, unsigned remainingInPage) BX_CPP_AttrRegparmN(3);
#endif
  BX_SMF void boundaryFetch(const Bit8u *fetchPtr, unsigned remainingInPage, bxInstruction_c *);
  BX_SMF void serveICacheMiss(bxICacheEntry_c *entry, Bit32u eipBiased, bx_phy_address pAddr);
  BX_SMF bxICacheEntry_c* getICacheEntry(void);
#if BX_SUPPORT_TRACE_CACHE
  BX_SMF bx_bool mergeTraces(bxICacheEntry_c *entry, bxInstruction_c *i, bx_phy_address pAddr);
#else
  BX_SMF bx_bool fetchInstruction(bxInstruction_c *iStorage, Bit32u eipBiased);
#endif
  BX_SMF void prefetch(void);
  BX_SMF void updateFetchModeMask(void);
  BX_SMF BX_CPP_INLINE void invalidate_prefetch_q(void)
  {
    BX_CPU_THIS_PTR eipPageWindowSize = 0;
  }

  BX_SMF bx_bool write_virtual_checks(bx_segment_reg_t *seg, Bit32u offset, unsigned len) BX_CPP_AttrRegparmN(3);
  BX_SMF bx_bool read_virtual_checks(bx_segment_reg_t *seg, Bit32u offset, unsigned len) BX_CPP_AttrRegparmN(3);
  BX_SMF bx_bool execute_virtual_checks(bx_segment_reg_t *seg, Bit32u offset, unsigned len) BX_CPP_AttrRegparmN(3);

  BX_SMF Bit8u read_virtual_byte_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit16u read_virtual_word_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit32u read_virtual_dword_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit64u read_virtual_qword_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
#if BX_CPU_LEVEL >= 6
  BX_SMF void read_virtual_dqword_32(unsigned seg, Bit32u off, BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
  BX_SMF void read_virtual_dqword_aligned_32(unsigned seg, Bit32u off, BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
#endif

  BX_SMF void write_virtual_byte_32(unsigned seg, Bit32u offset, Bit8u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_word_32(unsigned seg, Bit32u offset, Bit16u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_dword_32(unsigned seg, Bit32u offset, Bit32u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_qword_32(unsigned seg, Bit32u offset, Bit64u data) BX_CPP_AttrRegparmN(3);
#if BX_CPU_LEVEL >= 6
  BX_SMF void write_virtual_dqword_32(unsigned seg, Bit32u offset, const BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_dqword_aligned_32(unsigned seg, Bit32u offset, const BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
#endif

  BX_SMF Bit8u read_RMW_virtual_byte_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit16u read_RMW_virtual_word_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit32u read_RMW_virtual_dword_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit64u read_RMW_virtual_qword_32(unsigned seg, Bit32u offset) BX_CPP_AttrRegparmN(2);

  BX_SMF void write_RMW_virtual_byte(Bit8u val8) BX_CPP_AttrRegparmN(1);
  BX_SMF void write_RMW_virtual_word(Bit16u val16) BX_CPP_AttrRegparmN(1);
  BX_SMF void write_RMW_virtual_dword(Bit32u val32) BX_CPP_AttrRegparmN(1);
  BX_SMF void write_RMW_virtual_qword(Bit64u val64) BX_CPP_AttrRegparmN(1);

#if BX_SUPPORT_X86_64
  BX_SMF void write_virtual_byte_64(unsigned seg, Bit64u offset, Bit8u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_word_64(unsigned seg, Bit64u offset, Bit16u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_dword_64(unsigned seg, Bit64u offset, Bit32u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_qword_64(unsigned seg, Bit64u offset, Bit64u data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_dqword_64(unsigned seg, Bit64u offset, const BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
  BX_SMF void write_virtual_dqword_aligned_64(unsigned seg, Bit64u offset, const BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);

  BX_SMF Bit8u read_virtual_byte_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit16u read_virtual_word_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit32u read_virtual_dword_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit64u read_virtual_qword_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF void read_virtual_dqword_64(unsigned seg, Bit64u off, BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);
  BX_SMF void read_virtual_dqword_aligned_64(unsigned seg, Bit64u off, BxPackedXmmRegister *data) BX_CPP_AttrRegparmN(3);

  BX_SMF Bit8u read_RMW_virtual_byte_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit16u read_RMW_virtual_word_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit32u read_RMW_virtual_dword_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit64u read_RMW_virtual_qword_64(unsigned seg, Bit64u offset) BX_CPP_AttrRegparmN(2);
#endif

  // write of word/dword to new stack could happen only in legacy mode
  BX_SMF void write_new_stack_word_32(bx_segment_reg_t *seg, Bit32u offset, unsigned curr_pl, Bit16u data);
  BX_SMF void write_new_stack_dword_32(bx_segment_reg_t *seg, Bit32u offset, unsigned curr_pl, Bit32u data);
  BX_SMF void write_new_stack_qword_32(bx_segment_reg_t *seg, Bit32u offset, unsigned curr_pl, Bit64u data);
#if BX_SUPPORT_X86_64
  BX_SMF void write_new_stack_word_64(Bit64u offset, unsigned curr_pl, Bit16u data);
  BX_SMF void write_new_stack_dword_64(Bit64u offset, unsigned curr_pl, Bit32u data);
  BX_SMF void write_new_stack_qword_64(Bit64u offset, unsigned curr_pl, Bit64u data);
#endif

#if BX_SUPPORT_X86_64

// write
#define write_virtual_byte(seg, offset, data)     \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_byte_64(seg, (Bit64u) offset, data) : \
      write_virtual_byte_32(seg, (Bit32u) offset, data)

#define write_virtual_word(seg, offset, data)     \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_word_64(seg, (Bit64u) offset, data) : \
      write_virtual_word_32(seg, (Bit32u) offset, data)

#define write_virtual_dword(seg, offset, data)    \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_dword_64(seg, (Bit64u) offset, data) : \
      write_virtual_dword_32(seg, (Bit32u) offset, data)

#define write_virtual_qword(seg, offset, data)    \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_qword_64(seg, (Bit64u) offset, data) : \
      write_virtual_qword_32(seg, (Bit32u) offset, data)

#define write_virtual_dqword(seg, offset, data)   \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_dqword_64(seg, (Bit64u) offset, (const BxPackedXmmRegister*)(data)) : \
      write_virtual_dqword_32(seg, (Bit32u) offset, (const BxPackedXmmRegister*)(data))

#define write_virtual_dqword_aligned(seg, offset, data) \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      write_virtual_dqword_aligned_64(seg, (Bit64u) offset, (const BxPackedXmmRegister*)(data)) : \
      write_virtual_dqword_aligned_32(seg, (Bit32u) offset, (const BxPackedXmmRegister*)(data))

// read
#define read_virtual_byte(seg, offset)             \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ?  \
      read_virtual_byte_64(seg, (Bit64u) offset) : \
      read_virtual_byte_32(seg, (Bit32u) offset)

#define read_virtual_word(seg, offset)             \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ?  \
      read_virtual_word_64(seg, (Bit64u) offset) : \
      read_virtual_word_32(seg, (Bit32u) offset)

#define read_virtual_dword(seg, offset)             \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ?   \
      read_virtual_dword_64(seg, (Bit64u) offset) : \
      read_virtual_dword_32(seg, (Bit32u) offset)

#define read_virtual_qword(seg, offset)             \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ?   \
      read_virtual_qword_64(seg, (Bit64u) offset) : \
      read_virtual_qword_32(seg, (Bit32u) offset)

#define read_virtual_dqword(seg, offset, data)    \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_virtual_dqword_64(seg, (Bit64u) offset, (BxPackedXmmRegister*)(data)) : \
      read_virtual_dqword_32(seg, (Bit32u) offset, (BxPackedXmmRegister*)(data))

#define read_virtual_dqword_aligned(seg, offset, data) \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_virtual_dqword_aligned_64(seg, (Bit64u) offset, (BxPackedXmmRegister*)(data)) : \
      read_virtual_dqword_aligned_32(seg, (Bit32u) offset, (BxPackedXmmRegister*)(data))

// RMW
#define read_RMW_virtual_byte(seg, offset)        \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_RMW_virtual_byte_64(seg, (Bit64u) offset) : \
      read_RMW_virtual_byte_32(seg, (Bit32u) offset)

#define read_RMW_virtual_word(seg, offset)        \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_RMW_virtual_word_64(seg, (Bit64u) offset) : \
      read_RMW_virtual_word_32(seg, (Bit32u) offset)

#define read_RMW_virtual_dword(seg, offset)       \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_RMW_virtual_dword_64(seg, (Bit64u) offset) : \
      read_RMW_virtual_dword_32(seg, (Bit32u) offset)

#define read_RMW_virtual_qword(seg, offset)       \
  (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) ? \
      read_RMW_virtual_qword_64(seg, (Bit64u) offset) : \
      read_RMW_virtual_qword_32(seg, (Bit32u) offset)

#else

// write
#define write_virtual_byte(seg, offset, data)  \
  write_virtual_byte_32(seg, offset, data)
#define write_virtual_word(seg, offset, data)  \
  write_virtual_word_32(seg, offset, data)
#define write_virtual_dword(seg, offset, data) \
  write_virtual_dword_32(seg, offset, data)
#define write_virtual_qword(seg, offset, data) \
  write_virtual_qword_32(seg, offset, data)
#define write_virtual_dqword(seg, offset, data) \
  write_virtual_dqword_32(seg, offset, (const BxPackedXmmRegister*)(data))
#define write_virtual_dqword_aligned(seg, offset, data) \
  write_virtual_dqword_aligned_32(seg, offset, (const BxPackedXmmRegister*)(data))

// read
#define read_virtual_byte(seg, offset)  \
  read_virtual_byte_32(seg, offset)
#define read_virtual_word(seg, offset)  \
  read_virtual_word_32(seg, offset)
#define read_virtual_dword(seg, offset) \
  read_virtual_dword_32(seg, offset)
#define read_virtual_qword(seg, offset) \
  read_virtual_qword_32(seg, offset)
#define read_virtual_dqword(seg, offset, data) \
  read_virtual_dqword_32(seg, offset, (BxPackedXmmRegister*)(data))
#define read_virtual_dqword_aligned(seg, offset, data) \
  read_virtual_dqword_aligned_32(seg, offset, (BxPackedXmmRegister*)(data))

// RMW
#define read_RMW_virtual_byte(seg, offset)  \
  read_RMW_virtual_byte_32(seg, offset)
#define read_RMW_virtual_word(seg, offset)  \
  read_RMW_virtual_word_32(seg, offset)
#define read_RMW_virtual_dword(seg, offset) \
  read_RMW_virtual_dword_32(seg, offset)
#define read_RMW_virtual_qword(seg, offset) \
  read_RMW_virtual_qword_32(seg, offset)

#endif

  BX_SMF Bit8u  system_read_byte(bx_address laddr) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit16u system_read_word(bx_address laddr) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit32u system_read_dword(bx_address laddr) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit64u system_read_qword(bx_address laddr) BX_CPP_AttrRegparmN(1);

  BX_SMF void system_write_byte(bx_address laddr, Bit8u data) BX_CPP_AttrRegparmN(2);
  BX_SMF void system_write_word(bx_address laddr, Bit16u data) BX_CPP_AttrRegparmN(2);
  BX_SMF void system_write_dword(bx_address laddr, Bit32u data) BX_CPP_AttrRegparmN(2);

  BX_SMF Bit8u* v2h_read_byte(bx_address laddr, bx_bool user) BX_CPP_AttrRegparmN(2);
  BX_SMF Bit8u* v2h_write_byte(bx_address laddr, bx_bool user) BX_CPP_AttrRegparmN(2);

  BX_SMF void branch_near16(Bit16u new_IP) BX_CPP_AttrRegparmN(1);
  BX_SMF void branch_near32(Bit32u new_EIP) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void branch_near64(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void branch_far32(bx_selector_t *selector,
       bx_descriptor_t *descriptor, Bit32u eip, Bit8u cpl);
  BX_SMF void branch_far64(bx_selector_t *selector,
       bx_descriptor_t *descriptor, bx_address rip, Bit8u cpl);

#if BX_SupportRepeatSpeedups
  BX_SMF Bit32u FastRepMOVSB(bxInstruction_c *i, unsigned srcSeg, bx_address srcOff,
       unsigned dstSeg, bx_address dstOff, Bit32u  byteCount);
  BX_SMF Bit32u FastRepMOVSW(bxInstruction_c *i, unsigned srcSeg, bx_address srcOff,
       unsigned dstSeg, bx_address dstOff, Bit32u  wordCount);
  BX_SMF Bit32u FastRepMOVSD(bxInstruction_c *i, unsigned srcSeg, bx_address srcOff,
       unsigned dstSeg, bx_address dstOff, Bit32u dwordCount);

  BX_SMF Bit32u FastRepSTOSB(bxInstruction_c *i, unsigned dstSeg, bx_address dstOff,
       Bit8u  val, Bit32u  byteCount);
  BX_SMF Bit32u FastRepSTOSW(bxInstruction_c *i, unsigned dstSeg, bx_address dstOff,
       Bit16u val, Bit32u  wordCount);
  BX_SMF Bit32u FastRepSTOSD(bxInstruction_c *i, unsigned dstSeg, bx_address dstOff,
       Bit32u val, Bit32u dwordCount);

  BX_SMF Bit32u FastRepINSW(bxInstruction_c *i, bx_address dstOff,
       Bit16u port, Bit32u wordCount);
  BX_SMF Bit32u FastRepOUTSW(bxInstruction_c *i, unsigned srcSeg, bx_address srcOff,
       Bit16u port, Bit32u wordCount);
#endif

  BX_SMF void repeat(bxInstruction_c *i, BxExecutePtr_tR execute) BX_CPP_AttrRegparmN(2);
  BX_SMF void repeat_ZF(bxInstruction_c *i, BxExecutePtr_tR execute) BX_CPP_AttrRegparmN(2);

  // linear address for access_linear expected to be canonical !
  BX_SMF void access_read_linear(bx_address laddr, unsigned len, unsigned curr_pl,
       unsigned rw, void *data);
  BX_SMF void access_write_linear(bx_address laddr, unsigned len, unsigned curr_pl,
       void *data);
  BX_SMF void page_fault(unsigned fault, bx_address laddr, unsigned user, unsigned rw);

  BX_SMF void access_read_physical(bx_phy_address paddr, unsigned len, void *data);
  BX_SMF void access_write_physical(bx_phy_address paddr, unsigned len, void *data);

  BX_SMF bx_hostpageaddr_t getHostMemAddr(bx_phy_address addr, unsigned rw);

  // linear address for translate_linear expected to be canonical !
  BX_SMF bx_phy_address translate_linear(bx_address laddr, unsigned curr_pl, unsigned rw);
#if BX_CPU_LEVEL >= 6
  BX_SMF bx_phy_address translate_linear_PAE(bx_address laddr, Bit32u &lpf_mask, Bit32u &combined_access, unsigned curr_pl, unsigned rw);
  BX_SMF int check_entry_PAE(const char *s, Bit64u entry, Bit64u reserved, unsigned rw, bx_bool *nx_fault);
#endif
#if BX_SUPPORT_X86_64
  BX_SMF bx_phy_address translate_linear_long_mode(bx_address laddr, Bit32u &lpf_mask, Bit32u &combined_access, unsigned curr_pl, unsigned rw);
#endif
#if BX_SUPPORT_VMX >= 2
  BX_SMF bx_phy_address translate_guest_physical(bx_phy_address guest_paddr, bx_address guest_laddr, bx_bool guest_laddr_valid, bx_bool is_page_walk, unsigned rw);
#endif
  BX_SMF BX_CPP_INLINE bx_phy_address dtranslate_linear(bx_address laddr, unsigned curr_pl, unsigned rw)
  {
    return translate_linear(laddr, curr_pl, rw);
  }

#if BX_CPU_LEVEL >= 6
  BX_SMF void TLB_flushNonGlobal(void);
#endif
  BX_SMF void TLB_flush(void);
  BX_SMF void TLB_invlpg(bx_address laddr);
  BX_SMF void set_INTR(bx_bool value);
  BX_SMF const char *strseg(bx_segment_reg_t *seg);
  BX_SMF void interrupt(Bit8u vector, unsigned type, bx_bool push_error,
                 Bit16u error_code);
  BX_SMF void real_mode_int(Bit8u vector, bx_bool push_error, Bit16u error_code);
  BX_SMF void protected_mode_int(Bit8u vector, unsigned soft_int, bx_bool push_error,
                 Bit16u error_code);
#if BX_SUPPORT_X86_64
  BX_SMF void long_mode_int(Bit8u vector, unsigned soft_int, bx_bool push_error,
                 Bit16u error_code);
#endif
  BX_SMF void exception(unsigned vector, Bit16u error_code)
                  BX_CPP_AttrNoReturn();
  BX_SMF void init_SMRAM(void);
  BX_SMF void smram_save_state(Bit32u *smm_saved_state);
  BX_SMF bx_bool smram_restore_state(const Bit32u *smm_saved_state);
  BX_SMF int  int_number(unsigned s);
  BX_SMF bx_bool SetCR0(bx_address val) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_bool check_CR0(bx_address val) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_bool SetCR3(bx_address val) BX_CPP_AttrRegparmN(1);
#if BX_CPU_LEVEL >= 4
  BX_SMF bx_bool SetCR4(bx_address val) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_bool check_CR4(bx_address val) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit32u get_cr4_allow_mask(void);
#endif
#if BX_CPU_LEVEL >= 6
  BX_SMF bx_bool CheckPDPTR(bx_phy_address cr3_val) BX_CPP_AttrRegparmN(1);
#endif
#if BX_SUPPORT_VMX >= 2
  BX_SMF bx_bool CheckPDPTR(Bit64u *pdptr) BX_CPP_AttrRegparmN(1);
#endif

  BX_SMF void reset(unsigned source);
  BX_SMF void shutdown(void);
  BX_SMF void handleCpuModeChange(void);
#if BX_CPU_LEVEL >= 4 && BX_SUPPORT_ALIGNMENT_CHECK
  BX_SMF void handleAlignmentCheck(void);
#endif
#if BX_CPU_LEVEL >= 6
  BX_SMF void handleSseModeChange(void);
#endif

#if BX_CPU_LEVEL >= 5
  BX_SMF bx_bool rdmsr(Bit32u index, Bit64u *val_64) BX_CPP_AttrRegparmN(2);
  BX_SMF bx_bool wrmsr(Bit32u index, Bit64u  val_64) BX_CPP_AttrRegparmN(2);
#endif

#if BX_SUPPORT_APIC
  BX_SMF bx_bool relocate_apic(Bit64u val_64);
#endif

  BX_SMF void task_gate(bxInstruction_c *i, bx_selector_t *selector, bx_descriptor_t *gate_descriptor, unsigned source);
  BX_SMF void jump_protected(bxInstruction_c *i, Bit16u cs, bx_address disp) BX_CPP_AttrRegparmN(3);
  BX_SMF void jmp_call_gate(bx_selector_t *selector, bx_descriptor_t *gate_descriptor) BX_CPP_AttrRegparmN(2);
  BX_SMF void call_gate(bx_descriptor_t *gate_descriptor) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void jmp_call_gate64(bx_selector_t *selector) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void call_protected(bxInstruction_c *i, Bit16u cs, bx_address disp) BX_CPP_AttrRegparmN(3);
#if BX_SUPPORT_X86_64
  BX_SMF void call_gate64(bx_selector_t *selector) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void return_protected(bxInstruction_c *i, Bit16u pop_bytes) BX_CPP_AttrRegparmN(2);
  BX_SMF void iret_protected(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_X86_64
  BX_SMF void long_iret(bxInstruction_c *) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF void validate_seg_reg(unsigned seg);
  BX_SMF void validate_seg_regs(void);
  BX_SMF void stack_return_to_v86(Bit32u new_eip, Bit32u raw_cs_selector, Bit32u flags32);
  BX_SMF void iret16_stack_return_from_v86(bxInstruction_c *);
  BX_SMF void iret32_stack_return_from_v86(bxInstruction_c *);
  BX_SMF int  v86_redirect_interrupt(Bit8u vector);
  BX_SMF void init_v8086_mode(void);
  BX_SMF void task_switch_load_selector(bx_segment_reg_t *seg,
                 bx_selector_t *selector, Bit16u raw_selector, Bit8u cs_rpl);
  BX_SMF void task_switch(bxInstruction_c *i, bx_selector_t *selector, bx_descriptor_t *descriptor,
                 unsigned source, Bit32u dword1, Bit32u dword2);
  BX_SMF void get_SS_ESP_from_TSS(unsigned pl, Bit16u *ss, Bit32u *esp);
#if BX_SUPPORT_X86_64
  BX_SMF Bit64u get_RSP_from_TSS(unsigned pl);
#endif
  BX_SMF void write_flags(Bit16u flags, bx_bool change_IOPL, bx_bool change_IF) BX_CPP_AttrRegparmN(3);
  BX_SMF void writeEFlags(Bit32u eflags, Bit32u changeMask) BX_CPP_AttrRegparmN(2); // Newer variant.
  BX_SMF void write_eflags_fpu_compare(int float_relation);
  BX_SMF Bit32u force_flags(void);
  BX_SMF Bit32u read_eflags(void) { return BX_CPU_THIS_PTR force_flags(); }

  BX_SMF bx_bool allow_io(bxInstruction_c *i, Bit16u addr, unsigned len) BX_CPP_AttrRegparmN(3);
  BX_SMF Bit32u  get_descriptor_l(const bx_descriptor_t *) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit32u  get_descriptor_h(const bx_descriptor_t *) BX_CPP_AttrRegparmN(1);
  BX_SMF bx_bool set_segment_ar_data(bx_segment_reg_t *seg, bx_bool valid, Bit16u raw_selector,
                         bx_address base, Bit32u limit_scaled, Bit16u ar_data);
  BX_SMF void    check_cs(bx_descriptor_t *descriptor, Bit16u cs_raw, Bit8u check_rpl, Bit8u check_cpl);
  // the basic assumption of the code that load_cs and load_ss cannot fail !
  BX_SMF void    load_cs(bx_selector_t *selector, bx_descriptor_t *descriptor, Bit8u cpl) BX_CPP_AttrRegparmN(3);
  BX_SMF void    load_ss(bx_selector_t *selector, bx_descriptor_t *descriptor, Bit8u cpl) BX_CPP_AttrRegparmN(3);
  BX_SMF void    touch_segment(bx_selector_t *selector, bx_descriptor_t *descriptor) BX_CPP_AttrRegparmN(2);
  BX_SMF void    fetch_raw_descriptor(const bx_selector_t *selector,
                         Bit32u *dword1, Bit32u *dword2, unsigned exception_no);
  BX_SMF bx_bool fetch_raw_descriptor2(const bx_selector_t *selector,
                         Bit32u *dword1, Bit32u *dword2) BX_CPP_AttrRegparmN(3);
  BX_SMF void    load_seg_reg(bx_segment_reg_t *seg, Bit16u new_value) BX_CPP_AttrRegparmN(2);
  BX_SMF void    load_null_selector(bx_segment_reg_t *seg, unsigned value) BX_CPP_AttrRegparmN(2);
#if BX_SUPPORT_X86_64
  BX_SMF void    fetch_raw_descriptor_64(const bx_selector_t *selector,
                         Bit32u *dword1, Bit32u *dword2, Bit32u *dword3, unsigned exception_no);
  BX_SMF bx_bool fetch_raw_descriptor2_64(const bx_selector_t *selector,
                         Bit32u *dword1, Bit32u *dword2, Bit32u *dword3);
#endif
  BX_SMF void    push_16(Bit16u value16) BX_CPP_AttrRegparmN(1);
  BX_SMF void    push_32(Bit32u value32) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit16u  pop_16(void);
  BX_SMF Bit32u  pop_32(void);
#if BX_SUPPORT_X86_64
  BX_SMF void    push_64(Bit64u value64) BX_CPP_AttrRegparmN(1);
  BX_SMF Bit64u  pop_64(void);
#endif
  BX_SMF void    sanity_checks(void);
  BX_SMF void    assert_checks(void);
  BX_SMF void    enter_system_management_mode(void);
  BX_SMF void    deliver_INIT(void);
  BX_SMF void    deliver_NMI(void);
  BX_SMF void    deliver_SMI(void);
  BX_SMF void    deliver_SIPI(unsigned vector);
  BX_SMF void    debug(bx_address offset);
#if BX_DISASM
  BX_SMF void    debug_disasm_instruction(bx_address offset);
#endif

#if BX_X86_DEBUGGER
  // x86 hardware debug support
  BX_SMF bx_bool hwbreakpoint_check(bx_address laddr);
  BX_SMF void    iobreakpoint_match(unsigned port, unsigned len);
  BX_SMF void    code_breakpoint_match(bx_address laddr);
  BX_SMF void    hwbreakpoint_match(bx_address laddr, unsigned len, unsigned rw);
  BX_SMF Bit32u  hwdebug_compare(bx_address laddr, unsigned len,
                                 unsigned opa, unsigned opb);
#endif

  BX_SMF Bit32u get_cpu_version_information(void);
  BX_SMF Bit32u get_extended_cpuid_features(void);
  BX_SMF Bit32u get_ext2_cpuid_features(void);
  BX_SMF Bit32u get_std_cpuid_features(void);
  BX_SMF void set_cpuid_defaults(void);

  BX_SMF void init_isa_features_bitmask(void);
  BX_SMF void init_FetchDecodeTables(void);
#if BX_SUPPORT_X2APIC
  BX_SMF void bx_cpuid_extended_topology_leaf(Bit32u subfunction);
#endif
#if BX_CPU_LEVEL >= 6
  BX_SMF void bx_cpuid_xsave_leaf(Bit32u subfunction);
  BX_SMF void bx_cpuid_extended_cpuid_leaf(Bit32u subfunction);
#endif

  BX_SMF BX_CPP_INLINE int bx_cpuid_support_debug_extensions(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_1g_paging(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_vme(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_pae(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_pge(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_pse(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_pse36(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_mmx(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_sse(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_sep(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_fxsave_fxrstor(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_pcid(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_xsave(void);
  BX_SMF BX_CPP_INLINE int bx_cpuid_support_fsgsbase(void);

  BX_SMF BX_CPP_INLINE unsigned which_cpu(void) { return BX_CPU_THIS_PTR bx_cpuid; }
  BX_SMF BX_CPP_INLINE const bx_gen_reg_t *get_gen_regfile() { return BX_CPU_THIS_PTR gen_reg; }

  BX_SMF BX_CPP_INLINE bx_address get_instruction_pointer(void);

  BX_SMF BX_CPP_INLINE Bit32u get_eip(void) { return (BX_CPU_THIS_PTR gen_reg[BX_32BIT_REG_EIP].dword.erx); }
  BX_SMF BX_CPP_INLINE Bit16u get_ip (void) { return (BX_CPU_THIS_PTR gen_reg[BX_16BIT_REG_IP].word.rx); }
#if BX_SUPPORT_X86_64
  BX_SMF BX_CPP_INLINE Bit64u get_rip(void) { return (BX_CPU_THIS_PTR gen_reg[BX_64BIT_REG_RIP].rrx); }
#endif

  BX_SMF BX_CPP_INLINE Bit8u get_reg8l(unsigned reg);
  BX_SMF BX_CPP_INLINE Bit8u get_reg8h(unsigned reg);
  BX_SMF BX_CPP_INLINE void  set_reg8l(unsigned reg, Bit8u val);
  BX_SMF BX_CPP_INLINE void  set_reg8h(unsigned reg, Bit8u val);

  BX_SMF BX_CPP_INLINE Bit16u get_reg16(unsigned reg);
  BX_SMF BX_CPP_INLINE void   set_reg16(unsigned reg, Bit16u val);
  BX_SMF BX_CPP_INLINE Bit32u get_reg32(unsigned reg);
  BX_SMF BX_CPP_INLINE void   set_reg32(unsigned reg, Bit32u val);
#if BX_SUPPORT_X86_64
  BX_SMF BX_CPP_INLINE Bit64u get_reg64(unsigned reg);
  BX_SMF BX_CPP_INLINE void   set_reg64(unsigned reg, Bit64u val);
#endif

  BX_SMF bx_address get_segment_base(unsigned seg);

  // The linear address must be truncated to the 32-bit when CPU is not
  // executing in long64 mode.  The function  must  be used  to compute
  // linear address everywhere when a code is shared between long64 and
  // legacy mode. For legacy mode only  just use Bit32u to store linear 
  // address value.
  BX_SMF bx_address get_laddr(unsigned seg, bx_address offset);

  BX_SMF Bit32u get_laddr32(unsigned seg, Bit32u offset);
#if BX_SUPPORT_X86_64
  BX_SMF Bit64u get_laddr64(unsigned seg, Bit64u offset);
#endif

  DECLARE_EFLAG_ACCESSOR   (ID,  21)
  DECLARE_EFLAG_ACCESSOR   (VIP, 20)
  DECLARE_EFLAG_ACCESSOR   (VIF, 19)
  DECLARE_EFLAG_ACCESSOR   (AC,  18)
  DECLARE_EFLAG_ACCESSOR   (VM,  17)
  DECLARE_EFLAG_ACCESSOR   (RF,  16)
  DECLARE_EFLAG_ACCESSOR   (NT,  14)
  DECLARE_EFLAG_ACCESSOR_IOPL(   12)
  DECLARE_EFLAG_ACCESSOR   (DF,  10)
  DECLARE_EFLAG_ACCESSOR   (IF,   9)
  DECLARE_EFLAG_ACCESSOR   (TF,   8)

  BX_SMF BX_CPP_INLINE bx_bool real_mode(void);
  BX_SMF BX_CPP_INLINE bx_bool smm_mode(void);
  BX_SMF BX_CPP_INLINE bx_bool protected_mode(void);
  BX_SMF BX_CPP_INLINE bx_bool v8086_mode(void);
  BX_SMF BX_CPP_INLINE bx_bool long_mode(void);
  BX_SMF BX_CPP_INLINE bx_bool long64_mode(void);
  BX_SMF BX_CPP_INLINE unsigned get_cpu_mode(void);

#define StackAddrSize64() long64_mode()

#if BX_SUPPORT_ALIGNMENT_CHECK && BX_CPU_LEVEL >= 4
  BX_SMF BX_CPP_INLINE bx_bool alignment_check(void);
#endif

#if BX_CPU_LEVEL >= 5
  BX_SMF Bit64u get_TSC();
  BX_SMF void   set_TSC(Bit64u tsc);
#endif

#if BX_SUPPORT_FPU
  BX_SMF void print_state_FPU(void);
  BX_SMF void prepareFPU(bxInstruction_c *i, bx_bool = 1);
  BX_SMF void FPU_check_pending_exceptions(void);
  BX_SMF void FPU_update_last_instruction(bxInstruction_c *i);
  BX_SMF void FPU_stack_underflow(int stnr, int pop_stack = 0);
  BX_SMF void FPU_stack_overflow(void);
  BX_SMF unsigned FPU_exception(unsigned exception, bx_bool = 0);
  BX_SMF bx_address fpu_save_environment(bxInstruction_c *i);
  BX_SMF bx_address fpu_load_environment(bxInstruction_c *i);
  BX_SMF Bit8u pack_FPU_TW(Bit16u tag_word);
  BX_SMF Bit16u unpack_FPU_TW(Bit16u tag_byte);
#endif

#if BX_CPU_LEVEL >= 5
  BX_SMF void prepareMMX(void);
  BX_SMF void prepareFPU2MMX(void); /* cause transition from FPU to MMX technology state */
  BX_SMF void print_state_MMX(void);
#endif

#if BX_CPU_LEVEL >= 6
  BX_SMF void check_exceptionsSSE(int);
  BX_SMF void print_state_SSE(void);
#endif

#if BX_CPU_LEVEL >= 6
  BX_SMF void prepareXSAVE(void);
#endif

#if BX_SUPPORT_MONITOR_MWAIT
  BX_SMF bx_bool    is_monitor(bx_phy_address addr, unsigned len);
  BX_SMF void    check_monitor(bx_phy_address addr, unsigned len);
#endif

  BX_SMF bx_address read_CR0(void);
#if BX_CPU_LEVEL > 3
  BX_SMF bx_address read_CR4(void);
#endif
#if BX_SUPPORT_VMX
  BX_SMF Bit16u VMread16(unsigned encoding);
  BX_SMF Bit32u VMread32(unsigned encoding);
  BX_SMF Bit64u VMread64(unsigned encoding);
  BX_SMF void VMwrite16(unsigned encoding, Bit16u val_16);
  BX_SMF void VMwrite32(unsigned encoding, Bit32u val_32);
  BX_SMF void VMwrite64(unsigned encoding, Bit64u val_64);
  BX_SMF void VMsucceed(void);
  BX_SMF void VMfailInvalid(void);
  BX_SMF void VMfail(Bit32u error_code);
  BX_SMF void VMabort(VMX_vmabort_code error_code);
  BX_SMF Bit32u LoadMSRs(Bit32u msr_cnt, bx_phy_address pAddr);
  BX_SMF Bit32u StoreMSRs(Bit32u msr_cnt, bx_phy_address pAddr);
  BX_SMF unsigned VMXReadRevisionID(bx_phy_address pAddr);
  BX_SMF VMX_error_code VMenterLoadCheckVmControls(void);
  BX_SMF VMX_error_code VMenterLoadCheckHostState(void);
  BX_SMF Bit32u VMenterLoadCheckGuestState(Bit64u *qualification);
  BX_SMF void VMenterInjectEvents(void);
  BX_SMF void VMexit(bxInstruction_c *i, Bit32u reason, Bit64u qualification);
  BX_SMF void VMexitSaveGuestState(void);
  BX_SMF void VMexitSaveGuestMSRs(void);
  BX_SMF void VMexitLoadHostState(void);
  BX_SMF void set_VMCSPTR(Bit64u vmxptr);
  BX_SMF void init_VMCS(void);
  BX_SMF bx_bool vmcs_field_supported(Bit32u encoding);
  BX_SMF void register_vmx_state(bx_param_c *parent);
#if BX_SUPPORT_VMX >= 2
  BX_SMF bx_bool is_virtual_apic_page(bx_phy_address paddr) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMX_Virtual_Apic_Read(bx_phy_address paddr, unsigned len, void *data);
  BX_SMF void VMX_Virtual_Apic_Write(bx_phy_address paddr, unsigned len, void *data);
  BX_SMF Bit16u VMX_Get_Current_VPID(void);
#endif
  BX_SMF Bit32u VMX_Read_VTPR(void);
  BX_SMF void VMX_Write_VTPR(Bit8u vtpr);
  BX_SMF Bit64s VMX_TSC_Offset(void);
  // vmexit reasons
  BX_SMF void VMexit_Instruction(bxInstruction_c *i, Bit32u reason) BX_CPP_AttrRegparmN(2);
  BX_SMF void VMexit_Event(bxInstruction_c *i, unsigned type, unsigned vector,
       Bit16u errcode, bx_bool errcode_valid, Bit64u qualification = 0);
  BX_SMF void VMexit_TripleFault(void);
  BX_SMF void VMexit_ExtInterrupt(void);
  BX_SMF void VMexit_TaskSwitch(bxInstruction_c *i, Bit16u tss_selector, unsigned source) BX_CPP_AttrRegparmN(3);
  BX_SMF void VMexit_SoftwareInterrupt(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_HLT(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_PAUSE(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_INVLPG(bxInstruction_c *i, bx_address laddr) BX_CPP_AttrRegparmN(2);
  BX_SMF void VMexit_RDTSC(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_RDPMC(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
#if BX_SUPPORT_VMX >= 2
  BX_SMF void VMexit_WBINVD(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
#endif
  BX_SMF bx_bool VMexit_CLTS(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_MSR(bxInstruction_c *i, unsigned op, Bit32u msr) BX_CPP_AttrRegparmN(3);
  BX_SMF void VMexit_IO(bxInstruction_c *i, unsigned port, unsigned len) BX_CPP_AttrRegparmN(3);
  BX_SMF Bit32u VMexit_LMSW(bxInstruction_c *i, Bit32u msw) BX_CPP_AttrRegparmN(2);
  BX_SMF bx_address VMexit_CR0_Write(bxInstruction_c *i, bx_address) BX_CPP_AttrRegparmN(2);
  BX_SMF void VMexit_CR3_Read(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_CR3_Write(bxInstruction_c *i, bx_address) BX_CPP_AttrRegparmN(2);
  BX_SMF bx_address VMexit_CR4_Write(bxInstruction_c *i, bx_address) BX_CPP_AttrRegparmN(2);
  BX_SMF void VMexit_CR8_Read(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_CR8_Write(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_DR_Access(bxInstruction_c *i, unsigned read) BX_CPP_AttrRegparmN(2);
#if BX_SUPPORT_MONITOR_MWAIT
  BX_SMF void VMexit_MONITOR(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
  BX_SMF void VMexit_MWAIT(bxInstruction_c *i) BX_CPP_AttrRegparmN(1);
#endif
#endif

#if BX_CONFIGURE_MSRS
  int load_MSRs(const char *file);
#endif
};

#if BX_CPU_LEVEL >= 5
BX_CPP_INLINE void BX_CPU_C::prepareMMX(void)
{
  if(BX_CPU_THIS_PTR cr0.get_EM())
    exception(BX_UD_EXCEPTION, 0);

  if(BX_CPU_THIS_PTR cr0.get_TS())
    exception(BX_NM_EXCEPTION, 0);

  /* check floating point status word for a pending FPU exceptions */
  FPU_check_pending_exceptions();
}

BX_CPP_INLINE void BX_CPU_C::prepareFPU2MMX(void)
{
  BX_CPU_THIS_PTR the_i387.twd = 0;
  BX_CPU_THIS_PTR the_i387.tos = 0; /* reset FPU Top-Of-Stack */
}
#endif

#if BX_CPU_LEVEL >= 6
BX_CPP_INLINE void BX_CPU_C::prepareXSAVE(void)
{
  if(! BX_CPU_THIS_PTR cr4.get_OSXSAVE())
    exception(BX_UD_EXCEPTION, 0);

  if(BX_CPU_THIS_PTR cr0.get_TS())
    exception(BX_NM_EXCEPTION, 0);
}
#endif

// Can be used as LHS or RHS.
#define RMAddr(i)  (BX_CPU_THIS_PTR address_xlation.rm_addr)

#if defined(NEED_CPU_REG_SHORTCUTS)

#include "stack.h"

#define RSP_SPECULATIVE {              \
  BX_CPU_THIS_PTR speculative_rsp = 1; \
  BX_CPU_THIS_PTR prev_rsp = RSP;      \
}

#define RSP_COMMIT {                   \
  BX_CPU_THIS_PTR speculative_rsp = 0; \
}

#endif // defined(NEED_CPU_REG_SHORTCUTS)

//
// bit 0 - CS.D_B
// bit 1 - long64 mode (CS.L)
// bit 2 - SSE_OK
//
// updateFetchModeMask - has to be called everytime 
//        CS.L or CS.D_B / CR0.TS or CR0.EM / CR4.OSFXSR changes
//
BX_CPP_INLINE void BX_CPU_C::updateFetchModeMask(void)
{
  BX_CPU_THIS_PTR fetchModeMask =
#if BX_CPU_LEVEL >= 6
     (BX_CPU_THIS_PTR sse_ok << 2) |
#endif
#if BX_SUPPORT_X86_64
    ((BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64)<<1) |
#endif
     (BX_CPU_THIS_PTR sregs[BX_SEG_REG_CS].cache.u.segment.d_b);

  BX_CPU_THIS_PTR user_pl = // CPL == 3
     (BX_CPU_THIS_PTR sregs[BX_SEG_REG_CS].selector.rpl == 3);
}

#if BX_X86_DEBUGGER
#define BX_HWDebugInstruction   0x00
#define BX_HWDebugMemW          0x01
#define BX_HWDebugIO            0x02
#define BX_HWDebugMemRW         0x03
#endif

BX_CPP_INLINE bx_address BX_CPU_C::get_segment_base(unsigned seg)
{
#if BX_SUPPORT_X86_64
  if (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) {
    if (seg < BX_SEG_REG_FS) return 0;
  }
#endif
  return BX_CPU_THIS_PTR sregs[seg].cache.u.segment.base;
}

BX_CPP_INLINE Bit32u BX_CPU_C::get_laddr32(unsigned seg, Bit32u offset)
{
  return (Bit32u) BX_CPU_THIS_PTR sregs[seg].cache.u.segment.base + offset;
}

#if BX_SUPPORT_X86_64
BX_CPP_INLINE Bit64u BX_CPU_C::get_laddr64(unsigned seg, Bit64u offset)
{
  if (seg < BX_SEG_REG_FS)
    return offset;
  else
    return BX_CPU_THIS_PTR sregs[seg].cache.u.segment.base + offset;
}
#endif

BX_CPP_INLINE bx_address BX_CPU_C::get_laddr(unsigned seg, bx_address offset)
{
#if BX_SUPPORT_X86_64
  if (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64) {
    return get_laddr64(seg, offset);
  }
#endif
  return get_laddr32(seg, (Bit32u) offset);
}

BX_CPP_INLINE Bit8u BX_CPU_C::get_reg8l(unsigned reg)
{
  assert(reg < BX_GENERAL_REGISTERS);
  return (BX_CPU_THIS_PTR gen_reg[reg].word.byte.rl);
}

BX_CPP_INLINE void BX_CPU_C::set_reg8l(unsigned reg, Bit8u val)
{
  assert(reg < BX_GENERAL_REGISTERS);
  BX_CPU_THIS_PTR gen_reg[reg].word.byte.rl = val;
}

BX_CPP_INLINE Bit8u BX_CPU_C::get_reg8h(unsigned reg)
{
  assert(reg < BX_GENERAL_REGISTERS);
  return (BX_CPU_THIS_PTR gen_reg[reg].word.byte.rh);
}

BX_CPP_INLINE void BX_CPU_C::set_reg8h(unsigned reg, Bit8u val)
{
  assert(reg < BX_GENERAL_REGISTERS);
  BX_CPU_THIS_PTR gen_reg[reg].word.byte.rh = val;
}

#if BX_SUPPORT_X86_64
BX_CPP_INLINE bx_address BX_CPU_C::get_instruction_pointer(void)
{
  return BX_CPU_THIS_PTR get_rip();
}
#else
BX_CPP_INLINE bx_address BX_CPU_C::get_instruction_pointer(void)
{
  return BX_CPU_THIS_PTR get_eip();
}
#endif

BX_CPP_INLINE Bit16u BX_CPU_C::get_reg16(unsigned reg)
{
  assert(reg < BX_GENERAL_REGISTERS);
  return (BX_CPU_THIS_PTR gen_reg[reg].word.rx);
}

BX_CPP_INLINE void BX_CPU_C::set_reg16(unsigned reg, Bit16u val)
{
  assert(reg < BX_GENERAL_REGISTERS);
  BX_CPU_THIS_PTR gen_reg[reg].word.rx = val;
}

BX_CPP_INLINE Bit32u BX_CPU_C::get_reg32(unsigned reg)
{
  assert(reg < BX_GENERAL_REGISTERS);
  return (BX_CPU_THIS_PTR gen_reg[reg].dword.erx);
}

BX_CPP_INLINE void BX_CPU_C::set_reg32(unsigned reg, Bit32u val)
{
   assert(reg < BX_GENERAL_REGISTERS);
   BX_CPU_THIS_PTR gen_reg[reg].dword.erx = val;
}

#if BX_SUPPORT_X86_64
BX_CPP_INLINE Bit64u BX_CPU_C::get_reg64(unsigned reg)
{
   assert(reg < BX_GENERAL_REGISTERS);
   return (BX_CPU_THIS_PTR gen_reg[reg].rrx);
}

BX_CPP_INLINE void BX_CPU_C::set_reg64(unsigned reg, Bit64u val)
{
   assert(reg < BX_GENERAL_REGISTERS);
   BX_CPU_THIS_PTR gen_reg[reg].rrx = val;
}
#endif

BX_CPP_INLINE bx_bool BX_CPU_C::real_mode(void)
{
  return (BX_CPU_THIS_PTR cpu_mode == BX_MODE_IA32_REAL);
}

BX_CPP_INLINE bx_bool BX_CPU_C::smm_mode(void)
{
  return (BX_CPU_THIS_PTR in_smm);
}

BX_CPP_INLINE bx_bool BX_CPU_C::v8086_mode(void)
{
  return (BX_CPU_THIS_PTR cpu_mode == BX_MODE_IA32_V8086);
}

BX_CPP_INLINE bx_bool BX_CPU_C::protected_mode(void)
{
  return (BX_CPU_THIS_PTR cpu_mode >= BX_MODE_IA32_PROTECTED);
}

BX_CPP_INLINE bx_bool BX_CPU_C::long_mode(void)
{
#if BX_SUPPORT_X86_64
  return BX_CPU_THIS_PTR efer.get_LMA();
#else
  return 0;
#endif
}

BX_CPP_INLINE bx_bool BX_CPU_C::long64_mode(void)
{
#if BX_SUPPORT_X86_64
  return (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64);
#else
  return 0;
#endif
}

BX_CPP_INLINE unsigned BX_CPU_C::get_cpu_mode(void)
{
  return (BX_CPU_THIS_PTR cpu_mode);
}

#if BX_SUPPORT_ALIGNMENT_CHECK && BX_CPU_LEVEL >= 4
BX_CPP_INLINE bx_bool BX_CPU_C::alignment_check(void)
{
  return BX_CPU_THIS_PTR alignment_check_mask;
}
#endif

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_xsave(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].ecx & BX_CPUID_EXT_XSAVE);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_pcid(void)
{
#if BX_SUPPORT_X86_64
  return (BX_CPU_THIS_PTR cpuid_std_function[1].ecx & BX_CPUID_EXT_PCID);
#else
  return 0;
#endif
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_fsgsbase(void)
{
#if BX_SUPPORT_X86_64
  return BX_CPU_THIS_PTR cpuid_std_function[7].ebx & 0x1;
#else
  return 0;
#endif
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_vme(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_VME);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_debug_extensions(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_DEBUG_EXTENSIONS);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_pse(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_PSE);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_pae(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_PAE);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_pge(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_GLOBAL_PAGES);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_pse36(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_PSE36);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_mmx(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_MMX);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_sse(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_SSE);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_sep(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_SYSENTER_SYSEXIT);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_fxsave_fxrstor(void)
{
  return (BX_CPU_THIS_PTR cpuid_std_function[1].edx & BX_CPUID_STD_FXSAVE_FXRSTOR);
}

BX_CPP_INLINE int BX_CPU_C::bx_cpuid_support_1g_paging(void)
{
#if BX_SUPPORT_X86_64
  return (BX_CPU_THIS_PTR cpuid_ext_function[1].edx & BX_CPUID_STD2_1G_PAGES);
#else
  return 0;
#endif
}

BX_CPP_INLINE void BX_CPU_C::set_PF_base(Bit8u val)
{
  BX_CPU_THIS_PTR lf_flags_status &= ~EFlagsPFMask;
  val = bx_parity_lookup[val]; // Always returns 0 or 1.
  BX_CPU_THIS_PTR eflags &= ~(1<<2);
  BX_CPU_THIS_PTR eflags |= val<<2;
}

//
// inline simple lazy flags implementation methods
//
BX_CPP_INLINE bx_bool BX_CPU_C::get_ZFLazy(void)
{
  return (BX_CPU_THIS_PTR oszapc.result == 0);
}

BX_CPP_INLINE bx_bool BX_CPU_C::get_SFLazy(void)
{
  return (BX_CPU_THIS_PTR oszapc.result >> BX_LF_SIGN_BIT);
}

BX_CPP_INLINE bx_bool BX_CPU_C::get_PFLazy(void)
{
  return bx_parity_lookup[(Bit8u) BX_CPU_THIS_PTR oszapc.result];
}

IMPLEMENT_EFLAG_ACCESSOR   (ID,  21)
IMPLEMENT_EFLAG_ACCESSOR   (VIP, 20)
IMPLEMENT_EFLAG_ACCESSOR   (VIF, 19)
IMPLEMENT_EFLAG_ACCESSOR   (AC,  18)
IMPLEMENT_EFLAG_ACCESSOR   (VM,  17)
IMPLEMENT_EFLAG_ACCESSOR   (RF,  16)
IMPLEMENT_EFLAG_ACCESSOR   (NT,  14)
IMPLEMENT_EFLAG_ACCESSOR_IOPL(   12)
IMPLEMENT_EFLAG_ACCESSOR   (DF,  10)
IMPLEMENT_EFLAG_ACCESSOR   (IF,   9)
IMPLEMENT_EFLAG_ACCESSOR   (TF,   8)

IMPLEMENT_EFLAG_SET_ACCESSOR         (ID,  21)
IMPLEMENT_EFLAG_SET_ACCESSOR         (VIP, 20)
IMPLEMENT_EFLAG_SET_ACCESSOR         (VIF, 19)
#if BX_SUPPORT_ALIGNMENT_CHECK && BX_CPU_LEVEL >= 4
IMPLEMENT_EFLAG_SET_ACCESSOR_AC      (     18)
#else
IMPLEMENT_EFLAG_SET_ACCESSOR         (AC,  18)
#endif
IMPLEMENT_EFLAG_SET_ACCESSOR_VM      (     17)
IMPLEMENT_EFLAG_SET_ACCESSOR_IF_RF_TF(RF,  16)
IMPLEMENT_EFLAG_SET_ACCESSOR         (NT,  14)
IMPLEMENT_EFLAG_SET_ACCESSOR         (DF,  10)
IMPLEMENT_EFLAG_SET_ACCESSOR_IF_RF_TF(IF,   9)
IMPLEMENT_EFLAG_SET_ACCESSOR_IF_RF_TF(TF,   8)
                               
#define BX_TASK_FROM_CALL       0
#define BX_TASK_FROM_IRET       1
#define BX_TASK_FROM_JUMP       2
#define BX_TASK_FROM_INT        3

// exception types for interrupt method
enum {
  BX_EXTERNAL_INTERRUPT = 0,
  BX_NMI = 2,
  BX_HARDWARE_EXCEPTION = 3,  // all exceptions except #BP and #OF
  BX_SOFTWARE_INTERRUPT = 4,
  BX_PRIVILEGED_SOFTWARE_INTERRUPT = 5,
  BX_SOFTWARE_EXCEPTION = 6   // they are software exceptions
};

// <TAG-DEFINES-DECODE-START>

//
// For decoding...
//

// If the BxImmediate mask is set, the lowest 4 bits of the attribute
// specify which kinds of immediate data required by instruction.

#define BxImmediate         0x000f // bits 3..0: any immediate
#define BxImmediate_I1      0x0001 // imm8 = 1
#define BxImmediate_Ib      0x0002 // 8 bit
#define BxImmediate_Ib_SE   0x0003 // sign extend to OS size
#define BxImmediate_Iw      0x0004 // 16 bit
#define BxImmediate_Id      0x0005 // 32 bit
#define BxImmediate_O       0x0006 // MOV_ALOd, mov_OdAL, mov_eAXOv, mov_OveAX
#if BX_SUPPORT_X86_64
#define BxImmediate_Iq      0x0007 // 64 bit override
#endif
#define BxImmediate_BrOff8  0x0008 // Relative branch offset byte

#define BxImmediate_BrOff16 BxImmediate_Iw    // Relative branch offset word, not encodable in 64-bit mode
#define BxImmediate_BrOff32 BxImmediate_Id    // Relative branch offset dword

// Lookup for opcode and attributes in another opcode tables
// Totally 15 opcode groups supported
#define BxGroupX            0x00f0 // bits 7..4: opcode groups definition
#define BxPrefixSSE66       0x0010 // Group encoding: 0001, SSE_PREFIX_66
#define BxPrefixSSEF3       0x0020 // Group encoding: 0010, SSE_PREFIX_F3
#define BxPrefixSSEF2       0x0030 // Group encoding: 0011, SSE_PREFIX_F2
#define BxPrefixSSE         0x0040 // Group encoding: 0100
#define BxGroupN            0x0050 // Group encoding: 0101
#define BxSplitGroupN       0x0060 // Group encoding: 0110
#define BxFPEscape          0x0070 // Group encoding: 0111
#define Bx3ByteOp           0x0080 // Group encoding: 1000
#define BxOSizeGrp          0x0090 // Group encoding: 1001

// The BxImmediate2 mask specifies kind of second immediate data
// required by instruction.
#define BxImmediate2        0x0300 // bits 8.9: any immediate
#define BxImmediate_Ib2     0x0100
#define BxImmediate_Iw2     0x0200
#define BxImmediate_Id2     0x0300

#define BxLockable          0x0400 // bit 10
#define BxArithDstRM        0x0800 // bit 11

#if BX_SUPPORT_TRACE_CACHE
  #define BxTraceEnd        0x1000 // bit 12
#else
  #define BxTraceEnd        0
#endif

#ifdef BX_TRACE_CACHE_NO_SPECULATIVE_TRACING
  #define BxTraceJCC      BxTraceEnd
#else
  #define BxTraceJCC      0
#endif

#define BxGroup1          BxGroupN
#define BxGroup1A         BxGroupN
#define BxGroup2          BxGroupN
#define BxGroup3          BxGroupN
#define BxGroup4          BxGroupN
#define BxGroup5          BxGroupN
#define BxGroup6          BxGroupN
#define BxGroup7          BxFPEscape
#define BxGroup8          BxGroupN
#define BxGroup9          BxGroupN

#define BxGroup11         BxGroupN
#define BxGroup12         BxGroupN
#define BxGroup13         BxGroupN
#define BxGroup14         BxGroupN
#define BxGroup15         BxSplitGroupN

#define BxGroupFP         BxSplitGroupN

// <TAG-DEFINES-DECODE-END>

#define setEFlagsOSZAPC(flags32) {                                      \
  BX_CPU_THIS_PTR eflags = (BX_CPU_THIS_PTR eflags & ~EFlagsOSZAPCMask) \
    | (flags32 & EFlagsOSZAPCMask);                                     \
  BX_CPU_THIS_PTR lf_flags_status = 0;                                  \
}

#define ASSERT_FLAGS_OxxxxC() {                                         \
  BX_CPU_THIS_PTR eflags |= (EFlagsOFMask | EFlagsCFMask);              \
  BX_CPU_THIS_PTR lf_flags_status &= ~(EFlagsOFMask | EFlagsCFMask);    \
}

#define SET_FLAGS_OxxxxC(new_of, new_cf) {                              \
  BX_CPU_THIS_PTR eflags &= ~((EFlagsOFMask | EFlagsCFMask));           \
  BX_CPU_THIS_PTR eflags |= ((new_of)<<11) | (new_cf);                  \
  BX_CPU_THIS_PTR lf_flags_status &= ~((EFlagsOFMask | EFlagsCFMask));  \
}

#endif  // #ifndef BX_CPU_H
