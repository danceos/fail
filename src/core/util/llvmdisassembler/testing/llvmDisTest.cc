#ifndef __puma
#include "../LLVMDisassembler.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;

int main(int argc, char* argv[]) {
	llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
	// Initialize targets and assembly printers/parsers.
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargetMCs();
	//	  llvm::InitializeAllAsmParsers();
	llvm::InitializeAllDisassemblers();

	std::string file;

	if(argc > 1){
		std::cout << "Trying to disassemble: " << argv[1] << std::endl;
		file = argv[1];
	} else {
		std::cerr << "No file to disassemble :(" << std::endl;
		return -1;
	}

	OwningPtr<Binary> binary;
	if (error_code ec = createBinary(file, binary)) {
		std::cerr << "Dis" << ": '" << file << "': " << ec.message() << ".\n";
		return -1;
	}

	ObjectFile *obj = dyn_cast<ObjectFile>(binary.get());

	LLVMDisassembler disas(obj);
	disas.disassemble();

	LLVMDisassembler::InstrMap &instr_map = disas.getInstrMap();
	std::cout << "Map Size: " << instr_map.size() << "\nTriple: " << disas.GetTriple() <<  std::endl;

	LLVMDisassembler::InstrMap::const_iterator itr;
	const MCRegisterInfo &reg_info = disas.getRegisterInfo();

	std::cout << std::endl << "Number of Registers: " << reg_info.getNumRegs() << std::endl;
	//	for(unsigned int i = 0; i < reg_info.getNumRegs(); ++i){
	//	  std::cout << i << " - " <<  reg_info.getName(i) << std::endl;
	//	}
	fail::LLVMtoFailTranslator & ltof =	 disas.getTranslator() ;

	for(itr = instr_map.begin(); itr != instr_map.end(); ++itr){
		const LLVMDisassembler::Instr &instr = (*itr).second;
		std::cout << std::hex << (*itr).first << " | "	<< instr.opcode << std::endl;
		std::cout << std::dec << "USES: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_uses.begin();
			 it != instr.reg_uses.end(); ++it) {
			std::cout << reg_info.getName(*it) <<"(" << *it << ") ";
			std::cout << "Fail: " << ltof.getFailRegisterId(*it) << " ";
		}
		std::cout << std::endl;

		std::cout << "DEFS: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_defs.begin();
			 it != instr.reg_defs.end(); ++it) {
			std::cout << reg_info.getName(*it) << "(" << *it << ") ";
		}

		if (instr.conditional_branch) {
			std::cout << "(conditional branch)";
		}
		std::cout << std::endl;
	}
}
#endif
