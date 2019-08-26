#include <algorithm>
#include "CapstoneToFailTranslator.hpp"
#include "sal/SALInst.hpp"

using namespace fail;

const CapstoneToFailTranslator::reginfo_t &	 CapstoneToFailTranslator::getFailRegisterInfo(unsigned int regid) {
	ctof_map_t::iterator it = capstone_to_fail_map.find(regid);
	if ( it != capstone_to_fail_map.end() ) {// found
		return (*it).second;
	} else { // not found
//		std::cout << "Fail ID for Capstone Register id " << std::dec << regid << " not found :(" << std::endl;
//		exit(EXIT_FAILURE);
		return notfound;
	}
}

regdata_t CapstoneToFailTranslator::getRegisterContent(ConcreteCPU& cpu, const reginfo_t &reginfo){
	regdata_t result;

	Register* reg = cpu.getRegister(reginfo.id);
	result = cpu.getRegisterContent(reg);

	result &= reginfo.mask;
	result >>= reginfo.offset;

	return result;
}

void CapstoneToFailTranslator::setRegisterContent(ConcreteCPU & cpu, const reginfo_t &reginfo, regdata_t value){
	Register* reg = cpu.getRegister(reginfo.id);

	regdata_t origval = cpu.getRegisterContent(reg); // Get register Value from fail
	origval &= ~(reginfo.mask); // clear bits to write

	value <<= reginfo.offset;	  // shift value to write up to position
	value &= reginfo.mask;	  // mask out trailing and leading bits
	value |= origval;	  //  set bits to write

	cpu.setRegisterContent( reg, value ); // write back register content
}

int CapstoneToFailTranslator::getMaxFailRegisterID()
{
	auto max = std::max_element(capstone_to_fail_map.cbegin(), capstone_to_fail_map.cend(),
		[] (const ctof_map_t::value_type& v1, const ctof_map_t::value_type& v2) {
			return v1.second.id < v2.second.id;
		});
	return max->second.id;
}
