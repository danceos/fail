#include "aluinstr.hpp"
#include <iostream>
#include <sstream>
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
	checkEquivClasses();
}

void BochsALUInstructions::buildEquivalenceClasses() {
	for (size_t i = 0; i < allInstrSize; i++) {
		InstrList &currVector = equivalenceClasses[allInstr[i].aluClass];
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			// add an entry for each possible opcode
			for (Bit8u j = 0; j < allInstr[i].opcodeRegisterOffset; j++) {
				Bit8u new_opcode = allInstr[i].opcode + j;
				BochsALUInstr newInstr = { allInstr[i].bochs_operation,
						new_opcode,
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

void BochsALUInstructions::checkEquivClasses() {
	for (EquivClassMap::iterator it = equivalenceClasses.begin();
	    it != equivalenceClasses.end();
	    it++) {
		InstrList &curr_vector = it->second;
		size_t curr_size = curr_vector.size();
		for (size_t i = 0; i < curr_size; i++) {
			for (size_t j = i + 1; j < curr_size; j++) {
				if (curr_vector[i] == curr_vector[j]) {
					std::cerr << "Two instructions in one equivalence class"
					    << "are equal to each other. Correct the"
					    << "source code." << std::endl;
					exit(50);
				}
			}
		}
	}
}

void BochsALUInstructions::bochsInstrToInstrStruct(bxInstruction_c const &src, BochsALUInstr &dest) const {
	//Note: it may be necessary to introduce a solution for two-byte
	//opcodes once they overlap with one-byte ones
	for (size_t i = 0; i < allInstrSize; i++) {
		// first, check the opcode
		if (allInstr[i].bochs_operation != src.getIaOpcode()) {
			continue;
		}
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			// the opcode listed in allInstr is the starting value for a range
			if (src.b1() < allInstr[i].opcode ||
					src.b1() > allInstr[i].opcode + BochsALUInstr::REG_COUNT) {
				continue;
			}
		} else if (src.b1() != allInstr[i].opcode) {
			// normal case -- just compare the opcode
			continue;
		}
		// second, check the opcode extension
		if (allInstr[i].reg < BochsALUInstr::REG_COUNT &&
				allInstr[i].reg != src.nnn()) {
			continue;
		}
		// found it - now copy
		if (allInstr[i].opcodeRegisterOffset <= BochsALUInstr::REG_COUNT) {
			dest.bochs_operation = allInstr[i].bochs_operation;
			dest.opcode = src.b1();
			dest.reg = allInstr[i].reg;
			dest.opcodeRegisterOffset = src.rm();
			dest.aluClass = allInstr[i].aluClass;
		} else {
			dest = allInstr[i];
		}
		return;
	}
	// not found - marking it undefined
	dest.aluClass = ALU_UNDEF;
}

bool BochsALUInstructions::isALUInstruction(bxInstruction_c const *src) {
	memcpy(&lastOrigInstr, src, sizeof(bxInstruction_c));
	bochsInstrToInstrStruct(lastOrigInstr, lastInstr);
	if (lastInstr.aluClass != ALU_UNDEF) {
		return true;
	}
	return false;
}

void BochsALUInstructions::randomEquivalent(bxInstruction_c &result,
                                                  std::string &details) const {
	// find a random member of the same equivalence class
	X86AluClass equClassID = lastInstr.aluClass;
	if (equClassID == ALU_UNDEF) {
		// something went wrong - just return the original instruction
		result = lastOrigInstr;
		return;
	}

	InstrList const &destList = equivalenceClasses.at(equClassID);
	BochsALUInstr dest;

	// make sure the two are not equal by chance
	do {
		int index = rand() % destList.size();
		dest = destList[index];
	} while (dest == lastInstr);

	// alternative chosen -- now store the necessary details
	std::ostringstream oss;
	oss << "Opcode 0x" << std::hex << static_cast<unsigned>(dest.opcode) << std::dec;
	if (dest.reg < dest.REG_COUNT) oss << " # " << static_cast<unsigned>(dest.reg);
	if (dest.opcodeRegisterOffset <= dest.REG_COUNT)
		oss << " # " << static_cast<unsigned>(dest.opcodeRegisterOffset);
	details = oss.str();

	// first, copy everything
	result = lastOrigInstr;

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
	result.setIaOpcode(dest.bochs_operation);
	result.setB1(dest.opcode);
	if (dest.opcodeRegisterOffset < BochsALUInstr::REG_COUNT) {
		result.setRm(dest.opcodeRegisterOffset);
	}
	if (dest.reg < BochsALUInstr::REG_COUNT) {
		result.setNnn(dest.reg);
	}
}

#ifdef DEBUG

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
