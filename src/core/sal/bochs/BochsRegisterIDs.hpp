#ifndef __BOCHS_REGISTER_IDS_HPP__
  #define __BOCHS_REGISTER_IDS_HPP__

// FIXME: This should be part of an x86 architecture-specific header, outside
// the bochs/ subdirectory, and be usable from a campaign without an existing
// simulator backend.  (Also, iterating over registers should be possible.)

namespace fail {
/**
 * \enum GPRegisterId
 * Symbolic identifier to access Bochs' general purpose register
 * (within the corresponding GP set), e.g.
 * \code
 * // Print %eax register data:
 * BochsController bc(...);
 * cout << bc.getRegisterManager().getSetOfType(RT_GP)
 *           .getRegister(RID_EAX)->getData();
 * \endcode
 */
enum GPRegisterId {
 #if BX_SUPPORT_X86_64 // 64 bit register id's:
	RID_RAX = 0, RID_RCX, RID_RDX, RID_RBX, RID_RSP, RID_RBP, RID_RSI, RID_RDI,
	RID_R8, RID_R9, RID_R10, RID_R11, RID_R12, RID_R13, RID_R14, RID_R15,
 #else // 32 bit register id's:
	RID_EAX = 0, RID_ECX, RID_EDX, RID_EBX, RID_ESP, RID_EBP, RID_ESI, RID_EDI,
 #endif // common register id's (independent of the current register width):
    RID_CAX = 0, RID_CCX, RID_CDX, RID_CBX, RID_CSP, RID_CBP, RID_CSI, RID_CDI,
    RID_LAST_GP_ID
};

/**
 * \enum PCRegisterId
 * Symbolic identifier to access Bochs' program counter register.
 */
enum PCRegisterId { RID_PC = RID_LAST_GP_ID, RID_LAST_PC_ID };

/**
 * \enum FlagsRegisterId
 * Symbolic identifier to access Bochs' flags register.
 */
enum FlagsRegisterId { RID_FLAGS = RID_LAST_PC_ID };

}

#endif
