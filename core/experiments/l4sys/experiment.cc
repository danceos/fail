#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "SAL/SALConfig.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Memory.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"
#include "config/FailConfig.hpp"

#include "l4sys.pb.h"

using std::endl;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_SUPPRESS_INTERRUPTS) || \
    !defined(CONFIG_EVENT_TRAP) || !defined(CONFIG_EVENT_IOPORT) || \
    !defined(CONFIG_EVENT_INTERRUPT)
#error This experiment needs: breakpoints, suppressed-interrupts, traps, I/O port and interrupt events, \
  save, and restore. Enable these in the configuration.
#endif

char const * const state_folder = "l4sys.state";
char const * const instr_list_fn = "ip.list";
char const * const golden_run_fn = "golden.out";
sal::address_t const aspace = 0x1fd44000;
std::string output;
std::vector<sal::address_t> instr_list;
std::string golden_run;
//the program needs to run 5 times without a fault
const unsigned times_run = 5;

std::string L4SysExperiment::sanitised(std::string in_str) {
	string result;
	result.reserve(in_str.size());
	for (string::iterator it = in_str.begin(); it != in_str.end(); it++) {
		unsigned char_value = static_cast<unsigned>(*it);
		if (char_value < 0x20 || char_value > 0x7E) {
			char str_nr[5];
			sprintf(str_nr, "\\%03o", char_value);
			result += str_nr;
		} else {
			result += *it;
		}
	}
	return result;
}

fi::BaseEvent* L4SysExperiment::waitIOOrOther(bool clear_output) {
	fi::IOPortEvent ev_ioport(0x3F8, true);
	fi::BaseEvent* ev = NULL;
	if (clear_output)
		output.clear();
	while (true) {
		sal::simulator.addEvent(&ev_ioport);
		ev = sal::simulator.waitAny();
		sal::simulator.removeEvent(&ev_ioport);
		if (ev == &ev_ioport) {
			output += ev_ioport.getData();
		} else {
			break;
		}
	}
	return ev;
}

