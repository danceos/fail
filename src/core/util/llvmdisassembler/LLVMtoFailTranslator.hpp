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
		regwidth_t width;
		regdata_t  mask;
		byte_t offset;

		int toDataAddress() const {
			// .. 5 4 | 3 2 1 0
			// <reg>  | <offset>
			return (id << 4) | (offset / 8);
		}

		static reginfo_t fromDataAddress(int addr, int width) {
			int id = addr >> 4;
			byte_t offset = (addr & 0xf) * 8;
			return reginfo_t(id, width * 8, offset);
		}

		reginfo_t(int id=-1, regwidth_t width = 32, byte_t offs = 0)
			: id(id), width(width), mask((((regdata_t) 1 << width) - 1) << offs), offset(offs)
		{
			if (width >= sizeof(regdata_t) * 8) { // all ones, (1 << width) == 0!
				mask = -1;
			}
#if 0
			std::cerr << "constructing reginfo_t: " << std::dec << id << " " << width << " " << ((int)offs) << std::hex << " 0x" << mask << std::endl;
#endif
		}
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

	static regdata_t getRegisterContent(ConcreteCPU & cpu, const reginfo_t & reg);
	static void setRegisterContent(ConcreteCPU & cpu, const reginfo_t &reg, regdata_t value);
	regdata_t getRegisterContent(ConcreteCPU & cpu, unsigned int llvmid) {
		return getRegisterContent(cpu, getFailRegisterInfo(llvmid));
	}
	void setRegisterContent(ConcreteCPU & cpu, unsigned int llvmid, regdata_t value) {
		setRegisterContent(cpu, getFailRegisterInfo(llvmid), value);
	}

	/**
	 * Translates a backend-specific register ID to a Fail register ID.
	 * @param regid A backend-specific register ID.
	 * @return A FAIL* register ID.  May do funny things if regid does not exist.
	 */
	int getFailRegisterID(unsigned int regid) { return this->getFailRegisterInfo(regid).id; };

	int getMaxFailRegisterID();
	fail::address_t getMaxDataAddress() { reginfo_t r(getMaxFailRegisterID() + 1); return r.toDataAddress() - 1; }

	reginfo_t notfound;

	static LLVMtoFailTranslator* createFromBinary(const std::string elf_path);
};

} // end of namespace

#endif
