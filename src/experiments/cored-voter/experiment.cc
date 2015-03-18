#include <assert.h>
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
#include <algorithm>

#include "util/llvmdisassembler/LLVMtoFailTranslator.hpp"
#include "util/llvmdisassembler/LLVMtoFailBochs.hpp"
#include "campaign.hpp"
#include "cored_voter.pb.h"

using namespace std;
using namespace fail;

#define SAFESTATE (1)

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
	!defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE) || !defined(CONFIG_EVENT_TRAP)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

void CoredVoter::redecodeCurrentInstruction() {
	/* Flush Instruction Caches and Prefetch queue */
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	cpu_context->invalidate_prefetch_q();
	cpu_context->iCache.flushICacheEntries();


	guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
	bxInstruction_c *currInstr = simulator.getCurrentInstruction();

	m_log << "REDECODE INSTRUCTION!" << endl;

	Bit32u eipBiased = pc + cpu_context->eipPageBias;
	Bit8u  instr_plain[32];

	MemoryManager& mm = simulator.getMemoryManager();
	mm.getBytes(pc, 32, instr_plain);

	unsigned remainingInPage = cpu_context->eipPageWindowSize - eipBiased;
	int ret;
#if BX_SUPPORT_X86_64
	if (cpu_context->cpu_mode == BX_MODE_LONG_64)
		ret = cpu_context->fetchDecode64(instr_plain, currInstr, remainingInPage);
	else
#endif
		ret = cpu_context->fetchDecode32(instr_plain, currInstr, remainingInPage);
	if (ret < 0) {
		// handle instrumentation callback inside boundaryFetch
		cpu_context->boundaryFetch(instr_plain, remainingInPage, currInstr);
	}

}

/** generate_random_bits does generate a vector of <count> bit numbers for a
	<register_width> wide register. All contained bitnumbers are
	unique, and the resulting vector is sorted in ascending order. The
	ordering makes the vectors easily comparable. */
static
std::vector<unsigned char> generate_randoms_bits(int register_width, unsigned int count) {
    std::vector<unsigned char> ret;
    while (ret.size() != count) {
    another_number:
        unsigned char candidate = rand() & ((1<< register_width)-1);
            for (std::vector<unsigned char>::const_iterator it = ret.begin(); it != ret.end(); ++it) {
            if (*it == candidate)
                goto another_number;
        }
        ret.push_back(candidate);
    }

	// Sort vector to avoid duplicates
	std::sort(ret.begin(), ret.end());

    return ret;
}



typedef std::pair<unsigned char, unsigned char> two_bit_error_t;
std::vector<two_bit_error_t> generate_two_bit_errors(int register_width_in_bits) {
    std::vector<two_bit_error_t> ret;
    for (unsigned char x = 0; x < register_width_in_bits; x++) {
        for (unsigned char y = 0; y < x; y++) {
            ret.push_back(std::make_pair(y, x));
        }
    }
    return ret;
}


unsigned CoredVoter::injectBitFlip(address_t data_address, unsigned data_width, unsigned bitpos){

    /* First 32 Registers, this might neeed adaption */
    if (data_address < (32 << 8)) {
        LLVMtoFailTranslator * ltof =  new LLVMtoFailBochs;
        LLVMtoFailTranslator::reginfo_t reginfo = LLVMtoFailTranslator::reginfo_t::fromDataAddress(data_address, data_width);

        unsigned int value, injectedval;

        value = ltof->getRegisterContent(simulator.getCPU(0), reginfo);
        injectedval = value ^ (1 << bitpos);
        ltof->setRegisterContent(simulator.getCPU(0), reginfo, injectedval);

        m_log << "INJECTING register (" << dec << reginfo.id
			  << " offset " << (int) reginfo.offset
			  << ") bitpos: " << bitpos
              << " value: 0x" << hex << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval 
			  << dec << endl;
		if (reginfo.id == RID_PC)
			redecodeCurrentInstruction();
        delete ltof;

        return value;
    } else {
        MemoryManager& mm = simulator.getMemoryManager();
        unsigned int value, injectedval;

        value = mm.getByte(data_address);
        injectedval = value ^ (1 << bitpos);
        mm.setByte(data_address, injectedval);

        m_log << "INJECTION at: 0x" << hex	<< setw(2) << setfill('0') << data_address
              << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval << endl;

		/* If it is the current instruction redecode it */
		guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
		bxInstruction_c *currInstr = simulator.getCurrentInstruction();
		unsigned length_in_bytes = currInstr->ilen();

		if (pc <= data_address && data_address <= (pc + length_in_bytes)) {
			redecodeCurrentInstruction();
		}

        return value;
    }
}


