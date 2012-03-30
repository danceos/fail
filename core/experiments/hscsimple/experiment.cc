#include <iostream>

#include <unistd.h>

#include "experiment.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"
#include "util/Logger.hpp"
#include "config/AspectConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || !defined(CONFIG_SR_SAVE)
  #error This experiment needs: breakpoints, save, and restore. Enable these in the configuration.
#endif

using std::endl;

bool hscsimpleExperiment::run()
{
	Logger log("HSC", false);
	log << "experiment start" << endl;

	// do funny things here...
#if 1
    // STEP 1
    //sal::simulator.dbgEnableInstrPtrOutput(500);
    while(1){
		int j = 0;
		for(j=0 ; j<=50 ; j++){
			cout << "durch" << endl;
			fi::BPEvent mainbp(0x1045f5);
			//fi::BPEvent mainbp(0x105bfa);
			sal::simulator.addEventAndWait(&mainbp);
		}
	
		int i;
		for(i=0 ; i<= 0 ; i++){
			cout << "Interrupt wird ausgeloest" << endl;
			//sleep(1);
			sal::simulator.fireInterrupt(9);
		}
	}
#elif 1
    // STEP 2
	sal::simulator.dbgEnableInstrPtrOutput(500);

#endif
	
	return true;
}
