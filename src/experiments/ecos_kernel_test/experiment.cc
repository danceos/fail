#include <iostream>
#include <fstream>
#include <algorithm>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "util/ElfReader.hpp"
#include "util/WallclockTimer.hpp"
#include "util/gzstream/gzstream.h"
#include "config/FailConfig.hpp"
#include "util/CommandLine.hpp"

// You need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"
// You need to have the serialoutput plugin enabled for this
#include "../plugins/serialoutput/SerialOutputLogger.hpp"

// for local experiment debugging: don't contact job server, use hard-coded parameters
#define LOCAL 0

#ifndef PREREQUISITES
	#error Configure experimentInfo.hpp properly!
#endif

// create/use multiple snapshots to speed up long experiments
// FIXME: doesn't work properly, trace changes! (reason unknown; incorrectly restored serial timers?)
#define MULTIPLE_SNAPSHOTS 0
#define MULTIPLE_SNAPSHOTS_DISTANCE 1000000

#define VIDEOMEM_START 0xb8000
#define VIDEOMEM_SIZE  (80*25*2 *2) // two text mode screens
#define VIDEOMEM_END   (VIDEOMEM_START + VIDEOMEM_SIZE)

#define LIMIT_SERIAL	1024*1024
#define LIMIT_MEMOUT	1024*1024*10

using namespace std;
using namespace fail;

const std::string EcosKernelTestExperiment::dir_images(DIR_IMAGES);
const std::string EcosKernelTestExperiment::dir_prerequisites(DIR_PREREQUISITES);

bool EcosKernelTestExperiment::writeTraceInfo(unsigned instr_counter, unsigned long long runtime,
	unsigned mem1_low, unsigned mem1_high, // < 1M
	unsigned mem2_low, unsigned mem2_high, // >= 1M
	const std::string& variant, const std::string& benchmark) {
	ofstream ti(filename_traceinfo(variant, benchmark).c_str(), ios::out);
	if (!ti.is_open()) {
		cout << "failed to open " << filename_traceinfo(variant, benchmark) << endl;
		return false;
	}
	ti << instr_counter << endl << runtime << endl
	   << mem1_low << endl << mem1_high << endl
	   << mem2_low << endl << mem2_high << endl;
	ti.flush();
	ti.close();
	return true;
}

bool EcosKernelTestExperiment::readTraceInfo(unsigned &instr_counter, unsigned long long &runtime,
	unsigned &mem1_low, unsigned &mem1_high, // < 1M
	unsigned &mem2_low, unsigned &mem2_high, // >= 1M
	const std::string& variant, const std::string& benchmark) {
	ifstream file(filename_traceinfo(variant, benchmark).c_str());
	if (!file.is_open()) {
		cout << "failed to open " << filename_traceinfo(variant, benchmark) << endl;
		return false;
	}

	string buf;
	unsigned count = 0;

	while (getline(file, buf)) {
		stringstream ss(buf, ios::in);
		switch (count) {
		case 0:
			ss >> instr_counter;
			break;
		case 1:
			ss >> runtime;
			break;
		case 2:
			ss >> mem1_low;
			break;
		case 3:
			ss >> mem1_high;
			break;
		case 4:
			ss >> mem2_low;
			break;
		case 5:
			ss >> mem2_high;
			break;
		}
		count++;
	}
	file.close();
	assert(count == 6);
	return (count == 6);
}

std::string EcosKernelTestExperiment::filename_memorymap(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "memorymap.txt";
	}
	return "memorymap.txt";
}

std::string EcosKernelTestExperiment::filename_state(unsigned instr_offset, const std::string& variant, const std::string& benchmark)
{
	stringstream ss;
	ss << instr_offset;
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "state" + "-" + ss.str();
	}
	return "state-" + ss.str();
}

std::string EcosKernelTestExperiment::filename_trace(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "trace.tc";
	}
	return "trace.tc";
}