void handleEvent(CoredVoterProtoMsg_Result& result, CoredVoterProtoMsg_Result_ResultType restype, const std::string &msg) {
	cout << msg << endl;
	result.set_resulttype(restype);
	result.set_details(msg);
}

std::string handleMemoryAccessEvent(fail::MemAccessListener& l_mem) {
	   stringstream sstr;
	   sstr << "mem access (";
	   switch (l_mem.getTriggerAccessType()) {
	   case MemAccessEvent::MEM_READ:
		   sstr << "r";
		   break;
	   case MemAccessEvent::MEM_WRITE:
		   sstr << "w";
		   break;
	   default: break;
	   }
	   sstr << ") @ 0x" << hex << l_mem.getTriggerAddress();

	   sstr << "; ip @ 0x" << hex << l_mem.getTriggerInstructionPointer();

	   return sstr.str();
}


bool CoredVoter::run() {
    //******* Boot, and store state *******//
    m_log << "STARTING EXPERIMENT" << endl;

    timeval start;
    gettimeofday(&start, NULL);
    srand(start.tv_usec);

	unsigned executed_jobs = 0;

    // Setup exit points
    const ElfSymbol &s_trace_end_marker = m_elf.getSymbol("_trace_end_marker");
    BPSingleListener l_trace_end_marker(s_trace_end_marker.getAddress());
	if (!s_trace_end_marker.isValid()) {
		m_log << "Couldn't find symbol: _trace_end_marker" << std::endl;
		simulator.terminate(1);
	}


    const ElfSymbol &s_subexperiment_end = m_elf.getSymbol("_subexperiment_end");
    BPSingleListener l_subexperiment_end(s_subexperiment_end.getAddress());
	if (!s_subexperiment_end.isValid()) {
		m_log << "Couldn't find symbol: _subexperiment_end" << std::endl;
		simulator.terminate(1);
	}

    const ElfSymbol &s_subexperiment_marker_1 = m_elf.getSymbol("_subexperiment_marker_1");
    BPSingleListener l_subexperiment_marker_1(s_subexperiment_marker_1.getAddress());
	if (!s_subexperiment_marker_1.isValid()) {
		m_log << "Couldn't find symbol: _subexperiment_marker_1" << std::endl;
		simulator.terminate(1);
	}

    const ElfSymbol &s_subexperiment_marker_1_ptr = m_elf.getSymbol("subexperiment_marker_1");
    BPSingleListener l_subexperiment_marker_1_ptr(s_subexperiment_marker_1_ptr.getAddress());
	if (!s_subexperiment_marker_1_ptr.isValid()) {
		m_log << "Couldn't find symbol: subexperiment_marker_1" << std::endl;
		simulator.terminate(1);
	}



	const ElfSymbol &s_experiment_number = m_elf.getSymbol("experiment_number");
	if (!s_experiment_number.isValid()) {
		m_log << "Couldn't find symbol: experiment_number" << std::endl;
		simulator.terminate(1);
	}

	const ElfSymbol &s_chose_right_input = m_elf.getSymbol("chose_right_input");
	if (!s_chose_right_input.isValid()) {
		m_log << "Couldn't find symbol: chose_right_input" << std::endl;
		simulator.terminate(1);
	}

	const ElfSymbol &s_chose_right_output = m_elf.getSymbol("chose_right_output");
	if (!s_chose_right_output.isValid()) {
		m_log << "Couldn't find symbol: chose_right_output" << std::endl;
		simulator.terminate(1);
	}

	const ElfSymbol &s_detected_error = m_elf.getSymbol("detected_error");
	if (!s_detected_error.isValid()) {
		m_log << "Couldn't find symbol: detected_error" << std::endl;
		simulator.terminate(1);
	}

	const ElfSymbol &s_voterImpl = m_elf.getSymbol("voterImpl");
	if (!s_voterImpl.isValid()) {
		m_log << "Couldn't find symbol: voterImpl" << std::endl;
		simulator.terminate(1);
	}






	address_t min_code = INT_MAX;
	address_t max_code = 0;
    std::vector<std::string> allowed_text_regions;
    allowed_text_regions.push_back("voterImpl");
    allowed_text_regions.push_back("Alpha::functionTaskTask0");
    allowed_text_regions.push_back("do_experiment");
    allowed_text_regions.push_back("_trace_end_marker");
    allowed_text_regions.push_back("_subexperiment_end");
    allowed_text_regions.push_back("_subexperiment_marker_1");



    for (std::vector<std::string>::iterator it = allowed_text_regions.begin();
         it != allowed_text_regions.end(); ++it) {
        const ElfSymbol &sym = m_elf.getSymbol(*it);
        if (!sym.isValid()) {
            m_log << "Couldn't find symbol: " << (*it) << std::endl;
            simulator.terminate(1);

        }
        min_code = std::min(min_code, sym.getStart());
        max_code = std::max(max_code, sym.getEnd());
    }


    m_log << "_trace_end_marker: " << std::hex << s_trace_end_marker << std::endl;
    m_log << "_subexperiment_end: " << std::hex << s_subexperiment_end << std::endl;
    m_log << "code_region: " << std::hex << min_code << " -- " << max_code << std::endl;


    TrapListener l_trap(ANY_TRAP);
    TimerListener l_timeout(1000 * 1000); // 1 second in microseconds
    BPRangeListener l_below_text(0, min_code - 1);
    BPRangeListener l_above_text(max_code + 1, 0xfffffff0);


	while (executed_jobs < 25 || m_jc.getNumberOfUndoneJobs() > 0) {
		m_log << "asking jobserver for parameters" << endl;
		CoredVoterExperimentData param;
		if(!m_jc.getParam(param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();
		unsigned data_width = param.msg.fsppilot().data_width();


		/* Detect wheter we should inject the PC and jump to a
		   random data address? */
		bool pc_injection = param.msg.fsppilot().benchmark().find("jump") != std::string::npos;
		bool random_injection = param.msg.fsppilot().benchmark().find("random") != std::string::npos;

		int randomly_injected_bits = 0;


		LLVMtoFailTranslator::reginfo_t reginfo;
		std::vector<two_bit_error_t> two_bit_errors;
		int experiments = 8;
		if (pc_injection)
			experiments = 1;
		else if (random_injection) {
			reginfo = LLVMtoFailTranslator::reginfo_t::fromDataAddress(data_address, data_width);
			experiments = 256;
			std::string benchmark = param.msg.fsppilot().benchmark();
			size_t idx = benchmark.find("random") + 7;
			std::stringstream ss(benchmark.substr(idx));
			ss >> randomly_injected_bits;

			if (randomly_injected_bits == 2) {
				two_bit_errors = generate_two_bit_errors(data_width * 8);
				experiments = two_bit_errors.size();
			}
		}

        for (int experiment_id = 0; experiment_id < experiments; ++experiment_id) {
			CoredVoterProtoMsg_Result *result = 0;

			m_log << "restoring state" << endl;

			// Restore to the image, which starts at address(main)
			simulator.restore("state");
			executed_jobs ++;

			address_t stack_pointer = simulator.getCPU(0).getStackPointer();
			m_log << "stackpointer: " << std::hex << stack_pointer << std::dec << std::endl;

            // Fast forward to injection address
			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;

			if (injection_instr > 0) {
				simulator.clearListeners();
                BPSingleListener bp;
				bp.setWatchInstructionPointer(ANY_ADDR);
				bp.setCounter(injection_instr);
				simulator.addListener(&bp);

                fail::BaseListener * listener = simulator.resume();
                if (listener != &bp) {
					result = param.msg.add_result();
                    handleEvent(*result, result->NOINJECTION, "WTF");
                    break;
                }
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

			// address_t stack_pointer = simulator.getCPU(0).getRegisterContent(simulator.getCPU(0).getRegister(RID_CSP));
			// m_log << "stack pointer: " << std::hex << stack_pointer << std::endl;
			// simulator.terminate(1);

			MemoryManager& mm = simulator.getMemoryManager();
			/* Which experiment was evaluated */
			char experiment_number = mm.getByte(s_experiment_number.getAddress());

			if (pc_injection) {
				// Jump to data address */
				address_t current_PC = simulator.getCPU(0).getInstructionPointer();
				address_t new_PC     = param.msg.fsppilot().data_address();
				m_log << "jump from 0x" << hex  << current_PC << " to 0x" << new_PC << std::endl;
				simulator.getCPU(0).setRegisterContent(simulator.getCPU(0).getRegister(RID_PC),
													   new_PC ); // set
																 // program
																 // counter
				result = param.msg.add_result();
				result->set_bitoffset(0);
				result->set_original_value(current_PC);
				redecodeCurrentInstruction();
			} else if (random_injection) {
				// Inject random bitflips
				LLVMtoFailTranslator * ltof =  new LLVMtoFailBochs;
				assert(reginfo.offset == 0);

				m_log << "Inject " << randomly_injected_bits << " bit flips" << endl;
				unsigned int ret_bitoffset = 0;

				std::vector<unsigned char> bits;
				if (randomly_injected_bits == 2) {
					m_log << two_bit_errors.size() << " " << experiment_id << endl;
					two_bit_error_t two_bit = two_bit_errors[experiment_id];
					bits.push_back(two_bit.first);
					bits.push_back(two_bit.second);
				} else {
					/* Generate a random vector */
					bits = generate_randoms_bits(5, randomly_injected_bits);
				}

				bool original_value_set = false;
				result = param.msg.add_result();
				for (std::vector<unsigned char>::const_iterator it = bits.begin(); it != bits.end(); ++it) {
					int bitoffset = *it;
					ret_bitoffset = (ret_bitoffset << 5) | bitoffset;
					int original_value = injectBitFlip(data_address, data_width, bitoffset);
					if (!original_value_set) {
						result->set_original_value(original_value);
						original_value_set = true;
					}
				}
				result->set_bitoffset(ret_bitoffset);
				delete ltof;
			} else {
				// INJECT BITFLIP into Register:
				// experiment_id == bitpos
				result = param.msg.add_result();
				result->set_bitoffset(experiment_id);
				result->set_original_value(injectBitFlip(data_address, data_width, experiment_id));
				result->set_experiment_number((unsigned int) experiment_number);

				address_t PC = simulator.getCPU(0).getInstructionPointer();
				if (PC < s_voterImpl.getStart() || PC > s_voterImpl.getEnd()) {
					handleEvent(*result, result->ERR_OUTSIDE_TEXT, "injection");
					simulator.clearListeners();
					continue; // next experiment
				}
			}

			result->set_experiment_number((unsigned int) experiment_number);


			/* We use the current stackpointer to determine a region,
			   that is used by the function. This region includes the
			   working data and the used arguments. */

			address_t min_data = stack_pointer - 152;
			address_t max_data = min_data + 0x5C;
			m_log << "data region: " << std::hex << min_data << " -- " << max_data << std::dec << std::endl;

			MemAccessListener l_below_data(0);
			l_below_data.setWatchWidth(min_data - 1);
			MemAccessListener l_above_data(max_data + 1);
			l_above_data.setWatchWidth(0xfffffff0);

            simulator.clearListeners();
            simulator.addListener(&l_trap);
            simulator.addListener(&l_timeout);
            simulator.addListener(&l_below_text);
            simulator.addListener(&l_above_text);
            simulator.addListener(&l_below_data);
            simulator.addListener(&l_above_data);
            simulator.addListener(&l_subexperiment_marker_1);
            simulator.addListener(&l_subexperiment_end);
            // simulator.addListener(&l_trace_end_marker);

            m_log << "Resuming till the crash" << std::endl;
            // resume and wait for results
            fail::BaseListener* l = simulator.resume();
			bool visited_marker_1 = false;
			address_t mem_access = 0x0;
			if (l == &l_below_data || l == &l_above_data) {
				fail::MemAccessListener *lm = (fail::MemAccessListener *) l;
				mem_access = lm->getTriggerAddress();
			}

			if (l == &l_subexperiment_marker_1 || mem_access == s_subexperiment_marker_1_ptr.getAddress()) {
				visited_marker_1 = true;
				simulator.clearListeners();
				simulator.addListener(&l_subexperiment_end);
				simulator.addListener(&l_trap);
				simulator.addListener(&l_timeout);
				l = simulator.resume();
			}

            // Evaluate result
            if(l == &l_subexperiment_end) {
				bool chose_right_output = mm.getByte(s_chose_right_output.getAddress());
				bool chose_right_input = mm.getByte(s_chose_right_input.getAddress());
				bool detected_error = mm.getByte(s_detected_error.getAddress());

				/* When we did not visit marker 1 before end marker,
				   we took the wrong exit point. The MMU would have
				   caught this error. */
				if (!visited_marker_1) {
					handleEvent(*result, result->ERR_OUTSIDE_TEXT, "wrong exit point");
				} else if (chose_right_output) {
					if (chose_right_input) {
						handleEvent(*result, result->OK, "OK");
					} else {
						handleEvent(*result, result->OK_WRONG_CONTROL_FLOW, "");
					}
				} else {
					if (detected_error) {
						handleEvent(*result, result->OK_DETECTED_ERROR, "");
					} else {
						handleEvent(*result, result->ERR_WRONG_RESULT, "");
					}
                }
            }  else if (l == &l_trap) {
                stringstream sstr;
                sstr << "trap #" << l_trap.getTriggerNumber();
                handleEvent(*result, result->ERR_TRAP, sstr.str());
            } else if ( l == &l_timeout ) {
                handleEvent(*result, result->ERR_TIMEOUT, "timeout");
            } else if (l == &l_below_text || l == &l_above_text) {
                std::stringstream ss;
                ss << ((l == &l_below_text) ? "< .text" : ">.text") << " ";
                ss << handleMemoryAccessEvent(*(fail::MemAccessListener *)l);
                handleEvent(*result, result->ERR_OUTSIDE_TEXT, ss.str());
            } else if (l == &l_below_data || l == &l_above_data) {
                std::stringstream ss;
                ss << ((l == &l_below_text) ? "< .data" : ">.data") << " ";
                ss << handleMemoryAccessEvent(*(fail::MemAccessListener *)l);
                handleEvent(*result, result->ERR_OUTSIDE_DATA, ss.str());
            } else {
                handleEvent(*result, result->UNKNOWN, "WTF");
			}
            simulator.clearListeners();

			// For RandomJump do only one experiment not 8
			if (pc_injection)
				break;
        }

        m_jc.sendResult(param);
    }
    // Explicitly terminate, or the simulator will continue to run.
    simulator.terminate();

}

