#include <algorithm>
#include "CapstoneToFailTranslator.hpp"
#include "sal/SALInst.hpp"

using namespace fail;

const RegisterView &  CapstoneToFailTranslator::getFailRegisterInfo(unsigned int regid) {
	ctof_map_t::iterator it = capstone_to_fail_map.find(regid);
	if ( it != capstone_to_fail_map.end() ) {// found
		return (*it).second;
	} else { // not found
//		std::cout << "Fail ID for Capstone Register id " << std::dec << regid << " not found :(" << std::endl;
//		exit(EXIT_FAILURE);
		return notfound;
	}
}
