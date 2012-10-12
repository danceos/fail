#include <iostream>

#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Listener.hpp"
#include "TracingPlugin.hpp"

using namespace std;
using namespace fail;

bool TracingPlugin::run()
{
	MemAccessListener ev_mem(ANY_ADDR);
	BPSingleListener ev_step(ANY_ADDR);
	BaseListener *ev;

	if (m_iponly || !m_memonly) {
		simulator.addListener(&ev_step);
	}
	if (m_memonly || !m_iponly) {
		simulator.addListener(&ev_mem);
	}
	if(m_protoStreamFile) {
		ps = new ProtoOStream(m_protoStreamFile);
	}
	
	while (true) {
		ev = simulator.resume();

		if (ev == &ev_step) {
			simulator.addListener(&ev_step);
			
			address_t ip = ev_step.getTriggerInstructionPointer();
			if (m_ipMap && !m_ipMap->isMatching(ip)) {
				continue;
			}

			if (m_os)
				*m_os << "[Tracing] IP " << hex << ip << "\n";
			if (m_protoStreamFile) {
				Trace_Event e;
				e.set_ip(ip);
				ps->writeMessage(&e);
			}
		} else if (ev == &ev_mem) {
			simulator.addListener(&ev_mem);

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
			if (m_protoStreamFile) {
				Trace_Event e;
				e.set_ip(ip);
				e.set_memaddr(addr);
				e.set_width(width);
				e.set_accesstype(
				  (ev_mem.getTriggerAccessType() & MemAccessEvent::MEM_READ) ?
				  e.READ : e.WRITE);
				ps->writeMessage(&e);
			}
		} else {
			if (m_os)
				*m_os << "[Tracing] SOMETHING IS SERIOUSLY WRONG\n";
		}
	}

	return true;
}
