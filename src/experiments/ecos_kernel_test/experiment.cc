#include <iostream>
#include <fstream>
//#include <string>

// getpid
#include <sys/types.h>
#include <unistd.h>


#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/bochs/BochsRegister.hpp"
#include "sal/bochs/BochsEvents.hpp"
#include "sal/Event.hpp"

// You need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#define LOCAL 0

#define PREREQUISITES 0 // 1: do step 0-2 ; 0: do step 3

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif


char const * const mm_filename = "memory_map.txt";
char const *statename = "ecos_kernel_test.state";

#if PREREQUISITES
bool EcosKernelTestExperiment::retrieveGuestAddresses() {
	log << "STEP 0: record memory map with addresses of 'interesting' objects" << endl;

	// workaround for 00002808875p[BIOS ] >>PANIC<< Keyboard error:21
	BPSingleEvent bp;
	bp.setWatchInstructionPointer(ANY_ADDR);
	simulator.addEventAndWait(&bp);

	GuestEvent g;
	simulator.addEvent(&g);

	// 10000us = 500000 instructions
	TimerEvent record_timeout(50000000); //TODO: how long to wait?
	simulator.addEvent(&record_timeout);

	// memory map serialization
	ofstream mm(mm_filename, ios::out | ios::app);
	if (!mm.is_open()) {
		log << "failed to open " << mm_filename << endl;
		return false;
	}

	string *str = new string; // buffer for guest events
	unsigned number_of_guest_events = 0;

	while (simulator.waitAny() != &record_timeout) {
		simulator.addEvent(&g);
		str->push_back(g.getData());
		if (g.getData() == '\t') {
			// addr complete?
			//cout << "full: " << *str << "sub: " << str->substr(str->find_last_of('x') - 1) << endl;
			// interpret the string obtained by the guest events as address in hex
			unsigned guest_addr;
			stringstream converter(str->substr(str->find_last_of('x') + 1));
			converter >> hex >> guest_addr;
			mm << guest_addr << '\t';
			str->clear();
		} else if (g.getData() == '\n') {
			// len complete?
			// interpret the string obtained by the guest events as length in decimal
			unsigned guest_len;
			stringstream converter(*str);
			converter >> dec >> guest_len;
			mm << guest_len << '\n';
			str->clear();
			number_of_guest_events++;
		}
	}
	assert(number_of_guest_events > 0);
	log << "Record timeout reached: created memory map (" << number_of_guest_events << " entries)" << endl;
	delete str;

	// close serialized mm
	mm.flush();
	mm.close();

	// clean up simulator
	simulator.clearEvents();
	return true;
}

bool EcosKernelTestExperiment::establishState() {
	log << "STEP 1: run until interesting function starts, and save state" << endl;

	GuestEvent g;

	while (true) {
		simulator.addEventAndWait(&g);
		if(g.getData() == 'Q') {
		  log << "Guest system triggered: " << g.getData() << endl;
		  break;
		}
	}

	BPSingleEvent bp;
	bp.setWatchInstructionPointer(ECOS_FUNC_ENTRY);
	simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << endl;
	//log << "error_corrected = " << dec << ((int)simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED)) << endl;
	simulator.save(statename);
	assert(bp.getTriggerInstructionPointer() == ECOS_FUNC_ENTRY);
	assert(simulator.getRegisterManager().getInstructionPointer() == ECOS_FUNC_ENTRY);

	// clean up simulator
	simulator.clearEvents();
	return true;
}

bool EcosKernelTestExperiment::performTrace() {
	log << "STEP 2: record trace for fault-space pruning" << endl;

	log << "restoring state" << endl;
	simulator.restore(statename);
	log << "EIP = " << hex << simulator.getRegisterManager().getInstructionPointer() << endl;
	assert(simulator.getRegisterManager().getInstructionPointer() == ECOS_FUNC_ENTRY);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	EcosKernelTestCampaign::readMemoryMap(mm, mm_filename);

	tp.restrictMemoryAddresses(&mm);

	// record trace
	char const *tracefile = "trace.tc";
	ofstream of(tracefile);
	tp.setTraceFile(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	BPSingleEvent bp;
#if 1
	// trace WEATHER_NUMITER_TRACING measurement loop iterations
	// -> calibration
	bp.setWatchInstructionPointer(ECOS_FUNC_FINISH);
	//bp.setCounter(WEATHER_NUMITER_TRACING); // single event, only
#else
	// FIXME this doesn't work properly: trace is one instruction too short as
	//       tp is removed before all events were delivered
	// trace WEATHER_NUMINSTR_TRACING instructions
	// -> campaign-ready traces with identical lengths
	bp.setWatchInstructionPointer(ANY_ADDR);
	bp.setCounter(OOSTUBS_NUMINSTR);
#endif
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

	log << dec << "tracing finished after " << instr_counter  << " instructions" << endl;
	//TODO: safe this value for experiment STEP 3

	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		simulator.clearEvents(this);
		return false;
	}
	of.close();
	log << "trace written to " << tracefile << endl;
	
	// clean up simulator
	simulator.clearEvents();
	return true;
}

