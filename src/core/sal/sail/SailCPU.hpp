#pragma once

#include "SailArchitecture.hpp"
#include "../CPUState.hpp"

namespace fail {

class SailBaseCPU: public SailArchitecture, public CPUState {
protected:
private:
	unsigned int id;

protected:
	Register *pc_reg;
	Register *sp_reg;

public:
	SailBaseCPU(): SailBaseCPU(0) { }
	SailBaseCPU(unsigned int id);
	virtual ~SailBaseCPU() {}

	////////////////////////////////////////////////////////////////
	// CPUState interface
	regdata_t getRegisterContent(const Register* r) const {
		return static_cast<const SailRegister*>(r)->read();
	}

	void setRegisterContent(const Register* r, regdata_t value) {
		static_cast<const SailRegister*>(r)->write(value);
	}

	address_t getInstructionPointer() const {
		return getRegisterContent(pc_reg);
	}

	address_t getStackPointer() const {
		return getRegisterContent(sp_reg);
	}

	simtime_t getInstructionCounter() const {
		return SailArchitecture::getInstructionCounter();
	}

	constexpr simtime_t getFrequency() const { return 100000; }

	unsigned int getId() const { return id; }

	// CPU serialization and deserialization
	void serialize(std::ostream& os);
	void unserialize(std::istream& is);

	// lifecycle
	void reset() {
		setRegisterContent(pc_reg, SAIL_RESET_VECTOR);
	}
};

} // end-of-namespace: fail

// Declares SailCPU
#include "SailArchCPU.hpp"

namespace fail {
typedef SailCPU ConcreteCPU;
}
