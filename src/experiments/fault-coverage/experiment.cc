#include <iostream>
#include <stdint.h>
#include <sstream>
#include <cassert>
#include <time.h>

#include "experiment.hpp"
#include "sal/Listener.hpp"
#include "sal/SALInst.hpp"
#include "util/Logger.hpp"

using namespace std;
using namespace fail;

bool FaultCoverageExperiment::run()
{
	/*
	Sketch of experiment:
	- start guest system
	- set breakpoint to the beginning of the function under consideration
	- wait until breakpoint triggers, save state
	- loop over all registers
	-- loop over all 32 bits of the each register
	--- loop over all instruction addresses of the function of interest
	---- set breakpoint to each address, wait until breakpoint triggers
	---- toggle bit x in register y
	---- set breakpoint to the last address of function, wait
	---- if breakpoint triggers: save result (eax/rax register)
	---- save result data:
	    a) result correct (compared to the valid result for the current input)
	    b) result wrong
	    c) breakpoint never reached, timeout (e.g. caught in endless loop)
	    d) trap triggered
	---- restore previously saved simulator state
	*/

	// log the results on std::cout
	Logger log("FaultCoverageExperiment", false);

	// set breakpoint at start address of the function to be analyzed ("observed");
	// wait until instruction pointer reaches that address
	log << "Setting up experiment. Allowing to start now." << endl;
	BPSingleListener ev_func_start(INST_ADDR_FUNC_START);
	simulator.addListener(&ev_func_start);

	log << "Waiting for function start address..." << endl;
	while (simulator.resume() != &ev_func_start)
		;

	// store current state
	log << "Saving state in ./bochs_save_point ..." << endl;
	simulator.save("./bochs_save_point");

	log << "Logging results on std::cout." << endl;

	// Note: This heavily uses the save-restore feature which causes
	//       causes a memory leak after several rounds (seg-fault).

	// iterate over all registers
	ConcreteCPU cpu = simulator.getCPU(0);
	for (ConcreteCPU::iterator it = cpu.begin(); it != cpu.end(); it++) {
		Register* pReg = *it; // get a ptr to the current register-object
		// loop over the 32 (64) bits within this register
		for (regwidth_t bitnr = 0; bitnr < pReg->getWidth(); ++bitnr) {
			// loop over all instruction addresses of observed function
			for (int instr = 0; ; ++instr) {
				// clear event queues
				simulator.clearListeners();

				// restore previously saved simulator state
				log << "Restoring previous simulator state..." << endl;
				simulator.restore("./bochs_save_point");

				// breakpoint at function exit
				BPSingleListener ev_func_end(INST_ADDR_FUNC_END);
				simulator.addListener(&ev_func_end);

				// no need to continue simulation if we want to
				// inject *now*
				if (instr > 0) {
					// breakpoint $instr instructions in the future
					BPSingleListener ev_instr_reached(ANY_ADDR);
					ev_instr_reached.setCounter(instr);
					simulator.addListener(&ev_instr_reached);

					// if we reach the exit first, this round is done
					if (simulator.resume() == &ev_func_end)
						break;
				}

				// inject bit-flip at bit $bitnr in register $reg
				regdata_t data = cpu.getRegisterContent(pReg);
				data ^= 1 << bitnr;
				cpu.setRegisterContent(pReg, data); // write back data to register

				// catch traps and timeout
				TrapListener ev_trap; // any traps
				simulator.addListener(&ev_trap);
				BPSingleListener ev_timeout(ANY_ADDR);
				ev_timeout.setCounter(1000);
				simulator.addListener(&ev_timeout);

				// wait for function exit, trap or timeout
				BaseListener* ev = simulator.resume();
				if (ev == &ev_func_end) {
					// log result
					Register* pCAX = cpu.getRegister(RID_CAX);
					regdata_t result = cpu.getRegisterContent(pCAX);
					log << "Reg: " << pCAX->getName() << ", #Bit: " << bitnr
					    << ", Instr-Idx: " << instr << ", Data: " << result << endl;
				}
				else if (ev == &ev_trap)
					log << "Reg: " << pReg->getName() << ", #Bit: " << bitnr
					    << ", Instr-Idx: " << instr << ", Trap#: "
					    << ev_trap.getTriggerNumber() << " (Trap)";
				else if (ev == &ev_timeout)
					log << "Reg: " << pReg->getName() << ", #Bit: "
					    << bitnr << ", Instr-Idx: " << instr << " (Timeout)";
				else
					log << "We've received an unkown event! "
					     << "What the hell is going on?" << endl;
			}
		}
	}

	simulator.clearListeners(this);
	return true;
}
