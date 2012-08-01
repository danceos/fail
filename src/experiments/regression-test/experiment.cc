#include <iostream>
#include <unistd.h>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"


// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS)
  #error This experiment needs: breakpoints. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool RegressionTest::run()
{
	int count = 0;
	
	Logger log("Regression-Test", false);
	log << "experiment start" << endl;

//Breakpoint-Test
	log << "Breakpoint-Test start." << endl;
	
	BPSingleListener mainbp(ANY_ADDR); 
	mainbp.setCounter(1000);
	
	BPRangeListener bprange(REGRESSION_FUNC_LOOP_DONE, REGRESSION_FUNC_LOOP_DONE);
	BPSingleListener breakcounter(REGRESSION_FUNC_LOOP_DONE);
	simulator.addListener(&breakcounter);


	while(true){
		
		BaseListener* ev = simulator.resume();
		
		if(ev == &breakcounter || ev == &bprange) {
			
			count++;
			//First 5 times test BPSingleListener
			if(count < 5){
				log << "Loop-Single-Test!" << endl;
				simulator.addListener(&breakcounter);
			//Next 5 times test BPRangeListener
			}else if(count < 10){
				log << "Loop-Range-Test!" << endl;
				simulator.addListener(&bprange);
			//At 10 run of loop start BPSingleListener, BPRangeListener, mainListener 
			//which waits 1000 instructions. 
			}else if(count == 10){
				log << "loop-limit reached..." << endl;
				simulator.addListener(&breakcounter);
				simulator.addListener(&bprange);
				simulator.addListener(&mainbp);
			//If mainListener fires not first the test failes.
			}else if(count >= 10){
				log << "Breakpoint-Test FAILED."<< endl;
				break;
			}
			//If mainListener fires first the test success.
		}else if(ev == &mainbp) {
			log << "Breakpoint-Test SUCCESS." <<endl;
			break;
		}
	}

	simulator.clearListeners(this);
	
	log << "Breakpoint-Test end" << endl;
	return true;
}
