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
#include "vezs.pb.h"
#include "util/Disassembler.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) 
     #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

void VEZSExperiment::redecodeCurrentInstruction() {
	/* Flush Instruction Caches and Prefetch queue */
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	cpu_context->invalidate_prefetch_q();
	cpu_context->iCache.flushICacheEntries();


	guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
	bxInstruction_c *currInstr = simulator.getCurrentInstruction();

	cout << "REDECODE INSTRUCTION!" << endl;

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

unsigned VEZSExperiment::injectBitFlip(address_t data_address, unsigned data_width, unsigned bitpos){
    MemoryManager& mm = simulator.getMemoryManager();
    unsigned int value, injectedval;

    value = mm.getByte(data_address);
    injectedval = value ^ (1 << bitpos);
    mm.setByte(data_address, injectedval);

    cout << "INJECTION at: 0x" << hex	<< setw(2) << setfill('0') << data_address
         << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval << endl;

    /* If it is the current instruction redecode it */
    guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
    bxInstruction_c *currInstr = simulator.getCurrentInstruction();
    if (currInstr) {
        unsigned length_in_bytes = currInstr->ilen();

        if (pc <= data_address && data_address <= (pc + length_in_bytes)) {
            redecodeCurrentInstruction();
        }
    }

    return value;
}


void handleEvent(VEZSProtoMsg_Result& result, VEZSProtoMsg_Result_ResultType restype, const std::string &msg) {
	cout << "Result details: " << msg << endl;
	result.set_resulttype(restype);
	result.set_details(msg);
}


bool VEZSExperiment::run()
{
    //  m_dis.init();
    //******* Boot, and store state *******//
    cout << "STARTING EXPERIMENT" << endl;

	unsigned executed_jobs = 0;

    // Setup exit points
    const ElfSymbol &s_positive = m_elf.getSymbol("fail_marker_positive");
    BPSingleListener l_positive(s_positive.getAddress());

    const ElfSymbol &s_negative = m_elf.getSymbol("fail_marker_negative");
    BPSingleListener l_negative(s_negative.getAddress());

    const ElfSymbol &s_end = m_elf.getSymbol("fail_trace_stop");
	BPSingleListener l_end(s_end.getAddress());
	
	TrapListener l_trap(ANY_TRAP);

    TimerListener l_timeout(500 * 1000); // 500 ms

    assert(s_positive.isValid() || "fail_marker_positive not found");
    assert(s_negative.isValid() || "fail_marker_negative not found");
    assert(s_end.isValid() ||"fail_trace_stop not found");


	while (executed_jobs < 25 || m_jc->getNumberOfUndoneJobs() > 0) {
		cout << "asking jobserver for parameters" << endl;
		VEZSExperimentData param;
		if(!m_jc->getParam(param)){
			cout << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();
		unsigned data_width = param.msg.fsppilot().data_width();

		for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
			// 8 results in one job
			VEZSProtoMsg_Result *result = param.msg.add_result();
			result->set_bitoffset(bit_offset);

			cout << "restoring state" << endl;
			// Restore to the image, which starts at address(main)
			simulator.restore("state");
			executed_jobs ++;

			cout << "Trying to inject @ instr #" << dec << injection_instr << endl;


			if (injection_instr > 0) {
				simulator.clearListeners();
				// XXX could be improved with intermediate states (reducing runtime until injection)
				simulator.addListener(&l_end);

                BPSingleListener bp;
				bp.setWatchInstructionPointer(ANY_ADDR);
				
				bp.setCounter(injection_instr);
				
				simulator.addListener(&bp);

				bool inject = true;
				while (1) {
					fail::BaseListener * listener = simulator.resume();
					// finish() before FI?
					if (listener == &l_end) {
						cout << "experiment reached finish() before FI" << endl;
						handleEvent(*result, result->NOINJECTION, "time_marker reached before instr2");
						inject = false;
						break;
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
           
			address_t injection_instr_absolute = param.msg.fsppilot().injection_instr_absolute();
			if (simulator.getCPU(0).getInstructionPointer() != injection_instr_absolute) {
				cout << "Invalid Injection address EIP=0x" 
					  << std::hex << simulator.getCPU(0).getInstructionPointer()
					  << " != 0x" << injection_instr_absolute << std::endl;
				simulator.terminate(1);
			}

			/// INJECT BITFLIP:
			result->set_original_value(injectBitFlip(data_address, data_width, bit_offset));

            simulator.clearListeners();
            simulator.addListener(&l_timeout);
            simulator.addListener(&l_trap);
            simulator.addListener(&l_positive);
            simulator.addListener(&l_negative);


            cout << "Resuming till the crash" << std::endl;
            // resume and wait for results
            fail::BaseListener* l = simulator.resume();
            cout << "End of execution" << std::endl;

            // Evaluate result
            if(l == &l_positive) {
                handleEvent(*result, result->POSITIVE_MARKER, "fail_marker_positive()");
            } else if(l == &l_negative) {
                handleEvent(*result, result->NEGATIVE_MARKER, "fail_marker_negative()");
            } else if (l == &l_timeout) {
                handleEvent(*result, result->TIMEOUT, "500ms");
            }  else if (l == &l_trap) {
                stringstream sstr;
                sstr << "trap #" << l_trap.getTriggerNumber();
                handleEvent(*result, result->TRAP, sstr.str());
            } else {
                handleEvent(*result, result->UNKNOWN, "UNKNOWN event");
            }
            simulator.clearListeners();
        }

        m_jc->sendResult(param);
    }
    // Explicitly terminate, or the simulator will continue to run.
    simulator.terminate();
}

