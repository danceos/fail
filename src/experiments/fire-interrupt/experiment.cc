/**
 * \brief This experiment demonstrates the fireInterrupt feature.
 *
 * The keyboard-interrupts are disabled. So nothing happens if you press a button
 * on keyboard. Only the pressed button will be stored in keyboard-buffer. With
 * the fireInterrupt feature keyboard-interrupts are generated manually. The result
 * is that the keyboard interrupts will be compensated manually. bootdisk.img can
 * be used as example image (Turbo-Pacman :-) ).
 */

#include <iostream>

#include "experiment.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_DISABLE_KEYB_INTERRUPTS) || !defined(CONFIG_FIRE_INTERRUPTS)
  #error This experiment needs: breakpoints, disabled keyboard interrupts and fire interrupts. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool FireInterruptExperiment::run()
{
	Logger log("FireInterrupt", false);
	log << "experiment start" << endl;

#if 1
	while (true) {
		int j = 0;
		for (j = 0; j <= 100; j++) {
			BPSingleListener mainbp(0x1045f5);
			simulator.addListenerAndResume(&mainbp);
		}
		simulator.fireInterrupt(1);
	}
#endif

	return true;
}
