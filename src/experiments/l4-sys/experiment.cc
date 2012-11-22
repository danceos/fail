#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "UDIS86.hpp"
#include "InstructionFilter.hpp"
#include "aluinstr.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/bochs/BochsRegister.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"

#include "l4sys.pb.h"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || \
    !defined(CONFIG_EVENT_IOPORT)
#error This experiment needs: breakpoints and I/O port events, \
  save, and restore. Enable these in the configuration.
#endif

string output;
string golden_run;

string L4SysExperiment::sanitised(const string &in_str) {
	string result;
	int in_str_size = in_str.size();
	result.reserve(in_str_size);
	for (int idx = 0; idx < in_str_size; idx++) {
		char cur_char = in_str[idx];
		unsigned cur_char_value = static_cast<unsigned>(cur_char);
		// also exclude the delimiter (',')
		if (cur_char_value < 0x20 || cur_char_value > 0x7E || cur_char_value == ',') {
			char str_nr[5];
			sprintf(str_nr, "\\%03o", cur_char_value);
			result += str_nr;
		} else {
			result += cur_char;
		}
	}
	return result;
}

BaseListener* L4SysExperiment::waitIOOrOther(bool clear_output) {
	IOPortListener ev_ioport(0x3F8, true);
	BaseListener* ev = NULL;
	if (clear_output)
		output.clear();
	while (true) {
		simulator.addListener(&ev_ioport);
		ev = simulator.resume();
		simulator.removeListener(&ev_ioport);
		if (ev == &ev_ioport) {
			output += ev_ioport.getData();
		} else {
			break;
		}
	}
	return ev;
}

Bit32u L4SysExperiment::eipBiased() {
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	Bit32u EIP = cpu_context->gen_reg[BX_32BIT_REG_EIP].dword.erx;
	return EIP + cpu_context->eipPageBias;
}

const Bit8u *L4SysExperiment::calculateInstructionAddress() {
	// pasted in from various nested Bochs functions and macros - I hope
	// they will not change too soon (as do the Bochs developers, probably)
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	const Bit8u *result = cpu_context->eipFetchPtr + eipBiased();
	return result;
}

bx_bool L4SysExperiment::fetchInstruction(BX_CPU_C *instance,
		const Bit8u *instr, bxInstruction_c *iStorage) {
	unsigned remainingInPage = instance->eipPageWindowSize - eipBiased();
	int ret;

#if BX_SUPPORT_X86_64
	if (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64)
	ret = instance->fetchDecode64(instr, iStorage, remainingInPage);
	else
#endif
	ret = instance->fetchDecode32(instr, iStorage, remainingInPage);

	if (ret < 0) {
		// handle instrumentation callback inside boundaryFetch
		instance->boundaryFetch(instr, remainingInPage, iStorage);
		return 0;
	}

	return 1;
}

void L4SysExperiment::logInjection() {
	// explicit type assignment necessary before sending over output stream
	int id = param->getWorkloadID();
	int instr_offset = param->msg.instr_offset();
	int bit_offset = param->msg.bit_offset();
	int exp_type = param->msg.exp_type();
	address_t injection_ip = param->msg.injection_ip();

	log << "job " << id << " exp_type " << exp_type << endl;
	log << "inject @ ip " << injection_ip << " (offset " << dec << instr_offset
			<< ")" << " bit " << bit_offset << endl;
}

BaseListener *L4SysExperiment::singleStep(bool preserveAddressSpace) {
	address_t aspace = (preserveAddressSpace ? L4SYS_ADDRESS_SPACE : ANY_ADDR);
	BPSingleListener singlestepping_event(ANY_ADDR, aspace);
	simulator.addListener(&singlestepping_event);
	/* prepare for the case that the kernel panics and never
	   switches back to this thread by introducing a scheduling timeout
	   of 10 seconds */
	TimerListener schedTimeout(10000);
	simulator.addListener(&schedTimeout);
	BaseListener *ev = waitIOOrOther(false);
	simulator.removeListener(&singlestepping_event);
	simulator.removeListener(&schedTimeout);

	if (ev == &schedTimeout) {
		// otherwise we just assume this thread is never scheduled again
		log << "Result TIMEOUT" << endl;
		param->msg.set_resulttype(param->msg.TIMEOUT);
		param->msg.set_resultdata(
				simulator.getRegisterManager().getInstructionPointer());
		param->msg.set_output(sanitised(output.c_str()));
		param->msg.set_details("Timed out immediately after injecting");

		m_jc.sendResult(*param);
		terminate(0);
	}
	return ev;
}

