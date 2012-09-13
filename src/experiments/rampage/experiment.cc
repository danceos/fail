#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#define LOCAL 1

#define STR_STATS    "calculating stats"
#define STR_START    "starting test pass"
#define STR_TESTED   "tested "
#define STR_BADFRAME "bad frame at pfn "

using namespace std;
using namespace fail;

/*
// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif
*/

bool RAMpageExperiment::run()
{
	address_t addr1 = 1024*1024*62+3;
	int bit1 = 3;
	address_t addr2 = 1024*1024*63+8;
	int bit2 = 5;

	// not implemented yet for QEMU:
	//simulator.restore("rampage-cext-started");

	MemWriteListener l_mem1(addr1);
	MemoryManager& mm = simulator.getMemoryManager();
	IOPortListener l_io(0x2f8, true); // ttyS1 aka COM2
	TimerListener l_timeout_local(1000*60*10); // 10m
	TimerListener l_timeout_global(1000*60*30); // 30m

	simulator.addListener(&l_mem1);
	simulator.addListener(&l_io);
	simulator.addListener(&l_timeout_local);
	simulator.addListener(&l_timeout_global);
	while (true) {
		BaseListener *l = simulator.resume();

		if (l == &l_mem1) {
			simulator.addListener(&l_mem1);
			unsigned char data = mm.getByte(addr1);
#if 1
			data |= 1 << bit1; // stuck-to-1
			mm.setByte(addr1, data);
#elif 0
			data &= ~(1 << bit1); // stuck-to-0
			mm.setByte(addr1, data);
#elif 0
			data &= ~(1 << bit2); // coupling bit2 := bit1
			data |= ((data & (1 << bit1)) != 0) << bit2;
			mm.setByte(addr1, data);
#elif 0
			data &= ~(1 << bit2); // coupling bit2 := !bit1
			data |= ((data & (1 << bit1)) == 0) << bit2;
			mm.setByte(addr1, data);
#elif 0
			unsigned char data2 = mm.getByte(addr2);
			data2 &= ~(1 << bit2); // coupling addr2:bit2 := !addr1:bit1
			data2 |= ((data & (1 << bit1)) == 0) << bit2;
			mm.setByte(addr2, data2);
#endif
			m_log << "access to addr 0x" << std::hex << addr1 << ", FI!" << endl;
		} else if (l == &l_io) {
			simulator.addListener(&l_io);
			if (handleIO(l_io.getData())) {
				// we saw some progress, restart local timer
				simulator.removeListener(&l_timeout_local);
				simulator.addListener(&l_timeout_local);
			}
		} else if (l == &l_timeout_local) {
			m_log << "local timeout" << std::endl;
		} else if (l == &l_timeout_global) {
			m_log << "global timeout" << std::endl;
		}
	}

	return true;
}

bool RAMpageExperiment::handleIO(char c)
{
	if (c != '\n') {
		m_output += c;
		return false;
	}

	// calculating stats
	if (!m_output.compare(0, sizeof(STR_STATS)-1, STR_STATS)) {
		if (last_line_was_startingtestpass) {
			// result: NO_PFNS_TESTED
			m_log << "no PFNs were tested this time" << std::endl;
			simulator.terminate();
		}
		m_log << STR_STATS << std::endl;

	// starting test pass
	} else if (!m_output.compare(0, sizeof(STR_START)-1, STR_START)) {
		last_line_was_startingtestpass = true;
		m_log << STR_START << std::endl;

	// tested %08x-%08x %08x-%08x ...
	} else if (!m_output.compare(0, sizeof(STR_TESTED)-1, STR_TESTED)) {
		last_line_was_startingtestpass = false;
		m_log << STR_TESTED << std::endl;

	// bad frame at pfn %08x
	} else if (!m_output.compare(0, sizeof(STR_BADFRAME)-1, STR_BADFRAME)) {
		m_log << STR_BADFRAME << std::endl;

		simulator.terminate();
	} else {
		// unknown
		m_log << "wtf unknown: " << m_output << std::endl;
		simulator.terminate();
	}

	m_output.clear();
	return true;
}
