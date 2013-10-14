#include <iostream>
#include <fstream>

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
#include "sal/bochs/BochsListener.hpp"
#include "sal/Listener.hpp"

// You need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#include "ecc_region.hpp"

#define LOCAL 0

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

bool ChecksumOOStuBSExperiment::run()
{
	char const *statename = "checksum-oostubs.state";
	Logger log("Checksum-OOStuBS", false);
	BPSingleListener bp;

	log << "startup" << endl;

#if 0
	// STEP 0: record memory map with addresses of "interesting" objects
	GuestListener g;
	while (true) {
		simulator.addListenerAndResume(&g);
		cout << g.getData() << flush;
	}
#elif 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(OOSTUBS_FUNC_ENTRY);
	simulator.addListenerAndResume(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << endl;
	log << "error_corrected = " << dec << ((int)simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED)) << endl;
	simulator.save(statename);
	assert(bp.getTriggerInstructionPointer() == OOSTUBS_FUNC_ENTRY);
	assert(simulator.getCPU(0).getInstructionPointer() == OOSTUBS_FUNC_ENTRY);
#elif 0
	// STEP 2: record trace for fault-space pruning
	log << "restoring state" << endl;
	simulator.restore(statename);
	log << "EIP = " << hex << simulator.getCPU(0).getInstructionPointer() << endl;
	assert(simulator.getCPU(0).getInstructionPointer() == OOSTUBS_FUNC_ENTRY);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	for (unsigned i = 0; i < sizeof(memoryMap)/sizeof(*memoryMap); ++i) {
		mm.add(memoryMap[i][0], memoryMap[i][1]);
	}
	tp.restrictMemoryAddresses(&mm);

	// record trace
	char const *tracefile = "trace.tc";
	ofstream of(tracefile);
	tp.setTraceFile(&of);

	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

#if 1
	// trace WEATHER_NUMITER_TRACING measurement loop iterations
	// -> calibration
	bp.setWatchInstructionPointer(OOSTUBS_FUNC_FINISH);
	//bp.setCounter(WEATHER_NUMITER_TRACING); // single event, only
#else
	// FIXME this doesn't work properly: trace is one instruction too short as
	//       tp is removed before all events were delivered
	// trace WEATHER_NUMINSTR_TRACING instructions
	// -> campaign-ready traces with identical lengths
	bp.setWatchInstructionPointer(ANY_ADDR);
	bp.setCounter(OOSTUBS_NUMINSTR);
#endif
	simulator.addListener(&bp);
	BPSingleListener ev_count(ANY_ADDR);
	simulator.addListener(&ev_count);

	// count instructions
	// FIXME add SAL functionality for this?
	int instr_counter = 0;
	while (simulator.resume() == &ev_count) {
		++instr_counter;
		simulator.addListener(&ev_count);
	}

	log << dec << "tracing finished after " << instr_counter  << endl;

	uint32_t results[OOSTUBS_RESULTS_BYTES / sizeof(uint32_t)];
	simulator.getMemoryManager().getBytes(OOSTUBS_RESULTS_ADDR, sizeof(results), results);
	for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
		log << "results[" << i << "]: " << dec << results[i] << endl;
	}

	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		simulator.clearListeners(this);
		return false;
	}
	of.close();
	log << "trace written to " << tracefile << endl;

#elif 1
	// STEP 3: The actual experiment.
