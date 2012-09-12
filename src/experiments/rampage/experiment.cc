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
	Logger log("RAMpage", false);
	address_t addr1 = 1024*1024*62+3;
	int bit1 = 3;
	address_t addr2 = 1024*1024*63+8;
	int bit2 = 5;

	// not implemented yet for QEMU:
	//simulator.restore("rampage-cext-started");

	// TODO: Timeout
	// TODO: Serial

	MemWriteListener l_mem1(addr1);
	MemoryManager& mm = simulator.getMemoryManager();
	IOPortListener l_io(0x2f8, true); // ttyS1 aka COM2

	simulator.addListener(&l_mem1);
	simulator.addListener(&l_io);
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
			log << "access to addr 0x" << std::hex << addr1 << ", FI!" << endl;
		} else if (l == &l_io) {
			simulator.addListener(&l_io);
			std::cout << l_io.getData() << std::endl;
		}
	}

	return true;
}
