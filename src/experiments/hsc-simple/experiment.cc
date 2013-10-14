#include <iostream>
#include <unistd.h>

#include "experiment.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || !defined(CONFIG_SR_SAVE)
  #error This experiment needs: breakpoints, save, and restore. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool HSCSimpleExperiment::run()
{
	Logger log("HSC", false);
	log << "experiment start" << endl;

	// do funny things here...
#if 1
	// STEP 1
	BPSingleListener mainbp(0x00003c34);
	simulator.addListenerAndResume(&mainbp);
	log << "breakpoint reached, saving" << endl;
	simulator.save("hello.state");
#elif 0
	// STEP 2
	log << "restoring ..." << endl;
	simulator.restore("hello.state");
	log << "restored!" << endl;

	log << "waiting for last square() instruction" << endl;
	BPSingleListener breakpoint(0x3c9e); // square(x) ret instruction
	simulator.addListenerAndResume(&breakpoint);
	log << "injecting hellish fault" << endl;
	// RID_CAX is the RAX register in 64 bit mode and EAX in 32 bit mode:
	Register* reg = simulator.getCPU(0).getRegister(RID_CAX);
	simulator.getCPU(0).setRegisterContent(reg, 666);
	log << "waiting for last main() instruction" << endl;
	breakpoint.setWatchInstructionPointer(0x3c92);
	simulator.addListenerAndResume(&breakpoint);

	log << "reached" << endl;

	simulator.addListenerAndResume(&breakpoint);
#endif

	return true;
}
