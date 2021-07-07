#pragma once

#include "../CPU.hpp"
#include "../SALConfig.hpp"
#include <iostream>


namespace fail {

/**
 * \class SailArchitecture
 *
 * For a SAIL architecture, we generate a custom CPU architecture that
 * depends on specific registers for that architecture.
 */
class SailArchitecture : public CPUArchitecture {
public:
	SailArchitecture();
	~SailArchitecture();

	simtime_t getInstructionCounter() const;
};

typedef SailArchitecture Architecture;

// Register IDs
enum RegisterID {
#define SAIL_REG_PTR(id,...) RID_##id,
#define SAIL_REG_FN(id,...) RID_##id,
#define SAIL_REG_TMPL(id,...) RID_##id,
#define SAIL_REG_CLASS(id,...) RID_##id,

#include "SailArchRegisters.inc"

#undef SAIL_REG_PTR
#undef SAIL_REG_FN
#undef SAIL_REG_TMPL
#undef SAIL_REG_CLASS
};

/**
 * \class SailRegister
 *
 * Sail Registers are at most regdata_t wide and have a read and a write function
 */
class SailRegister: public Register {
private:
	friend std::ostream& operator<<(std::ostream& os, const SailRegister& r);
	friend std::istream& operator>>(std::istream& is, const SailRegister& r);
public:
	SailRegister(id_t id, regwidth_t w) : Register(id, w) {
		assert(w <= sizeof(regdata_t) * 8);
	}
	virtual regdata_t read() const = 0;
	virtual void     write(regdata_t val) const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const SailRegister& r) {
	regdata_t value = r.read();
	os << value;
	return os;
}
inline std::istream& operator>>(std::istream& is, const SailRegister& r) {
	regdata_t dummy;
	is >> dummy;
	r.write(dummy);
	return is;
}

template <typename T>
class SailRegisterPtr: public SailRegister {
	T*  m_ref;

public:
	SailRegisterPtr(id_t id, regwidth_t w, T* ref)
		: SailRegister(id, w), m_ref(ref) {}

	virtual regdata_t read() const {
		assert(m_ref);
		return (regdata_t) *m_ref;
	}

	virtual void write(regdata_t val) const {
		assert(m_ref);
		*m_ref = (T) val;
	}
};

template<regdata_t (*RFn)(void), void (*WFn)(regdata_t)>
class SailRegisterFn: public SailRegister {
public:
	SailRegisterFn(id_t id, regwidth_t w): SailRegister(id, w) {}
	virtual regdata_t read() const {
		return RFn();
	}
	virtual void write(regdata_t val) const {
		return WFn(val);
	}
};


} // end-of-namespace: fail