void L4SysExperiment::injectInstruction(
        bxInstruction_c *oldInstr, bxInstruction_c *newInstr) {
	// backup the current and insert the faulty instruction
	bxInstruction_c backupInstr;
	memcpy(&backupInstr, oldInstr, sizeof(bxInstruction_c));
	memcpy(oldInstr, newInstr, sizeof(bxInstruction_c));

	// execute the faulty instruction, then return
	singleStep(false);

	//restore the old instruction
	memcpy(oldInstr, &backupInstr, sizeof(bxInstruction_c));
}

unsigned L4SysExperiment::calculateTimeout(unsigned instr_left) {
	// the timeout in seconds, plus one backup second (avoids rounding overhead)
	// [instr] / [instr / s] = [s]
	unsigned seconds = instr_left / L4SYS_BOCHS_IPS + 1;
	// 1.1 (+10 percent) * 1000 ms/s * [s]
	return 1100 * seconds;
}

L4SysExperiment::L4SysExperiment()
 : m_jc("localhost"), log("L4Sys", false)
{
	 param = new L4SysExperimentData;
}

L4SysExperiment::~L4SysExperiment() {
	destroy();
}

void L4SysExperiment::destroy() {
	delete param;
	param = NULL;
}

void L4SysExperiment::terminate(int reason) {
	destroy();
	simulator.terminate(reason);
}

