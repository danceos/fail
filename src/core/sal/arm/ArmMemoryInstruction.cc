#include "ArmMemoryInstruction.hpp"

#include <iostream>
using namespace std;

namespace fail {
static ArmMemoryInstructionAnalyzer anal;
MemoryInstructionAnalyzer& meminstruction = anal;

address_t ArmMemoryInstructionAnalyzer::findPrevious(address_t address){
	if (m_dis.hasInstructionAt(address)) {
		return address;
	} else if (m_dis.hasInstructionAt(address - 2)) {
		return address - 2;
	} else {
		return ADDR_INV;
	}
}

void ArmMemoryInstructionAnalyzer::evaluate(arm_instruction & inst, MemoryInstruction& result){
	cout << "Memory Access: " << inst.text << " - Size: " << inst.instruction_size << " Type " << inst.type <<  endl;
	inst.info.load_store.evaluate();
	result.setValue(inst.info.load_store.value);
	result.setAddress(inst.info.load_store.address);
	result.setWidth(4); // TODO;
	result.setWriteAccess(inst.isStoreInstruction());
}

// The Cortex M3 Lauterbach is a pain in the ass, as a Memory Watchpoint does not stop
// at the accessing instruction, but 1 or 2 instructions later.
bool ArmMemoryInstructionAnalyzer::eval_cm3(address_t address, MemoryInstruction& result){

	arm_instruction inst;
	uint32_t opcode =0;
	address = findPrevious(address);
	opcode = m_dis.disassemble(address).opcode;

	// OpenOCDs thumb2_opcode evaluation is not complete yet. :(
	thumb2_opcode(address, opcode, &inst);

	if (inst.isMemoryAccess()) {
		evaluate(inst, result);
		return true;
	} else {
		return false;
	}

#if 0
	arm_instruction inst;
	uint32_t opcode =0;
	address = findPrevious(address); // Cortex M3: memory access is at the previous instruction
	opcode = m_dis.disassemble(address).opcode;

	// OpenOCDs thumb2_opcode evaluation is not complete yet. :(
	thumb2_opcode(address, opcode, &inst);

	if (inst.isMemoryAccess()) {
		evaluate(inst, result);
		return true;
	} else if (inst.isBranchInstruction()) {
		// The memory access took place within the function previously branched
		int regop = inst.info.b_bl_bx_blx.reg_operand;
		uint32_t addr = inst.info.b_bl_bx_blx.target_address;
		//cout << " Reg:" << hex << regop << " address " << hex << addr << endl;
		// Lets look into this function
		if ( regop == -1 ) {
			// address should be set..
			const ElfSymbol & sym = m_elf.getSymbol(addr|1); //  | 1 to set first bit -> thumbmode
			addr += sym.getSize(); // Go to end of function.
			// THIS IS DANGEROUS: The memory access can be anywhere within this function, one instruction before a ret.
			// OR, the memory access itself can result in leaving the function :e.g., ldr pc, [r3]
			// We cannot be sure :( Here we assume the first memory access from the back.
			do { // go backwards until there is a memory instruction.
				addr = findPrevious(addr); // find previous
				thumb2_opcode(addr, m_dis.disassemble(addr).opcode, &inst);
			} while ( !inst.isMemoryAccess() );
			evaluate(inst, result);
			return true;
		}
	} else {
		// There was a memory access before, but the previous instruction
		// is neither an access nor a branch.
		// This can happen if we came here from anywhere, e.g. by ldr pc, [r4]
	}
	return false;
#endif
}

bool ArmMemoryInstructionAnalyzer::eval_ca9(address_t address, MemoryInstruction& result){
	arm_instruction inst;
	uint32_t opcode = m_dis.disassemble(address).opcode;
	arm_evaluate_opcode(address, opcode, &inst);
	if ( inst.isMemoryAccess() ) {
		evaluate(inst, result);
		return true;
	}
	return false;
}

#define CORTEXM3


bool ArmMemoryInstructionAnalyzer::eval(address_t address, MemoryInstruction & result){
#ifdef CORTEXM3
#warning "Memory Accesses cannot be evaluated completely!"
	return eval_cm3(address, result);
#elif defined CORTEXA9
	return eval_ca9(address, result);
#else
#warning "Memory Accesses are not evaluated!"
	return false;
#endif
}

};
