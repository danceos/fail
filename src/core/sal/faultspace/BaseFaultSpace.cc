#include "BaseFaultSpace.hpp"
#include "util/Logger.hpp"

namespace fail {

using namespace std;

static Logger LOG("FaultSpace", false);

void BaseFaultSpace::set_point(fsp_address_t point) {
	if (point <= bumping_ptr) {
		throw new std::runtime_error("cannot decrease fault-space bumping ptr");
	}
	bumping_ptr = point;
}

void
BaseFaultSpace::add_area(unique_ptr<FaultSpaceArea> area) {
	Logger LOG("FaultSpace", false);
	LOG << "mapping " << area->get_name() << " @ 0x" << std::hex << bumping_ptr << std::endl;
	area->set_offset(this->bumping_ptr);
	size_t size = area->get_size();
	m_areas.emplace(bumping_ptr, std::move(area));
	bumping_ptr += size;
}


FaultSpaceArea&
BaseFaultSpace::get_area(const std::string& name) {
	for(auto& kv: m_areas) {
		if(kv.second->get_name() == name) {
			return *kv.second;
		}
	}
	throw std::invalid_argument("couldn't find area with name: " + name);
}

// recreate an element from a previously generated fault space address
unique_ptr<FaultSpaceElement> BaseFaultSpace::decode(fsp_address_t addr) {
	auto upper_bound = m_areas.upper_bound(addr);
	if(upper_bound == m_areas.begin()) {
		throw std::invalid_argument("invalid address in fault space: too low");
	}
	auto& area = std::prev(upper_bound)->second;
	LOG << "decoding 0x" << std::hex << addr << " with area: " << area->get_name() << std::endl;
	fsp_offset_t relative = addr - area->get_offset();
	return area->decode(relative);
}

FaultSpaceElement::injector_result
FaultSpaceElement::inject(FaultSpaceElement::injector_fn injector) {
	 throw new std::runtime_error("can't inject a non-subclassed fault space element");
 };

}