bool L4SysExperiment::run() {
	Logger log("L4Sys", false);
	fi::BPSingleEvent bp(0, aspace);

	log << "startup" << endl;

	//FIXME: this is a race condition:
	//only one L4SysExperiment instance should execute this block
	//at a time

	struct stat teststruct;
	// STEP 1: run until interesting function starts, and save state
	if (stat(state_folder, &teststruct) == -1) {
		bp.setWatchInstructionPointer(COOL_ECC_FUNC_ENTRY);
		sal::simulator.addEventAndWait(&bp);

		log << "test function entry reached, saving state" << endl;
		log << "EIP = " << std::hex << bp.getTriggerInstructionPointer()
				<< " or "
				<< sal::simulator.getRegisterManager().getInstructionPointer()
				<< endl;
		sal::simulator.save(state_folder);
	}

	// STEP 2: determine instructions executed
	if (stat(instr_list_fn, &teststruct) == -1) {
		log << "restoring state" << endl;
		sal::simulator.restore(state_folder);
		log << "EIP = " << std::hex
				<< sal::simulator.getRegisterManager().getInstructionPointer()
				<< endl;

		// make sure the timer interrupt doesn't disturb us
		sal::simulator.addSuppressedInterrupt(32);

		std::ofstream instr_list_file(instr_list_fn);
		instr_list_file << std::hex;
		bp.setWatchInstructionPointer(fi::ANY_ADDR);
		while (bp.getTriggerInstructionPointer() != COOL_ECC_CALCDONE) {
			sal::simulator.addEventAndWait(&bp);
			//short sanity check
			sal::address_t curr_instr = bp.getTriggerInstructionPointer();
			assert(
					curr_instr == sal::simulator.getRegisterManager().getInstructionPointer());
			instr_list.push_back(curr_instr);
			instr_list_file << curr_instr << endl;
		}
		log << "saving instructions triggered during normal execution" << endl;
		instr_list_file.close();
	} else {
		std::ifstream instr_list_file(instr_list_fn);
		instr_list_file >> std::hex;
		while (!instr_list_file.eof()) {
			sal::address_t curr_instr;
			instr_list_file >> curr_instr;
			instr_list.push_back(curr_instr);
		}
		instr_list_file.close();
	}

	// STEP 3: determine the output of a "golden run"
	if (stat(golden_run_fn, &teststruct) == -1) {
		log << "restoring state" << endl;
		sal::simulator.restore(state_folder);
		log << "EIP = " << std::hex
				<< sal::simulator.getRegisterManager().getInstructionPointer()
				<< endl;

		// make sure the timer interrupt doesn't disturb us
		sal::simulator.addSuppressedInterrupt(32);

		std::ofstream golden_run_file(golden_run_fn);
		bp.setWatchInstructionPointer(COOL_ECC_CALCDONE);
		bp.setCounter(times_run);
		sal::simulator.addEvent(&bp);
		fi::BaseEvent* ev = waitIOOrOther(true);
		if (ev == &bp) {
			golden_run.assign(output.c_str());
			golden_run_file << output.c_str();
			log << "Output successfully logged!" << endl;
		} else {
			log
					<< "Obviously, there is some trouble with the events registered - aborting simulation!"
					<< endl;
			golden_run_file.close();
			sal::simulator.terminate(10);
		}
		sal::simulator.clearEvents();
		bp.setCounter(1);
		log << "saving output generated during normal execution" << endl;
		golden_run_file.close();
	} else {
		std::ifstream golden_run_file(golden_run_fn);

		//shamelessly copied from http://stackoverflow.com/questions/2602013/:
		golden_run_file.seekg(0, std::ios::end);
		size_t flen = golden_run_file.tellg();
		golden_run.reserve(flen);
		golden_run_file.seekg(0, std::ios::beg);

		golden_run.assign((std::istreambuf_iterator<char>(golden_run_file)),
				std::istreambuf_iterator<char>());

		golden_run_file.close();

		//the generated output probably has a similar length
		output.reserve(flen);
	}

	//end of critical section

	// STEP 4: The actual experiment.
	for (int i = 0; i < COOL_ECC_NUMINSTR; i++) {
		log << "restoring state" << endl;
		sal::simulator.restore(state_folder);

		log << "asking job server for experiment parameters" << endl;
		L4SysExperimentData param;
		if (!m_jc.getParam(param)) {
			log << "Dying." << endl;
			// communicate that we were told to die
			sal::simulator.terminate(1); // "return (false);" ?
		}
		int id = param.getWorkloadID();
		int instr_offset = param.msg.instr_offset();
		int bit_offset = param.msg.bit_offset();
		log << "job " << id << " instr " << instr_offset << " bit "
				<< bit_offset << endl;

		bp.setWatchInstructionPointer(instr_list[instr_offset]);
		sal::simulator.addEvent(&bp);
		//and log the output
		waitIOOrOther(true);

		// inject
		sal::RegisterManager& rm = sal::simulator.getRegisterManager();
		sal::Register *ebx = rm.getRegister(sal::RID_EBX);
		sal::regdata_t data = ebx->getData();
		sal::regdata_t newdata = data ^ (1 << bit_offset);
		ebx->setData(newdata);
		// note at what IP we did it
		sal::address_t injection_ip =
				sal::simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_injection_ip(injection_ip);
		log << "inject @ ip " << injection_ip << " (offset " << std::dec
				<< instr_offset << ")" << " bit " << bit_offset << ": 0x"
				<< std::hex << ((int) data) << " -> 0x" << ((int) newdata)
				<< endl;

		// sanity check (only works if we're working with an instruction trace)
		if (injection_ip != instr_list[instr_offset]) {
			std::stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip << " != "
					<< instr_list[instr_offset] << endl;
			log << ss.str();
			param.msg.set_resulttype(param.msg.UNKNOWN);
			param.msg.set_resultdata(injection_ip);
			param.msg.set_details(ss.str());

			sal::simulator.clearEvents();
			m_jc.sendResult(param);
			continue;
		}

		// aftermath
		fi::BPSingleEvent ev_done(COOL_ECC_CALCDONE, aspace);
		ev_done.setCounter(times_run);
		sal::simulator.addEvent(&ev_done);
		const unsigned instr_run = times_run * COOL_ECC_NUMINSTR;
		fi::BPSingleEvent ev_timeout(fi::ANY_ADDR, aspace);
		ev_timeout.setCounter(instr_run + 3000);
		sal::simulator.addEvent(&ev_timeout);
		fi::TrapEvent ev_trap(fi::ANY_TRAP);
		sal::simulator.addEvent(&ev_trap);
		fi::InterruptEvent ev_intr(fi::ANY_INTERRUPT);
		//ten times as many interrupts as instructions justify an exception
		ev_intr.setCounter(instr_run * 10);
		sal::simulator.addEvent(&ev_intr);

		//do not discard output recorded so far
		fi::BaseEvent *ev = waitIOOrOther(false);

		/* copying a string object that contains control sequences
		 * unfortunately does not work with the library I am using,
		 * which is why output is passed on as C string and
		 * the string compare is done on C strings
		 */
		if (ev == &ev_done) {
			if (strcmp(output.c_str(), golden_run.c_str()) == 0) {
				log << std::dec << "Result DONE" << endl;
				param.msg.set_resulttype(param.msg.CALCDONE);
			} else {
				log << std::dec << "Result WRONG" << endl;
				param.msg.set_resulttype(param.msg.WRONG);
				param.msg.set_output(sanitised(output.c_str()));
			}
		} else if (ev == &ev_timeout) {
			log << std::dec << "Result TIMEOUT" << endl;
			param.msg.set_resulttype(param.msg.TIMEOUT);
			param.msg.set_resultdata(
					sal::simulator.getRegisterManager().getInstructionPointer());
			param.msg.set_output(sanitised(output.c_str()));
		} else if (ev == &ev_trap) {
			log << std::dec << "Result TRAP #" << ev_trap.getTriggerNumber()
					<< endl;
			param.msg.set_resulttype(param.msg.TRAP);
			param.msg.set_resultdata(
					sal::simulator.getRegisterManager().getInstructionPointer());
			param.msg.set_output(sanitised(output.c_str()));
		} else if (ev == &ev_intr) {
			log << std::hex << "Result INT FLOOD; Last INT #:"
					<< ev_intr.getTriggerNumber() << endl;
			param.msg.set_resulttype(param.msg.INTR);
			param.msg.set_resultdata(
					sal::simulator.getRegisterManager().getInstructionPointer());
			param.msg.set_output(sanitised(output.c_str()));
		} else {
			log << std::dec << "Result WTF?" << endl;
			param.msg.set_resulttype(param.msg.UNKNOWN);
			param.msg.set_resultdata(
					sal::simulator.getRegisterManager().getInstructionPointer());
			param.msg.set_output(sanitised(output.c_str()));

			std::stringstream ss;
			ss << "eventid " << ev << " EIP "
					<< sal::simulator.getRegisterManager().getInstructionPointer()
					<< endl;
			param.msg.set_details(ss.str());
		}

		sal::simulator.clearEvents();
		m_jc.sendResult(param);
	}

#ifdef HEADLESS_EXPERIMENT
	sal::simulator.terminate(0);
#endif
// experiment successfully conducted
	return true;
}
