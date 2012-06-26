#ifndef __L4SYS_ALUINSTR_HPP__
  #define __L4SYS_ALUINSTR_HPP__

/**
 * Forward declaration of the Bochs instruction decode table.
 * This is necessary because, inconveniently, it is not declared in a header file.
 */
#include "cpu/fetchdecode.h"
static const BxOpcodeInfo_t *BxOpcodeInfo32;

/**
 * Trying to order X86 ALU instructions.
 * Each instruction class contains instructions
 * of roughly equal length and operands, so that
 * all the bxInstruction_c structures residing within
 * one class contain the same member fields.
 */
enum X86AluClass {
	ALU_UNDEF = 0,
	ALU_RM8,
	ALU_RM16,
	ALU_RM32,
	ALU_RM8_ONE,
	ALU_RM16_ONE,
	ALU_RM32_ONE,
	ALU_REG,
	ALU_IMM8_REG,
	ALU_IMM16_REG,
	ALU_IMM32_REG,
	ALU_IMM8_RM8,
	ALU_IMM8_RM16,
	ALU_IMM8_RM32,
	ALU_IMM16_RM16,
	ALU_IMM32_RM32,
	ALU_REG_RM8,
	ALU_REG_RM16,
	ALU_REG_RM32
};

/**
 * A struct describing a specific x86
 * ALU instruction in terms of Bochs.
 */
struct BochsALUInstr {
	/**
	 * the Bochs operation ID (confusingly called ia_opcode
	 * in bxInstruction_c), pointing to several bits of information,
	 * for instance what simulator function to execute
	 */
    Bit16u bochs_operation;
    /**
     * the x86 opcode (known as b1 in bxInstruction_c)
     */
	Bit8u opcode;
	/**
	 * the reg part of the modr/m field (known as nnn in bxInstruction_c)
	 * A value of 8 or higher marks this field unused.
	 */
	Bit8u reg;
	/**
	 * the register offset of the instruction byte of this instruction
	 * Some x86 instructions, like INC register, add a register offset
	 * to their opcode. It is necessary to store this separately for
	 * several reasons, one being the ability to separate
	 * ALU input latch faults from ALU instruction latch faults.
	 * A value of 8 or higher marks this field unused.
	 */
	Bit8u opcodeRegisterOffset;
	/**
	 * the ALU class this instruction belongs to
	 */
	X86AluClass aluClass;
};


/**
 * Array containing most (all?) Bochs ALU commands.
 * Attention: here, \a reg and \a opcodeRegisterOffset, if less than 8,
 * define the maximum value possible in this field,
 * according to the Bochs instruction decoder function.
 * (see the BxOpcodeInfoG... arrays in cpu/fetchdecode.h)
 */
