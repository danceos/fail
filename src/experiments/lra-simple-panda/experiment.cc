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
#include "util/WallclockTimer.hpp"

#include "efw/JobClient.hpp"

#include "lra_simple.pb.h"
#include "campaign.hpp"

#include "config/FailConfig.hpp"

#include <fstream>

#include <unistd.h>

#include <math.h>

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, watchpoints and traps. Enable them in the configuration.
#endif

#define PREPARATION 0

int32_t correct_result[100] = {
		 0, 6, 5, 1, 6, 2, 7, 3,
		 8, 4, 1, 6, 5, 9, 5, 3,
		 0, 6, 5, 3, 7, 1, 7, 2,
		 8, 3, 9, 4, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0
};

// ToDo: Move this functionality to SimulatorController
bool LRASimplePandaExperiment::navigateToInjectionPoint(ConcreteInjectionPoint &ip) {
	Logger log_nav("navigator", false);

#ifdef CONFIG_INJECTIONPOINT_HOPS
	// Hop nav
	InjectionPointMessage ipm;
	ip.copyInjectionPointMessage(ipm);
	if (ipm.has_checkpoint_id()) {
		// ToDo: Load CP state!

		log_nav << "FATAL ERROR: CPs not yet implemented!" << endl;
		simulator.terminate(1);
	}

	log_nav << "Navigating to next instruction at navigational costs of " << ipm.costs() << endl;
	log_nav << "Length of hop-chain: " << ipm.hops_size() << endl;

	for (int i = 0; i < ipm.hops_size(); i++) {
		InjectionPointMessage_Hops h = ipm.hops(i);
		BaseListener *hop, *ev;

		// Nav-fail-bp
		BPSingleListener ev_func_end(elfReader->getSymbol("main").getEnd() - 4 - LRASP_MAIN_ENDOFFSET);
		simulator.addListener(&ev_func_end);

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

		if (ev == &ev_func_end) {
			log_nav << "Navigational error..." << endl;
			simulator.clearListeners();
			return false;
		} else {
			simulator.removeListener(&ev_func_end);
		}

		if (ev != hop) {
			log_nav << "FATAL ERROR: Unexpected event while navigating!" << endl;
			simulator.terminate(1);
		}
	}
#else
	// Step nav
	InjectionPointMessage ipm;
	ip.copyInjectionPointMessage(ipm);
	log_nav << "Navigating to instruction " << ipm.injection_instr() << endl;
	BPSingleListener step(ANY_ADDR);
	step.setCounter(ipm.injection_instr());
	simulator.addListenerAndResume(&step);
#endif
	return true;
}

