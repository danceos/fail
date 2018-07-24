#include "llvm/Support/raw_os_ostream.h"
#include "../LLVMDisassembler.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;

bool show_mapping(fail::LLVMtoFailTranslator *ltof, const MCRegisterInfo &reg_info, unsigned llvmid)
{
	const LLVMtoFailTranslator::reginfo_t& failreg = ltof->getFailRegisterInfo(llvmid);
	std::cout << reg_info.getName(llvmid) << "(" << std::dec << llvmid << "->";
	if (&failreg != &ltof->notfound) {
		std::cout << failreg.id;
	} else {
		std::cout << "NOTFOUND!";
	}
	std::cout << ") ";
	return &failreg != &ltof->notfound;
}

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

	Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(file);
	if (!BinaryOrErr) {
		std::cerr << "Dis: '" << file << "': ";
		raw_os_ostream OS(std::cerr);
		logAllUnhandledErrors(BinaryOrErr.takeError(), OS, "");
		return -1;
	}
	Binary *binary = BinaryOrErr.get().getBinary();

	ObjectFile *obj = dyn_cast<ObjectFile>(binary);

	LLVMDisassembler disas(obj);
	disas.disassemble();

	LLVMDisassembler::InstrMap &instr_map = disas.getInstrMap();
	std::cout << "Map Size: " << instr_map.size() << "\nTriple: " << disas.GetTriple() <<  std::endl;

	LLVMDisassembler::InstrMap::const_iterator itr;
	const MCRegisterInfo &reg_info = disas.getRegisterInfo();

	std::cout << std::endl << "Number of Registers: " << reg_info.getNumRegs() << std::endl;
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i) {
		std::cout << i << "=" << reg_info.getName(i) << " ";
	}
	std::cout << std::endl;
	fail::LLVMtoFailTranslator *ltof = disas.getTranslator();

	for (itr = instr_map.begin(); itr != instr_map.end(); ++itr){
		const LLVMDisassembler::Instr &instr = (*itr).second;
		std::cout << std::hex << (*itr).first << " | "	<< instr.opcode << std::endl;
		std::cout << std::dec << "USES: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_uses.begin();
			 it != instr.reg_uses.end(); ++it) {
			show_mapping(ltof, reg_info, *it);
		}

		std::cout << " |  DEFS: ";
		for (std::vector<uint16_t>::const_iterator it = instr.reg_defs.begin();
			 it != instr.reg_defs.end(); ++it) {
			show_mapping(ltof, reg_info, *it);
		}

		if (instr.conditional_branch) {
			std::cout << "(conditional branch)";
		}
		std::cout << std::endl;
	}
}
