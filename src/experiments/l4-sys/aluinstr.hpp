#ifndef __L4SYS_ALUINSTR_HPP__
  #define __L4SYS_ALUINSTR_HPP__

#if 0
/**
 * Forward declaration of the Bochs instruction decode table.
 * This is necessary because, inconveniently, it is not declared in a header file.
 */
#include "cpu/fetchdecode.h"
static const BxOpcodeInfo_t *BxOpcodeInfo32;
#endif

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
	 * the reg part of the modr/m field (known as "nnn" in bxInstruction_c)
	 * it is used to
	 *   a) further subdivide the functionality of a given opcode
	 *   b) specify a register the instruction is supposed to use
	 * In this class, a value of 8 or higher marks this field unused.
	 */
	Bit8u reg;
	/**
	 * the register offset of the instruction byte of this instruction
	 * Some x86 instructions, like INC register, add a register offset
	 * to their opcode. This offset is stored as "rm" in bxInstruction_c.
	 * In this class, a value of 8 or higher marks this field unused.
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
	SHIFTOPS(0xD0, b, ALU_RM8),
	SHIFTOPS(0xD1, w, ALU_RM16),
	SHIFTOPS(0xD1, d, ALU_RM32),

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

	/* --- /// BINARY OPERATIONS \\\ --- */

	// register ax, immediate
#define BINOPS(IACODE, OPCODE8, OPCODE16) \
		{ BX_IA_##IACODE##_ALIb, OPCODE8, 8, 8, ALU_IMM8_REG }, \
		{ BX_IA_##IACODE##_AXIw, OPCODE16, 8, 8, ALU_IMM16_REG }, \
		{ BX_IA_##IACODE##_EAXId, OPCODE16, 8, 8, ALU_IMM32_REG }

	BINOPS(ADC, 0x14, 0x15),
	BINOPS(ADD, 0x04, 0x05),
	BINOPS(AND, 0x24, 0x25),
	BINOPS(CMP, 0x3C, 0x3D),
	BINOPS(OR, 0x0C, 0x0D),
	BINOPS(SBB, 0x1C, 0x1D),
	BINOPS(SUB, 0x2C, 0x2D),
	BINOPS(XOR, 0x34, 0x35),
#undef BINOPS

	// r/m, immediate
#define BINOPS(OPCODE, WDE, WDI, CLASS) \
		{ BX_IA_ADC_E##WDE##I##WDI, OPCODE, 2, 8, CLASS }, \
		{ BX_IA_ADD_E##WDE##I##WDI, OPCODE, 0, 8, CLASS }, \
		{ BX_IA_AND_E##WDE##I##WDI, OPCODE, 4, 8, CLASS }, \
		{ BX_IA_CMP_E##WDE##I##WDI, OPCODE, 7, 8, CLASS }, \
		{ BX_IA_OR_E##WDE##I##WDI, OPCODE, 1, 8, CLASS }, \
		{ BX_IA_SBB_E##WDE##I##WDI, OPCODE, 3, 8, CLASS }, \
		{ BX_IA_SUB_E##WDE##I##WDI, OPCODE, 5, 8, CLASS }, \
		{ BX_IA_XOR_E##WDE##I##WDI, OPCODE, 6, 8, CLASS }

	BINOPS(0x80, b, b, ALU_IMM8_RM8),
	BINOPS(0x81, w, w, ALU_IMM16_RM16),
	BINOPS(0x81, d, d, ALU_IMM32_RM32),
	BINOPS(0x83, w, w, ALU_IMM8_RM16),
	BINOPS(0x83, d, d, ALU_IMM8_RM32),
#undef BINOPS

	// r/m, arbitrary register
#define BINOPS(IACODE, OPCODE8, OPCODE16) \
		{ BX_IA_##IACODE##_EbGb, OPCODE8, 7, 8, ALU_REG_RM8 }, \
		{ BX_IA_##IACODE##_EwGw, OPCODE16, 7, 8, ALU_REG_RM16 }, \
		{ BX_IA_##IACODE##_EdGd, OPCODE16, 7, 8, ALU_REG_RM32 }

	BINOPS(ADC, 0x10, 0x11),
	BINOPS(ADD, 0x00, 0x01),
	BINOPS(AND, 0x20, 0x21),
	BINOPS(CMP, 0x38, 0x39),
	BINOPS(OR, 0x08, 0x09),
	BINOPS(SBB, 0x18, 0x19),
	BINOPS(SUB, 0x28, 0x29),
	BINOPS(XOR, 0x30, 0x31),
#undef BINOPS

	// arbitrary register, r/m
#define BINOPS(IACODE, OPCODE8, OPCODE16) \
		{ BX_IA_##IACODE##_GbEb, OPCODE8, 7, 8, ALU_REG_RM8 }, \
		{ BX_IA_##IACODE##_GwEw, OPCODE16, 7, 8, ALU_REG_RM16 }, \
		{ BX_IA_##IACODE##_GdEd, OPCODE16, 7, 8, ALU_REG_RM32 }

	BINOPS(ADC, 0x12, 0x13),
	BINOPS(ADD, 0x02, 0x03),
	BINOPS(AND, 0x22, 0x23),
	BINOPS(CMP, 0x3a, 0x3b),
	BINOPS(OR, 0x0a, 0x0b),
	BINOPS(SBB, 0x1a, 0x1b),
	BINOPS(SUB, 0x2a, 0x2b),
	BINOPS(XOR, 0x32, 0x33),
#undef BINOPS

	/* --- \\\ BINARY OPERATIONS /// --- */
};

#endif // __L4SYS_ALUINSTR_HPP__
