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


void handleEvent(DCIAOKernelProtoMsg_Result& result,
				 DCIAOKernelProtoMsg_Result_ResultType restype, 
				 simtime_t time_of_fail,
				 const std::string &msg) {
	cout << msg << endl;
	result.set_fail_time(time_of_fail);
	result.set_resulttype(restype);
	result.set_details(msg);
}

// std::string handleMemoryAccessEvent(fail::MemAccessListener& l_mem) {
// 	   stringstream sstr;
// 	   sstr << "mem access (";
// 	   switch (l_mem.getTriggerAccessType()) {
// 	   case MemAccessEvent::MEM_READ:
// 		   sstr << "r";
// 		   break;
// 	   case MemAccessEvent::MEM_WRITE:
// 		   sstr << "w";
// 		   break;
// 	   default: break;
// 	   }
// 	   sstr << ") @ 0x" << hex << l_mem.getTriggerAddress();

// 	   sstr << " ip @ 0x" << hex << l_mem.getTriggerInstructionPointer();

// 	   return sstr.str();
// }

DCIAOKernelStructs::time_markers_t DCIAOKernelStructs::getTimeMarkerList() {
	std::ifstream task_activation("task_activation");
	if (!task_activation) {
		m_log << "could not open `task_activation'" << std::endl;
		simulator.terminate(-1);
	}

	DCIAOKernelStructs::time_markers_t ret;

	while (task_activation) {
		uint32_t time, at;
		task_activation >> time >> at;
		time_marker marker;
		marker.time = time;
		marker.at = at;
		if (!task_activation)
			break;
		ret.push_back(marker);
	}

	return ret;
}

