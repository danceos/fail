#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;

char const * const results_csv = "l4sys.csv";
const char *l4sys_output_strings[] = { "Unknown", "Done", "Timeout", "Trap", "Interrupt", "Wrong output", "Error" };

std::string L4SysCampaign::output_result(L4SysProtoMsg_ResultType res) {
#define OUTPUT_CASE(OUTPUT) case L4SysProtoMsg::OUTPUT: return l4sys_output_strings[L4SysProtoMsg::OUTPUT];
	switch (res) {
	OUTPUT_CASE(DONE);
	OUTPUT_CASE(TIMEOUT);
	OUTPUT_CASE(TRAP);
	OUTPUT_CASE(INTR);
	OUTPUT_CASE(WRONG);
	OUTPUT_CASE(UNKNOWN);
	default:
		return l4sys_output_strings[0];
	}
#undef OUTPUT_CASE
}

bool L4SysCampaign::run() {
	Logger log("L4SysCampaign");

#if 0
	ifstream test(results_csv);
	if (test.is_open()) {
		log << results_csv << " already exists" << endl;
		return false;
	}
#endif
	ofstream results(results_csv);
	if (!results.is_open()) {
		log << "failed to open " << results_csv << endl;
		return false;
	}

	log << "startup" << endl;

	int count = 0;
	srand(time(NULL));
	for (int i = 0; i < 3000; ++i) {
		L4SysExperimentData *d = new L4SysExperimentData;
		d->msg.set_exp_type(d->msg.RATFLIP);
		d->msg.set_instr_offset(rand() % L4SYS_NUMINSTR);
		// 15 bytes (120 bits) are the longest instruction Bochs still executes
		int bit_offset = rand() % 120;
		d->msg.set_bit_offset(bit_offset);

		campaignmanager.addParam(d);
		++count;
	}
	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// collect results
	L4SysExperimentData *res;
	int rescount = 0;
	results
			<< "injection_ip,instr_offset,injection_bit,resulttype,resultdata,output,details"
			<< endl;
	while ((res = static_cast<L4SysExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		results << hex << res->msg.injection_ip() << "," << dec
				<< res->msg.instr_offset() << "," << res->msg.bit_offset()
				<< "," << output_result(res->msg.resulttype()) << ","
				<< res->msg.resultdata();
		if (res->msg.has_output())
			results << "," << res->msg.output();
		if (res->msg.has_details())
			results << "," << res->msg.details();
		results << endl;
		delete res;
	}

	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
