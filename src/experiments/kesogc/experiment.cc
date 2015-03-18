#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>


#include <stdlib.h>
#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include <string>
#include <vector>

#include "campaign.hpp"
#include "kesogc.pb.h"
#include "util/Disassembler.hpp"

const char *parity_types[] = {
	"LIST",
	"STACK",
	"STACK_POINTER",
	"DOMAIN",
	"BITMAP",
	"LLREFS",
	"COLOR"
};

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) 
     #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

unsigned KESOgc::injectBitFlip(address_t data_address, unsigned bitpos){

	MemoryManager& mm = simulator.getMemoryManager();
	unsigned int value, injectedval;

	value = mm.getByte(data_address);
	injectedval = value ^ (1 << bitpos);
	mm.setByte(data_address, injectedval);

	m_log << "INJECTION at: 0x" << hex<< setw(2) << setfill('0') << data_address
		  << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << mm.getByte(data_address) << endl;

	return value;
}


void handleEvent(KesoGcProtoMsg_Result& result, KesoGcProtoMsg_Result_ResultType restype, const std::string &msg) {
	cout << "Result details: " << msg << endl;
	result.set_resulttype(restype);
	result.set_details(msg);
}


void handleMemoryAccessEvent(KesoGcProtoMsg_Result& result, const fail::MemAccessListener& l_mem){
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

    sstr << " ip @ 0x" << hex << l_mem.getTriggerInstructionPointer();

    handleEvent(result, result.MEMACCESS, sstr.str());
}

