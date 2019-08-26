#include "util/ElfReader.hpp"
#include <elf.h>
#include "../CapstoneDisassembler.hpp"

using namespace fail;

bool show_mapping(fail::CapstoneToFailTranslator *ctof, unsigned llvmid)
{
	const CapstoneToFailTranslator::reginfo_t& failreg = ctof->getFailRegisterInfo(llvmid);
	std::cout /*<< reg_info.getName(llvmid)*/ << "(" << std::dec << llvmid << "->";
	if (&failreg != &ctof->notfound) {
		std::cout << failreg.id;
	} else {
		std::cout << "NOTFOUND!";
	}
	std::cout << ") ";
	return &failreg != &ctof->notfound;
}

int main(int argc, char* argv[]) {
	std::string file;

	if(argc > 1){
		std::cout << "Trying to disassemble: " << argv[1] << std::endl;
		file = argv[1];
	} else {
		std::cerr << "No file to disassemble :(" << std::endl;
		return -1;
	}

	ElfReader *m_elf = new ElfReader(file.c_str());

	CapstoneDisassembler disas(m_elf);
	disas.disassemble();

	CapstoneDisassembler::InstrMap &instr_map = disas.getInstrMap();
	std::cout << "Map Size: " << instr_map.size() <<  std::endl;

	CapstoneDisassembler::InstrMap::const_iterator itr;

	fail::CapstoneToFailTranslator *ctof = disas.getTranslator();

	for (itr = instr_map.begin(); itr != instr_map.end(); ++itr){
		const CapstoneDisassembler::Instr &instr = (*itr).second;
		std::cout << std::hex << (*itr).first << " | "	<< instr.opcode << std::endl;
		std::cout << std::dec << "USES: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_uses.begin();
			 it != instr.reg_uses.end(); ++it) {
			show_mapping(ctof, *it);
		}

		std::cout << " |  DEFS: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_defs.begin();
			 it != instr.reg_defs.end(); ++it) {
			show_mapping(ctof, *it);
		}

		if (instr.conditional_branch) {
			std::cout << "(conditional branch)";
		}
		std::cout << std::endl;
	}
}
