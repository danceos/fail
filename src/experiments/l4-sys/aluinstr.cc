#include "aluinstr.hpp"
#include <string.h>
#include <time.h>
#include "bochs.h"
#include "cpu/cpu.h"
#include "cpu/fetchdecode.h"

/**
 * Forward declaration of the Bochs instruction decode table.
 * This is necessary because, inconveniently, it is not declared in a header file.
 */
//extern bxIAOpcodeTable *BxOpcodesTable;
bxIAOpcodeTable MyBxOpcodesTable[] = {
#define bx_define_opcode(a, b, c, d, e) { b, c, e },
#include "cpu/ia_opcodes.h"
};
#undef  bx_define_opcode

BochsALUInstructions::BochsALUInstructions(BochsALUInstr const *initial_array, size_t array_size)
{
	allInstr = reinterpret_cast<BochsALUInstr*>(malloc(array_size));
	memcpy(allInstr, initial_array, array_size);
	allInstrSize = array_size / sizeof(BochsALUInstr);
	srand(time(NULL));
	buildEquivalenceClasses();
}

void BochsALUInstructions::buildEquivalenceClasses() {
	for (size_t i = 0; i < allInstrSize; i++) {
		InstrList &currVector = equivalenceClasses[allInstr[i].aluClass];
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			// add an entry for each possible opcode
			for (int j = 0; j < allInstr[i].opcodeRegisterOffset; j++) {
				BochsALUInstr newInstr = { allInstr[i].bochs_operation,
						allInstr[i].opcode + j,
						allInstr[i].reg,
						j,
						allInstr[i].aluClass };
				currVector.push_back(newInstr);
			}
		} else {
			// normal case -- just add the instruction
			currVector.push_back(allInstr[i]);
		}
	}
}

void BochsALUInstructions::bochsInstrToInstrStruct(bxInstruction_c const *src, BochsALUInstr *dest) const {
	if (dest == NULL) return;
	//Note: it may be necessary to introduce a solution for two-byte
	//opcodes once they overlap with one-byte ones
	for (size_t i = 0; i < allInstrSize; i++) {
		// first, check the opcode
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			// the opcode listed in allInstr is the starting value for a range
			if (src->b1() < allInstr[i].opcode ||
					src->b1() > allInstr[i].opcode + BochsALUInstr::REG_COUNT) {
				continue;
			}
		} else if (src->b1() != allInstr[i].opcode) {
			// normal case -- just compare the opcode
			continue;
		}
		// second, check the opcode extension
		if (allInstr[i].reg < BochsALUInstr::REG_COUNT &&
				allInstr[i].reg != src->nnn()) {
			continue;
		}
		// found it - now copy
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			BochsALUInstr result = { allInstr[i].bochs_operation,
			src->b1(),
			allInstr[i].reg,
			src->rm(),
			allInstr[i].aluClass};
			memcpy(dest, &result, sizeof(BochsALUInstr));
		} else {
			memcpy(dest, &allInstr[i], sizeof(BochsALUInstr));
		}
		return;
	}
	// not found - marking it undefined
	dest->aluClass = ALU_UNDEF;
}

bool BochsALUInstructions::isALUInstruction(const bxInstruction_c *src) {
	lastOrigInstr = src;
	bochsInstrToInstrStruct(src, &lastInstr);
	if (lastInstr.aluClass != ALU_UNDEF) {
		return true;
	}
	return false;
}

bxInstruction_c BochsALUInstructions::randomEquivalent() const {
	// find a random member of the same equivalence class
	X86AluClass equClassID = lastInstr.aluClass;
	if (equClassID == ALU_UNDEF) {
		// something went wrong - just return the original instruction
		return *lastOrigInstr;
	}

	InstrList const &destList = equivalenceClasses.at(equClassID);
	BochsALUInstr dest;
	// make sure the two are not equal by chance
	do {
		int index = rand() % destList.size();
		dest = destList[index];
	} while (!memcmp(&dest, &lastInstr, sizeof(BochsALUInstr)));

	// first, copy everything
	bxInstruction_c result;
	memcpy(&result, lastOrigInstr, sizeof(bxInstruction_c));

	// then change what has to be different
	// execute functions
	bxIAOpcodeTable entry = MyBxOpcodesTable[dest.bochs_operation];
	if (result.execute2 == NULL) {
		result.execute = entry.execute2;
	} else {
		result.execute = entry.execute1;
		result.execute2 = entry.execute2;
	}
	// opcodes
	result.metaInfo.ia_opcode = dest.bochs_operation;
	result.setB1(dest.opcode);
	if (dest.opcodeRegisterOffset < BochsALUInstr::REG_COUNT) {
		result.setRm(dest.opcodeRegisterOffset);
	}
	// finally, return the result
	return result;
}

#ifdef DEBUG
#include <iostream>

void BochsALUInstructions::printNestedMap() {
	for (EquivClassMap::iterator it = equivalenceClasses.begin();
			it != equivalenceClasses.end(); it++) {
		std::cerr << it->first << ":" << std::endl;
		for (InstrList::iterator jt = it->second.begin();
				jt != it->second.end(); jt++) {
			std::cerr << std::hex << "  " << jt->bochs_operation
					<< "," << (unsigned) jt->opcode << std::dec << "," << (unsigned) jt->reg
					<< "," << (unsigned) jt->opcodeRegisterOffset << ","
					<< jt->aluClass << std::endl;
		}
	}
}
#endif
