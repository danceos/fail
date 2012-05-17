#ifndef __MEM_WRITE_EXPERIMENT_HPP__
  #define __MEM_WRITE_EXPERIMENT_HPP__

// Author: Adrian Böckenkamp
// Date:   16.06.2011

#include <iostream>

#include "../controller/ExperimentFlow.hpp"
#include "../SAL/SALInst.hpp"
#include "config/FailConfig.hpp"

// Check aspect dependencies:
#if !defined(CONFIG_EVENT_CPULOOP) || !defined(CONFIG_EVENT_MEMACCESS) || !defined(CONFIG_SR_SAVE) || !defined(CONFIG_FI_MEM_ACCESS_BITFLIP)
  #error Event dependecies not satisfied! Enabled needed aspects in FailConfig.hpp!
#endif

using namespace fi;
using namespace std;
using sal::simulator;

class MemWriteExperiment : public ExperimentFlow
{
	public:
		bool run() // Example experiment (defines "what we wanna do")
		{
			/************************************
			 *  Description of experiment flow. *
			 ************************************/

			// 1. Add some events (set up the experiment):
			cout << "[MemWriteExperiment] Setting up experiment. Allowing to"
				 << " start now." << endl;
			MemWriteEvent mem1(0x000904F0), mem2(0x02ff0916), mem3(0x0050C8E8);
			BPEvent breakpt(0x4ae6);
			simulator.addEvent(&mem1);
			simulator.addEvent(&mem2);
			simulator.addEvent(&mem3);
			simulator.addEvent(&breakpt);

			// 2. Wait for event condition "(id1 && id2) || id3" to become true:
			cout << "[MemWriteExperiment] Waiting for condition (1) (\"(id1 &&"
				 << " id2) || id3\") to become true..." << endl;
			bool f1 = false, f2 = false, f3 = false, f4 = false;
			while(!(f1 || f2 || f3 || f4))
			{
				BPEvent* pev = simulator.waitAny();
				cout << "[MemWriteExperiment] Received event id=" << id
					 << "." << endl;
				if(pev == &mem4)
					f4 = true;				
				if(pev == &mem3)
					f3 = true;
				if(pev == &mem2)
					f2 = true;
				if(pev == &mem1)
					f1 = true;
			}
			cout << "[MemWriteExperiment] Condition (1) satisfied! Ready to "
				 << "add next event..." << endl;
			// 3. Add a new event now:
			cout << "[MemWriteExperiment] Adding new Event..."; cout.flush();
			simulator.clearEvents(); // remove residual events in the buffer
			// (we're just interested in the new event)
			simulator.save("./bochs_save_point");
			cout << "done!" << endl;

			// 4. Continue simulation (waitAny) and inject bitflip:
			// ...

			simulator.clearEvents(this);
			return true;
		}
};

#endif /* __MEM_WRITE_EXPERIMENT_HPP__ */

