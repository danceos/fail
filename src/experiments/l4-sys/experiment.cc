#include <iostream>

#include "experiment.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;

L4SysExperiment::L4SysExperiment()
 : log("L4Sys", false)
{
	 param = new L4SysExperimentData;
}

L4SysExperiment::~L4SysExperiment() {
	destroy();
}

void L4SysExperiment::destroy() {
	delete param;
	param = NULL;
}

void L4SysExperiment::terminate(int reason) {
	destroy();
	simulator.terminate(reason);
}

bool L4SysExperiment::run()
{
	srand(time(NULL));

	parseOptions(conf);

	switch(conf.step) {
		case L4SysConfig::GET_CR3: {
			log << "CR_3Run: Watching for instruction " << hex << conf.func_entry << endl;
    	runToStart(new BPSingleListener(0));
			log << "CR3 = " << hex << conf.address_space << endl;
			break;
		}
		case L4SysConfig::CREATE_CHECKPOINT: {
			// STEP 1: run until interesting function starts, and save state
			// -> needs L4SYS_ADDRESS_SPACE, because it runs until L4SYS_FUNC_ENTRY
    	runToStart(new BPSingleListener(0, conf.address_space));
			simulator.save(conf.state_folder);
			break;
		}
		case L4SysConfig::COLLECT_INSTR_TRACE: {
			// STEP 2: determine instructions executed
	    collectInstructionTrace(new BPSingleListener(0, ANY_ADDR));
			break;
		}
		case L4SysConfig::GOLDEN_RUN: {
			// STEP 3: determine the output of a "golden run"
			// -> golden run needs L4SYS_ADDRESS_SPACE as it breaks on
			//    L4SYS_FUNC_EXIT
   		goldenRun(new BPSingleListener(0, conf.address_space));
			break;
		}
		case L4SysConfig::FULL_PREPARATION: {
    	runToStart(new BPSingleListener(0));
			simulator.save(conf.state_folder);
      simulator.clearListeners();
	    collectInstructionTrace(new BPSingleListener(0, ANY_ADDR));
      simulator.clearListeners();
   		goldenRun(new BPSingleListener(0, conf.address_space));
			break;
		}
		default: {
			BPSingleListener *bp = 0;
			m_jc = fail::JobClient(conf.campain_server.c_str());
			doExperiments(bp);
		}
	}


  terminate(0);
	// experiment successfully conducted
	return true;
}
