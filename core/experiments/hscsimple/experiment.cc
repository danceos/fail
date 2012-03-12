#include <iostream>

#include <unistd.h>

#include "experiment.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"

using std::cout;
using std::endl;

bool hscsimpleExperiment::run()
{
	cout << "[HSC] experiment start" << endl;

	// do funny things here...
#if 0
	fi::BPEvent mainbp(0x00003c34);
	sal::simulator.addEventAndWait(&mainbp);
	cout << "[HSC] breakpoint reached, saving" << endl;
	sal::simulator.save("hello.main");
#elif 1
	cout << "[HSC] restoring ..." << endl;
	sal::simulator.restore("hello.main");
	cout << "[HSC] restored!" << endl;

	cout << "[HSC] waiting for last square() instruction" << endl;
	fi::BPEvent breakpoint(0x3c9e); // square(x) ret instruction
	sal::simulator.addEventAndWait(&breakpoint);
	cout << "[HSC] injecting hellish fault" << endl;
#if BX_SUPPORT_X86_64
	int reg = sal::RID_RAX;
#else
	int reg = sal::RID_EAX;
#endif
	sal::simulator.getRegisterManager().getRegister(reg)->setData(666);
	cout << "[HSC] waiting for last main() instruction" << endl;
	breakpoint.setWatchInstructionPointer(0x3c92);
	sal::simulator.addEventAndWait(&breakpoint);

	cout << "[HSC] reached" << endl;

	// FIXME this shouldn't fail:
	sal::simulator.addEventAndWait(&breakpoint);
#endif
	
	return true;
}
