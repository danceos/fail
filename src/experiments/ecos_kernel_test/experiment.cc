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
#include "sal/bochs/BochsListener.hpp"
#include "sal/Listener.hpp"
#include "util/ElfReader.hpp"
#include "util/WallclockTimer.hpp"
#include "util/gzstream/gzstream.h"

// You need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#define LOCAL 0

#ifndef PREREQUISITES
  #define PREREQUISITES 0 // 1: do step 0-2 ; 0: do step 3
#endif

// create/use multiple snapshots to speed up long experiments
// FIXME: doesn't work properly, trace changes! (reason unknown; incorrectly restored serial timers?)
#define MULTIPLE_SNAPSHOTS 0
#define MULTIPLE_SNAPSHOTS_DISTANCE 1000000

#define TIMER_GRANULARITY 10 // microseconds

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

#if PREREQUISITES
bool EcosKernelTestExperiment::retrieveGuestAddresses(guest_address_t addr_finish) {
	log << "STEP 0: record memory map with addresses of 'interesting' objects" << endl;

	// run until func_finish is reached
	BPSingleListener bp;
	bp.setWatchInstructionPointer(addr_finish);

	// memory map serialization
	ofstream mm(EcosKernelTestCampaign::filename_memorymap().c_str(), ios::out);
	if (!mm.is_open()) {
		log << "failed to open " << EcosKernelTestCampaign::filename_memorymap() << endl;
		return false;
	}

	GuestListener g;
	string *str = new string; // buffer for guest listeners' data
	unsigned number_of_guest_events = 0;

	while (simulator.addListenerAndResume(&g) == &g) {
		if (g.getData() == '\t') {
			// addr complete?
			//cout << "full: " << *str << "sub: " << str->substr(str->find_last_of('x') - 1) << endl;
			// interpret the string obtained by the guest listeners as address in hex
			unsigned guest_addr;
			stringstream converter(str->substr(str->find_last_of('x') + 1));
			converter >> hex >> guest_addr;
			mm << guest_addr << '\t';
			str->clear();
		} else if (g.getData() == '\n') {
			// len complete?
			// interpret the string obtained by the guest listeners as length in decimal
			unsigned guest_len;
			stringstream converter(*str);
			converter >> dec >> guest_len;
			mm << guest_len << '\n';
			str->clear();
			number_of_guest_events++;
		} else if (g.getData() == 'Q') {
			// when the guest system triggers the guest event 'Q',
			// we can assume that we are in protected mode
			simulator.addListener(&bp);
		} else {
			str->push_back(g.getData());
		}
	}
	assert(number_of_guest_events > 0);
	log << "Breakpoint at func_finish reached: created memory map (" << number_of_guest_events << " entries)" << endl;
	delete str;

	// close serialized mm
	mm.close();

	return true;
}

