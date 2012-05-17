#ifndef __SINGLE_STEPPING_EXPERIMENT_HPP__
  #define __SINGLE_STEPPING_EXPERIMENT_HPP__

// Author: Adrian Böckenkamp
// Date:   09.11.2011

#include <iostream>

#include "../controller/ExperimentFlow.hpp"
#include "../SAL/SALInst.hpp"
#include "config/FailConfig.hpp"
#include "../SAL/bochs/BochsRegister.hpp"

// Check if aspect dependency is satisfied:
#ifndef CONFIG_EVENT_CPULOOP
  #error Breakpoint-events needed! Enable aspect first (see FailConfig.hpp)!
#endif

using namespace fi;
using namespace std;
using namespace sal;

#define FUNCTION_ENTRY_ADDRESS 0x3c1f

class SingleSteppingExperiment : public fi::ExperimentFlow
{
	public:
		bool run()
		{
			/************************************
			 *  Description of experiment flow. *
			 ************************************/
			// Wait for function entry adresss:
			cout << "[SingleSteppingExperiment] Setting up experiment. Allowing"
			     << " to start now." << endl;
			BPEvent mainFuncEntry(FUNCTION_ENTRY_ADDRESS);
			simulator.addEvent(&mainFuncEntry);
			if(&mainFuncEntry != simulator.waitAny())
			{
				cerr << "[SingleSteppingExperiment] Now, we are completely lost!"
				     << " It's time to cry! :-(" << endl;
				simulator.clearEvents(this);
				return false;
			}
			cout << "[SingleSteppingExperiment] Entry of main function reached!"
			     << " Beginning single-stepping..." << endl;
			char action;
			while(true)
			{
				BPEvent bp(ANY_ADDR);
				simulator.addEvent(&bp);
				simulator.waitAny();
				cout << "0x" << hex
				     << simulator.getRegisterManager().getInstructionPointer()
				     << endl;
				cout << "Continue (y/n)? ";
				cin >> action; cin.sync(); cin.clear();
				if(action != 'y')
					break;
			}

			simulator.clearEvents(this);
			return true;
		}
};

#endif /* __SINGLE_STEPPING_EXPERIMENT_HPP__ */
