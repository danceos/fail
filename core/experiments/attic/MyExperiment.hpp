#ifndef __MY_EXPERIMENT_HPP__
  #define __MY_EXPERIMENT_HPP__

// Author: Adrian Böckenkamp
// Date:   16.06.2011

#include <iostream>

#include "../controller/ExperimentFlow.hpp"
#include "../SAL/SALInst.hpp"

using namespace fi;
using namespace std;
using sal::simulator;

class MyExperiment : public fi::ExperimentFlow
{
	public:
		bool run() // Example experiment (defines "what we wanna do")
		{
			/************************************
			 *  Description of experiment flow. *
			 ************************************/
			
			// 1. Add some events (set up the experiment):
			cout << "[MyExperiment] Setting up experiment. Allowing to start"
				 << " now." << endl;
			BPEvent ev1(0x8048A00), ev2(0x8048F01), ev3(0x3c1f);
			simulator.addEvent(&ev1);
			simulator.addEvent(&ev2);
			simulator.addEvent(&ev3);

			// 2. Wait for event condition "(id1 && id2) || id3" to become true:
			BPEvent* pev;
			cout << "[MyExperiment] Waiting for condition (1) (\"(id1 && id2)"
				 << " || id3\") to become true..." << endl;
			bool f1 = false, f2 = false, f3 = false;
			while(!((f1 && f2) || f3))
			{
				pev = simulator.waitAny();
				cout << "[MyExperiment] Received event id=" << pev->getId()
					 << "." << endl;
				if(pev == &ev3)
					f3 = true;
				if(pev == &ev2)
					f2 = true;
				if(pev == &ev1)
					f1 = true;
			}
			cout << "[MyExperiment] Condition (1) satisfied! Ready..." << endl;
			// Remove residual (for all active experiments!)
			// events in the buffer:
			simulator.clearEvents();
			BPEvent foobar(ANY_ADDR);
			foobar.setCounter(400);
			cout << "[MyExperiment] Adding breakpoint-event, firing after the"
				 << " next 400 instructions..."; cout.flush();
			simulator.addEventAndWait(&foobar);
			cout << "cought! Exiting now." << endl;
			return (true);
		}
};

#endif /* __MY_EXPERIMENT_HPP__ */
