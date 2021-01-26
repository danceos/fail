#include <algorithm>
#include "LLVMDisassembler.hpp"
#include "LLVMtoFailTranslator.hpp"
#include "sal/SALInst.hpp"

using namespace fail;
using namespace llvm;
using namespace llvm::object;

const RegisterView& LLVMtoFailTranslator::getFailRegisterInfo(unsigned int regid) {
	ltof_map_t::iterator it = llvm_to_fail_map.find(regid);
	if ( it != llvm_to_fail_map.end() ) {// found
		return (*it).second;
	} else { // not found
		//std::cout << "Fail ID for LLVM Register id " << std::dec << regid << " not found :(" << std::endl;
		//exit(EXIT_FAILURE);
		return notfound;
	}
}

// LLVMtoFailTranslator* LLVMtoFailTranslator::createFromBinary(const std::string elf_path) {
// 	//llvm_shutdown_obj Y;
// 	llvm::InitializeAllTargetInfos();
// 	llvm::InitializeAllTargetMCs();
// 	llvm::InitializeAllDisassemblers();

// 	Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(elf_path);
// 	assert (BinaryOrErr);
// 	Binary *binary = BinaryOrErr.get().getBinary();

// 	#ifndef __puma
// 	LLVMDisassembler disas(dyn_cast<ObjectFile>(binary));
// 	return disas.getTranslator();
// 	#else
// 	return 0;
// 	#endif
// }
