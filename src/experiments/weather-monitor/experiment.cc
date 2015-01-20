#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/CommandLine.hpp"
#include "util/gzstream/gzstream.h"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/bochs/BochsListener.hpp"
#include "sal/Listener.hpp"

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#include "vptr_map.hpp"

#define LOCAL 0

using namespace std;
using namespace fail;
const std::string WeatherMonitorExperiment::dir_images(DIR_IMAGES);
const std::string WeatherMonitorExperiment::dir_prerequisites(DIR_PREREQUISITES);

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

bool WeatherMonitorExperiment::readElfSymbols(
	guest_address_t& entry,
	guest_address_t& text_start,
	guest_address_t& text_end,
	guest_address_t& data_start,
	guest_address_t& data_end,
	guest_address_t& wait_begin,
	guest_address_t& wait_end,
	guest_address_t& vptr_panic)
{
	ElfReader elfreader(filename_elf(m_variant, m_benchmark).c_str());

	entry = elfreader.getSymbol("main").getAddress();
	text_start = elfreader.getSymbol("___TEXT_START__").getAddress();
	text_end = elfreader.getSymbol("___TEXT_END__").getAddress();
	data_start = elfreader.getSymbol("___DATA_START__").getAddress();
	data_end = elfreader.getSymbol("___BSS_END__").getAddress();
	wait_begin = elfreader.getSymbol("wait_begin").getAddress();
	wait_end = elfreader.getSymbol("wait_end").getAddress();

	// vptr_panic only exists in guarded version
	vptr_panic = elfreader.getSymbol("vptr_panic").getAddress();
	// use a dummy address, in case the symbol cannot be found
	if (vptr_panic == ADDR_INV) {
		vptr_panic = 99999999;
	}

	if (entry == ADDR_INV || text_start == ADDR_INV || text_end == ADDR_INV ||
		data_start == ADDR_INV || data_end == ADDR_INV ||
		wait_begin == ADDR_INV || wait_end == ADDR_INV) {
		return false;
	}

	return true;
}

std::string WeatherMonitorExperiment::filename_elf(const std::string& variant, const std::string& benchmark)
{
    if (variant.size() && benchmark.size()) {
        return dir_images + "/" + variant + ".elf";
    }
    return "weather.elf";
}

std::string WeatherMonitorExperiment::filename_state(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + ".state";
	}
	return "state.weather";
}

std::string WeatherMonitorExperiment::filename_trace(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + ".trace";
	}
	return "trace.weather";
}

std::string WeatherMonitorExperiment::filename_traceinfo(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + ".info";
	}
	return "weather.info";
}

bool WeatherMonitorExperiment::writeTraceInfo(unsigned numinstr_tracing, unsigned numinstr_after)
{
	ofstream ti(filename_traceinfo(m_variant, m_benchmark).c_str(), ios::out);
	if (!ti.is_open()) {
		cout << "failed to open " << filename_traceinfo(m_variant, m_benchmark) << endl;
		return false;
	}
	ti << numinstr_tracing << endl << numinstr_after << endl;
	ti.flush();
	ti.close();
	return true;
}

bool WeatherMonitorExperiment::readTraceInfo(unsigned& numinstr_tracing, unsigned& numinstr_after)
{
	ifstream file(filename_traceinfo(m_variant, m_benchmark).c_str());
	if (!file.is_open()) {
		cout << "failed to open " << filename_traceinfo(m_variant, m_benchmark) << endl;
		return false;
	}

	string buf;
	unsigned count = 0;

	while (getline(file, buf)) {
		stringstream ss(buf, ios::in);
		switch (count) {
		case 0:
			ss >> numinstr_tracing;
			break;
		case 1:
			ss >> numinstr_after;
			break;
		}
		count++;
	}
	file.close();
	assert(count == 2);
	return (count == 2);
}


