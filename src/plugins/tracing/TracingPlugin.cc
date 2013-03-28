#include <iostream>
#include <assert.h>

#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "TracingPlugin.hpp"

using namespace std;
using namespace fail;

bool TracingPlugin::run()
{
	MemoryManager& mm = simulator.getMemoryManager();

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

#ifdef BUILD_X86
	size_t ids[] = {RID_CSP, RID_CBP, RID_FLAGS};
#else
    size_t ids[] = {};
#endif

	Register *regs[sizeof(ids)/sizeof(*ids)];
	for (unsigned i = 0; i < sizeof(ids)/sizeof(*ids); ++i)
		regs[i] = simulator.getCPU(0).getRegister(ids[i]);

	// the first event gets an absolute time stamp, all others a delta to their
	// predecessor
	simtime_t prevtime = 0, curtime;
	simtime_diff_t deltatime;

	while (true) {
		ev = simulator.resume();

		curtime = simulator.getTimerTicks();
		deltatime = curtime - prevtime;

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
				// only store deltas != 0
				if (deltatime != 0) {
					e.set_time_delta(deltatime);
				}
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
				// only store deltas != 0
				if (deltatime != 0) {
					e.set_time_delta(deltatime);
				}

				/* When we're doing a full trace, we log more data in
				   the case of a memory event */
				if (m_full_trace) {
					Trace_Event_Extended &ext = *e.mutable_trace_ext();
					// Read the accessed data
					assert(width <= 8);
					uint64_t data = 0;
					mm.getBytes(addr, width, &data);
					ext.set_data(data);

					for (unsigned i = 0; i < sizeof(ids)/sizeof(*ids); ++i) {
						Trace_Event_Extended_Registers *er = ext.add_registers();
						er->set_id(ids[i]);
						er->set_value(simulator.getCPU(0).getRegisterContent(regs[i]));
						if (er->value() <= mm.getPoolSize() - width) {
							uint32_t value_deref;
							mm.getBytes(er->value(), 4, &value_deref);
							er->set_value_deref(value_deref);
						}
					}
				}

				ps->writeMessage(&e);
			}
		} else {
			if (m_os)
				*m_os << "[Tracing] SOMETHING IS SERIOUSLY WRONG\n";
		}

		// do this only if the last delta was written
		// (no, e.g., memory map mismatch)
		prevtime = curtime;
	}

	return true;
}
