#include "experiment.hpp"
#include "MHTestCampaign.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Register.hpp"
#include "controller/Event.hpp"

#include <iostream>

bool MHTestExperiment::run()
{
	
	cout << "[MHTestExperiment] Let's go" << endl;
#if 0
	fi::BPEvent mainbp(0x00003c34);
	sal::simulator.addEventAndWait(&mainbp);
	cout << "[MHTestExperiment] breakpoint reached, saving" << endl;
	sal::simulator.save("hello.main");
#else
	MHExperimentData par;
	if(m_jc.getParam(par)){

		int num = par.msg.input();
		cout << "[MHExperiment] stepping " << num << " instructions" << endl;
		if (num > 0) {
			fi::BPEvent nextbp(fi::ANY_ADDR);
			nextbp.setCounter(num);
			sal::simulator.addEventAndWait(&nextbp);
		}
		sal::address_t instr = sal::simulator.getRegisterManager().getInstructionPointer();
		cout << "[MHTestExperiment] Reached instruction: "
		<< hex << instr
		<< endl;
		par.msg.set_output(instr);
		m_jc.sendResult(par);
	} else {
		cout << "No data for me? :(" << endl;
	}
#endif
	sal::simulator.terminate();
	return true;
}

