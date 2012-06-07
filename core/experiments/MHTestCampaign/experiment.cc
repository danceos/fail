#include <iostream>

#include "experiment.hpp"
#include "MHTestCampaign.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Register.hpp"
#include "controller/Event.hpp"

// FIXME: You should provide a dependency check here!

using namespace std;
using namespace fail;

bool MHTestExperiment::run()
{
	cout << "[MHTestExperiment] Let's go" << endl;
#if 0
	BPSingleEvent mainbp(0x00003c34);
	simulator.addEventAndWait(&mainbp);
	cout << "[MHTestExperiment] breakpoint reached, saving" << endl;
	simulator.save("hello.main");
#else
	MHExperimentData par;
	if (m_jc.getParam(par)) {
		int num = par.msg.input();
		cout << "[MHExperiment] stepping " << num << " instructions" << endl;
		if (num > 0) {
			BPSingleEvent nextbp(ANY_ADDR);
			nextbp.setCounter(num);
			simulator.addEventAndWait(&nextbp);
		}
		address_t instr = simulator.getRegisterManager().getInstructionPointer();
		cout << "[MHTestExperiment] Reached instruction: "
		<< hex << instr
		<< endl;
		par.msg.set_output(instr);
		m_jc.sendResult(par);
	} else {
		cout << "No data for me? :(" << endl;
	}
#endif
	simulator.clearEvents(this);

	simulator.terminate();
	return true;
}
