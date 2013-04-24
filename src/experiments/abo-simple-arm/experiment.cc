#include "util/Logger.hpp"
#include "experiment.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) && !defined(CONFIG_EVENT_BREAKPOINTS_RANGE)
  #error This experiment needs: breakpoints. Enable them in the configuration.
#endif

bool ABOSimpleARMExperiment::run()
{
	Logger log("abo-simple", false);
	log << "Startup!" << endl;

	BPSingleListener* ret;
	BPSingleListener bp(0x800334);
	ret = (BPSingleListener*)simulator.addListenerAndResume(&bp);
	log << hex << ret->getTriggerInstructionPointer() << ": "
		<< simulator.getMnemonic() << endl;
	bp.setWatchInstructionPointer(ANY_ADDR);
	simulator.clearListeners();
	while ((ret = (BPSingleListener*)simulator.addListenerAndResume(&bp)) != 0) {
		log << hex << ret->getTriggerInstructionPointer() << ": "
		    << simulator.getMnemonic() << "   => Abort? " << flush;
		char ch;
		cin.clear(); cin.sync(); cin >> ch;
		cout << endl;
		if (ch == 'y' || ch == 'Y')
			break;
	}

	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
