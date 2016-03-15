#ifndef __ARM_ARCHITECURE_HPP__
#define __ARM_ARCHITECURE_HPP__

#include "../CPU.hpp"
#include "../CPUState.hpp"

namespace fail {

/**
 * \class ArmArchitecture
 * This class adds ARM specific functionality to the base architecture.
 * This can be used for every simulator backend that runs on ARM.
 */
class ArmArchitecture : public CPUArchitecture {
public:
	ArmArchitecture();
	~ArmArchitecture();
};

#ifdef BUILD_ARM
typedef ArmArchitecture Architecture;
#endif

/**
 * \enum GPRegIndex
 * Defines the general purpose (GP) register identifier for the ARM
 * plattform. Some of them are just aliases.
 *
 * Must currently stay in sync with register IDs defined in
 *   simulators/gem5/src/arch/arm/miscregs.hh and
 *   simulators/gem5/src/arch/arm/intregs.hh
 * which we cannot include because they are generated (probably after we need
 * them).  As integer and "misc" register IDs overlap in gem5, and FAIL* needs
 * unique IDs, we split at RI_INTREGS_MAX and map to the original IDs in
 * sal/gem5/Gem5Wrapper.cc .  If more ARM backends emerge, we may need to find
 * more sophisticated backend<->FAIL* register ID mappings.
 */
enum GPRegIndex {
	RI_R0,
	RI_R1,
	RI_R2,
	RI_R3,
	RI_R4,
	RI_R5,
	RI_R6,
	RI_R7,
	RI_R8,
	RI_R9,
	RI_R10,
	RI_R11,
	RI_R12,
	RI_R13,
	RI_SP = RI_R13,
	RI_R14,
	RI_LR = RI_R14,
	RI_R15,
	RI_IP = RI_R15,

	RI_R13_SVC,
	RI_SP_SVC = RI_R13_SVC,
	RI_R14_SVC,
	RI_LR_SVC = RI_R14_SVC,

	RI_R13_MON,
	RI_SP_MON = RI_R13_MON,
	RI_R14_MON,
	RI_LR_MON = RI_R14_MON,

	RI_R13_ABT,
	RI_SP_ABT = RI_R13_ABT,
	RI_R14_ABT,
	RI_LR_ABT = RI_R14_ABT,

	RI_R13_UND,
	RI_SP_UND = RI_R13_UND,
	RI_R14_UND,
	RI_LR_UND = RI_R14_UND,

	RI_R13_IRQ,
	RI_SP_IRQ = RI_R13_IRQ,
	RI_R14_IRQ,
	RI_LR_IRQ = RI_R14_IRQ,

	RI_R8_FIQ,
	RI_R9_FIQ,
	RI_R10_FIQ,
	RI_R11_FIQ,
	RI_R12_FIQ,
	RI_R13_FIQ,
	RI_SP_FIQ = RI_R13_FIQ,
	RI_R14_FIQ,
	RI_LR_FIQ = RI_R14_FIQ,

	RI_INTREGS_MAX,

	// misc regs
	RI_CPSR = RI_INTREGS_MAX,
	RI_CPSR_Q,
	RI_SPSR,
	RI_SPSR_FIQ,
	RI_SPSR_IRQ,
	RI_SPSR_SVC,
	RI_SPSR_MON,
	RI_SPSR_UND,
	RI_SPSR_ABT,
	RI_FPSR,
	RI_FPSID,
	RI_FPSCR,
	RI_FPSCR_QC,  // Cumulative saturation flag
	RI_FPSCR_EXC,  // Cumulative FP exception flags
	RI_FPEXC,
	RI_MVFR0,
	RI_MVFR1,
	RI_SCTLR_RST,
	RI_SEV_MAILBOX,

