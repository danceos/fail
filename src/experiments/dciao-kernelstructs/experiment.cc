#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>


#include <stdlib.h>
#include "experiment.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include <string>
#include <vector>
#include <set>


#include "campaign.hpp"
#include "dciao_kernel.pb.h"

using namespace std;
using namespace fail;

#define SAFESTATE (1)

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
	!defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

unsigned DCIAOKernelStructs::injectBitFlip(address_t data_address, unsigned bitpos){

	MemoryManager& mm = simulator.getMemoryManager();
	unsigned int value, injectedval;

	value = mm.getByte(data_address);
	injectedval = value ^ (1 << bitpos);
	mm.setByte(data_address, injectedval);

	m_log << "INJECTION at: 0x" << hex	<< setw(2) << setfill('0') << data_address
		  << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval << endl;

	return value;
}


void handleEvent(DCIAOKernelProtoMsg_Result& result, DCIAOKernelProtoMsg_Result_ResultType restype, const std::string &msg) {
	cout << msg << endl;
	result.set_resulttype(restype);
	result.set_details(msg);
}

// void handleMemoryAccessEvent(DCIAOKernelExperimentData& param, const fail::MemAccessListener& l_mem) {
//	   stringstream sstr;
//	   sstr << "mem access (";
//	   switch (l_mem.getTriggerAccessType()) {
//	   case MemAccessEvent::MEM_READ:
//		   sstr << "r";
//		   break;
//	   case MemAccessEvent::MEM_WRITE:
//		   sstr << "w";
//		   break;
//	   default: break;
//	   }
//	   sstr << ") @ 0x" << hex << l_mem.getTriggerAddress();

//	   sstr << " ip @ 0x" << hex << l_mem.getTriggerInstructionPointer();

//	   handleEvent(param, param.msg.ERR_MEMACCESS, sstr.str());
// }

DCIAOKernelStructs::time_markers_t *DCIAOKernelStructs::getTimeMarkerList() {
	const ElfSymbol & sym_time_marker_index = m_elf.getSymbol("time_marker_index");
	const ElfSymbol & sym_time_markers		= m_elf.getSymbol("time_markers");

	assert(sym_time_marker_index.isValid());
	assert(sym_time_markers.isValid());


	unsigned int time_marker_index;

	simulator.getMemoryManager().getBytes(sym_time_marker_index.getAddress(),
										  sym_time_marker_index.getSize(),
										  &time_marker_index);
	if (time_marker_index > 500) {
		time_marker_index = 500;
	}
	time_markers_t *time_markers = new time_markers_t(time_marker_index);

	simulator.getMemoryManager().getBytes(sym_time_markers.getAddress(),
										  time_marker_index * sizeof(time_marker),
										  time_markers->data());
	return time_markers;
}

int DCIAOKernelStructs::time_markers_compare(const time_markers_t &a, const time_markers_t &b) {
	int pos = -1;
	unsigned max_index = std::min(a.size(), b.size());
	for (unsigned i = 0; i < max_index; i++) {
		if (a[i].time != b[i].time
			|| a[i].at != b[i].at) {
			pos = i;
			break;
		}
	}
	if (pos == -1 && (a.size() != b.size())) {
		pos = max_index;
	}
	return pos;
}



