#include "util/Logger.hpp"
#include "util/WallclockTimer.hpp"

#include "experiment.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"

#define PLUGIN_ENABLED 0

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMWRITE)
    !defined(CONFIG_EVENT_MEMREAD)
  #error This experiment needs break- and watchpoints. You may want to enable \
         Fast-Breakpoints or Fast-Watchpoints as well.
#endif

#if PLUGIN_ENABLED
#include "../plugins/tracing/TracingPlugin.hpp"
#endif

// Required to maintain functionality
#define MEM_NEVER_ACCESSED 0x20000001 // 512MB + 1
#define IP_NEVER_ACCESSED  0x0FFFFFFF
#define FINAL_INSTRUCTION  0x00003c34 // to be used with the hsc-simple eCos image

// Parameters:
#define NON_BP_COUNT 50
#define BP_COUNT     50

using namespace std;
using namespace fail;

bool PerfTestExperiment::run()
{
	Logger log("PERF", false);
	log << "Experiment started (measuring elapsed time using wallclock timer)..." << endl;

#if PLUGIN_ENABLED
	TracingPlugin tp;
	tp.setOstream(&std::cerr);
	simulator.addFlow(&tp);
#endif

	// Performance tests:
	WallclockTimer tm;
	tm.startTimer();
#if 1
	log << "Activated: CASE A (Best-Case)..." << endl;
	// Case A): A lot of non-BP listeners a only one (or none) BPs:

	log << "Adding " << NON_BP_COUNT << " non-BP listeners..." << endl;
	MemReadListener mrl[NON_BP_COUNT];
	for (unsigned i = 0; i < NON_BP_COUNT; ++i) {
		mrl[i].setWatchAddress(static_cast<address_t>(MEM_NEVER_ACCESSED));
		simulator.addListener(&mrl[i]);
	}
	log << "Adding one breakpoint listener and returning to simulator..." << endl;
	BPSingleListener bp(FINAL_INSTRUCTION);
	log << (simulator.addListenerAndResume(&bp) == &bp ? "Done!" : "Interrupted!") << endl;
#else
	log << "Activated: CASE B (Worst-Case)..." << endl;
	// Case B): n (non matching) BP listeners and no other listener types
	log << "Adding " << BP_COUNT << " BPSingleListeners..." << endl;
	BPSingleListener bsl[BP_COUNT];
	for (unsigned i = 0; i < BP_COUNT; ++i) {
		bsl[i].setWatchInstructionPointer(IP_NEVER_ACCESSED); // we do not want them to trigger...
		simulator.addListener(&bsl[i]);
	}
	log << "Adding final BPSingleListener and continuing simulation..." << endl;
	// This is required to terminate the experiment:
	BPSingleListener final(FINAL_INSTRUCTION);
	log << (simulator.addListenerAndResume(&final) == &final ? "Done!" : "Interrupted!") << endl;
#endif

	tm.stopTimer();
	log << "Time elapsed: " << tm << "s. Done, Bye!" << endl;
	simulator.terminate();
	return true;
}
