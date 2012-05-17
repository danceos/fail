#include <iostream>
#include <unistd.h>

#include "experiment.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_DISABLE_KEYB_INTERRUPTS) || !defined(CONFIG_FIRE_INTERRUPTS)
  #error This experiment needs: breakpoints, disabled keyboard interrupts and fire interrupts. Enable these in the configuration.
#endif

/* This experiment demonstrates the fireInterrupt feature.
 * The keyboard-interrupts are disabled. So nothing happens if you press a button on keyboard.
 * Only the pressed button will be stored in keyboard-buffer.
 * With the fireInterrupt feature keyboard-interrupts are generated manually.
 * The result is that the keyboard interrupts will be compensated manually.
 * bootdisk.img can be used as example image. (turbo-pacman :) )
 */

using std::endl;

bool fireinterruptExperiment::run()
{
	Logger log("FireInterrupt", false);
	log << "experiment start" << endl;
#if 1
    while(1){
		int j = 0;
		for(j=0 ; j<=100 ; j++){
			fi::BPEvent mainbp(0x1045f5);
			sal::simulator.addEventAndWait(&mainbp);
		}
		sal::simulator.fireInterrupt(1);
	}
#elif 1

	sal::simulator.dbgEnableInstrPtrOutput(500);
	
#endif
	
	sal::simulator.clearEvents(this);
	return true;
}