bool DCIAOKernelStructs::run() {
	//******* Boot, and store state *******//
	m_log << "STARTING EXPERIMENT" << endl;

	char * statedir = getenv("FAIL_STATEDIR");
	if(statedir == NULL){
		m_log << "FAIL_STATEDIR not set :(" << std::endl;
		simulator.terminate(1);
	}

	address_t minimal_ip = 0x100000; // 1 Mbyte
	address_t minimal_data = 0x100000; // 1 Mbyte
	address_t maximal_ip = 0;
	address_t maximal_data = 0;
	for (ElfReader::symbol_iterator it = m_elf.sym_begin();
		 it != m_elf.sym_end(); ++it) {
		const ElfSymbol &symbol = *it;
		if (symbol.getSymbolType() == STT_FUNC
			|| symbol.getSymbolType() == STT_GNU_IFUNC	/*indirect codeasm object */) {
			maximal_ip = std::max(maximal_ip, symbol.getEnd());
		}
		maximal_data = std::max(maximal_data, symbol.getEnd());
	}

	std::cout << "Code section from " << hex << minimal_ip << " to " << maximal_ip << std::endl;
	std::cout << "Data section from " << hex << minimal_ip << " to " << maximal_data << std::endl;

	m_log << "Booting, and saving state at main";
	BPSingleListener bp;

	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(m_elf.getSymbol("main").getAddress());
	if(simulator.addListenerAndResume(&bp) == &bp){
		m_log << "main function entry reached, saving state" << endl;
	} else {
		m_log << "Couldn't reach entry function. Dying" << std::endl;
		simulator.terminate(1);
	}

	simulator.save(statedir);
	guest_address_t enter_kernel_address = m_elf.getSymbol("os::dep::KernelStructs::correct").getAddress();
	BPSingleListener l_enter_kernel(enter_kernel_address);
	BPSingleListener l_time_marker_print(m_elf.getSymbol("time_marker_print").getAddress());


	simulator.clearListeners();
	simulator.addListener(&l_enter_kernel);
	simulator.addListener(&l_time_marker_print);

	bool in_kernelspace = false;
	unsigned kernel_activations = 0;
	while (1) {
		fail::BaseListener *l = simulator.resume();
		simulator.addListener(l);

		if (l == &l_time_marker_print) {
			break;
		} else if (l == &l_enter_kernel) {
			kernel_activations ++;
		} else {
			m_log << "THIS SHOULD'T HAPPEN" << std::endl;
			simulator.terminate(1);
		}

	}

	correct.time_markers = getTimeMarkerList();
	correct.kernel_activation_count = kernel_activations;

	assert(kernel_activations > 0);
	assert(correct.time_markers->size() > 0);

	m_log << "correct run is done:" << dec << std::endl;
	m_log << "	kernel_transitions " << correct.kernel_activation_count << std::endl;
	m_log << "	time_markers	   " << correct.time_markers->size()	 << std::endl;

	// //******* Fault injection *******//
	// // #warning "Building restore state variant"

	unsigned executed_jobs = 0;

	while (executed_jobs < 25 || m_jc.getNumberOfUndoneJobs() > 0) {
		m_log << "asking jobserver for parameters" << endl;
		DCIAOKernelExperimentData param;
		if(!m_jc.getParam(param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();

		for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
			// 8 results in one job
			DCIAOKernelProtoMsg_Result *result = param.msg.add_result();
			result->set_bitoffset(bit_offset);

			m_log << "restoring state" << endl;

			// Restore to the image, which starts at address(main)
			simulator.restore(statedir);
			executed_jobs ++;

			kernel_activations = 0;

			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;


			if (injection_instr > 0) {
				simulator.clearListeners();
				// XXX could be improved with intermediate states (reducing runtime until injection)
				simulator.addListener(&l_time_marker_print);
				simulator.addListener(&l_enter_kernel);

				bp.setWatchInstructionPointer(ANY_ADDR);
				bp.setCounter(injection_instr);
				simulator.addListener(&bp);

				bool inject = true;
				while (1) {
					fail::BaseListener * listener = simulator.resume();
					// finish() before FI?
					if (listener == &l_time_marker_print) {
						m_log << "experiment reached finish() before FI" << endl;
						handleEvent(*result, result->NOINJECTION, "time_marker reached before instr2");
						inject = false;
						break;
					} else if (listener == &l_enter_kernel) {
						// Count all kernel activations
						simulator.addListener(&l_enter_kernel);
						kernel_activations++;
					} else if (listener == &bp) {
						break;
					} else {
						inject = false;
						handleEvent(*result, result->NOINJECTION, "WTF");
						break;
					}
				}

				// Next experiment
				if (!inject)
					continue;
			}

			// Not a working sanitiy check. Because of instruction
			// offsets!
			// 
			// if (simulator.getCPU(0).getInstructionPointer() != param.msg.fsppilot().instr2_absolute()) {
			// 	m_log << "Invalid Injection address EIP=0x" 
			// 		  << std::hex << simulator.getCPU(0).getInstructionPointer()
			// 		  << " != enter_kernel=0x" << param.msg.fsppilot().instr2_absolute() << std::endl;
			// 	simulator.terminate(1);
			// }

			/// INJECT BITFLIP:
			result->set_original_value(injectBitFlip(data_address, bit_offset));

			// // Setup exit points
			BPSingleListener l_error_hook(m_elf.getSymbol("copter_mock_panic").getAddress());

			TrapListener l_trap(ANY_TRAP);
			TimerListener l_timeout(1000 * 1000); // 1 second in microseconds

			simulator.clearListeners();
			simulator.addListener(&l_enter_kernel);
			simulator.addListener(&l_timeout);
			simulator.addListener(&l_trap);
			simulator.addListener(&l_time_marker_print);
			simulator.addListener(&l_error_hook);

			// jump outside text segment
			BPRangeListener ev_below_text(ANY_ADDR, minimal_ip - 1);
			BPRangeListener ev_beyond_text(maximal_ip + 1, ANY_ADDR);
			simulator.addListener(&ev_below_text);
			simulator.addListener(&ev_beyond_text);

			// memory access outside of bound determined in the golden run [lowest_addr, highest_addr]
			MemAccessListener ev_mem_low(0x0, MemAccessEvent::MEM_READWRITE);
			ev_mem_low.setWatchWidth(minimal_data);
			MemAccessListener ev_mem_high(maximal_data + 1, MemAccessEvent::MEM_READWRITE);
			ev_mem_high.setWatchWidth(0xFFFFFFFFU - (maximal_data + 1));
			simulator.addListener(&ev_mem_low);
			simulator.addListener(&ev_mem_high);


			// resume and wait for results while counting kernel
			// activations
			fail::BaseListener* l;
			while (1) {
				l = simulator.resume();

				// Evaluate result
				if (l == &l_enter_kernel) {
					kernel_activations++;
					simulator.addListener(&l_enter_kernel);
					// continue experiment
				} else if (l == &l_time_marker_print) {
					m_log << "experiment ran to the end" << std::endl;
					DCIAOKernelStructs::time_markers_t * time_markers = getTimeMarkerList();
					int pos = time_markers_compare(*time_markers, *correct.time_markers);
					if (pos != -1) {
						m_log << "Different activation scheme" << std::endl;
						m_log << "	size " << std::dec << time_markers->size() << std::endl;
						m_log << "	at	 " << std::dec << pos << std::endl;

						stringstream sstr;
						sstr << "diff after #" << pos;
						handleEvent(*result, result->ERR_DIFFERENT_ACTIVATION, sstr.str());
						/* In case of an error append the activation scheme */
						for (unsigned i = pos; i < time_markers->size(); ++i) {
							result->add_activation_scheme( (*time_markers)[i].time );
							result->add_activation_scheme( (*time_markers)[i].at );
						}
					} else if (kernel_activations != correct.kernel_activation_count) {
						stringstream sstr;
						sstr << "kernel activations " << kernel_activations << " (expt: " << correct.kernel_activation_count << ")";
						handleEvent(*result, result->ERR_DIFFERENT_KERNEL_TRANSITIONS, sstr.str());
					} else {
						stringstream sstr;
						sstr << "calc done (kernel #" << kernel_activations << ")";
						handleEvent(*result, result->OK, sstr.str());
					}
					delete time_markers;
					// End of experiment
					break;
				}  else if (l == &l_trap) {
					stringstream sstr;
					sstr << "trap #" << l_trap.getTriggerNumber();
					handleEvent(*result, result->TRAP, sstr.str());
					break; // EOExperiment
				} else if (l == &l_timeout){
					handleEvent(*result, result->TIMEOUT, "timeout: 1 second");
					break; // EOExperiment
				} else if (l == &l_error_hook){
					handleEvent(*result, result->ERR_ERROR_HOOK, "called error hook");
					break; // EOExperiment
				} else if (l == &ev_below_text || l == &ev_beyond_text) {
					handleEvent(*result, result->ERR_OUTSIDE_TEXT, (l == &ev_below_text) ? "< .text" : ">.text");
					break; // EOExperiment
				} else if (l == &ev_mem_low || l == &ev_mem_high) {
					handleEvent(*result, result->ERR_MEMACCESS, (l == &ev_mem_low) ? "< .data" : ">.data");
					break; // EOFExperiment
				} else {
					handleEvent(*result, result->UNKNOWN, "UNKNOWN event");
					break; // EOExperiment
				}
			}

			simulator.clearListeners();
		} // injection done, continue with next bit

		m_jc.sendResult(param);
	} // end while (1)

	// Explicitly terminate, or the simulator will continue to run.
	 simulator.terminate();
}