bool KESOgc::run()
{
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



    //  m_dis.init();
    //******* Boot, and store state *******//
    m_log << "STARTING EXPERIMENT" << endl;

	unsigned executed_jobs = 0;

    // Setup exit points
    const ElfSymbol &s_error = m_elf.getSymbol("keso_throw_error");
    BPSingleListener l_error(s_error.getAddress());
    
	const ElfSymbol &s_nullp = m_elf.getSymbol("keso_throw_nullpointer");
    BPSingleListener l_nullp(s_nullp.getAddress());
    
	const ElfSymbol &s_oobounds = m_elf.getSymbol("keso_throw_index_out_of_bounds");
    BPSingleListener l_oobounds(s_oobounds.getAddress());
    
	const ElfSymbol &s_gc_parity = m_elf.getSymbol("keso_throw_gc_parity");
	BPSingleListener l_gc_parity(s_gc_parity.getAddress());

	const ElfSymbol &s_parity = m_elf.getSymbol("keso_throw_parity");
    BPSingleListener l_parity(s_parity.getAddress());

	//BPSingleListener l_dump(m_elf.getSymbol("c17_Main_m4_dumpResults_console").getAddress());
	BPSingleListener l_end(m_elf.getSymbol("keso_end").getAddress());
	
	MemAccessListener l_mem_text(minimal_ip, MemAccessEvent::MEM_WRITE);
    l_mem_text.setWatchWidth(maximal_ip - minimal_ip);

	MemAccessListener l_mem_outerspace( maximal_data, MemAccessEvent::MEM_WRITE);
    l_mem_outerspace.setWatchWidth(0xfffffff0);
    
	TrapListener l_trap(ANY_TRAP);

    TimerListener l_timeout(5 * 1000 * 1000); // 5 seconds in microseconds

	const ElfSymbol &s_resilient_heap_repaired = m_elf.getSymbol("keso_resilient_gcHeapPointer_repaired");
    BPSingleListener l_resilient_heap_repaired(s_resilient_heap_repaired.getAddress());
	
	const ElfSymbol &s_ft_list_repaired = m_elf.getSymbol("keso_ft_list_repaired");
    BPSingleListener l_ft_list_repaired(s_ft_list_repaired.getAddress());

	const ElfSymbol &s_ft_list_broken = m_elf.getSymbol("keso_ft_list_broken");
    BPSingleListener l_ft_list_broken(s_ft_list_broken.getAddress());

	const ElfSymbol &s_resilient_stack_repaired = m_elf.getSymbol("keso_resilient_gcStackPointer_repaired");
    BPSingleListener l_resilient_stack_repaired(s_resilient_stack_repaired.getAddress());

	while (executed_jobs < 25 || m_jc->getNumberOfUndoneJobs() > 0) {
		m_log << "asking jobserver for parameters" << endl;
		KesoGcExperimentData param;
		if(!m_jc->getParam(param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();

		for (int bit_offset = 0; bit_offset < 8; ++bit_offset) {
			// 8 results in one job
			KesoGcProtoMsg_Result *result = param.msg.add_result();
			result->set_bitoffset(bit_offset);

			m_log << "restoring state" << endl;
			// Restore to the image, which starts at address(main)
			simulator.restore("state");
			executed_jobs ++;

			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;


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
						m_log << "experiment reached finish() before FI" << endl;
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
				m_log << "Invalid Injection address EIP=0x" 
					  << std::hex << simulator.getCPU(0).getInstructionPointer()
					  << " != 0x" << injection_instr_absolute << std::endl;
				simulator.terminate(1);
			}

			/// INJECT BITFLIP:
			result->set_original_value(injectBitFlip(data_address, bit_offset));

            cout << " outerspace : " << l_mem_outerspace.getWatchWidth() << " --- @ :" << l_mem_outerspace.getWatchAddress() << endl;
            simulator.clearListeners();
            simulator.addListener(&l_trap);
            if (s_error.isValid())
                simulator.addListener(&l_error);
            if (s_nullp.isValid())
                simulator.addListener(&l_nullp);
            if (s_oobounds.isValid())
                simulator.addListener(&l_oobounds);
            if (s_gc_parity.isValid())
                simulator.addListener(&l_gc_parity);
            if (s_parity.isValid())
                simulator.addListener(&l_parity);
            simulator.addListener(&l_end);
            simulator.addListener(&l_mem_text);
            simulator.addListener(&l_mem_outerspace);
            simulator.addListener(&l_timeout);

			if(s_resilient_heap_repaired.isValid()){
				simulator.addListener(&l_resilient_heap_repaired);
			}
			
			if(s_resilient_stack_repaired.isValid()){
				simulator.addListener(&l_resilient_stack_repaired);
			}
			
			if(s_ft_list_repaired.isValid()){
				simulator.addListener(&l_ft_list_repaired);
			}
			
			if(s_ft_list_broken.isValid()){
				simulator.addListener(&l_ft_list_broken);
			}

            m_log << "Resuming till the crash" << std::endl;
            // resume and wait for results
            fail::BaseListener* l = simulator.resume();
            m_log << "CDX has ended" << std::endl;

            // Evaluate result
            if(l == &l_error) {
                handleEvent(*result, result->EXC_ERROR, "exc error");
            } else if (l == &l_nullp) {
                handleEvent(*result, result->EXC_NULLPOINTER, "exc nullpointer");
            
			} else if ( l == &l_oobounds ) {
                handleEvent(*result, result->EXC_OOBOUNDS, "exc out of bounds");

			} else if (l == &l_gc_parity) {
				address_t stack_pointer = simulator.getCPU(0).getStackPointer();
				int32_t parity_type = simulator.getMemoryManager().getByte(stack_pointer + 4); // 1st argument is at esp+4
				stringstream sstr;
				sstr << "exc gc_parity of type: " << parity_types[parity_type];
                handleEvent(*result, result->EXC_GC_PARITY, sstr.str());

            } else if (l == &l_parity) {
                handleEvent(*result, result->EXC_PARITY, "exc parity");
            
            } else if (l == &l_end) {
                handleEvent(*result, result->CALCDONE, "calculation done");

            } else if (l == &l_timeout) {
                handleEvent(*result, result->TIMEOUT, "5s");

            }  else if (l == &l_trap) {
                stringstream sstr;
                sstr << "trap #" << l_trap.getTriggerNumber();
                handleEvent(*result, result->TRAP, sstr.str());

            } else if (l == &l_mem_text){
                handleMemoryAccessEvent(*result, l_mem_text);

            } else if (l == &l_mem_outerspace){
                handleMemoryAccessEvent(*result, l_mem_outerspace);

			} else if (l == &l_resilient_heap_repaired){
				handleEvent(*result, result->RESILIENT_HEAP_REPAIRED, "resilient heap pointer repaired");

			} else if (l == &l_resilient_stack_repaired){
				handleEvent(*result, result->RESILIENT_STACK_REPAIRED, "resilient stack pointer repaired");

			} else if (l == &l_ft_list_repaired){
				handleEvent(*result, result->FT_LIST_REPAIRED, "ft list repaired");

			} else if (l == &l_ft_list_broken){
				handleEvent(*result, result->FT_LIST_BROKEN, "ft list broken");

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

