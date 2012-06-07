#include <iostream>

#include <unistd.h>

#include "experiment.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || !defined(CONFIG_SR_SAVE)
  #error This experiment needs: breakpoints, save, and restore. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool hscsimpleExperiment::run()
{
	Logger log("HSC", false);
	log << "experiment start" << endl;

	// do funny things here...
#if 1
    // STEP 1
	BPSingleEvent mainbp(0x00003c34);
	simulator.addEventAndWait(&mainbp);
	log << "breakpoint reached, saving" << endl;
	simulator.save("hello.state");
#elif 0
    // STEP 2
	log << "restoring ..." << endl;
	simulator.restore("hello.state");
	log << "restored!" << endl;

	log << "waiting for last square() instruction" << endl;
	BPSingleEvent breakpoint(0x3c9e); // square(x) ret instruction
	simulator.addEventAndWait(&breakpoint);
	log << "injecting hellish fault" << endl;
	// RID_CAX is the RAX register in 64 bit mode and EAX in 32 bit mode:
	simulator.getRegisterManager().getRegister(RID_CAX)->setData(666);
	log << "waiting for last main() instruction" << endl;
	breakpoint.setWatchInstructionPointer(0x3c92);
	simulator.addEventAndWait(&breakpoint);

	log << "reached" << endl;

	simulator.addEventAndWait(&breakpoint);
#endif

	simulator.clearEvents(this);
	return true;
}
