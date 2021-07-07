#pragma once

#include "../../SALConfig.hpp"
#include "SailArchRegistersExtern.hpp"

namespace fail {

#if defined(BUILD_64BIT) // CHERI64
#include "gmp.h"

extern "C" {
    typedef struct {
    mp_bitcnt_t len;
    mpz_t * bits;
} lbits;
}

extern "C" __attribute__((weak)) struct zCapability zmemBitsToCapability(bool, lbits);
extern "C" __attribute__((weak)) void zcapToMemBits(lbits*, struct zCapability);
extern "C" __attribute__((weak)) void create_lbits(lbits*);

#else // CHERI32
extern "C" __attribute__((weak)) struct zCapability zmemBitsToCapability(bool, uint64_t);
extern "C" __attribute__((weak)) uint64_t zcapToMemBits(struct zCapability);
#endif


class SailCapabilityRegister: public SailRegister {
public:
	struct zCapability*  m_ref;

	SailCapabilityRegister(id_t id, regwidth_t w, struct zCapability* ref)
		: SailRegister(id, 2*w), m_ref(ref) {}

	virtual regdata_t read() const {
		assert(m_ref);
#if defined(BUILD_64BIT)
		lbits c;
		create_lbits(&c);
		zcapToMemBits(&c, *m_ref);
		unsigned __int128 hi,lo;
		lo = mpz_get_ui(*c.bits);
		mpz_cdiv_q_2exp(*c.bits,*c.bits,64);
		hi = mpz_get_ui(*c.bits);
		return hi << 64 | lo;
#else
		return static_cast<regdata_t>(zcapToMemBits(*m_ref));
#endif
	}

	virtual void write(regdata_t val) const {
		assert(m_ref);
#if defined(BUILD_64BIT)
		lbits c;
		create_lbits(&c);
		uint64_t hi,lo;							\
		hi = (val >> 64) & UINT64_MAX;
		lo = val & UINT64_MAX;
		mpz_set_ui(*c.bits, hi);
		mpz_mul_2exp(*c.bits, *c.bits, 64);		\
		mpz_add_ui(*c.bits, *c.bits, lo);
		c.len = 128;
		*m_ref = zmemBitsToCapability(m_ref->ztag, c);
#else
		*m_ref = zmemBitsToCapability(m_ref->ztag, static_cast<regdata_t>(val));
#endif
	}
};


class SailRegisterCheriTags: public SailRegister {
	std::vector<SailCapabilityRegister *>  m_regs;

public:
	SailRegisterCheriTags(id_t id, regwidth_t w, SailArchitecture* arch)
		: SailRegister(id, w) {
		for (Register *r : *arch) {
			if (r == nullptr) continue;
			if (auto *reg = dynamic_cast<SailCapabilityRegister*>(r)) {
				m_regs.push_back(reg);
			}
		}
		assert(m_regs.size() < w && "can't fit all register tags into one regdata_t");
	}

	virtual regdata_t read() const {
		regdata_t val = 0;
		for(size_t i = 0; i < m_regs.size(); ++i) {
			val |= (m_regs[i]->m_ref->ztag << i);
		}
		return val;
	}

	virtual void write(regdata_t val) const {
		for(size_t i = 0; i < m_regs.size(); ++i) {
			m_regs[i]->m_ref->ztag = !!(val & (1 << i));
		}
	}

	unsigned getTagIndex(SailCapabilityRegister *reg) const {
		for(size_t i = 0; i < m_regs.size(); ++i) {
			if (m_regs[i]->getId() == reg->getId()) {
				return i;
			}
		}
		throw std::runtime_error("Unknown capability register");
	}
};


extern regdata_t minstret_written();
extern void minstret_written(regdata_t value);

extern regdata_t parity_mismatch();
extern void parity_mismatch(regdata_t value);

};
