#include "util/Logger.hpp"
#include "util/WallclockTimer.hpp"

#include "experiment.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS)
  #error This experiment just needs breakpoints. You may want to enable Fast-Breakpoints as well.
#endif

using namespace std;
using namespace fail;

bool PerfTestExperiment::run()
{
	Logger log("PERF", false);
	log << "Experiment started (measuring ellapsed time using wallclock timer)..." << endl;

	// Performance tests:
	WallclockTimer tm;
	tm.startTimer();
#if 1
	log << "Activated: CASE A (Best-Case)..." << endl;
	// Case A): A lot of non-BP listeners a only one (or none) BPs:
	const unsigned NON_BP_COUNT = 50;
	log << "Adding " << NON_BP_COUNT << " non-BP listeners..." << endl;
	MemReadListener mrl[NON_BP_COUNT];
	for (unsigned i = 0; i < NON_BP_COUNT; ++i) {
		mrl[i].setWatchAddress(static_cast<address_t>(-1));
		simulator.addListener(&mrl[i]);
	}
	log << "Adding one breakpoint listener and returning to simulator..." << endl;
	BPSingleListener bp(0x00003c34);
	simulator.addListenerAndResume(&bp);
#else
	log << "Activated: CASE B (Worst-Case)..." << endl;
	// Case B): n (non matching) BP listeners and no other listener types
	const unsigned BP_COUNT = 50;
	log << "Adding " << BP_COUNT << " BPSingleListeners..." << endl;
	BPSingleListener bsl[BP_COUNT];
	for (unsigned i = 0; i < BP_COUNT; ++i) {
		bsl[i].setWatchInstructionPointer(0xFFFFFFF); // we do not want them to trigger...
		simulator.addListener(&bsl[i]);
	}
	log << "Adding final BPSingleListener and continuing simulation..." << endl;
	// This is required to terminate the experiment:
	BPSingleListener final(0x00003c34);
	simulator.addListenerAndResume(&final);
#endif

	tm.stopTimer();
	log << "Time elapsed: " << tm << "s. Done, Bye!" << endl;
	simulator.terminate();
	return true;
}