#else // !PREREQUISITES
bool EcosKernelTestExperiment::faultInjection() {
	log << "STEP 3: The actual experiment." << endl;

	BPSingleEvent bp;
	
#if !LOCAL
	for (int i = 0; i < 400; ++i) { // more than 400 will be very slow (500 is max)
#endif

	// get an experiment parameter set
	log << "asking job server for experiment parameters" << endl;
	EcosKernelTestExperimentData param;
#if !LOCAL
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
#else
	// XXX debug
	param.msg.set_instr_offset(7462);
	//param.msg.set_instr_address(12345);
	param.msg.set_mem_addr(44540);
#endif

	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int mem_addr = param.msg.mem_addr();

	// for each job we're actually doing *8* experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		// 8 results in one job
		EcosKernelTestProtoMsg_Result *result = param.msg.add_result();
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
		BPSingleEvent func_finish(ECOS_FUNC_FINISH);
		simulator.addEvent(&func_finish);
		bool finish_reached = false;

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(instr_offset);
			simulator.addEvent(&bp);

			// finish() before FI?
			if (simulator.waitAny() == &func_finish) {
				finish_reached = true;
				log << "experiment reached finish() before FI" << endl;

				// wait for bp
				simulator.waitAny();
				//TODO: why wait here? it seems that something went completely wrong?
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
		BPRangeEvent ev_below_text(ANY_ADDR, ECOS_TEXT_START - 1);
		BPRangeEvent ev_beyond_text(ECOS_TEXT_END + 1, ANY_ADDR);
		simulator.addEvent(&ev_below_text);
		simulator.addEvent(&ev_beyond_text);
		// timeout (e.g., stuck in a HLT instruction)
		// 10000us = 500000 instructions
		TimerEvent ev_timeout(500000);
		simulator.addEvent(&ev_timeout);

		// remaining instructions until "normal" ending
		BPSingleEvent ev_end(ANY_ADDR);
		ev_end.setCounter(ECOS_NUMINSTR + ECOS_RECOVERYINSTR - instr_offset);
		simulator.addEvent(&ev_end);
		
		// eCos' test output function, which will show if the test PASSed or FAILed
		BPSingleEvent func_test_output(ECOS_FUNC_TEST_OUTPUT);
		simulator.addEvent(&func_test_output);

#if LOCAL && 0
		// XXX debug
		log << "enabling tracing" << endl;
		TracingPlugin tp;
		tp.setLogIPOnly(true);
		tp.setOstream(&cout);
		// this must be done *after* configuring the plugin:
		simulator.addFlow(&tp);
#endif

		BaseEvent* ev = simulator.waitAny();

		bool ecos_test_passed = false;
		bool ecos_test_failed = false;

		while ( (ev == &func_test_output) || (ev == &func_finish) ) {
			// Do we reach finish() while waiting for ev_trap/ev_done?
			if (ev == &func_finish) {
				finish_reached = true;
				log << "experiment reached finish()" << endl;
			}
			else if(ev == &func_test_output) {
				// 1st argument of cyg_test_output shows what has happened (FAIL or PASS)
				address_t stack_ptr = simulator.getRegisterManager().getStackPointer(); // esp
				int32_t cyg_test_output_argument = simulator.getMemoryManager().getByte(stack_ptr + 4); // 1st argument is at esp+4
				
				log << "cyg_test_output_argument (#1): " << cyg_test_output_argument << endl;
				
				/*
				typedef enum {
					CYGNUM_TEST_FAIL,
					CYGNUM_TEST_PASS,
					CYGNUM_TEST_EXIT,
					CYGNUM_TEST_INFO,
					CYGNUM_TEST_GDBCMD,
					CYGNUM_TEST_NA
				} Cyg_test_code;
				*/				
				
				if (cyg_test_output_argument == 0) {
					ecos_test_failed = true;
				} else if (cyg_test_output_argument == 1) {
					ecos_test_passed = true;
				}
			}

			// wait for ev_trap/ev_done
			ev = simulator.waitAny();
		}

		// record latest IP regardless of result
		result->set_latest_ip(simulator.getRegisterManager().getInstructionPointer());

		// record finish_reached and error_corrected regardless of result
		result->set_finish_reached(finish_reached);
		int32_t error_corrected = simulator.getMemoryManager().getByte(ECOS_ERROR_CORRECTED);
		result->set_error_corrected(error_corrected);
		
		// record ecos_test_result
		if (ecos_test_failed) {
			result->set_ecos_test_result(result->FAIL);
		} else if (ecos_test_passed) {
			result->set_ecos_test_result(result->PASS);
		} else {
			result->set_ecos_test_result(result->FAIL);
		}

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
	} else {
#if !LOCAL
		m_jc.sendResult(param);
#endif
	}

#if !LOCAL
	}
#endif
}
#endif // PREREQUISITES

bool EcosKernelTestExperiment::run()
{
	log << "startup" << endl;

	#if PREREQUISITES
	// step 0
	if(retrieveGuestAddresses()) {
		log << "STEP 0 finished: rebooting ..." << endl;
		simulator.reboot();
	} else { return false; }

	// step 1
	if(establishState()) {
		log << "STEP 1 finished: rebooting ..." << endl;
		simulator.reboot();
	} else { return false; }

	// step 2
	if(performTrace()) {
		log << "STEP 2 finished: terminating ..." << endl;
	} else { return false; }

	#else // !PREREQUISITES
	// step 3
	faultInjection();
	#endif // PREREQUISITES

	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
	return true;
}