#if !LOCAL
	for (int i = 0; i < 50; ++i) { // only do 50 sequential experiments, to prevent swapping
	// 50 exp ~ 0.5GB RAM usage per instance (linearly increasing)
#endif

	// get an experiment parameter set
	log << "asking job server for experiment parameters" << endl;
	ChecksumOOStuBSExperimentData param;
#if !LOCAL
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
#else
	// XXX debug
	param.msg.set_instr_offset(301324);
	//param.msg.set_instr_address(12345);
	param.msg.set_mem_addr(1105120);
#endif

	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int mem_addr = param.msg.mem_addr();

	// for each job we're actually doing *8* experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		// 8 results in one job
		OOStuBSProtoMsg_Result *result = param.msg.add_result();
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

		// reaching finish() could happen before OR after FI
		BPSingleListener func_finish(OOSTUBS_FUNC_FINISH);
		simulator.addListener(&func_finish);
		bool finish_reached = false;

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(instr_offset);
			simulator.addListener(&bp);

			// finish() before FI?
			if (simulator.resume() == &func_finish) {
				finish_reached = true;
				log << "experiment reached finish() before FI" << endl;

				// wait for bp
				simulator.resume();
				//TODO: why wait here? it seems that something went completely wrong?
			}
		}

		// --- fault injection ---
		MemoryManager& mm = simulator.getMemoryManager();
		byte_t data = mm.getByte(mem_addr);
		byte_t newdata = data ^ (1 << bit_offset);
		mm.setByte(mem_addr, newdata);
		// note at what IP we did it
		int32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
		param.msg.set_injection_ip(injection_ip);
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

			simulator.clearListeners();
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
		TrapListener ev_trap(ANY_TRAP);
		simulator.addListener(&ev_trap);
		// jump outside text segment
		BPRangeListener ev_below_text(ANY_ADDR, OOSTUBS_TEXT_START - 1);
		BPRangeListener ev_beyond_text(OOSTUBS_TEXT_END + 1, ANY_ADDR);
		simulator.addListener(&ev_below_text);
		simulator.addListener(&ev_beyond_text);
		// timeout (e.g., stuck in a HLT instruction)
		// 10000us = 500000 instructions
		TimerListener ev_timeout(1000000); // 50,000,000 instructions !!
		simulator.addListener(&ev_timeout);

		// remaining instructions until "normal" ending
		BPSingleListener ev_end(ANY_ADDR);
		ev_end.setCounter(OOSTUBS_NUMINSTR + OOSTUBS_RECOVERYINSTR - instr_offset);
		simulator.addListener(&ev_end);

#if LOCAL && 0
		// XXX debug
		log << "enabling tracing" << endl;
		TracingPlugin tp;
		tp.setLogIPOnly(true);
		tp.setOstream(&cout);
		// this must be done *after* configuring the plugin:
		simulator.addFlow(&tp);
#endif

		BaseListener* ev = simulator.resume();

		// Do we reach finish() while waiting for ev_trap/ev_done?
		if (ev == &func_finish) {
			finish_reached = true;
			log << "experiment reached finish()" << endl;

			// wait for ev_trap/ev_done
			ev = simulator.resume();
		}

		// record latest IP regardless of result
		result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());

		// record resultdata, finish_reached and error_corrected regardless of result
		uint32_t results[OOSTUBS_RESULTS_BYTES / sizeof(uint32_t)];
		simulator.getMemoryManager().getBytes(OOSTUBS_RESULTS_ADDR, sizeof(results), results);
		for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
			log << "results[" << i << "]: " << dec << results[i] << endl;
			result->add_resultdata(results[i]);
		}
		result->set_finish_reached(finish_reached);
		int32_t error_corrected = simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED);
		result->set_error_corrected(error_corrected);

		if (ev == &ev_end) {
			log << dec << "Result FINISHED" << endl;
			result->set_resulttype(result->FINISHED);
		} else if (ev == &ev_timeout) {
			log << "Result TIMEOUT" << endl;
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
		} else {
			log << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "event addr " << ev << " EIP " << simulator.getCPU(0).getInstructionPointer();
			result->set_details(ss.str());
		}
		// explicitly remove all events before we leave their scope
		// FIXME event destructors should remove them from the queues
		simulator.clearListeners();
	}
	// sanity check: do we have exactly 8 results?
	if (param.msg.result_size() != 8) {
		log << "WTF? param.msg.result_size() != 8" << endl;
	} else {
#if !LOCAL
		m_jc.sendResult(param);
#endif
	}

#if !LOCAL
	}
#endif

#endif
	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
