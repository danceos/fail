#ifndef __LLVMTOFAILTRANSLATOR_HPP_
#define __LLVMTOFAILTRANSLATOR_HPP_

#include <map>
#include "sal/SALConfig.hpp"
#include "sal/Register.hpp"

namespace fail {

/**
 * Translates LLVM disassembler ids
 * to FAIL* SAL representations.
 */
class LLVMtoFailTranslator {
public:

protected:

	LLVMtoFailTranslator(){};

	typedef std::map<unsigned int, RegisterView> ltof_map_t;
	ltof_map_t llvm_to_fail_map;

public:
	/**
	 * Translates a backend-specific register ID to a FAIL* RegisterView.
	 * @param  regid   A backend-specific register ID.
	 * @return A FAIL* register-info struct, or LLVMtoFailTranslator::notfound
	 *         if no mapping was found.
	 */
	const RegisterView&  getFailRegisterInfo(unsigned int regid);

	RegisterView notfound;
};

} // end of namespace

#endif
