#include "SailFailInterface.h"
#include "sal/SALInst.hpp"
#include "sal/SALConfig.hpp"
#include "SailSimulator.hpp"
#include <vector>
#include <utility>

static bool inIFetch = false;

using namespace fail;

extern "C" {
void fail_startup(int *argc, char*** argv) {
	// This will manipulate the argc and argv, such that the FAIL*
	// arguments are filtered out
	fail::simulator.startup(*argc, *argv);
}

int fail_onGuestSystem(char data, unsigned port) {
	#if defined(CONFIG_EVENT_GUESTSYS)
		fail::simulator.onGuestSystem(data, port);
	#endif
	return 0;
}

int fail_onMemoryRead(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr) {
#if defined(CONFIG_EVENT_MEMREAD)
	if(!inIFetch) {
		static bool warned_once = false;
		if(!mpz_fits_ulong_p(data) && !warned_once) {
			std::cerr << "[sail wrapper] warning once: can't fit accessed data into 8-byte, extended trace information will be truncated!" << std::endl;
			warned_once = true;
		}

		// alternatively one could create a vector of arbitrary length here, but trace_event_extended only supports
		// upto 8 byte extended trace information anyway.
		// fixme: uint64_t udata = mpz_get_ui(data);
		size_t bitsize = mpz_get_ui(size);
		assert(bitsize >= mpz_sizeinbase(data, 2));

		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);

		fail::simulator.onMemoryAccess(&triggerCPU, addr, bitsize, false, instrPtr,
									   static_cast<memory_type_t>(mem_type));
	}
#endif
	return 0;
}

int fail_onMemoryWrite(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr) {
#if defined(CONFIG_EVENT_MEMWRITE)
		static bool warned_once = false;
		if(!mpz_fits_ulong_p(data) && !warned_once) {
			std::cerr << "[sail wrapper] warning once: can't fit accessed data into 8-byte, extended trace information will be truncated!" << std::endl;
			warned_once = true;
		}

		// alternatively one could create a vector of arbitrary length here, but Trace_Event_Extended only supports
		// upto 8 byte extended trace information anyway.
		// FIXME: uint64_t udata = mpz_get_ui(data);
		size_t bitsize = mpz_get_ui(size);
		assert(bitsize >= mpz_sizeinbase(data, 2));
		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);

		fail::simulator.onMemoryAccess(&triggerCPU, addr,  bitsize, true, instrPtr, static_cast<memory_type_t>(mem_type));
#endif
	return 0;
}

int fail_willExecuteInstruction(uint64_t instrPtr) {
#if defined(CONFIG_EVENT_BREAKPOINTS) || defined(CONFIG_EVENT_BREAKPOINTS_RANGE)
	fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
	// always address space 0
	fail::simulator.onBreakpoint(&triggerCPU, instrPtr, 0);
#endif
	return 0;
}

int fail_executeRequests() {
#if defined(CONFIG_SR_SAVE) || defined(CONFIG_SR_RESTORE)
	fail::simulator.checkTimers();
	fail::simulator.executeRequests();
#endif
	return 0;
}

int fail_didExecuteInstruction(uint64_t instrPtr, uint64_t instr) {
#if defined(CONFIG_EVENT_JUMP)
	if (fail::simulator::isJump(instrPtr, instr)) {
		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
		fail::simulator.onJump(&triggerCPU, false, instr);
	}
#endif
	return 0;
}

int fail_onTrap(unsigned trapNum) {
#if defined(CONFIG_EVENT_TRAP)
		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
		fail::simulator.onTrap(&triggerCPU, trapNum);
#endif
	return 0;
}

int fail_onInterrupt(unsigned interruptNum, bool nmi) {
#if defined(CONFIG_EVENT_INTERRUPT)
		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
		fail::simulator.onInterrupt(&triggerCPU, interruptNum, nmi);
#endif
	return 0;
}

bool fail_isSuppressedInterrupt(unsigned interruptNum) {
#if defined(CONFIG_SUPPRESS_INTERRUPTS)
		fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
		return triggerCPU.isSuppressedInterrupt(interruptNum);
#endif
	return false;
}


int fail_setInstructionFetch(bool instructionFetch) {
	inIFetch = instructionFetch;
	return 0;
}

} // extern "C"