int DCIAOKernelStructs::time_markers_compare(const time_markers_t &a, const time_markers_t &b) {
	int pos = -1;
	unsigned max_index = std::min(a.size(), b.size());
	for (unsigned i = 0; i < max_index; i++) {
		if (a[i].time != b[i].time || a[i].at != b[i].at) {
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
	MemoryManager& mm = simulator.getMemoryManager();
	address_t minimal_ip = INT_MAX; // 1 Mbyte
	address_t maximal_ip = 0;
    address_t minimal_data = 0x100000; // 1 Mbyte
    address_t maximal_data = 0;

	for (ElfReader::section_iterator it = m_elf.sec_begin();
		 it != m_elf.sec_end(); ++it) {
		const ElfSymbol &symbol = *it;
        std::string prefix(".text");
        if (symbol.getName().compare(0, prefix.size(), prefix) == 0) {
            minimal_ip = std::min(minimal_ip, symbol.getStart());
            maximal_ip = std::max(maximal_ip, symbol.getEnd());
        } else {
            minimal_data = std::min(minimal_data, symbol.getStart());
            maximal_data = std::max(maximal_data, symbol.getEnd());
        }
	}

	std::cout << "Code section from " << hex << minimal_ip << " to " << maximal_ip << std::endl;
	std::cout << "Whole programm section from " << hex << minimal_data << " to " << maximal_data << std::endl;

	//******* Boot, and store state *******//
	m_log << "STARTING EXPERIMENT" << endl;

	/* Read time Markers from list */
	DCIAOKernelStructs::time_markers_t correct_time_markers = getTimeMarkerList();
	m_log << "correct run is done:" << dec << std::endl;
	m_log << "	time_markers	   " << correct_time_markers.size()	 << std::endl;

	// //******* Fault injection *******//

	const ElfSymbol &s_time_marker_print = m_elf.getSymbol("time_marker_print");
	assert(s_time_marker_print.isValid());
	BPSingleListener l_time_marker_print(s_time_marker_print.getAddress());

	const ElfSymbol &s_fail_virtual_port = m_elf.getSymbol("fail_virtual_port");
	assert(s_fail_virtual_port.isValid());
	MemAccessListener l_fail_virtual_port(s_fail_virtual_port.getAddress());


	BPSingleListener bp;
	unsigned executed_jobs = 0;

	while (executed_jobs < 25 || m_jc.getNumberOfUndoneJobs() > 0) {
		m_log << "asking jobserver for parameters" << endl;
		DCIAOKernelExperimentData param;
		if(!m_jc.getParam(param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(99);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();

		for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
			// 8 results in one job
			DCIAOKernelProtoMsg_Result *result = param.msg.add_result();
			result->set_bitoffset(bit_offset);
			result->set_invalid_memaccess_read(false);
			result->set_invalid_memaccess_write(false);
			result->set_invalid_jump(false);

			m_log << "restoring state" << endl;

			// Restore to the image, which starts at address(main)
			simulator.restore("state");
			executed_jobs ++;

			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;

			DCIAOKernelStructs::time_markers_t recorded_time_markers;

			if (injection_instr > 0) {
				simulator.clearListeners();
				// XXX could be improved with intermediate states (reducing runtime until injection)
				simulator.addListener(&l_time_marker_print);

				bp.setWatchInstructionPointer(ANY_ADDR);
				bp.setCounter(injection_instr);
				simulator.addListener(&bp);

				// Add vport listener
				simulator.addListener(&l_fail_virtual_port);


				bool inject = true;
				while (1) {
					fail::BaseListener * listener = simulator.resume();
					// finish() before FI?

					// Count kernel activations
					if (listener == &l_time_marker_print) {
						m_log << "experiment reached finish() before FI" << endl;
						handleEvent(*result, result->NOINJECTION,
									0,
									"time_marker reached before injection_instr");
						inject = false;
						break;
					} else if (listener == &l_fail_virtual_port) {
						simulator.addListener(&l_fail_virtual_port);
						assert(l_fail_virtual_port.getTriggerAccessType() == MemAccessEvent::MEM_WRITE);
						unsigned int fail_virtual_port_data;
						mm.getBytes(s_fail_virtual_port.getAddress(), 4, &fail_virtual_port_data);
						uint32_t at = fail_virtual_port_data >> 16;
						uint32_t time = fail_virtual_port_data & 0xffff;
						DCIAOKernelStructs::time_marker marker;
						assert(at != 0xffff);
						marker.at = at;
						marker.time = time;
						recorded_time_markers.push_back(marker);
						continue; // fast forward
					} else if (listener == &bp) {
						break;
					} else {
						inject = false;
						handleEvent(*result, result->NOINJECTION, 0, "WTF");
						break;
					}
				}

				// Next experiment
				if (!inject)
					continue;
			}

			// Not a working sanitiy check. Because of instruction
			// offsets!
			if (param.msg.fsppilot().has_injection_instr_absolute()) {
				address_t PC = param.msg.fsppilot().injection_instr_absolute();
				if (simulator.getCPU(0).getInstructionPointer() != PC) {
					m_log << "Invalid Injection address EIP=0x"
						  << std::hex << simulator.getCPU(0).getInstructionPointer()
						  << " != injection_instr_absolute=0x" << PC << std::endl;
					simulator.terminate(1);
				}
			}

			/// INJECT BITFLIP:
			result->set_original_value(injectBitFlip(data_address, bit_offset));

			TrapListener l_trap(ANY_TRAP);
			unsigned int i_timeout = 100 * 1000;
			TimerListener l_timeout(i_timeout); // miliseconds in
			// microseconds
			TimerListener l_timeout_hard(5 * 1000 * 1000); // miliseconds in
			// microseconds

			simulator.clearListeners();
			simulator.addListener(&l_timeout);
			simulator.addListener(&l_timeout_hard);
			simulator.addListener(&l_trap);
			simulator.addListener(&l_time_marker_print);

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

			// add listener for virtual fail port
			simulator.addListener(&l_fail_virtual_port);

			// resume and wait for results while counting kernel
			// activations
			fail::BaseListener* l;
			time_t last_activity = time(NULL);
			unsigned int last_timer;
			while (1) {
				l = simulator.resume();
				if (l == &l_fail_virtual_port) {
					simulator.addListener(l);
					/* Virtual Port monitoring */
					if(!l_fail_virtual_port.getTriggerAccessType() == MemAccessEvent::MEM_WRITE) {
						handleEvent(*result, result->TRAP,
									simulator.getTimerTicks(),
									"Invalid read from vport");
						break;
					}
					unsigned int fail_virtual_port_data;
					mm.getBytes(s_fail_virtual_port.getAddress(), 4, &fail_virtual_port_data);
					uint32_t at = fail_virtual_port_data >> 16;
					uint32_t timer = fail_virtual_port_data & 0xffff;
					if (at == 0xffff) {
						/* Called error hook */
						handleEvent(*result, result->ERR_ERROR_HOOK,
									simulator.getTimerTicks(),
									"called error hook");
						break;
					} else {
						/* Reset timeout listener */
						if (last_timer != timer) {
							last_activity = time(NULL);
							m_log << "Reset timeout @" << dec << at << " #" << timer << std::endl;
						}
						last_timer = timer;

						DCIAOKernelStructs::time_marker marker;
						marker.at = at;
						marker.time = timer;

						// m_log << "Marker " << at << " " << time << " " << l_fail_virtual_port.getTriggerInstructionPointer() << std::endl;
						recorded_time_markers.push_back(marker);
						continue; // till crash
					}
				} else if (l == &l_time_marker_print) {
					m_log << "experiment ran to the end" << std::endl;
					int pos = time_markers_compare(correct_time_markers, recorded_time_markers);
					if (pos != -1) {
						m_log << "Different activation scheme" << std::endl;
						m_log << "	size " << std::dec << recorded_time_markers.size() << std::endl;
						m_log << "	at	 " << std::dec << pos << std::endl;

						stringstream sstr;
						sstr << "diff after #" << pos;
						handleEvent(*result, result->ERR_DIFFERENT_ACTIVATION,
									simulator.getTimerTicks(),
									sstr.str());
						// In case of an error append the activation scheme
						for (unsigned i = 0; i < recorded_time_markers.size(); ++i) {
							result->add_activation_scheme( recorded_time_markers[i].time );
							result->add_activation_scheme( recorded_time_markers[i].at );

							m_log << i << " "
								  << recorded_time_markers[i].time << " " <<  recorded_time_markers[i].at <<  " "
								  << correct_time_markers[i].time << " " <<  correct_time_markers[i].at
								  << std::endl;
						}
					} else {
						stringstream sstr;
						sstr << "calc done (markers #" << correct_time_markers.size() << ")";
						handleEvent(*result, result->OK,
									simulator.getTimerTicks(),
									sstr.str());
					}
					// End of experiment
					break;
				}  else if (l == &l_trap) {
					stringstream sstr;
					sstr << "trap #" << l_trap.getTriggerNumber();
					handleEvent(*result, result->TRAP,
								simulator.getTimerTicks(),
								sstr.str());
					break; // EOExperiment
				} else if (l == &l_timeout || l == &l_timeout_hard){
					time_t current_time = time(NULL);
					if ((l == &l_timeout_hard) || (current_time - last_activity) > 4) {
						handleEvent(*result, result->TIMEOUT,
									simulator.getTimerTicks(),
									"timeout: 5 second");
						break; // EOExperiment
					} else {
						static int delta = 0;
						if (delta != current_time) {
							m_log << "Wait another n = " << (current_time - last_activity) << std::endl;
							delta = current_time;
						}
						// Wait another i_timeout simulator time
						simulator.removeListener(&l_timeout);
						simulator.addListener(&l_timeout);
						assert(l_timeout.getTimeout() == i_timeout);
						continue; // till crash
					}
				} else if (l == &ev_below_text || l == &ev_beyond_text) {
					/* Jump outside text segment. We do only note this
					   event and */
					result->set_invalid_jump(true);
					continue; // till crash
				} else if (l == &ev_mem_low || l == &ev_mem_high) {
					/* The target has done an access outside the
					   memory segment. Note this and continue; */
					if (((MemAccessListener *)l)->getTriggerAccessType() == MemAccessEvent::MEM_READ) {
						result->set_invalid_memaccess_read(true);
					} else {
						result->set_invalid_memaccess_write(true);
					}
					continue; //till crash
				} else {
					handleEvent(*result, result->UNKNOWN, 0,
								"UNKNOWN event");
					break; // EOExperiment
				}
			}

			simulator.clearListeners();
		} // injection done, continue with next bit

		m_jc.sendResult(param);
	} // end while (1)

	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate(0);
}