bool EcosKernelTestExperiment::establishState(guest_address_t addr_entry, guest_address_t addr_finish, guest_address_t addr_errors_corrected) {
	log << "STEP 1: run until interesting function starts, and save state" << endl;

	GuestListener g;

	while (true) {
		simulator.addListenerAndResume(&g);
		if(g.getData() == 'Q') {
		  log << "Guest system triggered: " << g.getData() << endl;
		  break;
		}
	}

	BPSingleListener bp;
	bp.setWatchInstructionPointer(addr_entry);
	simulator.addListenerAndResume(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << endl;
	//log << "error_corrected = " << dec << ((int)simulator.getMemoryManager().getByte(addr_errors_corrected)) << endl;

	// run until 'ECOS_FUNC_FINISH' is reached
	BPSingleListener finish;
	finish.setWatchInstructionPointer(addr_finish);

	// one save every MULTIPLE_SNAPSHOTS_DISTANCE instructions
	BPSingleListener step;
	step.setWatchInstructionPointer(ANY_ADDR);
	step.setCounter(MULTIPLE_SNAPSHOTS_DISTANCE);

	for (unsigned i = 0; ; ++i) {
		log << "saving state at offset " << dec << (i * MULTIPLE_SNAPSHOTS_DISTANCE) << endl;
		simulator.save(EcosKernelTestCampaign::filename_state(i * MULTIPLE_SNAPSHOTS_DISTANCE));
#if MULTIPLE_SNAPSHOTS
		simulator.restore(EcosKernelTestCampaign::filename_state(i * MULTIPLE_SNAPSHOTS_DISTANCE));

		simulator.addListener(&step);
		simulator.addListener(&finish);

		if (simulator.resume() == &finish) {
			break;
		}
#else
		break;
#endif
	}

	return true;
}

bool EcosKernelTestExperiment::performTrace(guest_address_t addr_entry, guest_address_t addr_finish) {
	log << "STEP 2: record trace for fault-space pruning" << endl;

	log << "restoring state" << endl;
	simulator.restore(EcosKernelTestCampaign::filename_state(0));
	log << "EIP = " << hex << simulator.getRegisterManager().getInstructionPointer() << endl;
	assert(simulator.getRegisterManager().getInstructionPointer() == addr_entry);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	EcosKernelTestCampaign::readMemoryMap(mm, EcosKernelTestCampaign::filename_memorymap().c_str());

	tp.restrictMemoryAddresses(&mm);

	// record trace
	ogzstream of(EcosKernelTestCampaign::filename_trace().c_str());
	tp.setTraceFile(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// again, run until 'ECOS_FUNC_FINISH' is reached
	BPSingleListener bp;
	bp.setWatchInstructionPointer(addr_finish);
	simulator.addListener(&bp);

	// on the way, count instructions // FIXME add SAL functionality for this?
	BPSingleListener ev_count(ANY_ADDR);
	simulator.addListener(&ev_count);
	unsigned instr_counter = 0;

	// on the way, count elapsed time
	TimerListener time_step(TIMER_GRANULARITY); //TODO: granularity?
	//elapsed_time.setCounter(0xFFFFFFFFU); // not working for TimerListener
	simulator.addListener(&time_step);
	unsigned elapsed_time = 1; // always run 1 step
	// just increase elapsed_time counter by 1, which serves as time for ECC recovery algorithm
	++elapsed_time; // (this is a rough guess ... TODO)

	// on the way, record lowest and highest memory address accessed
	MemAccessListener ev_mem(ANY_ADDR, MemAccessEvent::MEM_READWRITE);
	simulator.addListener(&ev_mem);
	unsigned lowest_addr = 0xFFFFFFFFUL;
	unsigned highest_addr = 0;

	// do the job, 'till the end
	BaseListener* ev = simulator.resume();
	while(ev != &bp) {
		if(ev == &ev_count) {
			if(instr_counter++ == 0xFFFFFFFFU) {
				log << "ERROR: instr_counter overflowed" << endl;
				return false;
			}
			simulator.addListener(&ev_count);
		}
		else if(ev == &time_step) {
			if(elapsed_time++ == 0xFFFFFFFFU) {
				log << "ERROR: elapsed_time overflowed" << endl;
				return false;
			}
			simulator.addListener(&time_step);
		}
		else if(ev == &ev_mem) {
			unsigned lo = ev_mem.getTriggerAddress();
			unsigned hi = lo + ev_mem.getTriggerWidth() - 1;

			if(hi > highest_addr) {
				highest_addr = hi;
			}
			if(lo < lowest_addr) {
				lowest_addr = lo;
			}
			simulator.addListener(&ev_mem);
		}
		ev = simulator.resume();
	}

	unsigned long long estimated_timeout_overflow_check = ((unsigned long long)elapsed_time) * time_step.getTimeout();
	if(estimated_timeout_overflow_check > 0xFFFFFFFFU) {
		log << "Timeout estimation overflowed" << endl;
		return false;
	}
	unsigned estimated_timeout = (unsigned)estimated_timeout_overflow_check;

	log << dec << "tracing finished after " << instr_counter  << " instructions" << endl;
	log << hex << "all memory accesses within [ 0x" << lowest_addr << " , 0x" << highest_addr << " ]" << endl;
	log << dec << "elapsed simulated time (plus safety margin): " << (estimated_timeout * TIMER_GRANULARITY / 1000000.0) << "s" << endl;

	// save these values for experiment STEP 3
	EcosKernelTestCampaign::writeTraceInfo(instr_counter, estimated_timeout, lowest_addr, highest_addr);

	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write " << EcosKernelTestCampaign::filename_trace() << endl;
		return false;
	}
	of.close();
	log << "trace written to " << EcosKernelTestCampaign::filename_trace() << endl;
	
	return true;
}

#else // !PREREQUISITES
bool EcosKernelTestExperiment::faultInjection() {
	log << "STEP 3: The actual experiment." << endl;

	// trace info
	unsigned instr_counter, estimated_timeout, lowest_addr, highest_addr;
	// ELF symbol addresses
	guest_address_t addr_entry, addr_finish, addr_test_output, addr_errors_corrected,
	                addr_panic, addr_text_start, addr_text_end;

	BPSingleListener bp;
	
#if !LOCAL
	for (int i = 0; i < 50; ++i) { // only do 50 sequential experiments, to prevent swapping
	// 50 exp ~ 0.5GB RAM usage per instance (linearly increasing)
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
	param.msg.set_variant("bitmap_CRC");
	param.msg.set_benchmark("bin_sem0");
	param.msg.set_instr2_offset(7462);
	//param.msg.set_instr_address(12345);
	param.msg.set_mem_addr(44540);
#endif

	WallclockTimer timer;
	timer.startTimer();

	int id = param.getWorkloadID();
	m_variant = param.msg.variant();
	m_benchmark = param.msg.benchmark();
	int instr_offset = param.msg.instr2_offset();
	int mem_addr = param.msg.mem_addr();

	EcosKernelTestCampaign::readTraceInfo(instr_counter, estimated_timeout,
		lowest_addr, highest_addr, m_variant, m_benchmark);
	readELFSymbols(addr_entry, addr_finish, addr_test_output,
		addr_errors_corrected, addr_panic, addr_text_start, addr_text_end);

	int state_instr_offset = instr_offset - (instr_offset % MULTIPLE_SNAPSHOTS_DISTANCE);
	string statename;
#if MULTIPLE_SNAPSHOTS
	if (access(EcosKernelTestCampaign::filename_state(state_instr_offset, m_variant, m_benchmark).c_str(), R_OK) == 0) {
		statename = EcosKernelTestCampaign::filename_state(state_instr_offset, m_variant, m_benchmark);
		log << "using state at offset " << state_instr_offset << endl;
		instr_offset -= state_instr_offset;
	} else { // fallback
#endif
		statename = EcosKernelTestCampaign::filename_state(0, m_variant, m_benchmark);
		state_instr_offset = 0;
		log << "using state at offset 0 (fallback)" << endl;
#if MULTIPLE_SNAPSHOTS
	}
#endif

	// for each job we're actually doing *8* experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		// 8 results in one job
		EcosKernelTestProtoMsg_Result *result = param.msg.add_result();
		result->set_bit_offset(bit_offset);
		log << dec << "job " << id << " " << m_variant << "/" << m_benchmark
		    << " instr " << (instr_offset + state_instr_offset)
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
		BPSingleListener func_finish(addr_finish);
		simulator.addListener(&func_finish);

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(instr_offset);
			simulator.addListener(&bp);

			// finish() before FI?
			if (simulator.resume() == &func_finish) {
				log << "experiment reached finish() before FI" << endl;

				// wait for bp
				simulator.resume();
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
		if (param.msg.has_instr2_address() &&
			injection_ip != param.msg.instr2_address()) {
			stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip
			   << " != " << param.msg.instr2_address();
			log << ss.str() << endl;
			result->set_resulttype(result->UNKNOWN);
			result->set_latest_ip(injection_ip);
			result->set_ecos_test_result(result->FAIL);
			result->set_details(ss.str());

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
		BPRangeListener ev_below_text(ANY_ADDR, addr_text_start - 1);
		BPRangeListener ev_beyond_text(addr_text_end + 1, ANY_ADDR);
		simulator.addListener(&ev_below_text);
		simulator.addListener(&ev_beyond_text);

		// memory access outside of bound determined in the golden run [lowest_addr, highest_addr]
		MemAccessListener ev_mem_low(0x0, MemAccessEvent::MEM_READWRITE);
		ev_mem_low.setWatchWidth(lowest_addr);
		MemAccessListener ev_mem_high(highest_addr + 1, MemAccessEvent::MEM_READWRITE);
		ev_mem_high.setWatchWidth(0xFFFFFFFFU - (highest_addr + 1));
		simulator.addListener(&ev_mem_low);
		simulator.addListener(&ev_mem_high);

		// timeout (e.g., stuck in a HLT instruction)
		TimerListener ev_timeout(estimated_timeout);
		simulator.addListener(&ev_timeout);

		// remaining instructions until "normal" ending
		// number of instructions that are executed additionally for error corrections
		//BPSingleListener ev_end(ANY_ADDR);
		//ev_end.setCounter(instr_counter - instr_offset + ECOS_RECOVERYINSTR);
		//simulator.addListener(&ev_end);
		
		// eCos' test output function, which will show if the test PASSed or FAILed
		BPSingleListener func_test_output(addr_test_output);
		simulator.addListener(&func_test_output);

		// function called by ecc aspects, when an uncorrectable error is detected
		BPSingleListener func_ecc_panic(addr_panic);
		if (addr_panic != ADDR_INV) {
			simulator.addListener(&func_ecc_panic);
		}

#if LOCAL && 0
		// XXX debug
		log << "enabling tracing" << endl;
		TracingPlugin tp;
		tp.setLogIPOnly(true);
		tp.setOstream(&cout);
		// this must be done *after* configuring the plugin:
		simulator.addFlow(&tp);
#endif

		// the outcome of ecos' test case
		bool ecos_test_passed = false;
		bool ecos_test_failed = false;

		BaseListener* ev;

		// wait until experiment-terminating event occurs
		while (true) {
			ev = simulator.resume();
			if (ev == &func_test_output) {
				// re-add this listener
				simulator.addListener(&func_test_output);

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

			// special case: except1 and clockcnv actively generate traps
			} else if (ev == &ev_trap
			        && ((m_benchmark == "except1" && ev_trap.getTriggerNumber() == 13)
					 || (m_benchmark == "clockcnv" && ev_trap.getTriggerNumber() == 7))) {
				// re-add this listener
				simulator.addListener(&ev_trap);
			} else {
				// in any other case, the experiment is finished
				break;
			}
		}

		// record latest IP regardless of result
		result->set_latest_ip(simulator.getRegisterManager().getInstructionPointer());

		// record error_corrected regardless of result
		if (addr_errors_corrected != ADDR_INV) {
			int32_t error_corrected = simulator.getMemoryManager().getByte(addr_errors_corrected);
			result->set_error_corrected(error_corrected);
		} else {
			result->set_error_corrected(0);
		}
		
		// record ecos_test_result
		if ( (ecos_test_passed == true) && (ecos_test_failed == false) ) {
			result->set_ecos_test_result(result->PASS);
			log << "Ecos Test PASS" << endl;
		} else {
			result->set_ecos_test_result(result->FAIL);
			log << "Ecos Test FAIL" << endl;
		}

		if (ev == &func_finish) {
			// do we reach finish?
			log << "experiment finished ordinarily" << endl;
			result->set_resulttype(result->FINISHED);
		} else if (ev == &ev_timeout /*|| ev == &ev_end*/) {
			log << "Result TIMEOUT" << endl;
			result->set_resulttype(result->TIMEOUT);
		} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
			log << "Result OUTSIDE" << endl;
			result->set_resulttype(result->OUTSIDE);
		} else if (ev == &ev_mem_low || ev == &ev_mem_high) {
			log << "Result MEMORYACCESS" << endl;
			result->set_resulttype(result->MEMORYACCESS);
		} else if (ev == &ev_trap) {
			log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
			result->set_resulttype(result->TRAP);

			stringstream ss;
			ss << ev_trap.getTriggerNumber();
			result->set_details(ss.str());
		} else if (ev == &func_ecc_panic) {
			log << "ECC Panic: uncorrectable error" << endl;
			result->set_resulttype(result->DETECTED); // DETECTED <=> ECC_PANIC <=> reboot
		} else {
			log << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "event addr " << ev << " EIP " << simulator.getRegisterManager().getInstructionPointer();
			result->set_details(ss.str());
		}
	}
	// sanity check: do we have exactly 8 results?
	if (param.msg.result_size() != 8) {
		log << "WTF? param.msg.result_size() != 8" << endl;
	} else {
		param.msg.set_runtime(timer);
#if !LOCAL
		m_jc.sendResult(param);
#endif
	}

#if !LOCAL
	}
#endif
	return true;
}
#endif // PREREQUISITES

bool EcosKernelTestExperiment::readELFSymbols(
	fail::guest_address_t& entry,
	fail::guest_address_t& finish,
	fail::guest_address_t& test_output,
	fail::guest_address_t& errors_corrected,
	fail::guest_address_t& panic,
	fail::guest_address_t& text_start,
	fail::guest_address_t& text_end)
{
	ElfReader elfreader(EcosKernelTestCampaign::filename_elf(m_variant, m_benchmark).c_str());
	entry            = elfreader.getAddressByName("cyg_start");
	finish           = elfreader.getAddressByName("cyg_test_exit");
	test_output      = elfreader.getAddressByName("cyg_test_output");
	errors_corrected = elfreader.getAddressByName("errors_corrected");
	panic            = elfreader.getAddressByName("_Z9ecc_panicv");
	text_start       = elfreader.getAddressByName("_stext");
	text_end         = elfreader.getAddressByName("_etext");

	// it's OK if errors_corrected or ecc_panic are missing
	if (entry == ADDR_INV || finish == ADDR_INV || test_output == ADDR_INV ||
	    text_start == ADDR_INV || text_end == ADDR_INV) {
		return false;
	}
	return true;
}

bool EcosKernelTestExperiment::run()
{
	log << "startup" << endl;

	#if PREREQUISITES
	log << "retrieving ELF symbol addresses ..." << endl;
	guest_address_t entry, finish, test_output, errors_corrected,
	                panic, text_start, text_end;
	if (!readELFSymbols(entry, finish, test_output, errors_corrected,
	               panic, text_start, text_end)) {
		log << "failed, essential symbols are missing!" << endl;
		simulator.terminate(1);
	}

	// step 0
	if(retrieveGuestAddresses(finish)) {
		log << "STEP 0 finished: rebooting ..." << endl;
		simulator.reboot();
	} else { return false; }

	// step 1
	if(establishState(entry, finish, errors_corrected)) {
		log << "STEP 1 finished: proceeding ..." << endl;
	} else { return false; }

	// step 2
	if(performTrace(entry, finish)) {
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
