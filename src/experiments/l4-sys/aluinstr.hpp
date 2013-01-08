#ifndef __L4SYS_ALUINSTR_HPP__
  #define __L4SYS_ALUINSTR_HPP__

#include <map>
#include <vector>
#include <string>
#include <stdlib.h>
#include "config.h"
#include "cpu/instr.h"

/**
 * \enum X86AluClass
 *
 * \brief An attempt to order X86 ALU instructions.
 *
 * An attempt to order X86 ALU instructions.
 * Each instruction class contains instructions
 * of roughly equal length and operands, so that
 * all the bxInstruction_c structures residing within
 * one class contain the same member fields (except for
 * b1, rm and the execute function pointers).
 */
enum X86AluClass {
	ALU_UNDEF = 0,
	ALU_GENERIC,
	ALU_RM8,
	ALU_RM16,
	ALU_RM32,
	ALU_IMM8,
	ALU_IMM16,
	ALU_IMM32,
	ALU_IMM8_RM8,
	ALU_IMM8_RM16,
	ALU_IMM8_RM32,
	ALU_IMM16_RM16,
	ALU_IMM32_RM32
};

/**
 * \struct BochsALUInstr
 *
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
	 * the x86 opcode, as stored by Bochs (known as b1 in bxInstruction_c)
	 */
	Bit8u opcode;
	/**
	 * the reg part of the modr/m field (known as "nnn" in bxInstruction_c)
	 * it is used to
	 *   a) further subdivide the functionality of a given opcode (reg < REG_COUNT)
	 *   b) specify a register the instruction is supposed to use (reg = REG_COUNT)
	 * In this class, a value greater than REG_COUNT marks this field unused.
	 */
	Bit8u reg;
	/**
	 * the register offset of the instruction byte of this instruction
	 * Some x86 instructions, like INC register, add a register offset
	 * to their opcode. This offset is stored as "rm" in bxInstruction_c.
	 * In this class, a value greater than REG_COUNT marks this field unused.
	 */
	Bit8u opcodeRegisterOffset;
	/**
	 * the ALU class this instruction belongs to
	 */
	X86AluClass aluClass;
	/**
	 * the count of registers
	 */
	static const unsigned REG_COUNT = 8;
	/**
	 * returns true if obj equals this object
	 * the Bochs opcode is generated dynamically and thus not relevant for comparison
	 */
	bool operator==(BochsALUInstr const &obj) const
	{
		return bochs_operation == obj.bochs_operation &&
			opcode == obj.opcode &&
			reg == obj.reg &&
			opcodeRegisterOffset == obj.opcodeRegisterOffset &&
			aluClass == obj.aluClass;
	}
	/**
	 * returns false if obj equals this object
	 * \see operator==
	 */
	bool operator!=(BochsALUInstr const &obj) const {
		return !(*this == obj);
	}
};


/**
 * \var aluInstructions
 *
 * Array containing the Bochs IA-32 Integer ALU commands.
 */
