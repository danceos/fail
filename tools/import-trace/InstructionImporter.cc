#include <sstream>
#include <iostream>
#include "InstructionImporter.hpp"
#include "util/Logger.hpp"
#include "sal/faultspace/MemoryArea.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif
using namespace fail;


static Logger LOG("InstructionImporter");

bool InstructionImporter::cb_initialize() {
	if (!m_elf) {
		LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
		return false;
	}

	m_disassembler.reset(new Disassembler(m_elf));
	m_disassembler->disassemble();
	m_instr_map = m_disassembler->getInstrMap();

	return true;
}

bool InstructionImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
										  Trace_Event &ev) {
	if (m_instr_map->find(ev.ip()) == m_instr_map->end()) {
		LOG << "Could not find instruction for IP " << std::hex << ev.ip()
			<< ", skipping" << std::endl;
		return true;
	}
	const Disassembler::Instr &opcode = m_instr_map->at(ev.ip());

	guest_address_t from = ev.ip(), to = ev.ip() + std::ceil(opcode.length/8.0);

	assert(std::ceil(opcode.length/8.0) == std::floor(opcode.length/8.0)
		   && "Instruction importer can't handle instruction length which don't have byte aligned length");
 
	auto filter = [this] (address_t a) -> bool { return !(this->m_mm && !this->m_mm->isMatching(a)); };
	auto area = dynamic_cast<fail::MemoryArea*>(&m_fsp.get_area("ram"));
	assert(area != nullptr && "InstructionImporter failed to get a MemoryArea from the fault space description");

	char access_type = ev.accesstype() == ev.READ ? 'R' : 'W';
	for (auto element : area->translate(from, to, filter)) {
		if (!add_faultspace_element(curtime, instr, element, 0xFF, access_type, ev)) {
			return false;
		}
	}
	return true;
}
