#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "util/Logger.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "sal/Register.hpp"
#include "sal/Memory.hpp"
#include "config/FailConfig.hpp"
#include "util/WallclockTimer.hpp"

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE)
  #error This experiment needs: breakpoints and watchpoints. Enable them in the configuration.
#endif

#define PREPARATION 1

bool LRASimplePandaExperiment::run()
{
	Logger log("lra-simple-panda", false);

#if PREPARATION == 1
	// STEP 1: run until main starts, save state, record trace
	// TODO: store golden run output
	BPSingleListener func_begin(LRASP_ADDR_FUNC_BEGIN);
	simulator.addListenerAndResume(&func_begin);

	log << "test_func() reached, beginning trace recording" << endl;

	TracingPlugin tp;
	tp.setLogIPOnly(true);
	ofstream of(LRASP_TRACE);
	if (!of.is_open()) {
		log << "FATAL ERROR: Trace file could not be opened." << endl;
		simulator.terminate();
		return false;
	}

	tp.setTraceFile(&of);
	//tp.setOstream(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// count instructions
	BPSingleListener func_end(LRASP_ADDR_FUNC_END);
	simulator.addListener(&func_end);
	BPSingleListener step(ANY_ADDR);
	long counter = 0;
	while (true) {
		BaseListener *l = simulator.addListenerAndResume(&step);
		if (l == &func_end) {
			break;
		}
		counter++;
	}
	log << "golden run took " << dec << counter << " instructions" << endl;
	simulator.removeFlow(&tp);
	of.close();
#else

	log << "Startup!" << endl;

	ConcreteCPU cpu = simulator.getCPU(0);
	for (ConcreteCPU::iterator it = cpu.begin(); it != cpu.end(); it++) {
		Register* pReg = *it; // get a ptr to the current register-object
		for (regwidth_t bitnr = 0; bitnr < pReg->getWidth(); ++bitnr) {
			int inst_offs;
			for (inst_offs = 0; inst_offs < 60; ++inst_offs) {
				WallclockTimer timer;
				timer.startTimer();

				unsigned int inj_inst = 0x830004a0 + inst_offs*4;

				static int experiment_counter = 1;
				log << "======================" << endl;
				log << "= Experiment " << setfill('0') << setw(7) << experiment_counter++ << " =" <<  endl;
				log << "======================" << endl;

				/*
				 * Trace navigation
				 */
				{
					log << "NavBP " << hex << inj_inst << dec << endl;
					BPSingleListener* bp_x;
					BPSingleListener bp(inj_inst);
					bp_x = (BPSingleListener*)simulator.addListenerAndResume(&bp);
					log << "Haltet at instruction " << hex << bp_x->getTriggerInstructionPointer() << endl;
				}

				/*
				 * Fault injection in register file
				 */
				log << "Injecting bit flip to register " << pReg->getName() << " at bit position " << dec << bitnr
						<<" at instruction " << hex << inj_inst << dec << endl;
				regdata_t data = cpu.getRegisterContent(pReg);
				data ^= 1 << bitnr;
				cpu.setRegisterContent(pReg, data); // write back data to register

				/*
				 * Aftermath: Check for traps, timeout or normal termination
				 */
				BPSingleListener ev_trap(0x80e80010);
				simulator.addListener(&ev_trap);

				TimerListener ev_timeout(LRASP_TIMEOUT); // 1000 millisecond timeout
				simulator.addListener(&ev_timeout);

				BPSingleListener ev_func_end(0x83000590);
				simulator.addListener(&ev_func_end);

				log << "waiting for function exit, trap or timeout(1 second)" << endl;
				BaseListener* ev = simulator.resume();

				if (ev == &ev_timeout) {
					log <<  "Timeout!" << endl;
				} else if (ev == &ev_trap) {
					log << "Trap!" << endl;
				} else if (ev == &ev_func_end) {
					log << "Function terminated!" << endl;

					uint32_t results[LRASP_RESULTS_BYTES/sizeof(uint32_t)];
					simulator.getMemoryManager().getBytes(LRASP_RESULT_ADDRESS, LRASP_RESULTS_BYTES, (unsigned char*)results);

					for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
						log << "results[" << i << "]: " << dec << results[i] << endl;
					}
				}

				simulator.reboot();
				log << "Experiment time: " << timer << endl;
			}
		}
	}

#endif // PREPARATION

	simulator.terminate();
	return true;
}
