#include <iostream>

#include "SAL/SALInst.hpp"
#include "SAL/Register.hpp"
#include "TracingPlugin.hpp"

using std::cout;
using std::endl;

using namespace fi;
using namespace sal;

bool TracingPlugin::run()
{
	MemAccessEvent ev_mem(ANY_ADDR);
	BPSingleEvent ev_step(ANY_ADDR);
	BaseEvent *ev;

	if (m_iponly || !m_memonly) {
		simulator.addEvent(&ev_step);
	}
	if (m_memonly || !m_iponly) {
		simulator.addEvent(&ev_mem);
	}

	while (true) {
		ev = simulator.waitAny();

		if (ev == &ev_step) {
			simulator.addEvent(&ev_step);
			
			address_t ip = ev_step.getTriggerInstructionPointer();
			if (m_ipMap && !m_ipMap->isMatching(ip)) {
				continue;
			}

			if (m_os)
				*m_os << "[Tracing] IP " << hex << ip << "\n";
			if (m_trace) {
				Trace_Event *e = m_trace->add_event();
				e->set_ip(ip);
			}
		} else if (ev == &ev_mem) {
			simulator.addEvent(&ev_mem);

			address_t ip = ev_mem.getTriggerInstructionPointer();
			address_t addr = ev_mem.getTriggerAddress();
			size_t width = ev_mem.getTriggerWidth();
			if ((m_ipMap && !m_ipMap->isMatching(ip)) ||
			    (m_memMap && !m_memMap->isMatching(addr, width))) {
				continue;
			}

			if (m_os)
				*m_os << hex << "[Tracing] MEM "
				      << ((ev_mem.getTriggerAccessType() &
				           MemAccessEvent::MEM_READ) ? "R " : "W ")
				      << addr << " width " << width << " IP " << ip << "\n";
			if (m_trace) {
				Trace_Event *e = m_trace->add_event();
				e->set_ip(ip);
				e->set_memaddr(addr);
				e->set_width(width);
				e->set_accesstype(
				  (ev_mem.getTriggerAccessType() & MemAccessEvent::MEM_READ) ?
				  e->READ : e->WRITE);
			}
		} else {
			if (m_os)
				*m_os << "[Tracing] SOMETHING IS SERIOUSLY WRONG\n";
		}
	}

	return true;
}