std::string EcosKernelTestExperiment::filename_traceinfo(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "traceinfo.txt";
	}
	return "traceinfo.txt";
}

std::string EcosKernelTestExperiment::filename_elf(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_images + "/" + variant + "/" + benchmark + ".elf";
	}
	return "kernel.elf";
}

std::string EcosKernelTestExperiment::filename_serial(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + ".serial";
	}
	return "serial";
}

std::string EcosKernelTestExperiment::filename_memout(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + ".memout";
	}
	return "memout";
}

std::vector<char> EcosKernelTestExperiment::loadFile(std::string filename)
{
	std::vector<char> data;
	FILE *f = fopen(filename.c_str(), "rb");
	if (!f) {
		return data;
	}
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (len > 0) {
		data.resize(len);
		fread(&data[0], len, 1, f);
	}
	fclose(f);
	return data;
}

// TODO integrate this in MemoryManager?
bool EcosKernelTestExperiment::isMapped(fail::MemoryManager& mm, guest_address_t start, unsigned len)
{
	const int granularity = 4096;
	for (guest_address_t a = start & ~(granularity - 1); a < start + len; a += granularity) {
		if (!mm.isMapped(a)) {
			return false;
		}
	}
	return true;
}

std::vector<char> EcosKernelTestExperiment::readFromMem(guest_address_t addr_testdata, guest_address_t addr_testdata_size)
{
	std::vector<char> data;
	if (addr_testdata == ADDR_INV || addr_testdata_size == ADDR_INV) {
		return data;
	}
	MemoryManager& mm = simulator.getMemoryManager();
	uint32_t size;
	if (!isMapped(mm, addr_testdata_size, sizeof(size))) {
		log << "testdata_size isn't mapped" << endl;
		return data;
	}
	mm.getBytes(addr_testdata_size, sizeof(size), &size);
	if (size > LIMIT_MEMOUT) {
		log << "truncated in-memory buffer from " << size << " to " << LIMIT_MEMOUT << endl;
		size = LIMIT_MEMOUT;
	}
	uint32_t real_addr_testdata;
	if (!isMapped(mm, addr_testdata, sizeof(real_addr_testdata))) {
		log << "testdata isn't mapped" << endl;
		return data;
	}
	mm.getBytes(addr_testdata, sizeof(real_addr_testdata), &real_addr_testdata);
	if (isMapped(mm, real_addr_testdata, size)) {
		data.resize(size);
		mm.getBytes(real_addr_testdata, size, &data.front());
	} else {
		log << "testdata array isn't mapped" << endl;
	}
	return data;
}

#if PREREQUISITES
bool EcosKernelTestExperiment::retrieveGuestAddresses(guest_address_t addr_finish, guest_address_t addr_data_start, guest_address_t addr_data_end) {
	log << "STEP 0: creating memory map spanning all of DATA and BSS" << endl;
	MemoryMap mm;
	mm.add(addr_data_start, addr_data_end - addr_data_start);
	mm.writeToFile(filename_memorymap(m_variant, m_benchmark).c_str());

	return true;
}

