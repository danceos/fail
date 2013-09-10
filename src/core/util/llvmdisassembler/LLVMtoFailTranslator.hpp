#ifndef __LLVMTOFAILTRANSLATOR_HPP_
#define __LLVMTOFAILTRANSLATOR_HPP_

#include "sal/SALConfig.hpp"
#include "sal/ConcreteCPU.hpp"
#include <map>

namespace fail {

/**
 * Translates LLVM disassembler ids
 * to Fail* SAL representations.
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
		regwidth_t width;
		regdata_t  mask;
		byte_t offset;

		int toDataAddress() const {
			// .. 5 4 | 3 2 1 0
			// <reg>  | <offset>
			return (id << 4) | (offset / 8);
		}
		// does not recreate width or mask
		static reginfo_t fromDataAddress(int addr) {
			int id = addr >> 4;
			byte_t offset = (addr & 0xf) * 8;
			return reginfo_t(id, 0, offset);
		}

		reginfo_t(int id=-1, regwidth_t width = 32, byte_t offs = 0)
			: id(id), width(width), mask((regwidth_t)((((long long)1 << width) - 1) << offs)), offset(offs) {};
	};
protected:

	LLVMtoFailTranslator(){};

	typedef std::map<unsigned int, struct reginfo_t> ltof_map_t;
	ltof_map_t llvm_to_fail_map;

public:
	/**
	 * Translates a backend-specific register ID to a Fail register ID.
	 * @param regid A backend-specific register ID.
	 * @return A Fail* register ID, or LLVMtoFailTranslator::notfound if no
	 *         mapping was found.
	 */
	const reginfo_t &  getFailRegisterID(unsigned int regid);

	regdata_t getRegisterContent(ConcreteCPU & cpu, const reginfo_t & reg);
	void setRegisterContent(ConcreteCPU & cpu, const reginfo_t &reg, regdata_t value);
	regdata_t getRegisterContent(ConcreteCPU & cpu, unsigned int llvmid) {
		return getRegisterContent(cpu, getFailRegisterID(llvmid));
	}
	void setRegisterContent(ConcreteCPU & cpu, unsigned int llvmid, regdata_t value) {
		setRegisterContent(cpu, getFailRegisterID(llvmid), value);
	}

	int getFailRegisterId(unsigned int regid) { return this->getFailRegisterID(regid).id; };

	reginfo_t notfound;
};

} // end of namespace

#endif