const BochsALUInstr aluInstructions [] = {
	// Now is a great time to open Volume 2 of Intel's IA-32 documentation.

	/* --- /// UNARY OPERATIONS \\\ --- */

	// 8-bit
	{ BX_IA_INC_Eb, 0xFE, 0, 8, ALU_RM8},
	{ BX_IA_DEC_Eb, 0xFE, 1, 8, ALU_RM8},
	{ BX_IA_NOT_Eb, 0xF6, 2, 8, ALU_RM8},
	{ BX_IA_NEG_Eb, 0xF6, 3, 8, ALU_RM8},

	// 16-bit
	{ BX_IA_INC_Ew, 0xFF, 0, 8, ALU_RM16},
	{ BX_IA_DEC_Ew, 0xFF, 1, 8, ALU_RM16},
	{ BX_IA_NOT_Ew, 0xF7, 2, 8, ALU_RM16},
	{ BX_IA_NEG_Ew, 0xF7, 3, 8, ALU_RM16},

	// 32-bit
	{ BX_IA_INC_Ed, 0xFF, 0, 8, ALU_RM32},
	{ BX_IA_DEC_Ed, 0xFF, 1, 8, ALU_RM32},
	{ BX_IA_NOT_Ed, 0xF7, 2, 8, ALU_RM32},
	{ BX_IA_NEG_Ed, 0xF7, 3, 8, ALU_RM32},

	// register
	{ BX_IA_INC_RX, 0x40, 8, 7, ALU_REG},
	{ BX_IA_DEC_RX, 0x48, 8, 7, ALU_REG},
	{ BX_IA_INC_ERX, 0x40, 8, 7, ALU_REG},
	{ BX_IA_DEC_ERX, 0x48, 8, 7, ALU_REG},

	/* --- \\\ UNARY OPERATIONS /// --- */

	/* --- /// SHIFT OPERATIONS \\\ --- */

	// a macro to reduce copy-paste overhead
#define SHIFTOPS(OPCODE, WD, CLASS) \
		{ BX_IA_RCL_E##WD, OPCODE, 2, 8, CLASS }, \
		{ BX_IA_RCR_E##WD, OPCODE, 3, 8, CLASS }, \
		{ BX_IA_ROL_E##WD, OPCODE, 0, 8, CLASS }, \
		{ BX_IA_ROR_E##WD, OPCODE, 1, 8, CLASS }, \
		{ BX_IA_SHL_E##WD, OPCODE, 4, 8, CLASS }, \
		{ BX_IA_SAR_E##WD, OPCODE, 7, 8, CLASS }, \
		{ BX_IA_SHL_E##WD, OPCODE, 6, 8, CLASS }, \
		{ BX_IA_SHR_E##WD, OPCODE, 5, 8, CLASS }

	// first shifting by one bit
	SHIFTOPS(0xD0, b, ALU_RM8_ONE),
	SHIFTOPS(0xD1, w, ALU_RM16_ONE),
	SHIFTOPS(0xD1, d, ALU_RM32_ONE),

	// then shifting by CL bits
	SHIFTOPS(0xD2, b, ALU_REG_RM8),
	SHIFTOPS(0xD3, w, ALU_REG_RM16),
	SHIFTOPS(0xD3, d, ALU_REG_RM32),

	// then shifting by a number of bits given via an immediate value
	SHIFTOPS(0xC0, b, ALU_IMM8_RM8),
	SHIFTOPS(0xC1, w, ALU_IMM8_RM16),
	SHIFTOPS(0xC1, d, ALU_IMM8_RM32),
#undef SHIFTOPS
	/* --- \\\ SHIFT OPERATIONS /// --- */

	/**
	 *
	 * The remaining instructions, roughly ordered,
	 * in the form of a (probably obsolete) experiment method
	 *	/**
	 * Assigns a given opcode a class of ALU instructions
	 * @param opcode the opcode to examine
	 * @returns an enum AluClass object

	X86AluClass isALUInstruction(unsigned opcode);
	 *
	 * X86AluClass L4SysExperiment::isALUInstruction(unsigned opcode) {
	switch (opcode) {
	case BX_IA_ADC_EbGb:
	case BX_IA_ADC_EdGd:
	case BX_IA_ADC_EwGw:
	case BX_IA_ADD_EbGb:
	case BX_IA_ADD_EdGd:
	case BX_IA_ADD_EwGw:
	case BX_IA_AND_EbGb:
	case BX_IA_AND_EdGd:
	case BX_IA_AND_EwGw:
	case BX_IA_CMP_EbGb:
	case BX_IA_CMP_EdGd:
	case BX_IA_CMP_EwGw:
	case BX_IA_OR_EbGb:
	case BX_IA_OR_EdGd:
	case BX_IA_OR_EwGw:
	case BX_IA_SBB_EbGb:
	case BX_IA_SBB_EdGd:
	case BX_IA_SBB_EwGw:
	case BX_IA_SUB_EbGb:
	case BX_IA_SUB_EdGd:
	case BX_IA_SUB_EwGw:
	case BX_IA_XOR_EbGb:
	case BX_IA_XOR_EdGd:
	case BX_IA_XOR_EwGw:
	case BX_IA_ADC_ALIb:
	case BX_IA_ADC_AXIw:
	case BX_IA_ADC_EAXId:
	case BX_IA_ADD_EbIb:
	case BX_IA_OR_EbIb:
	case BX_IA_ADC_EbIb:
	case BX_IA_SBB_EbIb:
	case BX_IA_AND_EbIb:
	case BX_IA_SUB_EbIb:
	case BX_IA_XOR_EbIb:
	case BX_IA_CMP_EbIb:
	case BX_IA_ADD_EwIw:
	case BX_IA_OR_EwIw:
	case BX_IA_ADC_EwIw:
	case BX_IA_SBB_EwIw:
	case BX_IA_AND_EwIw:
	case BX_IA_SUB_EwIw:
	case BX_IA_XOR_EwIw:
	case BX_IA_CMP_EwIw:
	case BX_IA_ADD_EdId:
	case BX_IA_OR_EdId:
	case BX_IA_ADC_EdId:
	case BX_IA_SBB_EdId:
	case BX_IA_AND_EdId:
	case BX_IA_SUB_EdId:
	case BX_IA_XOR_EdId:
	case BX_IA_CMP_EdId:
	case BX_IA_ADC_GbEb:
	case BX_IA_ADC_GwEw:
	case BX_IA_ADC_GdEd:
	case BX_IA_ADD_ALIb:
	case BX_IA_ADD_AXIw:
	case BX_IA_ADD_EAXId:
	case BX_IA_ADD_GbEb:
	case BX_IA_ADD_GwEw:
	case BX_IA_ADD_GdEd:
	case BX_IA_AND_ALIb:
	case BX_IA_AND_AXIw:
	case BX_IA_AND_EAXId:
	case BX_IA_AND_GbEb:
	case BX_IA_AND_GwEw:
	case BX_IA_AND_GdEd:
	default:
		return ALU_UNDEF;
	}
}
	 *
	 */
};
#endif // __L4SYS_ALUINSTR_HPP__
