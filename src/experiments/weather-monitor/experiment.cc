#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/bochs/BochsRegister.hpp"
#include "sal/Event.hpp"

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#include "vptr_map.hpp"

#define LOCAL 0

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

bool WeatherMonitorExperiment::run()
{
	char const *statename = "bochs.state" WEATHER_SUFFIX;
	Logger log("Weathermonitor", false);
	BPSingleEvent bp;
	
	log << "startup" << endl;

#if 1
	// STEP 0: record memory map with vptr addresses
	GuestEvent g;
	while (true) {
		simulator.addEventAndWait(&g);
		cout << g.getData() << flush;
	}
#elif 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(WEATHER_FUNC_MAIN);
	simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << endl;
	simulator.save(statename);
	assert(bp.getTriggerInstructionPointer() == WEATHER_FUNC_MAIN);
	assert(simulator.getRegisterManager().getInstructionPointer() == WEATHER_FUNC_MAIN);

	// STEP 2: record trace for fault-space pruning
	log << "restoring state" << endl;
	simulator.restore(statename);
	log << "EIP = " << hex << simulator.getRegisterManager().getInstructionPointer() << endl;
	assert(simulator.getRegisterManager().getInstructionPointer() == WEATHER_FUNC_MAIN);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// TODO: record max(ESP)

	// restrict memory access logging to injection target
	MemoryMap mm;
	mm.add(WEATHER_DATA_START, WEATHER_DATA_END - WEATHER_DATA_START);
	tp.restrictMemoryAddresses(&mm);
	//tp.setLogIPOnly(true);

	// record trace
	char const *tracefile = "trace.tc" WEATHER_SUFFIX;
	ofstream of(tracefile);
	tp.setTraceFile(&of);

	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// trace WEATHER_NUMITER_TRACING measurement loop iterations
	bp.setWatchInstructionPointer(WEATHER_FUNC_WAIT_END);
	bp.setCounter(WEATHER_NUMITER_TRACING);
	simulator.addEvent(&bp);
	BPSingleEvent ev_count(ANY_ADDR);
	simulator.addEvent(&ev_count);

	// count instructions
	// FIXME add SAL functionality for this?
	int instr_counter = 0;
	while (simulator.waitAny() == &ev_count) {
		++instr_counter;
		simulator.addEvent(&ev_count);
	}

	log << dec << "tracing finished after " << instr_counter
	    << " instructions, seeing wait_end " << WEATHER_NUMITER_TRACING << " times" << endl;
	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		simulator.clearEvents(this); // cleanup
		return false;
	}
	of.close();
	log << "trace written to " << tracefile << endl;

	// wait another WEATHER_NUMITER_AFTER measurement loop iterations
	bp.setWatchInstructionPointer(WEATHER_FUNC_WAIT_END);
	bp.setCounter(WEATHER_NUMITER_AFTER);
	simulator.addEvent(&bp);

	// count instructions
	// FIXME add SAL functionality for this?
	instr_counter = 0;
	while (simulator.waitAny() == &ev_count) {
		++instr_counter;
		simulator.addEvent(&ev_count);
	}

	log << dec << "experiment finished after " << instr_counter
	    << " instructions, seeing wait_end " << WEATHER_NUMITER_AFTER << " times" << endl;

#elif 0
	// STEP 3: The actual experiment.
