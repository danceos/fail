#include "sal/faultspace/RegisterArea.hpp"
#include "sal/Register.hpp"
#include "sal/SALInst.hpp"
#include <cmath> // std::ceil
#include <sstream>

namespace fail {

static Logger LOG("RegisterArea", false);

RegisterArea::RegisterArea()
	: FaultSpaceArea("register"), m_state(nullptr) {
	fsp_offset_t offset = 0;
	for (Register *r : m_arch) {
		if (r == nullptr)
			continue; // Register IDs can have wholes
		m_addr_to_reg[offset] = r;
		m_reg_to_addr[r] = offset;
		offset += std::ceil(r->getWidth()/8.0);
	}
	m_size = offset;
}

std::vector<std::pair<RegisterElement, unsigned>>
RegisterArea::translate(const RegisterView &view) {
	std::vector<std::pair<RegisterElement, unsigned>> ret;

	// Generate access mask. There is surely a more efficient way to
	// do this. But this implementation can be understood more easily.
	unsigned char masks[sizeof(regdata_t)] = {0};
	for (unsigned i = view.offset; i < (view.offset + view.width); i++) {
		masks[i / 8] |= 1 << (i% 8);
	}

	// Get the FAIL* base register and its location in the faultspace
	Register*    base = m_arch.getRegister(view.id);
	fsp_offset_t base_offset = m_reg_to_addr[base];
	for (int byte = 0; byte < sizeof(regdata_t); byte++) {
		if (masks[byte] == 0) continue;
		ret.push_back(
			std::make_pair(
				RegisterElement(this, base_offset + byte, base, byte),
				masks[byte])
			);
	}

	return ret;
}

std::unique_ptr<FaultSpaceElement>
RegisterArea::decode(fsp_address_t addr) {
	// find next Register that is mapped below this address.
	auto upper_bound = m_addr_to_reg.upper_bound(addr);
	if(upper_bound == m_addr_to_reg.begin()) {
		throw std::invalid_argument("invalid address in fault area: too low");
	}
	Register* reg = std::prev(upper_bound)->second;
	// if we subtract this registers base address, we get the byte offset
	unsigned byte = addr - m_reg_to_addr[reg];

	auto e = std::make_unique<RegisterElement>(this,addr,reg,byte);
	return std::move(e);
}


FaultSpaceElement::injector_result
RegisterElement::inject(injector_fn injector)
{
	RegisterArea *area = dynamic_cast<RegisterArea*>(get_area());
	assert(area != nullptr);
	auto cpu = area->get_state();
	Register *base = get_base();


	regdata_t value = cpu->getRegisterContent(base);

	assert(sizeof(injector_value) == 1);

	injector_value original_byte = static_cast<injector_value>((value >> m_byte*8) & 0xFF);
	injector_value injected_byte = injector(original_byte);
	// mask out original byte
	regdata_t mask = ~(static_cast<regdata_t>(0xFF) << m_byte*8);
	regdata_t injected_value = value & mask;
	// and update with injected version
	injected_value |= static_cast<regdata_t>(injected_byte) << m_byte*8;

	cpu->setRegisterContent(base, injected_value);

	LOG << "injecting register " << base->getName() << "(id=" << base->getId() << ") at offset " << std::dec << m_byte << std::endl
			  << std::hex << std::showbase
			  << '\t' << "previous value: " << value << std::endl
			  << '\t' << "injected value: " << injected_value << std::endl
			  << std::dec << std::noshowbase;

	return { .original = original_byte, .injected = injected_byte };
}

std::ostream&  RegisterElement::dump(std::ostream & os) const {
	return (
		os << "{ RegisterElement '"
		<< get_base()->getName()
		<< "' at byte "
		<< std::hex << std::showbase
		<< m_byte
		<< " (mapped @ " << get_area()->get_offset() << "+"<< get_offset()
		<< " ->" << (get_offset() + get_area()->get_offset())
		<< ") } ");
}

} // end-of-namespace: fail
