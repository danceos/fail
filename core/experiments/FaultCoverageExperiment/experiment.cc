#include <iostream>
#include <stdint.h>
#include <sstream>
#include <cassert>
#include <time.h>

#include "experiment.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "../../util/Logger.hpp"

using namespace std;
using namespace sal;
using namespace fi;
using namespace sal;

bool FaultCoverageExperiment::run()
{
	/*
	Experimentskizze:
	- starte Gastsystem
	- setze Breakpoint auf Beginn der betrachteten Funktion; warte darauf
	- sichere Zustand
	- iteriere über alle Register
	-- iteriere über alle 32 Bit in diesem Register
	--- iteriere über alle Instruktionsadressen innerhalb der betrachteten Funktion
	---- setze Breakpoint auf diese Adresse; warte darauf
	---- flippe Bit x in Register y
	---- setze Breakpoint auf Verlassen der Funktion; warte darauf
	---- bei Erreichen des Breakpoint: sichere Funktionsergebnis (irgendein bestimmtes Register)
	---- lege Ergebnisdaten ab:
	    a) Ergebnis korrekt (im Vergleich zum bekannt korrekten Ergebnis für die Eingabe)
	    b) Ergebnis falsch
	    c) Breakpoint wird nicht erreicht, Timeout (z.B. gefangen in Endlosschleife)
	    d) Trap wurde ausgelöst
	---- stelle zuvor gesicherten Zustand wieder her
	*/

	// set breakpoint at start address of the function to be analyzed ("observed");
	// wait until instruction pointer reaches that address
	cout << "[FaultCoverageExperiment] Setting up experiment. Allowing to start now." << endl;
	BPEvent ev_func_start(INST_ADDR_FUNC_START);
	simulator.addEvent(&ev_func_start);

	cout << "[FaultCoverageExperiment] Waiting for function start address..." << endl;
	while (simulator.waitAny() != &ev_func_start)
		;

	// store current state
	cout << "[FaultCoverageExperiment] Saving state in ./bochs_save_point ..."; cout.flush();
	simulator.save("./bochs_save_point");
	cout << "done!" << endl;
	
	// log the results on std::cout
	Logger res;
	cout << "[FaultCoverageExperiment] Logging results on std::cout." << endl;

	RegisterManager& regMan = simulator.getRegisterManager();
	// iterate over all registers
	for (RegisterManager::iterator it = regMan.begin(); it != regMan.end(); it++) {
		Register* pReg = *it; // get a ptr to the current register-object
		// loop over the 32 bits within this register
		for (regwidth_t bitnr = 0; bitnr < pReg->getWidth(); ++bitnr) {
			// loop over all instruction addresses of observed function
			for (int instr = 0; ; ++instr) {
				// clear event queues
				simulator.clearEvents();

				// restore previously saved simulator state
				cout << "[FaultCoverageExperiment] Restoring previous simulator state..."; cout.flush();
				simulator.restore("./bochs_save_point");
				cout << "done!" << endl;

				// breakpoint at function exit
				BPEvent ev_func_end(INST_ADDR_FUNC_END);
				simulator.addEvent(&ev_func_end);

				// no need to continue simulation if we want to
				// inject *now*
				if (instr > 0) {
					// breakpoint $instr instructions in the future
					BPEvent ev_instr_reached(ANY_ADDR);
					ev_instr_reached.setCounter(instr);
					simulator.addEvent(&ev_instr_reached);

					// if we reach the exit first, this round is done
					if (simulator.waitAny() == &ev_func_end)
						break;
				}

				// inject bit-flip at bit $bitnr in register $reg
				regdata_t data = pReg->getData();
				data ^= 1 << bitnr;
				pReg->setData(data); // write back data to register

				// catch traps and timeout
				TrapEvent ev_trap; // any traps
				simulator.addEvent(&ev_trap);
				BPEvent ev_timeout(ANY_ADDR);
				ev_timeout.setCounter(1000);
				simulator.addEvent(&ev_timeout);

				// wait for function exit, trap or timeout
				BaseEvent* ev = simulator.waitAny();
				if (ev == &ev_func_end) {
					// log result
				  #if BX_SUPPORT_X86_64
					const size_t expected_size = sizeof(uint32_t)*8;
				  #else
					const size_t expected_size = sizeof(uint64_t)*8;
				  #endif
					Register* pCAX = simulator.getRegisterManager().getSetOfType(RT_GP)->getRegister(sal::RID_CAX);
					assert(expected_size == pCAX->getWidth()); // we assume to get 32(64) bits...
					regdata_t result = pCAX->getData();
					res << "[FaultCoverageExperiment] Reg: " << pCAX->getName()
					    << ", #Bit: " << bitnr << ", Instr-Idx: " << instr
					    << ", Data: " << result;
				}
				else if (ev == &ev_trap)
					res << "[FaultCoverageExperiment] Reg: " << pReg->getName()
						<< ", #Bit: " << bitnr << ", Instr-Idx: " << instr
						<< ", Trap#: " << ev_trap.getTriggerNumber() << " (Trap)";
				else if (ev == &ev_timeout)
					res << "[FaultCoverageExperiment] Reg: " << pReg->getName()
						<< ", #Bit: " << bitnr << ", Instr-Idx: " << instr
						<< " (Timeout)";
				else
					cout << "We've received an unkown event! "
					     << "What the hell is going on?" << endl;
			}
		}
	}

	simulator.clearEvents(this);
	return true;
}
