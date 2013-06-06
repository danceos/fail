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
	struct reginfo_t {
		int id;
		regwidth_t width;
		regdata_t  mask;
		byte_t offset;

		int toDataAddress() const {
			// .. 5 4 | 7 6 5 4 | 3 2 1 0
			// <reg>  | <width> | <offset>
			return (id << 8) | ((width/8) << 4) | (offset / 8);
		}
		static reginfo_t fromDataAddress(int addr) {
			int id = addr >> 8;
			regwidth_t width = ((addr >> 4) & 0xf) * 8;
			byte_t	  offset = (addr & 0xf) * 8;
			return reginfo_t(id, width, offset);
		}


		reginfo_t(int id=-1, regwidth_t width = 32, byte_t offs = 0)
			: id(id), width(width), mask((regwidth_t)((((long long)1 << width) - 1) << offs)), offset(offs) {};
	};
protected:

	LLVMtoFailTranslator(){};

#ifndef __puma
	typedef std::map<unsigned int, struct reginfo_t> ltof_map_t;
	ltof_map_t llvm_to_fail_map;
#endif


public:
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
private:
	reginfo_t notfound;
};

} // end of namespace

#endif