bool L4SysExperiment::run() {
	BPSingleListener bp(0, L4SYS_ADDRESS_SPACE);
	srand(time(NULL));

	log << "startup" << endl;

#if PREPARATION_STEP == 1
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(L4SYS_FUNC_ENTRY);
	simulator.addListenerAndResume(&bp);

	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << " or "
			<< simulator.getRegisterManager().getInstructionPointer()
			<< endl;
	simulator.save(L4SYS_STATE_FOLDER);
#elif PREPARATION_STEP == 2
	// STEP 2: determine instructions executed
	log << "restoring state" << endl;
	simulator.restore(L4SYS_STATE_FOLDER);
	log << "EIP = " << hex
			<< simulator.getRegisterManager().getInstructionPointer()
			<< endl;

#ifdef L4SYS_FILTER_INSTRUCTIONS
	ofstream instr_list_file(L4SYS_INSTRUCTION_LIST, ios::binary);
	bp.setWatchInstructionPointer(ANY_ADDR);

	size_t count = 0, accepted = 0;
	map<address_t, unsigned> times_called_map;
	InstructionFilter *instrFilter = NULL;
#if defined(L4SYS_ADDRESS_LBOUND) && defined(L4SYS_ADDRESS_UBOUND)
	instrFilter = new RangeInstructionFilter(L4SYS_ADDRESS_LBOUND, L4SYS_ADDRESS_UBOUND);
#endif
	while (bp.getTriggerInstructionPointer() != L4SYS_FUNC_EXIT) {
		simulator.addListenerAndResume(&bp);
		count++;
		//short sanity check
		address_t curr_addr = bp.getTriggerInstructionPointer();
		assert(
				curr_addr == simulator.getRegisterManager().getInstructionPointer());

		unsigned times_called = times_called_map[curr_addr];
		times_called++;
		times_called_map[curr_addr] = times_called;

		// now check if we want to add the instruction for fault injection
		if (instrFilter != NULL &&
		    instrFilter->isValidInstr(curr_addr,
                reinterpret_cast<char const*>(calculateInstructionAddress()))) {
			accepted++;
			TraceInstr new_instr;
			new_instr.trigger_addr = curr_addr;
			new_instr.bp_counter = times_called;

			instr_list_file.write(reinterpret_cast<char*>(&new_instr), sizeof(TraceInstr));
		}
	}
	delete instrFilter;
	log << "saving instructions triggered during normal execution" << endl;
	instr_list_file.close();
	log << "test function calculation position reached after "
			<< dec << count << " instructions; " << accepted << " accepted" << endl;
#else
	int count = 0;
	int ul = 0, kernel = 0;
	bp.setWatchInstructionPointer(ANY_ADDR);
	for (; bp.getTriggerInstructionPointer() != L4SYS_FUNC_EXIT; ++count) {
		simulator.addListenerAndResume(&bp);
		if (bp.getTriggerInstructionPointer() < 0xC0000000) {
			ul++;
		} else {
			kernel++;
		}
	}
	log << "EIP = " << hex
			<< simulator.getRegisterManager().getInstructionPointer() << endl;
	log << "test function calculation position reached after "
			<< dec << count << " instructions; "
			<< "ul: " << ul << ", kernel: " << kernel << endl;
#endif

#elif PREPARATION_STEP == 3
	// STEP 3: determine the output of a "golden run"
	log << "restoring state" << endl;
	simulator.restore(L4SYS_STATE_FOLDER);
	log << "EIP = " << hex
			<< simulator.getRegisterManager().getInstructionPointer()
			<< endl;

	ofstream golden_run_file(L4SYS_CORRECT_OUTPUT);
	bp.setWatchInstructionPointer(L4SYS_FUNC_EXIT);
	simulator.addListener(&bp);
	BaseListener* ev = waitIOOrOther(true);
	if (ev == &bp) {
		golden_run.assign(output.c_str());
		golden_run_file << output.c_str();
		log << "Output successfully logged!" << endl;
	} else {
		log
				<< "Obviously, there is some trouble with"
				<< " the events registered - aborting simulation!"
				<< endl;
		golden_run_file.close();
		terminate(10);
	}

	log << "saving output generated during normal execution" << endl;
	golden_run_file.close();

#elif PREPARATION_STEP == 0
	// LAST STEP: The actual experiment.
	struct stat teststruct;
	if (stat(L4SYS_STATE_FOLDER, &teststruct) == -1 ||
		stat(L4SYS_CORRECT_OUTPUT, &teststruct) == -1) {
		log << "Important data missing - call \"prepare\" first." << endl;
		terminate(10);
	}
	ifstream golden_run_file(L4SYS_CORRECT_OUTPUT);

	if (!golden_run_file.good()) {
		log << "Could not open file " << L4SYS_CORRECT_OUTPUT << endl;
		terminate(20);
	}
	golden_run.reserve(teststruct.st_size);

	golden_run.assign((istreambuf_iterator<char>(golden_run_file)),
			istreambuf_iterator<char>());

	golden_run_file.close();

	//the generated output probably has a similar length
	output.reserve(teststruct.st_size);

	log << "restoring state" << endl;
	simulator.restore(L4SYS_STATE_FOLDER);

	log << "asking job server for experiment parameters" << endl;
	if (!m_jc.getParam(*param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		terminate(1);
	}

	int instr_offset = param->msg.instr_offset();
	int bit_offset = param->msg.bit_offset();
	int exp_type = param->msg.exp_type();

#ifdef L4SYS_FILTER_INSTRUCTIONS
	ifstream instr_list_file(L4SYS_INSTRUCTION_LIST, ios::binary);

	if (!instr_list_file.good()) {
		log << "Missing instruction trace" << endl;
		terminate(21);
	}

	TraceInstr curr_instr;
	instr_list_file.seekg(instr_offset * sizeof(TraceInstr));
	instr_list_file.read(reinterpret_cast<char*>(&curr_instr), sizeof(TraceInstr));
	instr_list_file.close();

	bp.setWatchInstructionPointer(curr_instr.trigger_addr);
	bp.setCounter(curr_instr.bp_counter);
#else
	bp.setWatchInstructionPointer(ANY_ADDR);
	bp.setCounter(instr_offset);
#endif
	simulator.addListener(&bp);
	//and log the output
	waitIOOrOther(true);

	// note at what IP we will do the injection
	address_t injection_ip =
			simulator.getRegisterManager().getInstructionPointer();
	param->msg.set_injection_ip(injection_ip);

#ifdef L4SYS_FILTER_INSTRUCTIONS
	// only works if we filter instructions
	// sanity check (only works if we're working with an instruction trace)
	if (injection_ip != curr_instr.trigger_addr) {
		stringstream ss;
		ss << "SANITY CHECK FAILED: " << injection_ip << " != "
		<< curr_instr.trigger_addr;
		log << ss.str() << endl;
		param->msg.set_resulttype(param->msg.UNKNOWN);
		param->msg.set_resultdata(injection_ip);
		param->msg.set_details(ss.str());

		m_jc.sendResult(*param);
		terminate(20);
	}
#endif

	// inject
	if (exp_type == param->msg.GPRFLIP) {
		if (!param->msg.has_register_offset()) {
			param->msg.set_resulttype(param->msg.UNKNOWN);
			param->msg.set_resultdata(
					simulator.getRegisterManager().getInstructionPointer());
			param->msg.set_output(sanitised(output.c_str()));

			stringstream ss;
			ss << "Sent package did not contain the injection location (register offset)";
			param->msg.set_details(ss.str());
			m_jc.sendResult(*param);
			terminate(30);
		}
		int reg_offset = param->msg.register_offset();
		RegisterManager& rm = simulator.getRegisterManager();
		Register *reg_target = rm.getRegister(reg_offset - 1);
		regdata_t data = reg_target->getData();
		regdata_t newdata = data ^ (1 << bit_offset);
		reg_target->setData(newdata);

		// do the logging in case everything worked out
		logInjection();
		log << "register data: 0x" << hex << ((int) data) << " -> 0x"
				<< ((int) newdata) << endl;
	} else if (exp_type == param->msg.IDCFLIP) {
		// this is a twisted one

		// initial definitions
		bxInstruction_c *currInstr = simulator.getCurrentInstruction();
		unsigned length_in_bits = currInstr->ilen() << 3;

		// get the instruction in plain text and inject the error there
		// Note: we need to fetch some extra bytes into the array
		// in case the faulty instruction is interpreted to be longer
		// than the original one
		Bit8u curr_instr_plain[MAX_INSTR_BYTES];
		const Bit8u *addr = calculateInstructionAddress();
		memcpy(curr_instr_plain, addr, MAX_INSTR_BYTES);

		// CampaignManager has no idea of the instruction length
		// (neither do we), therefore this small adaption
		bit_offset %= length_in_bits;
		param->msg.set_bit_offset(bit_offset);

		// do some access calculation
		int byte_index = bit_offset >> 3;
		Bit8u bit_index = bit_offset & 7;

		// apply the fault
		curr_instr_plain[byte_index] ^= 1 << bit_index;

		// decode the instruction
		bxInstruction_c bochs_instr;
		memset(&bochs_instr, 0, sizeof(bxInstruction_c));
		fetchInstruction(simulator.getCPUContext(), curr_instr_plain,
				&bochs_instr);

		// inject it
		injectInstruction(currInstr, &bochs_instr);

		// do the logging
		logInjection();
	} else if (exp_type == param->msg.RATFLIP) {
		ud_type_t which = UD_NONE;
		unsigned rnd = 0;
		Udis86 udis(injection_ip);
		do {
			bxInstruction_c *currInstr = simulator.getCurrentInstruction();
			udis.setInputBuffer(calculateInstructionAddress(), currInstr->ilen());
			if (!udis.fetchNextInstruction()) {
				param->msg.set_resulttype(param->msg.UNKNOWN);
				param->msg.set_resultdata(
						simulator.getRegisterManager().getInstructionPointer());
				param->msg.set_output(sanitised(output.c_str()));

				stringstream ss;
				ss << "Could not decode instruction using UDIS86";
				param->msg.set_details(ss.str());
				m_jc.sendResult(*param);
				terminate(32);
			}
			ud_t _ud = udis.getCurrentState();

			/* start Bjoern Doebel's code (slightly modified) */
			/* ============================================== */
			unsigned opcount     = 0;
			unsigned operands[4] = { ~0U, ~0U, ~0U, ~0U };
			enum {
				RAT_IDX_MASK   = 0x0FF,
				RAT_IDX_OFFSET = 0x100
			};

			for (unsigned i = 0; i < 3; ++i) {
				/*
				 * Case 1: operand is a register
				 */
				if (_ud.operand[i].type == UD_OP_REG) {
					operands[opcount++] = i;
				} else if (_ud.operand[i].type == UD_OP_MEM) {
					/*
					 * Case 2: operand is memory op.
					 *
					 * In this case, we may have 2 registers involved for the
					 * index-scale address calculation.
					 */
					if (_ud.operand[i].base != 0)  // 0 if hard-wired mem operand
						operands[opcount++] = i;
					if (_ud.operand[i].index != 0)
						operands[opcount++] = i + RAT_IDX_OFFSET;
				}
			}

			if (opcount == 0) {
				// try the next instruction
				singleStep(true);
			} else {
				// assign the necessary variables
				rnd = rand() % opcount;

				if (operands[rnd] > RAT_IDX_OFFSET) {
					which = _ud.operand[operands[rnd] - RAT_IDX_OFFSET].index;
				} else {
					which = _ud.operand[operands[rnd]].base;
				}
			}
			/* ============================================ */
			/* end Bjoern Doebel's code (slightly modified) */

		} while (which == UD_NONE &&
				 simulator.getRegisterManager().getInstructionPointer() != L4SYS_FUNC_EXIT);

		if (simulator.getRegisterManager().getInstructionPointer() == L4SYS_FUNC_EXIT) {
			param->msg.set_resulttype(param->msg.UNKNOWN);
			param->msg.set_resultdata(
					simulator.getRegisterManager().getInstructionPointer());
			param->msg.set_output(sanitised(output.c_str()));

			stringstream ss;
			ss << "Reached the end of the experiment without finding an appropriate instruction";
			param->msg.set_details(ss.str());
			m_jc.sendResult(*param);
			terminate(33);
		}

		// store the real injection point
		param->msg.set_injection_ip(simulator.getRegisterManager().getInstructionPointer());

		// so we are able to flip the associated registers
		// for details on the algorithm, see Bjoern Doebel's SWIFI/RATFlip class

		// some declarations
		GPRegisterId bochs_reg = Udis86::udisGPRToFailBochsGPR(which);
		int exchg_reg = -1;
		RegisterManager &rm = simulator.getRegisterManager();

		// first, decide if the fault hits a register bound to this thread
		// (ten percent chance)
		if (rand() % 10 == 0) {
			// assure exchange of registers
			exchg_reg = rand() % 7;
			if (exchg_reg == bochs_reg)
				exchg_reg++;

		}

		// prepare the fault
		regdata_t data = rm.getRegister(bochs_reg)->getData();
		if (rnd > 0) {
			//input register - do the fault injection here
			regdata_t newdata = 0;
			if (exchg_reg >= 0) {
				// the data is taken from a process register chosen before
				newdata = rm.getRegister(exchg_reg)->getData();
			} else {
				// the data comes from an uninitialised register
				newdata = rand();
			}
			rm.getRegister(bochs_reg)->setData(newdata);
		}

		// execute the instruction
		singleStep(true);

		// restore the register if we are still in the thread
		if (rnd == 0) {
			// output register - do the fault injection here
			if (exchg_reg >= 0) {
				// write the result into the wrong local register
				regdata_t newdata = rm.getRegister(bochs_reg)->getData();
				rm.getRegister(exchg_reg)->setData(newdata);
			}
			// otherwise, just assume it is stored in an unused register
		}
		// restore the actual value of the register
		// in reality, it would never have been overwritten
		rm.getRegister(bochs_reg)->setData(data);

		// log the injection
		logInjection();

	} else if (exp_type == param->msg.ALUINSTR) {
		static BochsALUInstructions aluInstrObject(aluInstructions, aluInstructionsSize);
		// find the closest ALU instruction after the current IP

		bxInstruction_c *currInstr;
		while (!aluInstrObject.isALUInstruction(
		       currInstr = simulator.getCurrentInstruction()) &&
			   simulator.getRegisterManager().getInstructionPointer() != L4SYS_FUNC_EXIT) {
			singleStep(true);
		}

		if (simulator.getRegisterManager().getInstructionPointer() == L4SYS_FUNC_EXIT) {
			param->msg.set_resulttype(param->msg.UNKNOWN);
			param->msg.set_resultdata(
					simulator.getRegisterManager().getInstructionPointer());
			param->msg.set_output(sanitised(output.c_str()));

			stringstream ss;
			ss << "Reached the end of the experiment without finding an appropriate instruction";
			param->msg.set_details(ss.str());
			m_jc.sendResult(*param);
			terminate(33);
		}

		// store the real injection point
		param->msg.set_injection_ip(simulator.getRegisterManager().getInstructionPointer());

		// now exchange it with a random equivalent
		bxInstruction_c newInstr;
		string details;
		aluInstrObject.randomEquivalent(newInstr, details);
		if (memcmp(&newInstr, currInstr, sizeof(bxInstruction_c)) == 0) {
			// something went wrong - exit experiment
			param->msg.set_resulttype(param->msg.UNKNOWN);
			param->msg.set_resultdata(
					simulator.getRegisterManager().getInstructionPointer());
			param->msg.set_output(sanitised(output.c_str()));

			ostringstream oss;
			oss << "Did not hit an ALU instruction - correct the source code please!";
			param->msg.set_details(oss.str());
			m_jc.sendResult(*param);
			terminate(40);
		}
		// record information on the new instruction
		param->msg.set_details(details);

		// inject it
		injectInstruction(currInstr, &newInstr);

		// do the logging
		logInjection();
	}

	// aftermath
	BPSingleListener ev_done(L4SYS_FUNC_EXIT, L4SYS_ADDRESS_SPACE);
	simulator.addListener(&ev_done);
	unsigned instr_left = L4SYS_NUMINSTR - instr_offset;
	BPSingleListener ev_incomplete(ANY_ADDR, L4SYS_ADDRESS_SPACE);
	ev_incomplete.setCounter(
	    static_cast<unsigned>(instr_left * 1.1));
	simulator.addListener(&ev_incomplete);
	TimerListener ev_timeout(calculateTimeout(instr_left));
	simulator.addListener(&ev_timeout);

	//do not discard output recorded so far
	BaseListener *ev = waitIOOrOther(false);

	/* copying a string object that contains control sequences
	 * unfortunately does not work with the library I am using,
	 * which is why output is passed on as C string and
	 * the string compare is done on C strings
	 */
	if (ev == &ev_done) {
		if (strcmp(output.c_str(), golden_run.c_str()) == 0) {
			log << "Result DONE" << endl;
			param->msg.set_resulttype(param->msg.DONE);
		} else {
			log << "Result WRONG" << endl;
			param->msg.set_resulttype(param->msg.WRONG);
			param->msg.set_output(sanitised(output.c_str()));
		}
	} else if (ev == &ev_incomplete) {
		log << "Result INCOMPLETE" << endl;
		param->msg.set_resulttype(param->msg.INCOMPLETE);
		param->msg.set_resultdata(
				simulator.getRegisterManager().getInstructionPointer());
		param->msg.set_output(sanitised(output.c_str()));
	} else if (ev == &ev_timeout) {
		log << "Result TIMEOUT" << endl;
		param->msg.set_resulttype(param->msg.TIMEOUT);
		param->msg.set_resultdata(
				simulator.getRegisterManager().getInstructionPointer());
		param->msg.set_output(sanitised(output.c_str()));
	} else {
		log << "Result WTF?" << endl;
		param->msg.set_resulttype(param->msg.UNKNOWN);
		param->msg.set_resultdata(
				simulator.getRegisterManager().getInstructionPointer());
		param->msg.set_output(sanitised(output.c_str()));

		stringstream ss;
		ss << "eventid " << ev << " EIP "
				<< simulator.getRegisterManager().getInstructionPointer();
		param->msg.set_details(ss.str());
	}

	m_jc.sendResult(*param);
#endif

	terminate(0);

	// experiment successfully conducted
	return true;
}
