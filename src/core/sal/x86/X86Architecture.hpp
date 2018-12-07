#ifndef __X86_ARCHITECTURE_HPP__
#define __X86_ARCHITECTURE_HPP__

#include "../CPU.hpp"
#include "../CPUState.hpp"
#include "../SALConfig.hpp"

namespace fail {

/**
 * \class X86Architecture
 * This class adds x86 specific functionality to the base architecture.
 * This can be used for every simulator backend that runs on x86.
 */
class X86Architecture : public CPUArchitecture {
public:
	X86Architecture();
	~X86Architecture();
};

#ifdef BUILD_X86
typedef X86Architecture Architecture;
#endif

/**
 * \enum GPRegisterId
 * Symbolic identifiers to access the x86 general purpose registers (within the
 * corresponding GP set). This enumeration is extended in case the activated
 * simulator has 64 bit ability.
 */
enum GPRegisterId {
 #ifdef SIM_SUPPORT_64 // 64 bit register IDs:
	RID_RAX = 0, RID_RCX, RID_RDX, RID_RBX, RID_RSP, RID_RBP, RID_RSI, RID_RDI,
	RID_R8, RID_R9, RID_R10, RID_R11, RID_R12, RID_R13, RID_R14, RID_R15,
 #else // 32 bit register IDs:
	RID_EAX = 0, RID_ECX, RID_EDX, RID_EBX, RID_ESP, RID_EBP, RID_ESI, RID_EDI,
	// skip a few IDs to get identical numbers for special-purpose registers in
	// 32 and 64 bit setups:
	SKIP_64BIT_IDS = 15,
 #endif
	RID_LAST_GP_ID,
	// common register IDs (independent of the current register width):
	RID_CAX = 0, RID_CCX, RID_CDX, RID_CBX, RID_CSP, RID_CBP, RID_CSI, RID_CDI
};

/**
 * \enum PCRegisterId
 * Symbolic identifier to access the program counter (PC, aka
 * instruction pointer, in short IP) register.
 */
enum PCRegisterId { RID_PC = RID_LAST_GP_ID, RID_LAST_PC_ID };

/**
 * \enum FlagsRegisterId
 * Symbolic identifier to access the flags register.
 */
enum FlagsRegisterId { RID_FLAGS = RID_LAST_PC_ID, RID_LAST_FLAGS_ID };

/**
 * \enum SegmentRegisterId
 * Symbolic identifiers to access the segment registers.
 */
enum SegmentRegisterId { RID_CS = RID_LAST_FLAGS_ID, RID_DS, RID_ES, RID_FS,
	RID_GS, RID_SS, RID_LAST_SEGMENT_ID};

/**
 * \enum ControlRegisterId
 * Symbolic identifiers to access the control registers.
 */
enum ControlRegisterId { RID_CR0 = RID_LAST_SEGMENT_ID, RID_CR1, RID_CR2, RID_CR3, RID_CR4, RID_LAST_CR_ID };

/**
 * \enum FPURegisterId
 * Symbolic identifiers to access FPU registers.
 */
enum FPURegisterId {
	RID_FSW = RID_LAST_CR_ID, RID_FCW, RID_FTW,
	/* FPRi_LO is fraction (64 bit), FPRi_HI is exponent (16 bit), total 80 bits */
	RID_FPR0_LO, RID_FPR0_HI, RID_FPR1_LO, RID_FPR1_HI, RID_FPR2_LO, RID_FPR2_HI, RID_FPR3_LO, RID_FPR3_HI,
	RID_FPR4_LO, RID_FPR4_HI, RID_FPR5_LO, RID_FPR5_HI, RID_FPR6_LO, RID_FPR6_HI, RID_FPR7_LO, RID_FPR7_HI,
	/* MMXi = RID_FPRi_LO */
	RID_MMX0 = RID_FPR0_LO, RID_MMX1 = RID_FPR1_LO, RID_MMX2 = RID_FPR2_LO, RID_MMX3 = RID_FPR3_LO,
	RID_MMX4 = RID_FPR4_LO, RID_MMX5 = RID_FPR5_LO, RID_MMX6 = RID_FPR6_LO, RID_MMX7 = RID_FPR7_LO,
	RID_MMXDUMMY = RID_FPR7_HI, RID_LAST_FP_ID
};

/**
 * \enum VectorRegisterId
 * Symbolic identifiers to access vector-unit registers (SSE, AVX, ...).
 */
enum VectorRegisterId {
	/* low / high 64 bits */
	RID_XMM0_LO = RID_LAST_FP_ID, RID_XMM0_HI, RID_XMM1_LO, RID_XMM1_HI, RID_XMM2_LO, RID_XMM2_HI, RID_XMM3_LO, RID_XMM3_HI,
	RID_XMM4_LO, RID_XMM4_HI, RID_XMM5_LO, RID_XMM5_HI, RID_XMM6_LO, RID_XMM6_HI, RID_XMM7_LO, RID_XMM7_HI,
#ifdef SIM_SUPPORT_64
	RID_XMM8_LO, RID_XMM8_HI, RID_XMM9_LO, RID_XMM9_HI, RID_XMM10_LO, RID_XMM10_HI, RID_XMM11_LO, RID_XMM11_HI,
	RID_XMM12_LO, RID_XMM12_HI, RID_XMM13_LO, RID_XMM13_HI, RID_XMM14_LO, RID_XMM14_HI, RID_XMM15_LO, RID_XMM15_HI,
#endif
	RID_MXCSR, /* MXCSR_MASK? */
	RID_LAST_VECTOR_ID
};

// TODO GDTR, LDTR, IDTR, TR6+7, DR0-7, TR, MSR*

} // end-of-namespace: fail

#endif // __X86_ARCHITECTURE_HPP__
