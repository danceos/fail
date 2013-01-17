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

/**
 * \enum GPRegisterId
 * Symbolic identifier to access the x86 general purpose register
 * (within the corresponding GP set). This enumeration is extended
 * in case the activated simulator has 64 bit ability.
 */
enum GPRegisterId {
 #ifdef SIM_SUPPORT_64 // 64 bit register id's:
	RID_RAX = 0, RID_RCX, RID_RDX, RID_RBX, RID_RSP, RID_RBP, RID_RSI, RID_RDI,
	RID_R8, RID_R9, RID_R10, RID_R11, RID_R12, RID_R13, RID_R14, RID_R15,
 #else // 32 bit register id's:
	RID_EAX = 0, RID_ECX, RID_EDX, RID_EBX, RID_ESP, RID_EBP, RID_ESI, RID_EDI,
 #endif // common register id's (independent of the current register width):
    RID_CAX = 0, RID_CCX, RID_CDX, RID_CBX, RID_CSP, RID_CBP, RID_CSI, RID_CDI,
    RID_LAST_GP_ID
};
// FIXME: RID_RSP/RID_ESP/RID_CSP are not GP registers but this definition makes
// it much easier to map the ids to Bochs' (!!!! -> FIXME!) internal register ids.

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
enum FlagsRegisterId { RID_FLAGS = RID_LAST_PC_ID };

} // end-of-namespace: fail

#endif // __X86_ARCHITECTURE_HPP__
