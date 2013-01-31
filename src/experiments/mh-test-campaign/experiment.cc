#include <iostream>

#include "experiment.hpp"
#include "MHTestCampaign.hpp"
#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Listener.hpp"

// FIXME: You should provide a dependency check here!

using namespace std;
using namespace fail;

bool MHTestExperiment::run()
{
	cout << "[MHTestExperiment] Let's go" << endl;
#if 0
	BPSingleListener mainbp(0x00003c34);
	simulator.addListenerAndResume(&mainbp);
	cout << "[MHTestExperiment] breakpoint reached, saving" << endl;
	simulator.save("hello.main");
#else
	MHExperimentData par;
	if (m_jc.getParam(par)) {
		int num = par.msg.input();
		cout << "[MHExperiment] stepping " << num << " instructions" << endl;
		if (num > 0) {
			BPSingleListener nextbp(ANY_ADDR);
			nextbp.setCounter(num);
			simulator.addListenerAndResume(&nextbp);
		}
		address_t instr = simulator.getCPU(0).getInstructionPointer();
		cout << "[MHTestExperiment] Reached instruction: "
		<< hex << instr
		<< endl;
		par.msg.set_output(instr);
		m_jc.sendResult(par);
	} else {
		cout << "No data for me? :(" << endl;
	}
#endif
	simulator.clearListeners(this);

	simulator.terminate();
	return true;
}