#if !LOCAL
	for (int i = 0; i < 500; ++i) {
#endif

	// get an experiment parameter set
	log << "asking job server for experiment parameters" << endl;
	WeatherMonitorExperimentData param;
#if !LOCAL
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
#else
	// XXX debug
	param.msg.set_instr_offset(1000);
	//param.msg.set_instr_address(12345);
	param.msg.set_mem_addr(0x00103bdc);
#endif

	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int mem_addr = param.msg.mem_addr();

	// for each job we're actually doing *8* experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		// 8 results in one job
		WeathermonitorProtoMsg_Result *result = param.msg.add_result();
		result->set_bit_offset(bit_offset);
		log << dec << "job " << id << " instr " << instr_offset
		    << " mem " << mem_addr << "+" << bit_offset << endl;

		log << "restoring state" << endl;
		simulator.restore(statename);

		// XXX debug
/*
		stringstream fname;
		fname << "job." << ::getpid();
		ofstream job(fname.str().c_str());
		job << "job " << id << " instr " << instr_offset << " (" << param.msg.instr_address() << ") mem " << mem_addr << "+" << bit_offset << endl;
		job.close();
*/

		// this marks THE END
		BPSingleEvent ev_end(ANY_ADDR);
		ev_end.setCounter(WEATHER_NUMINSTR_TRACING + WEATHER_NUMINSTR_AFTER);
		simulator.addEvent(&ev_end);

		// count loop iterations by counting wait_begin() calls
		// FIXME would be nice to have a callback API for this as this needs to
		//       be done "in parallel"
		BPSingleEvent ev_wait_begin(WEATHER_FUNC_WAIT_BEGIN);
		simulator.addEvent(&ev_wait_begin);
		int count_loop_iter_before = 0;

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(instr_offset);
			simulator.addEvent(&bp);

			// count loop iterations until FI
			while (simulator.waitAny() == &ev_wait_begin) {
				++count_loop_iter_before;
				simulator.addEvent(&ev_wait_begin);
			}
		}

		// --- fault injection ---
		MemoryManager& mm = simulator.getMemoryManager();
		byte_t data = mm.getByte(mem_addr);
		byte_t newdata = data ^ (1 << bit_offset);
		mm.setByte(mem_addr, newdata);
		// note at what IP we did it
		int32_t injection_ip = simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_injection_ip(injection_ip);
		result->set_iter_before_fi(count_loop_iter_before);
		log << "fault injected @ ip " << injection_ip
			<< " 0x" << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
		// sanity check
		if (param.msg.has_instr_address() &&
			injection_ip != param.msg.instr_address()) {
			stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip
			   << " != " << param.msg.instr_address();
			log << ss.str() << endl;
			result->set_resulttype(result->UNKNOWN);
			result->set_latest_ip(injection_ip);
			result->set_details(ss.str());
			result->set_iter_after_fi(0);

			simulator.clearEvents();
			continue;
		}

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
		BPRangeEvent ev_below_text(ANY_ADDR, WEATHER_TEXT_START - 1);
		BPRangeEvent ev_beyond_text(WEATHER_TEXT_END + 1, ANY_ADDR);
		simulator.addEvent(&ev_below_text);
		simulator.addEvent(&ev_beyond_text);
		// error detected
		BPSingleEvent ev_detected(WEATHER_FUNC_VPTR_PANIC);
		simulator.addEvent(&ev_detected);
		// timeout (e.g., stuck in a HLT instruction)
		// 10000us = 500000 instructions
		TimerEvent ev_timeout(10000, true);
		simulator.addEvent(&ev_timeout);

#if LOCAL && 0
		// XXX debug
		log << "enabling tracing" << endl;
		TracingPlugin tp;
		tp.setLogIPOnly(true);
		tp.setOstream(&cout);
		// this must be done *after* configuring the plugin:
		simulator.addFlow(&tp);
#endif

		BaseEvent* ev;

		// count loop iterations
		int count_loop_iter_after = 0;
		while ((ev = simulator.waitAny()) == &ev_wait_begin) {
			++count_loop_iter_after;
			simulator.addEvent(&ev_wait_begin);
		}
		result->set_iter_after_fi(count_loop_iter_after);

		// record latest IP regardless of result
		result->set_latest_ip(simulator.getRegisterManager().getInstructionPointer());

		if (ev == &ev_end) {
			log << "Result FINISHED (" << dec
			    << count_loop_iter_before << "+" << count_loop_iter_after << ")" << endl;
			result->set_resulttype(result->FINISHED);
		} else if (ev == &ev_timeout) {
			log << "Result TIMEOUT (" << dec
			    << count_loop_iter_before << "+" << count_loop_iter_after << ")" << endl;
			result->set_resulttype(result->TIMEOUT);
		} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
			log << "Result OUTSIDE" << endl;
			result->set_resulttype(result->OUTSIDE);
		} else if (ev == &ev_trap) {
			log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
			result->set_resulttype(result->TRAP);

			stringstream ss;
			ss << ev_trap.getTriggerNumber();
			result->set_details(ss.str());
		} else if (ev == &ev_detected) {
			log << dec << "Result DETECTED" << endl;
			result->set_resulttype(result->DETECTED);
		} else {
			log << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "eventid " << ev->getId() << " EIP " << simulator.getRegisterManager().getInstructionPointer();
			result->set_details(ss.str());
		}
		// explicitly remove all events before we leave their scope
		// FIXME event destructors should remove them from the queues
		simulator.clearEvents();
	}
	// sanity check: do we have exactly 8 results?
	if (param.msg.result_size() != 8) {
		log << "WTF? param.msg.result_size() != 8" << endl;
	}
#if !LOCAL
	m_jc.sendResult(param);

	}
#endif

#endif
	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
