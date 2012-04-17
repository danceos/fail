#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "SAL/SALConfig.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Memory.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"

// you need to have the tracing plugin enabled for this
#include "plugins/tracing/TracingPlugin.hpp"

#include "vptr_map.hpp"

#define LOCAL 1

using std::endl;

bool WeathermonitorExperiment::run()
{
	char const *statename = "bochs.state";
	Logger log("Weathermonitor", false);
	fi::BPEvent bp;
	
	log << "startup" << endl;

#if 1
	// STEP 0: record memory map with vptr addresses
	fi::GuestEvent g;
	while (true) {
		sal::simulator.addEventAndWait(&g);
		std::cout << g.getData() << std::flush;
	}
#elif 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(WEATHER_FUNC_MAIN);
	sal::simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << std::hex << bp.getTriggerInstructionPointer() << endl;
	sal::simulator.save(statename);
	assert(bp.getTriggerInstructionPointer() == WEATHER_FUNC_MAIN);
	assert(sal::simulator.getRegisterManager().getInstructionPointer() == WEATHER_FUNC_MAIN);
#elif 0
	// STEP 2: record trace for fault-space pruning
	// XXX

#elif 0
	// STEP 3: The actual experiment.
	// XXX

#endif
	// Explicitly terminate, or the simulator will continue to run.
	sal::simulator.terminate();
}
