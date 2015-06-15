#include <iostream>
#include <fstream>
#include <algorithm>

#include <sys/types.h>
#include <unistd.h>


#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "util/WallclockTimer.hpp"
#include "config/FailConfig.hpp"

// You need to have the serialoutput plugin enabled for this
#include "../plugins/serialoutput/SerialOutputLogger.hpp"

#define LIMIT_SERIAL	1024*1024

using namespace std;
using namespace fail;

#define LOCAL 0

const std::string FiascoFailExperiment::dir_images(DIR_IMAGES);
const std::string FiascoFailExperiment::dir_prerequisites(DIR_PREREQUISITES);

bool FiascoFailExperiment::readTraceInfo(unsigned &instr_counter, unsigned long long &runtime, fail::guest_address_t &addr_finish,
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
			ss >> addr_finish;
			break;
		}
		count++;
	}
	file.close();
	assert(count == 3);
	return (count == 3);
}

std::string FiascoFailExperiment::filename_state(unsigned instr_offset, const std::string& variant, const std::string& benchmark)
{
	stringstream ss;
	ss << instr_offset;
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "state" + "-" + ss.str();
	}
	return "state-" + ss.str();
}

std::string FiascoFailExperiment::filename_traceinfo(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "traceinfo.txt";
	}
	return "traceinfo.txt";
}

std::string FiascoFailExperiment::filename_elf(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_images + "/" + variant + "/" + "fiasco.image";
	}
	return "fiasco.image";
}

std::string FiascoFailExperiment::filename_serial(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + ".serial";
	}
	return "serial";
}


std::vector<char> FiascoFailExperiment::loadFile(std::string filename)
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


bool FiascoFailExperiment::run()
{
	log << "startup" << endl;

	// Do the actual fault injection
	faultInjection();

	// Experiment finished
	simulator.terminate();
	return true;
}


