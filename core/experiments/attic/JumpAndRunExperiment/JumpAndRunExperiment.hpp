#ifndef __JUMP_AND_RUN_EXPERIMENT_HPP__
  #define __JUMP_AND_RUN_EXPERIMENT_HPP__

// Author: Adrian Böckenkamp
// Date:   07.11.2011

#include <iostream>

#include "../controller/ExperimentFlow.hpp"
#include "../SAL/SALInst.hpp"
#include "../SAL/bochs/BochsRegister.hpp"
#include "config/AspectConfig.hpp"

// Check if aspect dependencies are satisfied:
#if !defined(CONFIG_EVENT_CPULOOP) || !defined(CONFIG_EVENT_JUMP)
  #error Breakpoint- and jump-events needed! Enable aspects first (see AspectConfig.hpp)!
#endif

using namespace fi;
using namespace std;
using namespace sal;

class JumpAndRunExperiment : public fi::ExperimentFlow
{
	public:
		bool run()
		{
			/************************************
			 *  Description of experiment flow. *
			 ************************************/
			// Wait for function entry adresss:
			cout << "[JumpAndRunExperiment] Setting up experiment. Allowing to "
			     << "start now." << endl;
			BPEvent mainFuncEntry(0x3c1f);
			simulator.addEvent(&mainFuncEntry);
			if(&mainFuncEntry != simulator.waitAny())
			{
				cerr << "[JumpAndRunExperiment] Now, we are completely lost! "
				     << "It's time to cry! :-(" << endl;
				simulator.clearEvents(this);
				return false;
			}
			else
				cout << "[JumpAndRunExperiment] Entry of main function reached! "
				     << " Let's see who's jumping around here..." << endl;

			const unsigned COUNTER = 20000;
			unsigned i = 0;
			BxFlagsReg* pFlags = dynamic_cast<BxFlagsReg*>(simulator.
				getRegisterManager().getSetOfType(RT_ST).snatch());
			assert(pFlags != NULL && "FATAL ERROR: NULL ptr not expected!");
			JumpEvent ev;
			// Catch the next "counter" jumps:
			while(++i <= COUNTER)
			{
				ev.setWatchInstructionPointer(ANY_INSTR);
				simulator.addEvent(&ev);
				if(simulator.waitAny() != &ev)
				{
					cerr << "[JumpAndRunExperiment] Damn! Something went "
					     << "terribly wrong! Who added that event?! :-(" << endl;
					simulator.clearEvents(this);
					return false;
				}
				else
					cout << "[JumpAndRunExperiment] Jump detected. Instruction: "
					     << "0x" hex << ev.getTriggerInstructionPointer()
					     << " -- FLAGS [CF, ZF, OF, PF, SF] = ["
					     << pFlags->getCarryFlag() << ", "
					     << pFlags->getZeroFlag() << ", "
					     << pFlags->getOverflowFlag() << ", "
					     << pFlags->getParityFlag() << ", " 
					     << pFlags->getSignFlag() << "]." << endl;
			}
			cout << "[JumpAndRunExperiment] " << dec << counter
				 << " jump(s) detected -- enough for today...exiting! :-)"
				 << endl;

			simulator.clearEvents(this);
			return true;
		}
};

#endif /* __JUMP_AND_RUN_EXPERIMENT_HPP__ */
