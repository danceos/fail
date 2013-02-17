#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "conversion.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;

char const * const results_csv = "l4sys.csv";

extern L4SysConversion l4sysResultConversion;
extern L4SysConversion l4sysExperimentConversion;
extern L4SysConversion l4sysRegisterConversion;

bool L4SysCampaign::run() {
	Logger log("L4SysCampaign");

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

	int count = 0;
	srand(time(NULL));

	for (int i = 0; i < 20000; ++i) {
		L4SysExperimentData *d = new L4SysExperimentData;
		d->msg.set_exp_type(d->msg.GPRFLIP);
		// affect a random register
		int reg_offset = rand() % 8 + 1;
		d->msg.set_register_offset(
				static_cast<L4SysProtoMsg_RegisterType>(reg_offset));
		// modify for a random instruction
		int instr_offset = rand() % L4SYS_NUMINSTR;
		d->msg.set_instr_offset(instr_offset);
		// modify a random bit
		int bit_offset = rand() % 32;
		d->msg.set_bit_offset(bit_offset);

		campaignmanager.addParam(d);
		++count;
	}
	for (int i = 0; i < 20000; ++i) {
		L4SysExperimentData *d = new L4SysExperimentData;
		d->msg.set_exp_type(d->msg.ALUINSTR);
		// modify for a random instruction
		int instr_offset = rand() % L4SYS_NUMINSTR;
		d->msg.set_instr_offset(instr_offset);
		// this value is not required for this experiment, so set it to an arbitrary value
		d->msg.set_bit_offset(0);

		campaignmanager.addParam(d);
		++count;
	}
	for (int i = 0; i < 20000; ++i) {
		L4SysExperimentData *d = new L4SysExperimentData;
		d->msg.set_exp_type(d->msg.IDCFLIP);
		// modify for a random instruction
		int instr_offset = rand() % L4SYS_NUMINSTR;
		d->msg.set_instr_offset(instr_offset);
		// modify a random bit - Bochs supports at most 15 bytes of instruction
		int bit_offset = rand() % 125;
		d->msg.set_bit_offset(bit_offset);

		campaignmanager.addParam(d);
		++count;
	}
	for (int i = 0; i < 20000; ++i) {
		L4SysExperimentData *d = new L4SysExperimentData;
		d->msg.set_exp_type(d->msg.RATFLIP);
		// modify for a random instruction
		int instr_offset = rand() % L4SYS_NUMINSTR;
		d->msg.set_instr_offset(instr_offset);
		// this value is not required for this experiment, so set it to an arbitrary value
		d->msg.set_bit_offset(0);

		campaignmanager.addParam(d);
		++count;
	}

	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// collect results
	L4SysExperimentData *res;
	int rescount = 0;
	results
			<< "exp_type,injection_ip,register,instr_offset,injection_bit,resulttype,resultdata,output,details"
			<< endl;
	while ((res = static_cast<L4SysExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		results << l4sysExperimentConversion.output(res->msg.exp_type())
		        << "," << hex << res->msg.injection_ip() << dec << ",";
		if (res->msg.has_register_offset())
			results << l4sysRegisterConversion.output(res->msg.register_offset());
		else
			results << "None";
		results	<< "," << res->msg.instr_offset() << "," << res->msg.bit_offset()
				<< ","
				<< l4sysResultConversion.output(res->msg.resulttype()) << ","
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
