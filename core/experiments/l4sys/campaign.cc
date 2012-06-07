#include <iostream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "controller/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "SAL/SALConfig.hpp"

using namespace std;
using namespace fail;

char const * const results_csv = "l4sys.csv";

bool L4SysCampaign::run()
{
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
	//iterate over one register
	for (int bit_offset = 0; bit_offset < 1; ++bit_offset) {
		for (int instr_offset = 0; instr_offset < L4SYS_NUMINSTR; ++instr_offset) {
			L4SysExperimentData *d = new L4SysExperimentData;
			d->msg.set_instr_offset(instr_offset);
			d->msg.set_bit_offset(bit_offset);
			d->msg.set_bit_offset(0);
	  
			campaignmanager.addParam(d);
			++count;
		}
	}
	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// collect results
	L4SysExperimentData *res;
	int rescount = 0;
	results << "injection_ip,instr_offset,injection_bit,resulttype,resultdata,output,details" << endl;
	while ((res = static_cast<L4SysExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		results << hex
		 << res->msg.injection_ip() << ","
		 << dec << res->msg.instr_offset() << ","
		 << res->msg.bit_offset() << ","
		 << res->msg.resulttype() << ","
		 << res->msg.resultdata();
		if(res->msg.has_output())
			results << "," << res->msg.output();
		if(res->msg.has_details())
			results << "," << res->msg.details();
		results << endl;
		delete res;
	}

	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