bool LRASimplePandaExperiment::run()
{
	Logger logger("lra-simpla-panda", false);
	logger << "Startup" << endl;

	elfReader = new ElfReader();
	fail::JobClient *jobClient = new JobClient();

#if PREPARATION == 1
	logger << "Preparation mode" << endl;

	// STEP 1: run until main starts, save state, record trace
	BPSingleListener func_begin(elfReader->getSymbol("main").getStart());
	simulator.addListenerAndResume(&func_begin);

	logger << "test_func() reached, beginning trace recording" << endl;

	TracingPlugin tp;

	ogzstream of(LRASP_TRACE);
	if (of.bad()) {
		logger << "FATAL ERROR: Trace file could not be opened." << endl;
		simulator.terminate();
		return false;
	}

	tp.setOstream(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	BPSingleListener func_end(elfReader->getSymbol("main").getEnd() - 4 - 28);
	simulator.addListener(&func_end);
	BPSingleListener step(ANY_ADDR);

	WallclockTimer timer;
	timer.startTimer();

	// count instructions
	long counter = 0;
	while (true) {
		BaseListener *l = simulator.addListenerAndResume(&step);
		if (l == &func_end) {
			break;
		}
		counter++;
		if ((counter % 1000) == 0) {
			timer.stopTimer();
			logger << "Traced " << counter << " insturctions in " << timer << " seconds" << endl;
			timer.reset();
			timer.startTimer();
		}
	}
	logger << "Traced " << counter << " insturctions in " << timer << " seconds" << endl << endl;

	logger << "golden run took " << dec << counter << " instructions" << endl;
	simulator.removeFlow(&tp);

	of.flush();
	// serialize trace to file
	if (of.fail()) {
		logger << "failed to write to trace file"<< std::endl;
		return false;
	}
	of.close();
#else // PREPARATION

	unsigned executed_jobs = 0;

	while (true) {
		logger << "asking jobserver for parameters. Undone: "<<jobClient->getNumberOfUndoneJobs() << endl;
		LraSimpleExperimentData param;
		if (!jobClient->getParam(param)) {
			logger << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}

		logger << "New param" << param.msg.DebugString() << endl;

		// Get input data from	Jobserver
		unsigned injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();
		// unsigned data_width = param.msg.fsppilot().data_width();
		ConcreteInjectionPoint ip;
		ip.parseFromCampaignMessage(param.msg.fsppilot());

		for (unsigned experiment_id = 0; experiment_id < LRASP_NUM_EXP_PER_PILOT; ++experiment_id) {
			LraSimpleProtoMsg_Result *result = param.msg.add_result();
			executed_jobs ++;

			WallclockTimer timer;
			timer.startTimer();

			/********************
			 *  INITIALIZATION  *
			 ********************/

			logger << "rebooting device" << endl;

			// Restore to the image, which starts at address(main)
			simulator.reboot();

			// In this case, no entry navigation is needed
			// Otherwise we will have to navigate to the pre entry instruction!!!
			BPSingleListener func_begin(elfReader->getSymbol("main").getStart());
			simulator.addListenerAndResume(&func_begin);

			timer.stopTimer();
			result->set_time_init(float(timer));
			timer.reset();

			/****************
			 *  NAVIGATION  *
			 ****************/
			logger << "Fastforwarding to instr #" << injection_instr << endl;

			timer.startTimer();
			if (!navigateToInjectionPoint(ip)) {
				continue;
			}
			timer.stopTimer();
			result->set_time_nav(float(timer));
			timer.reset();

			/***************
			 *  INJECTION  *
			 ***************/
			// We do a 8-bit flip burst

			// Alternatively for single-bit-flips:
			// data ^= 1 << experiment_id;

			logger << "Injection bitflips at address " << hex << data_address << dec << endl;

			timer.startTimer();
			byte_t data = simulator.getMemoryManager().getByte(data_address);
			data = ~data;
			simulator.getMemoryManager().setByte(data_address, data); // write back data to register
			timer.stopTimer();
			result->set_time_inject(float(timer));
			timer.reset();

			/***************
			 *  AFTERMATH  *
			 ***************/
			logger << "Aftermath: Checking for wrong mem access, traps, timeout, wrong result or success" << endl;

			timer.startTimer();

			// Setup MMU
			/*
			 * As the false positive rate is very high, if we watch a
			 * page with actively used memory and because of the high
			 * costs for a false positive (in the magnitude of seconds),
			 * we don't want to watch those areas.
			 *
			 * We must not watch the following areas:
			 * - UART I/O Port at 0x48020014 => don't watch page 0x48020000
			 * - .text-, .bss- and .data-segment at 0x83000000 to 0x83411000 (exclusive)
			 * - Stack at 0xBFD00000 to 0xC0000000
			 */

			MemAccessListener ev_unauth_mem_acc_1(0x0);
			ev_unauth_mem_acc_1.setWatchWidth(0x48020000);
			simulator.addListener(&ev_unauth_mem_acc_1);

			MemAccessListener ev_unauth_mem_acc_2(0x48021000);
			ev_unauth_mem_acc_2.setWatchWidth(0x83000000 - 0x48021000);
			simulator.addListener(&ev_unauth_mem_acc_2);

			MemAccessListener ev_unauth_mem_acc_3(0x83411000);
			ev_unauth_mem_acc_3.setWatchWidth(0xBFD00000 - 0x83411000);
			simulator.addListener(&ev_unauth_mem_acc_3);

			MemAccessListener ev_unauth_mem_acc_4(0xC0000000);
			ev_unauth_mem_acc_4.setWatchWidth(0xFFFFFFFF - 0xC0000000);
			simulator.addListener(&ev_unauth_mem_acc_4);

			// Traps

			TrapListener ev_trap(fail::ANY_TRAP, NULL);
			simulator.addListener(&ev_trap);

			// Timeout

			TimerListener ev_timeout(LRASP_TIMEOUT);
			simulator.addListener(&ev_timeout);

			// Termination

			BPSingleListener ev_func_end(elfReader->getSymbol("main").getEnd() - 4 - LRASP_MAIN_ENDOFFSET);
			simulator.addListener(&ev_func_end);

			logger << "waiting for function exit, trap or timeout(1 second)" << endl;
			BaseListener* ev = simulator.resume();

			if (ev == &ev_timeout) {
				logger << "Timeout!" << endl;
				result->set_resulttype(result->ERR_TIMEOUT);
			} else if (ev == &ev_trap) {
				logger << "Trap!" << endl;
				result->set_resulttype(result->ERR_TRAP);
			} else if (ev == &ev_unauth_mem_acc_1 ||
					ev == &ev_unauth_mem_acc_2 ||
					ev == &ev_unauth_mem_acc_3 ||
					ev == &ev_unauth_mem_acc_4) {

				result->set_resulttype(result->ERR_OUTSIDE_TEXT);
				logger << hex << "Unauthorized memory access (" << ((((MemAccessListener*)ev)->getTriggerAccessType() == fail::MemAccessEvent::MEM_READ) ? "R" : "W") <<
						") at instruction " << ((MemAccessListener*)ev)->getTriggerInstructionPointer() << " access address: " <<
						((MemAccessListener*)ev)->getTriggerAddress() << dec << endl;
			} else if (ev == &ev_func_end) {
				logger << "Function terminated!" << endl;

				int32_t results[elfReader->getSymbol("result").getSize()/sizeof(int32_t)];
				simulator.getMemoryManager().getBytes(elfReader->getSymbol("result").getAddress(),
						elfReader->getSymbol("result").getSize(),
						(unsigned char*)results);

				result->set_resulttype(result->OK);
				for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
					if (correct_result[i] != results[i]) {
						result->set_resulttype(result->ERR_WRONG_RESULT);
						break;
					}
				}
			}

			timer.stopTimer();
			result->set_time_aftermath(float(timer));

			result->set_experiment_number(executed_jobs);

			simulator.clearListeners();
		}

		jobClient->sendResult(param);
		logger << param.debugString();
	}

	logger << "jobClient.getNumberOfUndoneJobs() = " << jobClient->getNumberOfUndoneJobs() << "executed_jobs = " << executed_jobs << endl;

#endif // PREPARATION

	delete elfReader;
	delete jobClient;

	simulator.terminate();
	return true;
}
