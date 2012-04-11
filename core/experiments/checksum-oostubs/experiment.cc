#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "SAL/SALConfig.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Memory.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"

// you need to have the tracing plugin enabled for this
#include "plugins/tracing/TracingPlugin.hpp"

#include "ecc_region.hpp"

using std::endl;

bool ChecksumOOStuBSExperiment::run()
{
	char const *statename = "checksum-oostubs.state";
	Logger log("Checksum-OOStuBS", false);
	fi::BPEvent bp;
	
	log << "startup" << endl;

#if 1
	// STEP 0: record memory map with addresses of "interesting" objects
	fi::GuestEvent g;
	while (true) {
		sal::simulator.addEventAndWait(&g);
		std::cout << g.getData() << std::flush;
	}
#elif 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(OOSTUBS_FUNC_ENTRY);
	sal::simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << std::hex << bp.getTriggerInstructionPointer() << endl;
	log << "error_corrected = " << std::dec << ((int)sal::simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED)) << endl;
	sal::simulator.save(statename);
	assert(bp.getTriggerInstructionPointer() == OOSTUBS_FUNC_ENTRY);
	assert(sal::simulator.getRegisterManager().getInstructionPointer() == OOSTUBS_FUNC_ENTRY);
#elif 1
	// STEP 2: record trace for fault-space pruning
	log << "restoring state" << endl;
	sal::simulator.restore(statename);
	log << "EIP = " << std::hex << sal::simulator.getRegisterManager().getInstructionPointer() << endl;
	assert(sal::simulator.getRegisterManager().getInstructionPointer() == OOSTUBS_FUNC_ENTRY);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	for (unsigned i = 0; i < sizeof(memoryMap)/sizeof(*memoryMap); ++i) {
		mm.add(memoryMap[i][0], memoryMap[i][1]);
	}
	tp.restrictMemoryAddresses(&mm);

	// record trace
	Trace trace;
	tp.setTraceMessage(&trace);

	// this must be done *after* configuring the plugin:
	sal::simulator.addFlow(&tp);

	bp.setWatchInstructionPointer(fi::ANY_ADDR);
	bp.setCounter(OOSTUBS_NUMINSTR);
	sal::simulator.addEvent(&bp);
	fi::BPEvent func_finish(OOSTUBS_FUNC_FINISH);
	sal::simulator.addEvent(&func_finish);

	if (sal::simulator.waitAny() == &func_finish) {
		log << "experiment reached finish()" << endl;
		// FIXME add instruction counter to SimulatorController
		sal::simulator.waitAny();
	}
	log << "experiment finished after " << std::dec << OOSTUBS_NUMINSTR << " instructions" << endl;
	
	uint32_t results[OOSTUBS_RESULTS_BYTES / sizeof(uint32_t)];
	sal::simulator.getMemoryManager().getBytes(OOSTUBS_RESULTS_ADDR, sizeof(results), results);
	for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
		log << "results[" << i << "]: " << std::dec << results[i] << endl;
	}

	sal::simulator.removeFlow(&tp);

	// serialize trace to file
	char const *tracefile = "trace.pb";
	std::ofstream of(tracefile);
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		return false;
	}
	trace.SerializeToOstream(&of);
	of.close();
	log << "trace written to " << tracefile << endl;
	
#elif 1
	// FIXME consider moving experiment repetition into Fail* or even the
	// SAL -- whether and how this is possible with the chosen backend is
	// backend specific
	while (true) {

	// STEP 3: The actual experiment.
	log << "restoring state" << endl;
	sal::simulator.restore(statename);

	// get an experiment parameter set
	log << "asking job server for experiment parameters" << endl;
	ChecksumOOStuBSExperimentData param;
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		sal::simulator.terminate(1);
	}
