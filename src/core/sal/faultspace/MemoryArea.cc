#include "MemoryArea.hpp"
#include "util/Logger.hpp"
#include <memory>

namespace fail {

static Logger LOG("FaultSpace", false);


guest_address_t
MemoryElement::get_guest_address() const {
	return static_cast<MemoryArea*>(get_area())->get_base()
		+ static_cast<guest_address_t>(get_offset());
}

FaultSpaceElement::injector_result
MemoryElement::inject(injector_fn injector) {
	const MemoryArea* ma = dynamic_cast<const MemoryArea*>(get_area());
	assert(ma != nullptr);
	auto mm = ma->get_manager();
	assert(mm != nullptr);

	guest_address_t addr = get_guest_address();
	byte_t byte = mm->getByte(addr);
	injector_value injected = injector(byte);

	mm->setByte(addr, injected);
	return { .original = byte, .injected = injected };
}

std::ostream&
MemoryElement::dump(std::ostream &os) const {
	return (os << "{ MemoryElement for addr "
			<< std::hex << std::showbase
			<< get_guest_address()
			<< " (mapped at="
			<< (get_area()->get_offset() + get_offset())
			<< ") } ");
}

std::vector<MemoryElement>
MemoryArea::translate(guest_address_t from, guest_address_t to,
					  std::function<bool(guest_address_t)> filter) {
	std::vector<MemoryElement> ret;
	for(guest_address_t it = from; it < to; ++it) {
		if(filter(it)) {
			ret.push_back(MemoryElement(this, it - get_base()));
		}
	}
	return ret;
}



std::unique_ptr<FaultSpaceElement>
MemoryArea::decode(fsp_offset_t addr) {
	auto e = std::make_unique<MemoryElement>(this, addr);
	return std::move(e);
}

} /* namespace fsp */