void WeatherMonitorExperiment::parseOptions()
{
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");
	CommandLine::option_handle VARIANT =
		cmd.addOption("", "variant", Arg::Required, "--variant v \texperiment variant");
	CommandLine::option_handle BENCHMARK =
		cmd.addOption("", "benchmark", Arg::Required, "--benchmark b \tbenchmark");

	if (!cmd.parse()) {
		cerr << "Error parsing arguments." << endl;
		simulator.terminate(1);
	} else if (cmd[HELP]) {
		cmd.printUsage();
		simulator.terminate(0);
	}

	if (cmd[VARIANT].count() > 0 && cmd[BENCHMARK].count() > 0) {
		m_variant = std::string(cmd[VARIANT].first()->arg);
		m_benchmark = std::string(cmd[BENCHMARK].first()->arg);
	} else {
		cerr << "Please supply parameters for --variant and --benchmark." << endl;
		simulator.terminate(1);
	}
}

bool WeatherMonitorExperiment::establishState(guest_address_t& entry)
{
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(entry);
	simulator.addListenerAndResume(&bp);
	LOG << "test function entry reached, saving state" << endl;
	LOG << "EIP = " << hex << bp.getTriggerInstructionPointer() << endl;
	simulator.save(filename_state(m_variant, m_benchmark).c_str());
	assert(bp.getTriggerInstructionPointer() == entry);
	assert(simulator.getCPU(0).getInstructionPointer() == entry);

	return true;
}

bool WeatherMonitorExperiment::performTrace(
			guest_address_t& entry,
			guest_address_t& data_start,
			guest_address_t& data_end,
			guest_address_t& wait_end)
{
	// STEP 2: record trace for fault-space pruning
	LOG << "STEP 2 restoring state" << endl;
	simulator.restore(filename_state(m_variant, m_benchmark).c_str());
	LOG << "EIP = " << hex << simulator.getCPU(0).getInstructionPointer() << endl;
	assert(simulator.getCPU(0).getInstructionPointer() == entry);

	LOG << "enabling tracing" << endl;
	TracingPlugin tp;

	// TODO: record max(ESP)

	// restrict memory access logging to injection target
	MemoryMap mm;
	mm.add(data_start, data_end - data_start);
	tp.restrictMemoryAddresses(&mm);
	//tp.setLogIPOnly(true);

	// record trace
	ogzstream of(filename_trace(m_variant, m_benchmark).c_str());
	tp.setTraceFile(&of);

	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

#if 1
	// trace WEATHER_NUMITER_TRACING measurement loop iterations
	// -> calibration
	bp.setWatchInstructionPointer(wait_end);
	bp.setCounter(WEATHER_NUMITER_TRACING);
#else
	// FIXME this doesn't work properly: trace is one instruction too short as
	//       tp is removed before all events were delivered
	// trace WEATHER_NUMINSTR_TRACING instructions
	// -> campaign-ready traces with identical lengths
	bp.setWatchInstructionPointer(ANY_ADDR);
	bp.setCounter(WEATHER_NUMINSTR_TRACING);
#endif
	simulator.addListener(&bp);
	BPSingleListener ev_count(ANY_ADDR);
	simulator.addListener(&ev_count);

	// count instructions
	// FIXME add SAL functionality for this?
	unsigned numinstr_tracing = 0;
	while (simulator.resume() == &ev_count) {
		++numinstr_tracing;
		simulator.addListener(&ev_count);
	}

	LOG << dec << "tracing finished after " << numinstr_tracing
	    << " instructions, seeing wait_end " << WEATHER_NUMITER_TRACING << " times" << endl;
	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		LOG << "failed to write " << filename_trace(m_variant, m_benchmark) << endl;
		simulator.clearListeners(this); // cleanup
		return false;
	}
	of.close();
	LOG << "trace written to " << filename_trace(m_variant, m_benchmark) << endl;

	// wait another WEATHER_NUMITER_AFTER measurement loop iterations
	bp.setWatchInstructionPointer(wait_end);
	bp.setCounter(WEATHER_NUMITER_AFTER);
	simulator.addListener(&bp);

	// count instructions
	// FIXME add SAL functionality for this?
	unsigned numinstr_after = 0;
	while (simulator.resume() == &ev_count) {
		++numinstr_after;
		simulator.addListener(&ev_count);
	}

	LOG << dec << "experiment finished after " << numinstr_after
	    << " instructions, seeing wait_end " << WEATHER_NUMITER_AFTER << " times" << endl;

	if (!writeTraceInfo(numinstr_tracing, numinstr_after)) {
		LOG << "failed to write " << filename_traceinfo(m_variant, m_benchmark) << endl;
		return false;
	}
	return true;
}

