#include "util/Logger.hpp"
#include "MemoryImporter.hpp"
#include "sal/faultspace/MemoryArea.hpp"

using namespace fail;
static fail::Logger LOG("MemoryImporter");


bool MemoryImporter::handle_ip_event(simtime_t curtime, instruction_count_t instr, Trace_Event &ev) {
	return true;
}

bool MemoryImporter::handle_mem_event(simtime_t curtime, instruction_count_t instr,
									  Trace_Event &ev) {
	memory_type_t mtype = static_cast<memory_type_t>(ev.memtype());
	if (mtype == MEMTYPE_UNKNOWN) // Legacy traces
		mtype = MEMTYPE_RAM;

	if(mtype == m_memtype || m_memtype == ANY_MEMORY) {
		// Get MemoryArea by memory type
		std::string area_id = memtype_descriptions[mtype];
		auto area = dynamic_cast<fail::MemoryArea*>(&m_fsp.get_area(area_id));
		assert(area != nullptr && "MemoryImporter failed to get a MemoryArea from the fault space description");

		char access_type = ev.accesstype() == ev.READ ? 'R' : 'W';

		guest_address_t from = ev.memaddr(), to = ev.memaddr() + ev.width();
		LOG << std::hex << std::showbase << ev.width() << " bytes -> from=" << from << " to=" << to << std::endl;

		auto filter = [this] (address_t a) -> bool { return !(this->m_mm && !this->m_mm->isMatching(a)); };
		for (auto &element : area->translate(from, to, filter)) {
			if(!add_faultspace_element(curtime, instr, element, 0xFF,
									   access_type, ev))
				return false;
		}
	}
	return true;
}
