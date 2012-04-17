#include <iostream>
#include <vector>
#include <map>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "controller/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/MemoryMap.hpp"

#include "vptr_map.hpp"

#include "plugins/tracing/TracingPlugin.hpp"
char const * const trace_filename = "trace.pb";

using namespace fi;
using std::endl;

char const * const results_csv = "weathermonitor.csv";

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
struct equivalence_class {
	sal::address_t data_address;
	int instr1, instr2;
	sal::address_t instr2_absolute;
};

bool WeathermonitorCampaign::run()
{
	Logger log("Weathermonitor Campaign");

	ifstream test(results_csv);
	if (test.is_open()) {
		log << results_csv << " already exists" << endl;
		return false;
	}
	ofstream results(results_csv);
	if (!results.is_open()) {
		log << "failed to open " << results_csv << endl;
		return false;
	}

	log << "startup" << endl;

	// XXX

	return true;
}
