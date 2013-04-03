#ifndef __UDIS86_HELPER_HPP__
#define __UDIS86_HELPER_HPP__

#include <udis86.h>
#include <vector>
#include <set>
#include <utility>
#include <map>

#include "sal/x86/X86Architecture.hpp"

class Udis86Helper {
public:
	// types
	enum OperandDirection { IN, OUT, INOUT, NONE };
	struct ModificationInfo {
		std::vector<OperandDirection> dir;
		// FIXME typedef for registerID/direction pair?
		std::vector<std::pair<ud_type, OperandDirection> > implicitOperands;

		// order in Intel style (destinations, then sources)
		ModificationInfo(OperandDirection d0 = NONE, OperandDirection d1 = NONE, OperandDirection d2 = NONE)
		{
			int i = (d0 != NONE) + (d1 != NONE) + (d2 != NONE);
			if (i--) { dir.push_back(d0);
				if (i--) { dir.push_back(d1);
					if (i) dir.push_back(d2); } }
		}
		void addImplicit(ud_type register_id, OperandDirection dir)
		{
			implicitOperands.push_back(std::pair<ud_type, OperandDirection>(register_id, dir));
		}
	};
	// mnemonic -> vector of ModificationInfo (some instructions exist in multiple variants)
	typedef std::map<unsigned, std::vector<ModificationInfo> > OpcodeModificationMap;
	typedef std::set<ud_type> UDRegisterSet;
private:
	ud_t *ud;
	OpcodeModificationMap m;
	void initOpcodeModMap();
	// target_set: target set
	// in_set: set of IN registers (an OUT memory operand may implicitly use IN regs)
	// idx: operand index
	void transferRegisters(UDRegisterSet& target_set, UDRegisterSet& in_set, int idx);
	void handleSpecialCases(UDRegisterSet& in, UDRegisterSet& out);
public:
	Udis86Helper() : ud(0) { initOpcodeModMap(); }
	Udis86Helper(ud_t *ud) : ud(ud) { initOpcodeModMap(); }
	void setUd(ud_t *newud) { ud = newud; }
	void inOutRegisters(UDRegisterSet& in, UDRegisterSet& out);
	char operandTypeToChar(unsigned op);
	unsigned operandCount();
	/**
	 * Returns the FailBochs equivalent to a UDIS86 GPR identifier.
	 * Attention: this only returns either 32-bit or 64-bit registers, no general IDs
	 * @param udisReg the udis86 GPR ID
	 * @returns the FailBochs GPR ID, usable with the BochsRegisterManager class
	 */
	static fail::GPRegisterId udisGPRToFailBochsGPR(ud_type_t udisReg, uint64_t& bitmask);
	static char const * mnemonicToString(unsigned mnemonic);
	static char const * typeToString(ud_type type);
};

#endif
