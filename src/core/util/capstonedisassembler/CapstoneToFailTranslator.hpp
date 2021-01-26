#ifndef __CAPSTONETOFAILTRANSLATOR_HPP_
#define __CAPSTONETOFAILTRANSLATOR_HPP_

#include "sal/SALConfig.hpp"
#include "sal/ConcreteCPU.hpp"
#include <map>

namespace fail {
class CapstoneDisassembler; // Forward

/**
 * Translates Capstone disassembler ids
 * to FAIL* SAL representations.
 */
class CapstoneToFailTranslator {
protected:

	CapstoneToFailTranslator(){};

	typedef std::map<unsigned int, RegisterView> ctof_map_t;
	ctof_map_t capstone_to_fail_map;

public:
	/**
	 * Translates a backend-specific register ID to a FAIl RegisterView
	 * @param regid A backend-specific register ID.
	 * @return A FAIL* register-info struct, or CapstonetoFailTranslator::notfound
	 *         if no mapping was found.
	 */
	const RegisterView &  getFailRegisterInfo(unsigned int regid);

	RegisterView notfound;
};

} // end of namespace

#endif
