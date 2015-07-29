#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "experiment.hpp"
#include "InstructionFilter.hpp"
#include "aluinstr.hpp"

#include "sal/SALConfig.hpp"
#include "util/gzstream/gzstream.h"
#include "util/ProtoStream.hpp"
#include "TracePlugin.pb.h"
#include "sal/Listener.hpp"

using namespace std;
using namespace fail;

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

void L4SysExperiment::runToStart(fail::BPSingleListener *bp)
{
	bp->setWatchInstructionPointer(conf.func_entry);

	log << "run until ip reaches 0x" << hex << conf.func_entry << endl;

	simulator.addListenerAndResume(bp);

	log << "test function entry reached, saving state" << endl;
	log << "EIP: expected " << hex << bp->getTriggerInstructionPointer()
			<< " and actually got "
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;
	log << "check the source code if the two instruction pointers are not equal" << endl;

	if(conf.address_space == conf.address_space_trace) {
		conf.address_space_trace = BX_CPU(0)->cr3;
	} 

	conf.address_space = BX_CPU(0)->cr3;

	char tmp[20];
	sprintf(tmp, "0x%lx", BX_CPU(0)->cr3);
	updateConfig("address_space", tmp );
}


void L4SysExperiment::collectInstructionTrace(fail::BPSingleListener* bp)
{
	fail::MemAccessListener ML(ANY_ADDR, MemAccessEvent::MEM_READWRITE);
	ogzstream out(conf.trace.c_str());
	ProtoOStream *os = new ProtoOStream(&out);

	size_t count = 0, inst_accepted = 0, mem = 0, mem_valid = 0;
	simtime_t prevtime = 0, currtime;
	simtime_diff_t deltatime;

	log << "restoring state" << endl;
	simulator.restore(conf.state_folder.c_str());
	currtime = simulator.getTimerTicks();

	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

    if (!simulator.addListener(&ML)) {
        log << "did not add memory listener..." << std::endl;
        exit(1);
    }
    if (!simulator.addListener(bp)) {
        log << "did not add breakpoint listener..." << std::endl;
        exit(1);
    }

	ofstream instr_list_file(conf.instruction_list.c_str(), ios::binary);
    RangeSetInstructionFilter filtering(conf.filter.c_str());
	bp->setWatchInstructionPointer(ANY_ADDR);

	map<address_t, unsigned> times_called_map;
	/* We run until the IP reaches func_entry. We will not
	 * reach the same IP again. So, if filter and func entry are
	 * equal, then we have to enable injections.
	 */
	bool injecting = conf.filter_entry == conf.func_entry;

	while (bp->getTriggerInstructionPointer() != conf.func_exit) {
		fail::BaseListener *res = simulator.resume();
        address_t curr_addr = 0;
        
        // XXX: See the API problem below!
        if (res == &ML) {
            curr_addr = ML.getTriggerInstructionPointer();
            simulator.addListener(&ML);
            if ((conf.address_space_trace != ANY_ADDR) && (BX_CPU(0)->cr3 != conf.address_space_trace)) {
                continue;
            }
            ++mem;
        } else if (res == bp) {
            curr_addr = bp->getTriggerInstructionPointer();
            assert(curr_addr == simulator.getCPU(0).getInstructionPointer());
            simulator.addListener(bp);
            ++count;
        }
        
        currtime = simulator.getTimerTicks();
        deltatime = currtime - prevtime;
        
        if (curr_addr == conf.filter_entry) {
            injecting = true;
        }

        if (curr_addr == conf.filter_exit) {
            injecting = false;
        }

        // Only trace if:
        // 1) we are between FILTER_ENTRY and FILTER_EXIT, and
        // 2) we have a valid instruction according to filter rules, and
        // 3) we are in the TRACE address space
        if (!injecting or
            !filtering.isValidInstr(curr_addr, reinterpret_cast<char const*>(calculateInstructionAddress()))
                       or
            (BX_CPU(0)->cr3 != conf.address_space_trace)
            ) {
            //log << "connt..." << std::endl;
            continue;
        }

        if (res == &ML) {
#if 0
            log << "Memory event IP " << std::hex << ML.getTriggerInstructionPointer()
                << " @ " << ML.getTriggerAddress() << "("
                << ML.getTriggerAccessType() << "," << ML.getTriggerWidth()
                << ")" << std::endl;
#endif
            ++mem_valid;

            Trace_Event te;
            if (deltatime != 0) { te.set_time_delta(1); };
            te.set_ip(curr_addr);
            te.set_memaddr(ML.getTriggerAddress());
            te.set_accesstype( (ML.getTriggerAccessType() & MemAccessEvent::MEM_READ) ? te.READ : te.WRITE );
            te.set_width(ML.getTriggerWidth());
            os->writeMessage(&te);
        } else if (res == bp) {
            unsigned times_called = times_called_map[curr_addr];
            ++times_called;
            times_called_map[curr_addr] = times_called;
        
            //log << "breakpoint event" << std::endl;
            // now check if we want to add the instruction for fault injection
            ++inst_accepted;

            // 1) The 'old' way of logging instructions -> DEPRECATE soon
            //    BUT: we are currently using the bp_counter stored in this
            //    file!
            TraceInstr new_instr;
            //log << "writing IP " << hex << curr_addr << " counter "
            //    << dec << times_called << "(" << hex << BX_CPU(0)->cr3 << ")"
            //    << endl;
            new_instr.trigger_addr = curr_addr;
            new_instr.bp_counter = times_called;

            instr_list_file.write(reinterpret_cast<char*>(&new_instr), sizeof(TraceInstr));

            // 2) The 'new' way -> generate Events that can be processed by
            // the generic *-trace tools
            // XXX: need to log CR3 if we want multiple binaries here
            Trace_Event e;
            if (deltatime != 0) { e.set_time_delta(1); };
            e.set_ip(curr_addr);
            os->writeMessage(&e);
        } else {
            printf("Unknown res? %p\n", res);
        }
        prevtime = currtime;

		//short sanity check
        //log << "continue..." << std::endl;
	}
	log << "saving instructions triggered during normal execution" << endl;
	instr_list_file.close();
	log << "test function calculation position reached after "
	    << dec << count << " instructions; " << inst_accepted << " accepted" << endl;
	log << "mem accesses: " << mem << ", valid: " << mem_valid << std::endl;
		
	conf.numinstr = inst_accepted;
	conf.totinstr = count;

	//Write new values into config file
	char numimstr_str[20];

	sprintf(numimstr_str, "%li", inst_accepted);
	updateConfig("numinstr", numimstr_str );

	char totinstr_str[20];
	sprintf(totinstr_str, "%li", count);
	updateConfig("totinstr", totinstr_str);

 	delete bp;
}

void L4SysExperiment::goldenRun(fail::BPSingleListener* bp)
{
	log << "restoring state" << endl;
	simulator.restore(conf.state_folder.c_str());
	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

	std::string golden_run;
	ofstream golden_run_file(conf.golden_run.c_str());
	bp->setWatchInstructionPointer(conf.func_exit);
	simulator.addListener(bp);
	BaseListener* ev = waitIOOrOther(true);
	if (ev == bp) {
		golden_run.assign(currentOutput.c_str());
		golden_run_file << currentOutput.c_str();
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
    delete bp;
}