	// CP14 registers
	RI_CP14_START,
	RI_DBGDIDR = RI_CP14_START,
	RI_DBGDSCR_INT,
	RI_DBGDTRRX_INT,
	RI_DBGTRTX_INT,
	RI_DBGWFAR,
	RI_DBGVCR,
	RI_DBGECR,
	RI_DBGDSCCR,
	RI_DBGSMCR,
	RI_DBGDTRRX_EXT,
	RI_DBGDSCR_EXT,
	RI_DBGDTRTX_EXT,
	RI_DBGDRCR,
	RI_DBGBVR,
	RI_DBGBCR,
	RI_DBGBVR_M,
	RI_DBGBCR_M,
	RI_DBGDRAR,
	RI_DBGBXVR_M,
	RI_DBGOSLAR,
	RI_DBGOSSRR,
	RI_DBGOSDLR,
	RI_DBGPRCR,
	RI_DBGPRSR,
	RI_DBGDSAR,
	RI_DBGITCTRL,
	RI_DBGCLAIMSET,
	RI_DBGCLAIMCLR,
	RI_DBGAUTHSTATUS,
	RI_DBGDEVID2,
	RI_DBGDEVID1,
	RI_DBGDEVID,

	// CP15 registers
	RI_CP15_START,
	RI_SCTLR = RI_CP15_START,
	RI_DCCISW,
	RI_DCCIMVAC,
	RI_DCCMVAC,
	RI_CONTEXTIDR,
	RI_TPIDRURW,
	RI_TPIDRURO,
	RI_TPIDRPRW,
	RI_CP15ISB,
	RI_CP15DSB,
	RI_CP15DMB,
	RI_CPACR,
	RI_CLIDR,
	RI_CCSIDR,
	RI_CSSELR,
	RI_ICIALLUIS,
	RI_ICIALLU,
	RI_ICIMVAU,
	RI_BPIMVA,
	RI_BPIALLIS,
	RI_BPIALL,
	RI_MIDR,
	RI_TTBR0,
	RI_TTBR1,
	RI_TLBTR,
	RI_DACR,
	RI_TLBIALLIS,
	RI_TLBIMVAIS,
	RI_TLBIASIDIS,
	RI_TLBIMVAAIS,
	RI_ITLBIALL,
	RI_ITLBIMVA,
	RI_ITLBIASID,
	RI_DTLBIALL,
	RI_DTLBIMVA,
	RI_DTLBIASID,
	RI_TLBIALL,
	RI_TLBIMVA,
	RI_TLBIASID,
	RI_TLBIMVAA,
	RI_DFSR,
	RI_IFSR,
	RI_DFAR,
	RI_IFAR,
	RI_MPIDR,
	RI_PRRR,
	RI_NMRR,
	RI_TTBCR,
	RI_ID_PFR0,
	RI_CTR,
	RI_SCR,
	RI_SDER,
	RI_PAR,
	RI_V2PCWPR,
	RI_V2PCWPW,
	RI_V2PCWUR,
	RI_V2PCWUW,
	RI_V2POWPR,
	RI_V2POWPW,
	RI_V2POWUR,
	RI_V2POWUW,
	RI_ID_MMFR0,
	RI_ID_MMFR2,
	RI_ID_MMFR3,
	RI_ACTLR,
	RI_PMCR,
	RI_PMCCNTR,
	RI_PMCNTENSET,
	RI_PMCNTENCLR,
	RI_PMOVSR,
	RI_PMSWINC,
	RI_PMSELR,
	RI_PMCEID0,
	RI_PMCEID1,
	RI_PMC_OTHER,
	RI_PMXEVCNTR,
	RI_PMUSERENR,
	RI_PMINTENSET,
	RI_PMINTENCLR,
	RI_ID_ISAR0,
	RI_ID_ISAR1,
	RI_ID_ISAR2,
	RI_ID_ISAR3,
	RI_ID_ISAR4,
	RI_ID_ISAR5,
	RI_CPSR_MODE,
	RI_LOCKFLAG,
	RI_LOCKADDR,
	RI_ID_PFR1,
	RI_L2CTLR,
	RI_CP15_UNIMP_START,
	RI_TCMTR = RI_CP15_UNIMP_START,
	RI_ID_DFR0,
	RI_ID_AFR0,
	RI_ID_MMFR1,
	RI_AIDR,
	RI_ADFSR,
	RI_AIFSR,
	RI_DCIMVAC,
	RI_DCISW,
	RI_MCCSW,
	RI_DCCMVAU,
	RI_NSACR,
	RI_VBAR,
	RI_MVBAR,
	RI_ISR,
	RI_FCEIDR,
	RI_L2LATENCY,
	RI_CRN15,


	RI_CP15_END,
};

} // end-of-namespace: fail

#endif // __ARM_ARCH_HPP__
