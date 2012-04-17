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
	log << "restoring state" << endl;
	sal::simulator.restore(statename);
	log << "EIP = " << std::hex << sal::simulator.getRegisterManager().getInstructionPointer() << endl;
	assert(sal::simulator.getRegisterManager().getInstructionPointer() == WEATHER_FUNC_MAIN);

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	mm.add(WEATHER_DATA_START, WEATHER_DATA_END - WEATHER_DATA_START);
	tp.restrictMemoryAddresses(&mm);
	//tp.setLogIPOnly(true);

	// record trace
	Trace trace;
	tp.setTraceMessage(&trace);

	// this must be done *after* configuring the plugin:
	sal::simulator.addFlow(&tp);

	bp.setWatchInstructionPointer(fi::ANY_ADDR);
	bp.setCounter(WEATHER_NUMINSTR);
	sal::simulator.addEvent(&bp);
	fi::BPEvent func_temp_measure(WEATHER_FUNC_TEMP_MEASURE);
	sal::simulator.addEvent(&func_temp_measure);

	int count_temp_measure;
	for (count_temp_measure = 0; sal::simulator.waitAny() == &func_temp_measure;
	     ++count_temp_measure) {
		log << "experiment reached Temperature::measure()" << endl;
		sal::simulator.addEvent(&func_temp_measure);
	}
	log << "experiment finished after " << std::dec << WEATHER_NUMINSTR << " instructions" << endl;
	log << "Temperature::measure() was called " << count_temp_measure << " times" << endl;

	sal::simulator.removeFlow(&tp);

	// serialize trace to file
	char const *tracefile = "trace.pb";
	std::ofstream of(tracefile);
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		return false;
	}
	trace.SerializeToOstream(&of);
	of.close();
	log << "trace written to " << tracefile << endl;

#elif 0
	// STEP 3: The actual experiment.
	// XXX

#endif
	// Explicitly terminate, or the simulator will continue to run.
	sal::simulator.terminate();
}