bool WeatherMonitorExperiment::faultInjection()
{
#if !LOCAL
	for (int i = 0; i < 50 || (m_jc.getNumberOfUndoneJobs() != 0) ; ++i) { // only do 50 sequential experiments, to prevent swapping
	// 50 exp ~ 0.5GB RAM usage per instance (linearly increasing)
#endif

	// get an experiment parameter set
	LOG << "asking job server for experiment parameters" << endl;
	WeatherMonitorExperimentData param;
#if !LOCAL
	if (!m_jc.getParam(param)) {
		LOG << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
#else
	// XXX debug
	param.msg.fsppilot().set_injection_instr(1000);
	param.msg.fsppilot().set_data_address(0x00103bdc);
#endif

	int id = param.getWorkloadID();
	m_variant = param.msg.fsppilot().variant();
	m_benchmark = param.msg.fsppilot().benchmark();
	unsigned  injection_instr = param.msg.fsppilot().injection_instr();

	/* get symbols from ELF */
	LOG << "retrieving ELF addresses..." << endl;
	guest_address_t entry, text_start, text_end, data_start, data_end, wait_begin, wait_end, vptr_panic;
	if (!readElfSymbols(entry, text_start, text_end, data_start, data_end, wait_begin, wait_end, vptr_panic)) {
		LOG << "failed, essential symbols are missing!" << endl;
		simulator.terminate(1);
	} else {
		LOG << "successfully retrieved ELF's addresses." << endl;
	}

	/* get NUMINSTR_TRACING and NUMINSTR_AFTER */
	unsigned numinstr_tracing, numinstr_after;
	if (!readTraceInfo(numinstr_tracing, numinstr_after)) {
		LOG << "failed to read trace info from " << filename_traceinfo(m_variant, m_benchmark) << endl;
		simulator.terminate(1);
		return false;
	}

	address_t data_address = param.msg.fsppilot().data_address();

	//old data. now it resides in the DatabaseCampaignMessage

	// for each job we're actually doing *8* experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		// 8 results in one job
		WeathermonitorProtoMsg_Result *result = param.msg.add_result();
		result->set_bitoffset(bit_offset);
		LOG << dec << "job " << id << " instr " << injection_instr
		    << " mem " << data_address << "+" << bit_offset << endl;

		LOG << "restoring state" << endl;
		simulator.restore(filename_state(m_variant, m_benchmark).c_str());

		// XXX debug
/*
		stringstream fname;
		fname << "job." << ::getpid();
		ofstream job(fname.str().c_str());
		job << "job " << id << " instr " << injection_instr << " (" << param.msg.fsppilot().injection_instr_absolute() << ") mem " << param.msg.fsppilot().data_address() << "+" << bit_offset << endl;
		job.close();
*/

		// this marks THE END
		BPSingleListener ev_end(ANY_ADDR);
		ev_end.setCounter(numinstr_tracing + numinstr_after);
		simulator.addListener(&ev_end);

		// count loop iterations by counting wait_begin() calls
		// FIXME would be nice to have a callback API for this as this needs to
		//       be done "in parallel"
		BPSingleListener ev_wait_begin(wait_begin);
		simulator.addListener(&ev_wait_begin);
		int count_loop_iter_before = 0;

		// no need to wait if offset is 0
		if (injection_instr > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(injection_instr);
			simulator.addListener(&bp);

			// count loop iterations until FI
			while (simulator.resume() == &ev_wait_begin) {
				++count_loop_iter_before;
				simulator.addListener(&ev_wait_begin);
			}
		}

		// --- fault injection ---
		MemoryManager& mm = simulator.getMemoryManager();
		byte_t data = mm.getByte(data_address);
		byte_t newdata = data ^ (1 << bit_offset);
		mm.setByte(data_address, newdata);
		// note at what IP we did it
		uint32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
		result->set_iter_before_fi(count_loop_iter_before);
		LOG << "fault injected @ ip " << injection_ip
			<< " 0x" << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
		// sanity check
		if (param.msg.fsppilot().has_injection_instr_absolute() &&
			injection_ip != param.msg.fsppilot().injection_instr_absolute()) {
			stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip
			   << " != " << param.msg.fsppilot().injection_instr_absolute();
			LOG << ss.str() << endl;
			result->set_resulttype(result->UNKNOWN);
			result->set_latest_ip(injection_ip);
			result->set_details(ss.str());
			result->set_iter_after_fi(0);

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
		BPRangeListener ev_below_text(ANY_ADDR, text_start - 1);
		BPRangeListener ev_beyond_text(text_end + 1, ANY_ADDR);
		simulator.addListener(&ev_below_text);
		simulator.addListener(&ev_beyond_text);
		// error detected
		BPSingleListener ev_detected(vptr_panic);
		simulator.addListener(&ev_detected);
		// timeout (e.g., stuck in a HLT instruction)
		// 10000us = 500000 instructions
		TimerListener ev_timeout(10000);
		simulator.addListener(&ev_timeout);

#if LOCAL && 0
		// XXX debug
		LOG << "enabling tracing" << endl;
		TracingPlugin tp;
		tp.setLogIPOnly(true);
		tp.setOstream(&cout);
		// this must be done *after* configuring the plugin:
		simulator.addFlow(&tp);
#endif

		BaseListener* ev;

		// count loop iterations
		int count_loop_iter_after = 0;
		while ((ev = simulator.resume()) == &ev_wait_begin) {
			++count_loop_iter_after;
			simulator.addListener(&ev_wait_begin);
		}
		result->set_iter_after_fi(count_loop_iter_after);

		// record latest IP regardless of result
		result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());

		if (ev == &ev_end) {
			LOG << "Result FINISHED (" << dec
			    << count_loop_iter_before << "+" << count_loop_iter_after << ")" << endl;
			result->set_resulttype(result->FINISHED);
		} else if (ev == &ev_timeout) {
			LOG << "Result TIMEOUT (" << dec
			    << count_loop_iter_before << "+" << count_loop_iter_after << ")" << endl;
			result->set_resulttype(result->TIMEOUT);
		} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
			LOG << "Result OUTSIDE" << endl;
			result->set_resulttype(result->OUTSIDE);
		} else if (ev == &ev_trap) {
			LOG << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
			result->set_resulttype(result->TRAP);

			stringstream ss;
			ss << ev_trap.getTriggerNumber();
			result->set_details(ss.str());
		} else if (ev == &ev_detected) {
			LOG << dec << "Result DETECTED" << endl;
			result->set_resulttype(result->DETECTED);
		} else {
			LOG << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "eventid " << ev << " EIP " << simulator.getCPU(0).getInstructionPointer();
			result->set_details(ss.str());
		}
	}
	// sanity check: do we have exactly 8 results?
	if (param.msg.result_size() != 8) {
		LOG << "WTF? param.msg.result_size() != 8" << endl;
	} else {
#if !LOCAL
		m_jc.sendResult(param);
#endif
	}

#if !LOCAL
	}
#endif

	return true;
}

bool WeatherMonitorExperiment::run()
{
	LOG << "startup" << endl;
#if PREREQUISITES
	parseOptions();

	/* get symbols from ELF */
	LOG << "retrieving ELF addresses..." << endl;
	guest_address_t entry, text_start, text_end, data_start, data_end, wait_begin, wait_end, vptr_panic;
	if (!readElfSymbols(entry, text_start, text_end, data_start, data_end, wait_begin, wait_end, vptr_panic)) {
		LOG << "failed, essential symbols are missing!" << endl;
		simulator.terminate(1);
	} else {
		LOG << "successfully retrieved ELF's addresses." << endl;
	}

	//STEP 1
	if (establishState(entry)) {
		LOG << "STEP 1 (establish state) finished." << endl;
	} else {
		LOG << "STEP 1 (establish state) failed!" << endl;
	}

	//STEP 2
	if (performTrace(entry, data_start, data_end, wait_end)) {
		LOG << "STEP 2 (perform trace) finished." << endl;
	} else {
		LOG << "STEP 2 (perform trace) failed!" << endl;
	}

#else // !PREREQUISITES  i.e. STEP 3 "the actual experiment"
	faultInjection();

#endif

	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
