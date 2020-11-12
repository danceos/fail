#ifndef __LLVMTOFAILTRANSLATOR_HPP_
#define __LLVMTOFAILTRANSLATOR_HPP_

#include "sal/SALConfig.hpp"
#include "sal/ConcreteCPU.hpp"
#include <map>

namespace fail {

/**
 * Translates LLVM disassembler ids
 * to FAIL* SAL representations.
 */
class LLVMtoFailTranslator {
public:
	/**
	 * Maps registers to/from linear addresses usable for def/use-pruning
	 * purposes and storage in the database.  Takes care that the linear
	 * addresses of x86 subregisters (e.g., AX represents the lower 16 bits of
	 * EAX) overlap with their siblings.
	 */
	struct reginfo_t {
		int id;
		reginfo_t(int id=-1)
			: id(id)
		{}
	};
protected:

	LLVMtoFailTranslator(){};

	typedef std::map<unsigned int, struct reginfo_t> ltof_map_t;
	ltof_map_t llvm_to_fail_map;

public:
	/**
	 * Translates a backend-specific register ID to a Fail register ID.
	 * @param regid A backend-specific register ID.
	 * @return A FAIL* register-info struct, or LLVMtoFailTranslator::notfound
	 *         if no mapping was found.
	 */
	const reginfo_t &  getFailRegisterInfo(unsigned int regid);

	/**
	 * Translates a backend-specific register ID to a Fail register ID.
	 * @param regid A backend-specific register ID.
	 * @return A FAIL* register ID.  May do funny things if regid does not exist.
	 */
	int getFailRegisterID(unsigned int regid) { return this->getFailRegisterInfo(regid).id; };
    int getMaxFailRegisterID();

	reginfo_t notfound;

	static LLVMtoFailTranslator* createFromBinary(const std::string elf_path);
};

} // end of namespace

#endif
