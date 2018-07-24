#include <algorithm>
#include "LLVMDisassembler.hpp"
#include "LLVMtoFailTranslator.hpp"
#include "sal/SALInst.hpp"

using namespace fail;
using namespace llvm;
using namespace llvm::object;

const LLVMtoFailTranslator::reginfo_t &	 LLVMtoFailTranslator::getFailRegisterInfo(unsigned int regid) {
	ltof_map_t::iterator it = llvm_to_fail_map.find(regid);
	if ( it != llvm_to_fail_map.end() ) {// found
		return (*it).second;
	} else { // not found
		//std::cout << "Fail ID for LLVM Register id " << std::dec << regid << " not found :(" << std::endl;
		//exit(EXIT_FAILURE);
		return notfound;
	}
}

regdata_t LLVMtoFailTranslator::getRegisterContent(ConcreteCPU& cpu, const reginfo_t &reginfo){
	regdata_t result;

	Register* reg = cpu.getRegister(reginfo.id);
	result = cpu.getRegisterContent(reg);

	result &= reginfo.mask;
	result >>= reginfo.offset;

	return result;
}

void LLVMtoFailTranslator::setRegisterContent(ConcreteCPU & cpu, const reginfo_t &reginfo, regdata_t value){
	Register* reg = cpu.getRegister(reginfo.id);

	regdata_t origval = cpu.getRegisterContent(reg); // Get register Value from fail
	origval &= ~(reginfo.mask); // clear bits to write

	value <<= reginfo.offset;	  // shift value to write up to position
	value &= reginfo.mask;	  // mask out trailing and leading bits
	value |= origval;	  //  set bits to write

	cpu.setRegisterContent( reg, value ); // write back register content
}

int LLVMtoFailTranslator::getMaxFailRegisterID()
{
	auto max = std::max_element(llvm_to_fail_map.cbegin(), llvm_to_fail_map.cend(),
		[] (const ltof_map_t::value_type& v1, const ltof_map_t::value_type& v2) {
			return v1.second.id < v2.second.id;
		});
	return max->second.id;
}

LLVMtoFailTranslator* LLVMtoFailTranslator::createFromBinary(const std::string elf_path) {
	//llvm_shutdown_obj Y;
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllDisassemblers();

	Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(elf_path);
	assert (BinaryOrErr);
	Binary *binary = BinaryOrErr.get().getBinary();

	#ifndef __puma
	LLVMDisassembler disas(dyn_cast<ObjectFile>(binary));
	return disas.getTranslator();
	#else
	return 0;
	#endif
}
