#include "util/Logger.hpp"
#include "MemoryImporter.hpp"

using namespace fail;
#include <bitset>
static fail::Logger LOG("MemoryImporter");


bool MemoryImporter::handle_ip_event(simtime_t curtime, instruction_count_t instr, Trace_Event &ev) {
	return true;
}

bool MemoryImporter::handle_mem_event(simtime_t curtime, instruction_count_t instr,
									  Trace_Event &ev) {
    memory_type_t mtype = static_cast<memory_type_t>(ev.memtype());
    if(mtype == m_memtype || m_memtype == ANY_MEMORY) {
        auto filter = [this] (address_t a) -> bool { return !(this->m_mm && !this->m_mm->isMatching(a)); };
        std::string area_id = "memory" + std::to_string(mtype);
        auto area = dynamic_cast<memory_area*>(m_fsp.get_area(area_id).get());
        assert(area != nullptr && "MemoryImporter failed to get a MemoryArea from the fault space description");

        access_t acc = ev.accesstype() == ev.READ ? access_t::READ : access_t::WRITE;
        unsigned nfull = std::floor(ev.width()/8.0);
        unsigned npartial = std::ceil(ev.width()/8.0) - nfull;

        assert(npartial <= 1);

        if(nfull > 0) {
            address_t from = ev.memaddr(), to = ev.memaddr() + nfull;
            LOG << std::hex << std::showbase << nfull << " full bytes -> from=" << from << " to=" << to << std::endl;
            auto elems = area->encode<address_t>(from,to,filter);
            if(!add_faultspace_elements(curtime, instr, std::move(elems), 0xFF, acc, ev))
                return false;
        }

        if(npartial > 0) {
            address_t from = ev.memaddr() + nfull, to = from + 1;
            unsigned nbits = ev.width() - nfull*8;
            unsigned mask = (1u << nbits) - 1;
            LOG << std::hex << std::showbase << npartial << " partial bytes -> from=" << from << " to=" << to << " mask: " << mask << std::endl;
            auto elems = area->encode<address_t>(from, to, filter);
            if(!add_faultspace_elements(curtime, instr, std::move(elems), mask, acc, ev))
                return false;
        }
        return true;
    }
    else {
        std::cout << "dropping element due to mismatched mtype expected: " << m_memtype << " got: " << mtype << std::endl;
        return true;
    }
}