bool EcosKernelTestExperiment::establishState(guest_address_t addr_entry, guest_address_t addr_finish, guest_address_t addr_errors_corrected) {
	log << "STEP 1: run until interesting function starts, and save state" << endl;

	GuestListener g;

	while (true) {
		simulator.addListenerAndResume(&g);
		if (g.getData() == 'Q') {
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
		if (!simulator.save(filename_state(i * MULTIPLE_SNAPSHOTS_DISTANCE, m_variant, m_benchmark))) {
			log << "state save failed!" << endl;
			simulator.terminate(1);
		}

#if MULTIPLE_SNAPSHOTS
		simulator.restore(filename_state(i * MULTIPLE_SNAPSHOTS_DISTANCE, m_variant, m_benchmark));

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

bool EcosKernelTestExperiment::performTrace(guest_address_t addr_entry, guest_address_t addr_finish,
	guest_address_t addr_testdata, guest_address_t addr_testdata_size) {
	log << "STEP 2: record trace for fault-space pruning" << endl;

	log << "restoring state" << endl;
	simulator.restore(filename_state(0, m_variant, m_benchmark));
	log << "EIP = " << hex << simulator.getCPU(0).getInstructionPointer() << endl;
	assert(simulator.getCPU(0).getInstructionPointer() == addr_entry);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mmap;
	mmap.readFromFile(filename_memorymap(m_variant, m_benchmark).c_str());

	tp.restrictMemoryAddresses(&mmap);
#if RECORD_FULL_TRACE
	tp.setFullTrace(true);
#endif

	// record trace
	ogzstream of(filename_trace(m_variant, m_benchmark).c_str());
	tp.setTraceFile(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// record serial output
	SerialOutputLogger sol(0x3f8, LIMIT_SERIAL);
	simulator.addFlow(&sol);

	// again, run until 'ECOS_FUNC_FINISH' is reached
	BPSingleListener bp;
	bp.setWatchInstructionPointer(addr_finish);
	simulator.addListener(&bp);

	// on the way, count instructions // FIXME add SAL functionality for this?
	BPSingleListener ev_count(ANY_ADDR);
	simulator.addListener(&ev_count);
	unsigned instr_counter = 0;

	// measure elapsed time
	simtime_t time_start = simulator.getTimerTicks();

	// on the way, record lowest and highest memory address accessed
	MemAccessListener ev_mem(ANY_ADDR, MemAccessEvent::MEM_READWRITE);
	simulator.addListener(&ev_mem);
	// range for mem accesses < 1M
	unsigned mem1_low = 0xFFFFFFFFUL;
	unsigned mem1_high = 0;
	// range for mem accesses >= 1M
	unsigned mem2_low = 0xFFFFFFFFUL;
	unsigned mem2_high = 0;

	// do the job, 'till the end
	BaseListener* ev = simulator.resume();
	while (ev != &bp) {
		if (ev == &ev_count) {
			if (instr_counter++ == 0xFFFFFFFFU) {
				log << "ERROR: instr_counter overflowed" << endl;
				return false;
			}
			simulator.addListener(&ev_count);
		} else if (ev == &ev_mem) {
			unsigned lo = ev_mem.getTriggerAddress();
			unsigned hi = lo + ev_mem.getTriggerWidth() - 1;

			if (lo < VIDEOMEM_START || lo >= VIDEOMEM_END) {
				if (hi < 1024*1024) { // < 1M
					if (hi > mem1_high) { mem1_high = hi; }
					if (lo < mem1_low)  { mem1_low = lo; }
				} else { // >= 1M
					if (hi > mem2_high) { mem2_high = hi; }
					if (lo < mem2_low)  { mem2_low = lo; }
				}
			}
			simulator.addListener(&ev_mem);
		}
		ev = simulator.resume();
	}

	unsigned long long goldenrun_runtime_ticks = simulator.getTimerTicks() - time_start;
	// convert to microseconds
	unsigned goldenrun_runtime = (unsigned)
		(goldenrun_runtime_ticks * 1000000.0 / simulator.getTimerTicksPerSecond());

	log << dec << "tracing finished after " << instr_counter  << " instructions" << endl;
	log << hex << "all memory accesses within [0x" << mem1_low << ", 0x" << mem1_high << "] u [0x" << mem2_low << ", 0x" << mem2_high << "] (ignoring VGA mem)" << endl;
	log << dec << "elapsed simulated time: "
		<< (goldenrun_runtime / 1000000.0) << "s ("
		<< goldenrun_runtime_ticks << " ticks)" << endl;

	// sanitize memory ranges
	if (mem1_low > mem1_high) {
		mem1_low = mem1_high = 0;
	}
	if (mem2_low > mem2_high) {
		mem2_low = mem2_high = 1024*1024;
	}

	// save these values for experiment STEP 3
	writeTraceInfo(instr_counter, goldenrun_runtime_ticks,
		mem1_low, mem1_high, mem2_low, mem2_high, m_variant, m_benchmark);

	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write " << filename_trace(m_variant, m_benchmark) << endl;
		return false;
	}
	of.close();
	log << "trace written to " << filename_trace(m_variant, m_benchmark) << endl;

	simulator.removeFlow(&sol);
	ofstream of_serial(filename_serial(m_variant, m_benchmark).c_str(), ios::out|ios::binary);
	if (!of_serial.fail()) {
		of_serial << sol.getOutput();
	} else {
		log << "failed to write " << filename_serial(m_variant, m_benchmark) << endl;
	}

	std::vector<char> memout = readFromMem(addr_testdata, addr_testdata_size);
	if (memout.size() > 0) {
		ofstream of_memout(filename_memout(m_variant, m_benchmark).c_str(), ios::out|ios::binary);
		of_memout.write(&memout.front(), memout.size());
	}

	return true;
}

#else // !PREREQUISITES
void EcosKernelTestExperiment::handle_func_test_output(bool &test_failed, bool& test_passed)
{
	// 1st argument of cyg_test_output shows what has happened (FAIL or PASS)
	address_t stack_ptr = simulator.getCPU(0).getStackPointer(); // esp
	MemoryManager& mm = simulator.getMemoryManager();
	if (!mm.isMapped(stack_ptr + 4)) {
		return;
	}
	int32_t cyg_test_output_argument = mm.getByte(stack_ptr + 4); // 1st argument is at esp+4

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
		test_failed = true;
	} else if (cyg_test_output_argument == 1) {
		test_passed = true;
	}
}

bool EcosKernelTestExperiment::faultInjection() {
	log << "STEP 3: The actual experiment." << endl;

	// trace info
	unsigned goldenrun_instr_counter, mem1_low, mem1_high, mem2_low, mem2_high;
	unsigned long long goldenrun_runtime_ticks;
	unsigned goldenrun_runtime, timeout_runtime;
	// ELF symbol addresses
	guest_address_t addr_entry, addr_finish, addr_testdata, addr_testdata_size,
		addr_test_output, addr_errors_corrected,
		addr_panic, addr_text_start, addr_text_end,
		addr_data_start, addr_data_end;

	BPSingleListener bp;

	int experiments = 0;
#if !LOCAL
	// TODO: measure these numbers again!
	// stop after ~500 experiments to prevent swapping
	// 50 exp ~ 0.5GB RAM usage per instance (linearly increasing)
	for (experiments = 0;
		experiments < 500 || (m_jc.getNumberOfUndoneJobs() != 0); ) {
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
	param.msg.mutable_fsppilot()->set_injection_instr(7462);
	//param.msg.mutable_fsppilot()->set_injection_instr_absolute(12345);
	param.msg.mutable_fsppilot()->set_data_address(44540);
	param.msg.mutable_fsppilot()->set_data_width(1);
	param.msg.mutable_fsppilot()->set_variant(m_variant);
	param.msg.mutable_fsppilot()->set_benchmark(m_benchmark);
#endif

	if (param.msg.fsppilot().data_width() != 1) {
		log << "cannot deal with data_width = " << param.msg.fsppilot().data_width() << endl;
		simulator.terminate(1);
	}

	int id = param.getWorkloadID();
	m_variant = param.msg.fsppilot().variant();
	m_benchmark = param.msg.fsppilot().benchmark();
	unsigned instr_offset = param.msg.fsppilot().injection_instr();
	unsigned mem_addr = param.msg.fsppilot().data_address();

	readTraceInfo(goldenrun_instr_counter, goldenrun_runtime_ticks,
		mem1_low, mem1_high, mem2_low, mem2_high, m_variant, m_benchmark);

	if (!readELFSymbols(addr_entry, addr_finish,
		addr_testdata, addr_testdata_size, addr_test_output,
		addr_errors_corrected, addr_panic, addr_text_start, addr_text_end,
		addr_data_start, addr_data_end)) {
		log << "retrieving ELF symbol addresses failed, essential symbols are missing!" << endl;
		simulator.terminate(1);
	}

	int state_instr_offset = instr_offset - (instr_offset % MULTIPLE_SNAPSHOTS_DISTANCE);
	string statename;
#if MULTIPLE_SNAPSHOTS
	if (access(filename_state(state_instr_offset, m_variant, m_benchmark).c_str(), R_OK) == 0) {
		statename = filename_state(state_instr_offset, m_variant, m_benchmark);
		log << "using state at offset " << state_instr_offset << endl;
		instr_offset -= state_instr_offset;
	} else { // fallback
#endif
		statename = filename_state(0, m_variant, m_benchmark);
		state_instr_offset = 0;
		log << "using state at offset 0 (fallback)" << endl;
#if MULTIPLE_SNAPSHOTS
	}
#endif

	// for each job with the SINGLEBITFLIP fault model we're actually doing *8*
	// experiments (one for each bit)
	for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
		++experiments;

		// TODO timing measurement should be part of the
		// DatabaseCampaignMessage
		WallclockTimer timer;
		timer.startTimer();

		// 8 results in one job
		EcosKernelTestProtoMsg_Result *result = param.msg.add_result();
		result->set_bit_offset(bit_offset);
		log << dec << "job " << id << " " << m_variant << "/" << m_benchmark
		    << " instr " << (instr_offset + state_instr_offset)
		    << " mem " << mem_addr << "+" << bit_offset << endl;

		log << "restoring state" << endl;
		simulator.restore(statename);

		// convert to microseconds (simulator.getTimerTicksPerSecond() only
		// works reliably when simulation has begun)
		goldenrun_runtime = (unsigned)
			(goldenrun_runtime_ticks * 1000000.0 / simulator.getTimerTicksPerSecond());
		timeout_runtime = goldenrun_runtime + 1000000/18.2; // + 1 timer tick

		// the outcome of ecos' test case
		bool ecos_test_passed = false;
		bool ecos_test_failed = false;

		// reaching finish() could happen before OR after FI
		BPSingleListener func_finish(addr_finish);
		simulator.addListener(&func_finish);
		bool reached_finish = false;

		// reaching cyg_test_output() could happen before OR after FI
		// eCos' test output function, which will show if the test PASSed or FAILed
		BPSingleListener func_test_output(addr_test_output);
		if (addr_test_output != ADDR_INV) {
			simulator.addListener(&func_test_output);
		}

		// record serial output
		SerialOutputLogger sol(0x3f8, LIMIT_SERIAL);
		simulator.addFlow(&sol);

		// measure elapsed time
		simtime_t time_start = simulator.getTimerTicks();

		BaseListener* ev;

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			bp.setWatchInstructionPointer(ANY_ADDR);
			bp.setCounter(instr_offset);
			simulator.addListener(&bp);

			while (true) {
				ev = simulator.resume();
				if (ev == &func_test_output) {
					// re-add this listener
					simulator.addListener(&func_test_output);
					handle_func_test_output(ecos_test_failed, ecos_test_passed);
				} else if (ev == &func_finish) {
					log << "experiment reached finish() before FI" << endl;
					reached_finish = true;
					break;
				} else {
					break;
				}
			}
		}

		// --- fault injection ---
		MemoryManager& mm = simulator.getMemoryManager();
		byte_t data = mm.getByte(mem_addr);
		byte_t newdata;
#if ECOS_FAULTMODEL_BURST
		newdata = data ^ 0xff;
		bit_offset = 8; // enforce loop termination
#else
		newdata = data ^ (1 << bit_offset);
#endif
		mm.setByte(mem_addr, newdata);
		// note at what IP we did it
		uint32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
		log << "fault injected @ ip " << injection_ip
			<< " 0x" << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
		// sanity check
		if (param.msg.fsppilot().has_injection_instr_absolute() &&
			injection_ip != param.msg.fsppilot().injection_instr_absolute()) {
			stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip
			   << " != " << param.msg.fsppilot().injection_instr_absolute();
			log << ss.str() << endl;
			result->set_resulttype(result->UNKNOWN);
			result->set_latest_ip(injection_ip);
			result->set_details(ss.str());
			result->set_runtime(timer);

			continue;
		}
		if (param.msg.fsppilot().has_injection_instr_absolute()) {
			log << "Absolute IP sanity check OK" << endl;
		} else {
			log << "Absolute IP sanity check skipped (job parameters insufficient)" << endl;
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

		// memory access outside of bound determined in the golden run
		// [mem1_low, mem1_high] u [mem2_low, mem2_high]
		// video memory accesses are OK, too
		// FIXME: It would be nice to have a MemAccessListener that accepts a
		// MemoryMap, to have MemoryMaps that store addresses in a compact way,
		// and that are invertible.
		assert(mem1_low <= mem1_high && mem1_high < VIDEOMEM_START && VIDEOMEM_END < mem2_low && mem2_low <= mem2_high);
		MemAccessListener ev_mem_outside1(0x0, MemAccessEvent::MEM_READWRITE);
		ev_mem_outside1.setWatchWidth(mem1_low);
		MemAccessListener ev_mem_outside2(mem1_high + 1, MemAccessEvent::MEM_READWRITE);
		ev_mem_outside2.setWatchWidth(VIDEOMEM_START - (mem1_high + 1));
		MemAccessListener ev_mem_outside3(VIDEOMEM_END, MemAccessEvent::MEM_READWRITE);
		ev_mem_outside3.setWatchWidth(mem2_low - VIDEOMEM_END);
		MemAccessListener ev_mem_outside4(mem2_high + 1, MemAccessEvent::MEM_READWRITE);
		ev_mem_outside4.setWatchWidth(0xFFFFFFFFU - (mem2_high + 1));
		simulator.addListener(&ev_mem_outside1);
		simulator.addListener(&ev_mem_outside2);
		simulator.addListener(&ev_mem_outside3);
		simulator.addListener(&ev_mem_outside4);

		// timeout (e.g., stuck in a HLT instruction)
		TimerListener ev_timeout(timeout_runtime);
		simulator.addListener(&ev_timeout);

		// grant generous (10x) more instructions before aborting to avoid false positives
		BPSingleListener ev_dyninstructions(ANY_ADDR);
		//ev_dyninstructions.setCounter((goldenrun_instr_counter - param.msg.fsppilot().injection_instr()) * 10);
		// FIXME overflow possible
		ev_dyninstructions.setCounter(goldenrun_instr_counter * 10);
		simulator.addListener(&ev_dyninstructions);

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

		// wait until experiment-terminating event occurs
		while (!reached_finish) {
			ev = simulator.resume();
			if (ev == &func_test_output) {
				// re-add this listener
				simulator.addListener(&func_test_output);
				handle_func_test_output(ecos_test_failed, ecos_test_passed);
			} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
				result->set_jump_outside(result->TRUE);
				// no need to re-add the affected listener
			} else if (ev == &ev_mem_outside1 || ev == &ev_mem_outside2
				|| ev == &ev_mem_outside3 || ev == &ev_mem_outside4) {
				MemAccessListener *mev = dynamic_cast<MemAccessListener *>(ev);
				if (mev->getTriggerAccessType() == MemAccessEvent::MEM_READ) {
					result->set_memaccess_outside(result->READ);
					// re-add this listener, may report a write later on
					simulator.addListener(mev);
				} else { // write
					result->set_memaccess_outside(result->WRITE);
					// remove all listeners to avoid downgrade to READ
					simulator.removeListener(&ev_mem_outside1);
					simulator.removeListener(&ev_mem_outside2);
					simulator.removeListener(&ev_mem_outside3);
					simulator.removeListener(&ev_mem_outside4);
				}
			// special case: except1 and clockcnv actively generate traps
			} else if (ev == &ev_trap
			        && ((m_benchmark == "except1" && ev_trap.getTriggerNumber() == 13)
					 || (m_benchmark == "clockcnv" && ev_trap.getTriggerNumber() == 7)
					 || (m_variant == "mibench" && ev_trap.getTriggerNumber() == 7))) {
				// re-add this listener
				simulator.addListener(&ev_trap);
			} else {
				// in any other case, the experiment is finished
				break;
			}
		}

		// record latest IP regardless of result
		// TODO: consider recording latest IP within text segment, too, which
		// would make this usable for the jump-outside case
		result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());

		// record error_corrected regardless of result
                if ( (addr_errors_corrected != ADDR_INV) && mm.isMapped(addr_errors_corrected) ) {
			int32_t error_corrected = mm.getByte(addr_errors_corrected);
			result->set_error_corrected(error_corrected ? result->TRUE : result->FALSE);
		} else {
			// not setting this yields NULL in the DB
			//result->set_error_corrected(0);
		}

		// record test result
		bool output_correct;
		if (m_variant != "mibench") {
			output_correct = ecos_test_passed && !ecos_test_failed;

			if (m_benchmark == "coptermock") {
				std::vector<char> serial_correct = loadFile(filename_serial(m_variant, m_benchmark));
				if (serial_correct.size() == 0) {
					log << "sanity check failed, golden run should have had output" << endl;
					simulator.terminate(0);
				}
				std::string serial_actual = sol.getOutput();
				if (serial_actual.size() != serial_correct.size() ||
					!equal(serial_actual.begin(), serial_actual.end(), serial_correct.begin())) {
					output_correct = false;
				}
			}
			log << "Ecos Test " << (output_correct ? "PASS" : "FAIL") << endl;
		} else {
			std::vector<char> serial_correct = loadFile(filename_serial(m_variant, m_benchmark));
			std::vector<char> memout_correct = loadFile(filename_memout(m_variant, m_benchmark));

			// sanity check
			if (serial_correct.size() == 0 && memout_correct.size() == 0) {
				log << "sanity check failed, golden run should have had output" << endl;
				simulator.terminate(0);
			}

			std::string serial_actual = sol.getOutput();
			std::vector<char> memout_actual = readFromMem(addr_testdata, addr_testdata_size);
			if (serial_actual.size() == serial_correct.size() &&
				memout_actual.size() == memout_correct.size() &&
				equal(serial_actual.begin(), serial_actual.end(), serial_correct.begin()) &&
				equal(memout_actual.begin(), memout_actual.end(), memout_correct.begin())) {
				output_correct = true;
			} else {
				output_correct = false;
			}
		}

		result->set_sim_runtime_factor(
			(simulator.getTimerTicks() - time_start) / (double) goldenrun_runtime_ticks);

		if (ev == &func_finish && output_correct) {
			// do we reach finish?
			log << "experiment finished ordinarily" << endl;
			result->set_resulttype(result->OK);
		} else if (ev == &func_finish && !output_correct) {
			// do we reach finish?
			log << "experiment finished, but output incorrect" << endl;
			result->set_resulttype(result->SDC);
		} else if (ev == &func_ecc_panic) {
			log << "ECC Panic: uncorrectable error" << endl;
			result->set_resulttype(result->DETECTED); // DETECTED <=> ECC_PANIC <=> reboot
		} else if (ev == &ev_trap) {
			log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
			result->set_resulttype(result->TRAP);

			stringstream ss;
			ss << ev_trap.getTriggerNumber();
			result->set_details(ss.str());
		} else if (ev == &ev_timeout || ev == &ev_dyninstructions) {
			log << "Result TIMEOUT" << endl;
			result->set_resulttype(result->TIMEOUT);
			if (ev == &ev_dyninstructions) {
				result->set_details("i");
			}
		} else {
			log << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "event addr " << ev << " EIP " << simulator.getCPU(0).getInstructionPointer();
			result->set_details(ss.str());
		}

		result->set_runtime(timer);
	}

#if !LOCAL
	m_jc.sendResult(param);
#endif

#if !LOCAL
	}
