#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"
#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/bochs/BochsRegister.hpp"
#include "sal/bochs/BochsEvents.hpp"
#include "sal/Event.hpp"


using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
	!defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

bool ChecksumOOStuBSExperiment::run()
{
	Logger log("VEZS-Example", false);
	BPSingleEvent bp;

	log << "startup" << endl;

	bp.setWatchInstructionPointer(ANY_ADDR);
	bp.setCounter(OOSTUBS_NUMINSTR);

	//simulator.addEvent(&bp);
	//	BPSingleEvent ev_count(ANY_ADDR);
	//	simulator.addEvent(&ev_count);

	for (int i = 0; i < 400; ++i) { // more than 400 will be very slow (500 is max)

		// XXX debug
		int instr_offset = 1000;

		// reaching finish() could happen before OR after FI
		BPSingleEvent func_finish(OOSTUBS_FUNC_FINISH);
		simulator.addEvent(&func_finish);
		bool finish_reached = false;

		bp.setWatchInstructionPointer(ANY_ADDR);
		bp.setCounter(instr_offset);
		simulator.addEvent(&bp);

		// finish() before FI?
		if (simulator.waitAny() == &func_finish) {
			finish_reached = true;
			log << "experiment reached finish() before FI" << endl;

			// wait for bp
			simulator.waitAny();
		}


		// --- fault injection ---
		//byte_t newdata = data ^ (1 << bit_offset);
		//mm.setByte(mem_addr, newdata);
		// note at what IP we did it
		int32_t injection_ip = simulator.getRegisterManager().getInstructionPointer();
		log << "fault injected @ ip " << injection_ip << endl;

		// --- aftermath ---
		// possible outcomes:
		// - trap, "crash"
		// - jump outside text segment
		// - (XXX unaligned jump inside text segment)
		// - (XXX weird instructions?)
		// - (XXX results displayed?)
		// - reaches THE END
		// - error detected, stop
		// additional info:
		// - #loop iterations before/after FI
		// - (XXX "sane" display?)

		// catch traps as "extraordinary" ending
		TrapEvent ev_trap(ANY_TRAP);
		simulator.addEvent(&ev_trap);
		// jump outside text segment
		BPRangeEvent ev_below_text(ANY_ADDR, OOSTUBS_TEXT_START - 1);
		BPRangeEvent ev_beyond_text(OOSTUBS_TEXT_END + 1, ANY_ADDR);
		simulator.addEvent(&ev_below_text);
		simulator.addEvent(&ev_beyond_text);
		// timeout (e.g., stuck in a HLT instruction)
		// 10000us = 500000 instructions
		TimerEvent ev_timeout(1000000); // 50,000,000 instructions !!
		simulator.addEvent(&ev_timeout);

		// remaining instructions until "normal" ending
		BPSingleEvent ev_end(ANY_ADDR);
		ev_end.setCounter(OOSTUBS_NUMINSTR + OOSTUBS_RECOVERYINSTR - instr_offset);
		simulator.addEvent(&ev_end);

		// Start simulator and wait for any result
		BaseEvent* ev = simulator.waitAny();

		// Do we reach finish() while waiting for ev_trap/ev_done?
		if (ev == &func_finish) {
			finish_reached = true;
			log << "experiment reached finish()" << endl;

			// wait for ev_trap/ev_done
			ev = simulator.waitAny();
		}

		// record latest IP regardless of result
		//simulator.getRegisterManager().getInstructionPointer();


		if (ev == &ev_end) {
			log << dec << "Result FINISHED" << endl;
		} else if (ev == &ev_timeout) {
			log << "Result TIMEOUT" << endl;
		} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
			log << "Result OUTSIDE" << endl;
		} else if (ev == &ev_trap) {
			log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
		} else {
			log << "Result WTF?" << endl;
		}
		// explicitly remove all events before we leave their scope
		// FIXME event destructors should remove them from the queues
		simulator.clearEvents();
	}

	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
