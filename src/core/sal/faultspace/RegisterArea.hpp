#pragma once

#include <map>
#include "sal/Architecture.hpp"
#include "sal/Register.hpp"
#include "sal/CPUState.hpp"
#include "BaseFaultSpace.hpp"

namespace fail {

class RegisterArea;

class RegisterElement: public FaultSpaceElement {
private:
	Register* m_register;
	unsigned m_byte;

public:
	RegisterElement(FaultSpaceArea *area, fsp_offset_t offset,
					Register* reg, unsigned byte_offset)
		: FaultSpaceElement(area, offset),
		  m_register(reg), m_byte(byte_offset) {
		assert(byte_offset < reg->getWidth()/8);
	};

	virtual injector_result inject(injector_fn injector) override;
	virtual std::ostream&  dump(std::ostream & os) const override;

	Register* get_base() const { return m_register; }
};

class CPUState;

class RegisterArea: public FaultSpaceArea {
private:
	std::map<fsp_offset_t, Register*> m_addr_to_reg;
	std::map<Register*, fsp_offset_t> m_reg_to_addr;
	size_t m_size;

protected:
	fail::Architecture  m_arch;
	fail::CPUState*     m_state;
public:
	RegisterArea();

	fail::Architecture * get_arch() { return &m_arch; }

	CPUState * get_state() { return m_state; }
	void set_state(CPUState *state) { m_state = state; }

	virtual const size_t get_size() const override  { return m_size; }

	virtual std::unique_ptr<FaultSpaceElement> decode(fsp_address_t addr) override;

	// For the RegisterImporter, we translate Register IDs to FaultSpaceElements
	virtual std::vector<std::pair<RegisterElement, unsigned>> translate(const RegisterView &);
};



} // end-of-namespace: fail