#endif
	return true;
}
#endif // PREREQUISITES

bool EcosKernelTestExperiment::readELFSymbols(
	fail::guest_address_t& entry,
	fail::guest_address_t& finish,
	fail::guest_address_t& testdata,
	fail::guest_address_t& testdata_size,
	fail::guest_address_t& test_output,
	fail::guest_address_t& errors_corrected,
	fail::guest_address_t& panic,
	fail::guest_address_t& text_start,
	fail::guest_address_t& text_end,
	fail::guest_address_t& data_start,
	fail::guest_address_t& data_end)
{
	ElfReader elfreader(filename_elf(m_variant, m_benchmark).c_str());

	if (m_variant == "mibench") {
		entry            = elfreader.getSymbol("main").getAddress();
		finish           = elfreader.getSymbol("ECOS_BENCHMARK_FINISHED").getAddress();
		testdata         = elfreader.getSymbol("ECOS_data").getAddress();
		testdata_size    = elfreader.getSymbol("ECOS_data_size").getAddress();
		test_output      = ADDR_INV;
		errors_corrected = ADDR_INV;
		panic            = ADDR_INV;
		// testdata / testdata_size may be optimized away (gc-sections)
	} else {
		// ecos_kernel_test
		entry            = elfreader.getSymbol("cyg_start").getAddress();
		finish           = elfreader.getSymbol("cyg_test_exit").getAddress();
		testdata         = ADDR_INV;
		testdata_size    = ADDR_INV;
		test_output      = elfreader.getSymbol("cyg_test_output").getAddress();
		errors_corrected = elfreader.getSymbol("errors_corrected").getAddress();
		panic            = elfreader.getSymbol("_Z9ecc_panicv").getAddress();
		// it's OK if errors_corrected or ecc_panic are missing
		if (test_output == ADDR_INV) {
			return false;
		}
	}
	text_start       = elfreader.getSymbol("_stext").getAddress();
	text_end         = elfreader.getSymbol("_etext").getAddress();
	data_start       = elfreader.getSymbol("__ram_data_start").getAddress();
	data_end         = elfreader.getSymbol("__bss_end").getAddress();

	if (entry == ADDR_INV || finish == ADDR_INV ||
	    text_start == ADDR_INV || text_end == ADDR_INV ||
	    data_start == ADDR_INV || data_end == ADDR_INV) {
		return false;
	}
	return true;
}

void EcosKernelTestExperiment::parseOptions()
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

bool EcosKernelTestExperiment::run()
{
	log << "startup" << endl;

#if PREREQUISITES || LOCAL
	parseOptions();
#endif

#if PREREQUISITES
	log << "retrieving ELF symbol addresses ..." << endl;
	guest_address_t entry, finish, testdata, testdata_size,
		test_output, errors_corrected,
		panic, text_start, text_end, data_start, data_end;
	if (!readELFSymbols(entry, finish, testdata, testdata_size,
			test_output, errors_corrected,
			panic, text_start, text_end, data_start, data_end)) {
		log << "failed, essential symbols are missing!" << endl;
		simulator.terminate(1);
	}

	// step 0
	if (retrieveGuestAddresses(finish, data_start, data_end)) {
		log << "STEP 0 finished: proceeding ..." << endl;
	} else { return false; }

	// step 1
	if (establishState(entry, finish, errors_corrected)) {
		log << "STEP 1 finished: proceeding ..." << endl;
	} else { return false; }

	// step 2
	if (performTrace(entry, finish, testdata, testdata_size)) {
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