const BochsALUInstr aluInstructions [] = {
	// Now is a great time to open Volume 2 of Intel's IA-32 documentation.

	/* --- /// UNARY OPERATIONS \\\ --- */

	// 8-bit
	{ BX_IA_INC_Eb, 0xFE, 0, BochsALUInstr::REG_COUNT + 1, ALU_RM8},
	{ BX_IA_DEC_Eb, 0xFE, 1, BochsALUInstr::REG_COUNT + 1, ALU_RM8},
	{ BX_IA_NOT_Eb, 0xF6, 2, BochsALUInstr::REG_COUNT + 1, ALU_RM8},
	{ BX_IA_NEG_Eb, 0xF6, 3, BochsALUInstr::REG_COUNT + 1, ALU_RM8},

	// 16-bit
	{ BX_IA_INC_Ew, 0xFF, 0, BochsALUInstr::REG_COUNT + 1, ALU_RM16},
	{ BX_IA_DEC_Ew, 0xFF, 1, BochsALUInstr::REG_COUNT + 1, ALU_RM16},
	{ BX_IA_NOT_Ew, 0xF7, 2, BochsALUInstr::REG_COUNT + 1, ALU_RM16},
	{ BX_IA_NEG_Ew, 0xF7, 3, BochsALUInstr::REG_COUNT + 1, ALU_RM16},

	// 32-bit
	{ BX_IA_INC_Ed, 0xFF, 0, BochsALUInstr::REG_COUNT + 1, ALU_RM32},
	{ BX_IA_DEC_Ed, 0xFF, 1, BochsALUInstr::REG_COUNT + 1, ALU_RM32},
	{ BX_IA_NOT_Ed, 0xF7, 2, BochsALUInstr::REG_COUNT + 1, ALU_RM32},
	{ BX_IA_NEG_Ed, 0xF7, 3, BochsALUInstr::REG_COUNT + 1, ALU_RM32},

	// register
	{ BX_IA_INC_RX, 0x40, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT, ALU_GENERIC},
	{ BX_IA_DEC_RX, 0x48, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT, ALU_GENERIC},
	{ BX_IA_INC_ERX, 0x40, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT, ALU_GENERIC},
	{ BX_IA_DEC_ERX, 0x48, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT, ALU_GENERIC},

	/* --- \\\ UNARY OPERATIONS /// --- */

	/* --- /// BCD ADJUSTMENT \\\ --- */

	{ BX_IA_DAA, 0x27, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_GENERIC},
	{ BX_IA_DAS, 0x2F, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_GENERIC},
	{ BX_IA_AAA, 0x37, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_GENERIC},
	{ BX_IA_AAS, 0x3F, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_GENERIC},
	{ BX_IA_AAM, 0xD4, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM8 },
	{ BX_IA_AAD, 0xD5, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM8 },

	/* --- \\\ BCD ADJUSTMENT /// --- */

	/* --- /// SHIFT OPERATIONS \\\ --- */

	// a macro to reduce copy-paste overhead
#define SHIFTOPS(OPCODE, WD, CLASS) \
		{ BX_IA_RCL_E##WD, OPCODE, 2, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_RCR_E##WD, OPCODE, 3, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_ROL_E##WD, OPCODE, 0, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_ROR_E##WD, OPCODE, 1, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SHL_E##WD, OPCODE, 4, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SAR_E##WD, OPCODE, 7, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SHL_E##WD, OPCODE, 6, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SHR_E##WD, OPCODE, 5, BochsALUInstr::REG_COUNT + 1, CLASS }

	// first shifting by one bit
	SHIFTOPS(0xD0, b, ALU_RM8),
	SHIFTOPS(0xD1, w, ALU_RM16),
	SHIFTOPS(0xD1, d, ALU_RM32),

	// then shifting by CL bits
	SHIFTOPS(0xD2, b, ALU_RM8),
	SHIFTOPS(0xD3, w, ALU_RM16),
	SHIFTOPS(0xD3, d, ALU_RM32),

	// then shifting by a number of bits given via an immediate value
	SHIFTOPS(0xC0, b, ALU_IMM8_RM8),
	SHIFTOPS(0xC1, w, ALU_IMM8_RM16),
	SHIFTOPS(0xC1, d, ALU_IMM8_RM32),

#undef SHIFTOPS
	// SHLD / SHRD (Note: Bochs Opcode; normally 0x0F...)
	{ BX_IA_SHLD_EwGw, 0xA4, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM16 },
	{ BX_IA_SHLD_EdGd, 0xA4, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM32 },
	{ BX_IA_SHLD_EwGw, 0xA5, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16 },
	{ BX_IA_SHLD_EdGd, 0xA5, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32 },
	{ BX_IA_SHRD_EwGw, 0xAC, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM16 },
	{ BX_IA_SHRD_EdGd, 0xAC, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM32 },
	{ BX_IA_SHRD_EwGw, 0xAD, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16 },
	{ BX_IA_SHRD_EdGd, 0xAD, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32 },

	/* --- \\\ SHIFT OPERATIONS /// --- */

	/* --- /// BINARY OPERATIONS \\\ --- */

	// register ax, immediate
#define BINOPS(IACODE, OPCODE8, OPCODE16) \
		{ BX_IA_##IACODE##_ALIb, OPCODE8, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM8 }, \
		{ BX_IA_##IACODE##_AXIw, OPCODE16, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM16 }, \
		{ BX_IA_##IACODE##_EAXId, OPCODE16, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM32 }

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
		{ BX_IA_ADC_E##WDE##I##WDI, OPCODE, 2, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_ADD_E##WDE##I##WDI, OPCODE, 0, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_AND_E##WDE##I##WDI, OPCODE, 4, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_CMP_E##WDE##I##WDI, OPCODE, 7, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_OR_E##WDE##I##WDI, OPCODE, 1, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SBB_E##WDE##I##WDI, OPCODE, 3, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_SUB_E##WDE##I##WDI, OPCODE, 5, BochsALUInstr::REG_COUNT + 1, CLASS }, \
		{ BX_IA_XOR_E##WDE##I##WDI, OPCODE, 6, BochsALUInstr::REG_COUNT + 1, CLASS }

	BINOPS(0x80, b, b, ALU_IMM8_RM8),
	BINOPS(0x81, w, w, ALU_IMM16_RM16),
	BINOPS(0x81, d, d, ALU_IMM32_RM32),
	BINOPS(0x82, b, b, ALU_IMM8_RM8),
	BINOPS(0x83, w, w, ALU_IMM8_RM16),
	BINOPS(0x83, d, d, ALU_IMM8_RM32),
#undef BINOPS

	// r/m, arbitrary register
#define BINOPS(IACODE, OPCODE8, OPCODE16) \
		{ BX_IA_##IACODE##_EbGb, OPCODE8, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM8 }, \
		{ BX_IA_##IACODE##_EwGw, OPCODE16, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16 }, \
		{ BX_IA_##IACODE##_EdGd, OPCODE16, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32 }

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
		{ BX_IA_##IACODE##_GbEb, OPCODE8, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM8 }, \
		{ BX_IA_##IACODE##_GwEw, OPCODE16, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16 }, \
		{ BX_IA_##IACODE##_GdEd, OPCODE16, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32 }

	BINOPS(ADC, 0x12, 0x13),
	BINOPS(ADD, 0x02, 0x03),
	BINOPS(AND, 0x22, 0x23),
	BINOPS(CMP, 0x3A, 0x3B),
	BINOPS(OR, 0x0A, 0x0B),
	BINOPS(SBB, 0x1A, 0x1B),
	BINOPS(SUB, 0x2A, 0x2B),
	BINOPS(XOR, 0x32, 0x33),
#undef BINOPS

	// TEST instruction
	{ BX_IA_TEST_ALIb, 0xA8, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM8},
	{ BX_IA_TEST_AXIw, 0xA9, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM16},
	{ BX_IA_TEST_EAXId, 0xA9, BochsALUInstr::REG_COUNT + 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM32},
	{ BX_IA_TEST_EbIb, 0xF6, 0, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM8 },
	{ BX_IA_TEST_EbIb, 0xF6, 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM8 },
	{ BX_IA_TEST_EwIw, 0xF7, 0, BochsALUInstr::REG_COUNT + 1, ALU_IMM16_RM16 },
	{ BX_IA_TEST_EwIw, 0xF7, 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM16_RM16 },
	{ BX_IA_TEST_EdId, 0xF7, 0, BochsALUInstr::REG_COUNT + 1, ALU_IMM32_RM32 },
	{ BX_IA_TEST_EdId, 0xF7, 1, BochsALUInstr::REG_COUNT + 1, ALU_IMM32_RM32 },
	{ BX_IA_TEST_EbGb, 0x84, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM8},
	{ BX_IA_TEST_EwGw, 0x85, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16},
	{ BX_IA_TEST_EdGd, 0x85, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32},

	// MUL and DIV
#define BINOPS(IACODE, OPCODE8, OPCODE16, REG) \
		{ BX_IA_##IACODE##_ALEb, OPCODE8, REG, BochsALUInstr::REG_COUNT + 1, ALU_IMM8 }, \
		{ BX_IA_##IACODE##_AXEw, OPCODE16, REG, BochsALUInstr::REG_COUNT + 1, ALU_IMM16 }, \
		{ BX_IA_##IACODE##_EAXEd, OPCODE16, REG, BochsALUInstr::REG_COUNT + 1, ALU_IMM32 }
	BINOPS(MUL, 0xF6, 0xF7, 4),
	BINOPS(IMUL, 0xF6, 0xF7, 5),
	BINOPS(DIV, 0xF6, 0xF7, 6),
	BINOPS(IDIV, 0xF6, 0xF7, 7),
#undef BINOPS
	{ BX_IA_IMUL_GwEwIw, 0x69, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM16_RM16 },
	{ BX_IA_IMUL_GwEwIw, 0x69, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM32_RM32 },
	{ BX_IA_IMUL_GwEwIw, 0x6B, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM16 },
	{ BX_IA_IMUL_GwEwIw, 0x6B, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_IMM8_RM32 },
	// two-byte opcode, see above
	{ BX_IA_IMUL_GwEw, 0xAF, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM16},
	{ BX_IA_IMUL_GdEd, 0xAF, BochsALUInstr::REG_COUNT, BochsALUInstr::REG_COUNT + 1, ALU_RM32},

	/* --- \\\ BINARY OPERATIONS /// --- */
};

/**
 * \var aluInstructionsSize
 *
 * the size of aluInstructions, in bytes
 */
size_t const aluInstructionsSize = sizeof(aluInstructions);

/**
 * \class BochsALUInstructions
 *
 * \brief This class handles Bochs ALU instructions.
 *
 * This class analyses a given bxInstruction_c object:
 * if it belongs to the instructions listed in
 * \a allInstr, the user can request a random
 * ALU instruction with an equivalent addressing mode.
 */
class BochsALUInstructions {
public:
	/**
	 * A list of Bochs instructions
	 */
	typedef std::vector<BochsALUInstr> InstrList;
	/**
	 * This data structure assigns lists of instructions on their ALU equivalence
	 * class.
	 *
	 * \see X86AluClass
	 */
	typedef std::map<X86AluClass, InstrList> EquivClassMap;
	/**
	 * Creates a new BochsALUInstructions object.
	 * @param initial_array initialises \a allInstr
	 * @param array_size the size of initial_array
	 */
	BochsALUInstructions(const BochsALUInstr *initial_array, size_t array_size);
	/**
	 * Destroys the BochsALUInstructions object.
	 */
	~BochsALUInstructions() { free(allInstr); }
	/**
	 * Determines if a given Bochs instruction is an ALU instruction.
	 * @param src the instruction to examine. It is stored internally
	 *            to be reused with \a randomEquivalent
	 * @returns \c true if the given instruction is an ALU instruction,
	 *          \c false otherwise
	 */
	bool isALUInstruction(const bxInstruction_c *src);
	/**
	 * Determines a new bxInstruction_c object with an equivalent
	 * addressing mode.
	 * @param result the resulting bxInstruction_c object as described above
	 * @param details after completion contains details about \c result
	 */
	void randomEquivalent(bxInstruction_c &result, std::string &details) const;
protected:
	/**
	 * Convert a bxInstruction_c object into its matching BochsALUInstr object.
	 * @params src the Bochs instruction to examine
	 * @params dest the resulting BochsALUInstr object. Its \a aluClass field
	 *         is set to \c ALU_UNDEF in case no matching ALU instruction
	 *         could be found. In this case, all other fields of dest are invalid.
	 */
	void bochsInstrToInstrStruct(bxInstruction_c const &src, BochsALUInstr &dest) const;
private:
	BochsALUInstr *allInstr; //<! array that contains all known ALU instructions of the object
	size_t allInstrSize; //<! the element count of the allInstr array
	BochsALUInstr lastInstr; //<! a buffer for the last generated ALU instruction
	bxInstruction_c lastOrigInstr; //!< a buffer for the last examined Bochs instruction
	EquivClassMap equivalenceClasses; //!< the object's \a EquivClassMap (see there)
	/**
	 * A function to build the equivalence classes from the given instructions.
	 */
	void buildEquivalenceClasses();
	void checkEquivClasses(); //!< checks if the equivalence classes are valid
#ifdef DEBUG
	void printNestedMap(); //!< prints the \a EquivClassMap of the oject
#endif
};

#endif // __L4SYS_ALUINSTR_HPP__