bool FiascoFailExperiment::faultInjection()
{
	// trace info
	unsigned goldenrun_instr_counter;
	unsigned long long goldenrun_runtime_ticks;
	unsigned goldenrun_runtime, timeout_runtime;
	guest_address_t addr_finish;

	BPSingleListener bp;
	int experiments = 0;
#if !LOCAL
	for(experiments = 0; experiments < 500 || (m_jc.getNumberOfUndoneJobs() != 0); )
	{
#endif
		log << "asking job server for experiment parameters" << endl;
		FiascoFailExperimentData param;
#if !LOCAL
		if(!m_jc.getParam(param))
		{
			log << "Dying." << endl;
			simulator.terminate(1);
		}
#else
		// XXX debug
		param.msg.mutable_fsppilot()->set_injection_instr(96);
		param.msg.mutable_fsppilot()->set_injection_instr_absolute(4026670033);
		param.msg.mutable_fsppilot()->set_data_address(4237508604);
		param.msg.mutable_fsppilot()->set_data_width(1);
		param.msg.mutable_fsppilot()->set_variant("baseline");
		param.msg.mutable_fsppilot()->set_benchmark("shared_ds");
#endif

		if(param.msg.fsppilot().data_width() != 1)
		{
			log << "cannot deal with data_width = " << param.msg.fsppilot().data_width() << endl;
			simulator.terminate(1);
		}

		// Get the experiment data from the Job-Server
		int id = param.getWorkloadID();
		m_variant = param.msg.fsppilot().variant();
		m_benchmark = param.msg.fsppilot().benchmark();
		// Offset to the IP where the fault injection has to be done:
		unsigned instr_offset = param.msg.fsppilot().injection_instr();
		// Memory address wehre the fault injection has to be done:
		guest_address_t mem_addr = param.msg.fsppilot().data_address();

		readTraceInfo(goldenrun_instr_counter, goldenrun_runtime_ticks, addr_finish, m_variant, m_benchmark);

		// ELF symbol addresses
		ElfReader elfreader(filename_elf(m_variant, m_benchmark).c_str());
		guest_address_t addr_errors_corrected = elfreader.getSymbol("errors_corrected").getAddress();
		guest_address_t ecc_panic_address     = elfreader.getSymbol("_Z9ecc_panicv").getAddress();
		if (ecc_panic_address == ADDR_INV && m_variant != "baseline") {
			log << "WARNING: retrieving ELF symbol 'ecc_panic' failed!" << endl;
		}

		// for compatibility with the ecos kernel test experiment (MULTIPLE_SNAPSHOTS)
		string statename = filename_state(0, m_variant, m_benchmark);

		// for each job with the SINGLEBITFLIP fault model we're actually doing *8*
		// experiments (one for each bit)
		for(int bit_offset = 0; bit_offset < 8; ++bit_offset)
		{
			++experiments;

			// Timer to log the actual time of the experiment:
			WallclockTimer timer;
			timer.startTimer();

			// 8 results in one job
			FiascofailProtoMsg_Result *result = param.msg.add_result();
			result->set_bit_offset(bit_offset);
			log << dec << "job " << id << " @bit: " << bit_offset << " " << m_variant << "/" << m_benchmark
					<< " instr-offset " << hex << instr_offset
					<< " mem " << hex << mem_addr << "+" << dec << bit_offset << endl;

			log << "restoring state" << endl;
			simulator.restore(statename);

			// convert to microseconds (simulator.getTimerTicksPerSecond() only
			// works reliably when simulation has begun)
			goldenrun_runtime = (unsigned)
				(goldenrun_runtime_ticks * 1000000.0 / simulator.getTimerTicksPerSecond());
			timeout_runtime = goldenrun_runtime + 1000000/18.2; // + 1 timer tick

			BPSingleListener func_finish(addr_finish);
			simulator.addListener(&func_finish);
			bool reached_finish = false;

			// record serial output
			SerialOutputLogger sol(0x3f8, LIMIT_SERIAL);
			simulator.addFlow(&sol);

			// measure elapsed time
			simtime_t time_start = simulator.getTimerTicks();

			// no need to wait if offset is 0
			BaseListener* ev;

			if (instr_offset > 0) {
				bp.setWatchInstructionPointer(ANY_ADDR);
				bp.setCounter(instr_offset);
				simulator.addListener(&bp);

				ev = simulator.resume();
				if (ev == &func_finish) {
					log << "experiment reached finish() before FI" << endl;
					reached_finish = true;
				}
			}

			// --- fault injection ---
			MemoryManager& mm = simulator.getMemoryManager();
			byte_t data = mm.getByte(mem_addr);
			byte_t newdata;
#if FIASCO_FAULTMODEL_BURST
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
			// catch traps as "extraordinary" ending
			TrapListener ev_trap(ANY_TRAP);
			simulator.addListener(&ev_trap);

			// timeout (e.g., stuck in a HLT instruction)
			TimerListener ev_timeout(timeout_runtime);
			simulator.addListener(&ev_timeout);

			// grant generous (10x) more instructions before aborting to avoid false positives
			BPSingleListener ev_dyninstructions(ANY_ADDR);
			//ev_dyninstructions.setCounter((goldenrun_instr_counter - param.msg.fsppilot().injection_instr()) * 10);
			// FIXME overflow possible
			ev_dyninstructions.setCounter(goldenrun_instr_counter * 10);
			simulator.addListener(&ev_dyninstructions);

			// incomplete (e.g. cursors blinks so nothing happens any more or a longjump occurs)
			BPSingleListener ev_blink(FIASCO_BREAK_BLINK);
			simulator.addListener(&ev_blink);
			BPSingleListener ev_longjmp(FIASCO_BREAK_LONGJMP);
			simulator.addListener(&ev_longjmp);

			// function called by ecc apsects, when an uncorrectable error is detected
			BPSingleListener func_ecc_panic(ecc_panic_address);
			if(ecc_panic_address != ADDR_INV) {
				simulator.addListener(&func_ecc_panic);
			}

			// wait until experiment-terminating event occurs
			while (!reached_finish) {
				ev = simulator.resume();
				if( (ev == &ev_trap) && (ev_trap.getTriggerNumber() == 14) ) {
					// Page fault trap is OK
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

			// record test result
			bool output_correct;
			std::vector<char> serial_correct = loadFile(filename_serial(m_variant, m_benchmark));

			// sanity check
			if (serial_correct.size() == 0) {
				log << "sanity check failed, golden run should have had output" << endl;
				simulator.terminate(0);
			}

			std::string serial_actual = sol.getOutput();
			if (serial_actual.size() == serial_correct.size() &&
				equal(serial_actual.begin(), serial_actual.end(), serial_correct.begin())) {
				output_correct = true;
			} else {
				output_correct = false;
			}


			// Get the runtime factor compared to the golden run
			result->set_sim_runtime_factor(
				(simulator.getTimerTicks() - time_start) / (double) goldenrun_runtime_ticks);

			if (ev == &func_finish && output_correct) {
				// do we reach finish?
				log << "experiment finished ordinarily" << endl;
				result->set_resulttype(result->OK);
				// record error_corrected only in the 'OK' case
				// isMapped() crashes in case something (e.g., paging) went horribly wrong
				if ( (addr_errors_corrected != ADDR_INV) && mm.isMapped(addr_errors_corrected) ) {
					int32_t error_corrected = mm.getByte(addr_errors_corrected);
					result->set_error_corrected(error_corrected ? result->TRUE : result->FALSE);
				} else {
					// not setting this yields NULL in the DB
					//result->set_error_corrected(0);
				}
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
			} else if (ev == &ev_timeout || ev == &ev_dyninstructions || ev == &ev_blink || ev == &ev_longjmp) {
				log << "Result TIMEOUT" << endl;
				result->set_resulttype(result->TIMEOUT);
				if (ev == &ev_dyninstructions) {
					result->set_details("i");
				}
				else if (ev == &ev_blink) {
					result->set_details("b");
				}
				else if (ev == &ev_longjmp) {
					result->set_details("l");
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
		// Send the result back to the job server and continue with next experiment (if there is one)
		m_jc.sendResult(param);
	}
#endif
	return true;
}