/*
	// XXX debug
	param.msg.set_instr_offset(2576034);
	param.msg.set_instr_address(1066640);
	param.msg.set_mem_addr(1099428);
	param.msg.set_bit_offset(4);
*/
	
	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int mem_addr = param.msg.mem_addr();
	int bit_offset = param.msg.bit_offset();
	log << "job " << id << " instr " << instr_offset << " mem " << mem_addr << "+" << bit_offset << endl;

	// XXX debug
	std::stringstream fname;
	fname << "job." << ::getpid();
	std::ofstream job(fname.str().c_str());
	job << "job " << id << " instr " << instr_offset << " (" << param.msg.instr_address() << ") mem " << mem_addr << "+" << bit_offset << endl;
	job.close();

	// reaching finish() could happen before OR after FI
	fi::BPEvent func_finish(OOSTUBS_FUNC_FINISH);
	sal::simulator.addEvent(&func_finish);
	bool finish_reached = false;

	// no need to wait if offset is 0
	if (instr_offset > 0) {
		// XXX test this with coolchecksum first (or reassure with sanity checks)
		// XXX could be improved with intermediate states (reducing runtime until injection)
		bp.setWatchInstructionPointer(fi::ANY_ADDR);
		bp.setCounter(instr_offset);
		sal::simulator.addEvent(&bp);

		// finish() before FI?
		if (sal::simulator.waitAny() == &func_finish) {
			finish_reached = true;
			log << "experiment reached finish() before FI" << endl;

			// wait for bp
			sal::simulator.waitAny();
		}
	}

	// --- fault injection ---
	sal::MemoryManager& mm = sal::simulator.getMemoryManager();
	sal::byte_t data = mm.getByte(mem_addr);
	sal::byte_t newdata = data ^ (1 << bit_offset);
	mm.setByte(mem_addr, newdata);
	// note at what IP we did it
	int32_t injection_ip = sal::simulator.getRegisterManager().getInstructionPointer();
	param.msg.set_injection_ip(injection_ip);
	log << "fault injected @ ip " << injection_ip
	    << " 0x" << std::hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
	// sanity check
	if (param.msg.has_instr_address() &&
	    injection_ip != param.msg.instr_address()) {
		std::stringstream ss;
		ss << "SANITY CHECK FAILED: " << injection_ip
		   << " != " << param.msg.instr_address();
		log << ss.str() << endl;
		param.msg.set_resulttype(param.msg.UNKNOWN);
		param.msg.set_latest_ip(injection_ip);
		param.msg.set_details(ss.str());

		sal::simulator.clearEvents();
		m_jc.sendResult(param);
		continue;
	}

	// --- aftermath ---
	// four possible outcomes:
	// - guest causes a trap, "crashes"
	// - guest reaches a "weird" state, stops with CLI+HLT ("panic")
	// - guest runs OOSTUBS_NUMINSTR+OOSTUBS_RECOVERYINSTR instructions but
	//   never reaches finish()
	// - guest reaches finish() within OOSTUBS_NUMINSTR+OOSTUBS_RECOVERYINSTR
	//   instructions with
	//   * a wrong result[0-2]
	//   * a correct result[0-2]

	// catch traps as "extraordinary" ending
	fi::TrapEvent ev_trap(fi::ANY_TRAP);
	sal::simulator.addEvent(&ev_trap);
	// OOStuBS' way to terminally halt (CLI+HLT)
	fi::BPEvent ev_halt(OOSTUBS_FUNC_CPU_HALT);
	sal::simulator.addEvent(&ev_halt);
	// remaining instructions until "normal" ending
	fi::BPEvent ev_done(fi::ANY_ADDR);
	ev_done.setCounter(OOSTUBS_NUMINSTR + OOSTUBS_RECOVERYINSTR - instr_offset);
	sal::simulator.addEvent(&ev_done);

/*
	// XXX debug
	log << "enabling tracing" << endl;
	TracingPlugin tp;
	tp.setLogIPOnly(true);
	tp.setOstream(&std::cout);
	// this must be done *after* configuring the plugin:
	sal::simulator.addFlow(&tp);
*/

	fi::BaseEvent* ev = sal::simulator.waitAny();

	// Do we reach finish() while waiting for ev_trap/ev_done?
	if (ev == &func_finish) {
		finish_reached = true;
		log << "experiment reached finish()" << endl;

		// wait for ev_trap/ev_done
		ev = sal::simulator.waitAny();
	}

	// record resultdata, finish_reached and error_corrected regardless of result
	uint32_t results[OOSTUBS_RESULTS_BYTES / sizeof(uint32_t)];
	sal::simulator.getMemoryManager().getBytes(OOSTUBS_RESULTS_ADDR, sizeof(results), results);
	for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
		log << "results[" << i << "]: " << std::dec << results[i] << endl;
		param.msg.add_resultdata(results[i]);
	}
	param.msg.set_finish_reached(finish_reached);
	int32_t error_corrected = sal::simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED);
	param.msg.set_error_corrected(error_corrected);
	param.msg.set_latest_ip(sal::simulator.getRegisterManager().getInstructionPointer());

	if (ev == &ev_done) {
		log << std::dec << "Result FINISHED" << endl;
		param.msg.set_resulttype(param.msg.FINISHED);
	} else if (ev == &ev_halt) {
		log << std::dec << "Result HALT #" << endl;
		param.msg.set_resulttype(param.msg.HALT);
	} else if (ev == &ev_trap) {
		log << std::dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
		param.msg.set_resulttype(param.msg.TRAP);
	} else {
		log << std::dec << "Result WTF?" << endl;
		param.msg.set_resulttype(param.msg.UNKNOWN);

		std::stringstream ss;
		ss << "eventid " << ev->getId() << " EIP " << sal::simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_details(ss.str());
	}
	m_jc.sendResult(param);

	}
#endif
	// Explicitly terminate, or the simulator will continue to run.
	sal::simulator.terminate();
}
