#include "experiment.hpp"

#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "sal/Register.hpp"
#include "sal/Memory.hpp"
#include "config/FailConfig.hpp"
#include "util/WallclockTimer.hpp"

#include "util/gzstream/gzstream.h"

#include "efw/JobClient.hpp"

#include "lra_simple.pb.h"
#include "campaign.hpp"

#include "cpn/InjectionPoint.hpp"
#include "config/FailConfig.hpp"

#include <fstream>

#include <unistd.h>

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, watchpoints and traps. Enable them in the configuration.
#endif

#define PREPARATION 0


// ToDo: Move this functionality to SimulatorController
void LRASimplePandaExperiment::navigateToInjectionPoint(ConcreteInjectionPoint &ip) {
	Logger log_nav("navigator");

#ifdef CONFIG_INJECTIONPOINT_HOPS
	// Hop nav
	InjectionPointMessage ipm;
	ip.copyInjectionPointMessage(ipm);
	if (ipm.has_checkpoint_id()) {
		// ToDo: Load CP state!

		log_nav << "FATAL ERROR: CPs not yet implemented!"  << endl;
		simulator.terminate(1);
	}


	log_nav << "Navigating to next instruction at navigational costs of " << ipm.costs() << endl;
	log_nav << "Length of hop-chain: " << ipm.hops_size() << endl;

	for (int i = 0; i < ipm.hops_size(); i++) {
		InjectionPointMessage_Hops h = ipm.hops(i);
		BaseListener *hop, *ev;

		if (h.accesstype() == h.EXECUTE) {
			log_nav << "BP at " << hex << h.address() << dec << endl;
			BPSingleListener bp (h.address());
			hop = &bp;
			ev = simulator.addListenerAndResume(&bp);
		} else {
			log_nav << "WP at " << hex << h.address() << dec << "Access: " <<
					(h.accesstype() == h.READ ? "R" :"W")<< endl;
			MemAccessListener ma (h.address(), h.accesstype() == h.READ ?
					MemAccessEvent::MEM_READ : MemAccessEvent::MEM_WRITE);
			hop = &ma;
			ev = simulator.addListenerAndResume(&ma);
		}

		log_nav << "Halted" << endl;

		if (ev != hop) {
			log_nav << "FATAL ERROR: Unexpected event while navigating!" << endl;
			simulator.terminate(1);
		}
	}
#else
	// Step nav
	InjectionPointMessage ipm;
	ip.copyInjectionPointMessage(ipm);
	log_nav << ipm.injection_instr() << endl;
#endif
}

bool LRASimplePandaExperiment::run()
{
	Logger _log("lra-simpla-panda", false);
	_log << "Startup" << endl;

	fail::ElfReader elfReader;
	fail::JobClient jobClient;


#if PREPARATION == 1
	_log << "Preparation mode" << endl;
	// STEP 1: run until main starts, save state, record trace
	// TODO: store golden run output
	BPSingleListener func_begin(elfReader.getSymbol("test_code_due").getAddress());
	simulator.addListenerAndResume(&func_begin);

	_log << "test_func() reached, beginning trace recording" << endl;

	TracingPlugin tp;

	// ogzstream of(LRASP_TRACE);
	ofstream of(LRASP_TRACE);
	if (of.bad()) {
		_log << "FATAL ERROR: Trace file could not be opened." << endl;
		simulator.terminate();
		return false;
	}

	tp.setTraceFile(&of);
	//tp.setOstream(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	BPSingleListener func_end(elfReader.getSymbol("test_code_due").getAddress() + elfReader.getSymbol("test_code_due").getSize() - 4);
	simulator.addListener(&func_end);
	BPSingleListener step(ANY_ADDR);

	// count instructions
	long counter = 0;
	while (true) {
		BaseListener *l = simulator.addListenerAndResume(&step);
		if (l == &func_end) {
			break;
		}
		counter++;
		if ((counter % 1000) == 0) {
			_log << "Traced " << counter << " insturctions" << endl;
			of.flush();
		}
	}
	_log << "golden run took " << dec << counter << " instructions" << endl;
	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		_log << "failed to write to trace file"<< std::endl;
		return false;
	}
	of.close();
#else

	unsigned executed_jobs = 0;

	_log << "Startup!" << endl;

	// std::ofstream exp_output ("exp.txt");

	ConcreteCPU cpu = simulator.getCPU(0);
	ConcreteCPU::iterator it = cpu.begin();
	it++; it++;
	while (jobClient.getNumberOfUndoneJobs() > 0 || executed_jobs < 500) {
		_log << "asking jobserver for parameters" << endl;
		LraSimpleExperimentData param;
		if(!jobClient.getParam(param)){
			_log << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		// Get input data from	Jobserver
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();
		unsigned data_width = param.msg.fsppilot().data_width();
		ConcreteInjectionPoint ip;
		ip.parseFromCampaignMessage(param.msg.fsppilot());

		// ToDo: Insert into all 8 Bits
        for (int experiment_id = 0; experiment_id < 1; ++experiment_id) {
        	LraSimpleProtoMsg_Result *result = 0;

			_log << "rebooting device" << endl;

			// Restore to the image, which starts at address(main)
			simulator.reboot();
			executed_jobs ++;

			// Fast forward to injection address
			_log << "Trying to inject @ instr #" << dec << injection_instr << endl;

			navigateToInjectionPoint(ip);

			uint32_t iteration_counter;
			simulator.getMemoryManager().getBytes(elfReader.getSymbol("test_code_idx").getAddress(), 4, (void*)(&iteration_counter));

			result = param.msg.add_result();

			result->set_bitoffset(experiment_id);
			result->set_loop_iteration(iteration_counter);
			result->set_resulttype(result->OK);
			result->set_experiment_number(executed_jobs);

            simulator.clearListeners();
        }

        jobClient.sendResult(param);
    }

	_log << "jobClient.getNumberOfUndoneJobs() = " << jobClient.getNumberOfUndoneJobs() << "executed_jobs = " << executed_jobs << endl;

#endif // PREPARATION

	simulator.terminate();
	return true;
}
