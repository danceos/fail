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
char const *l4sys_output_result_strings[] = { "Unknown", "Done", "Incomplete", "Timeout", "Wrong output", "Error" };
char const *l4sys_output_experiment_strings[] = { "Unknown", "GPR Flip", "RAT Flip", "Instr Flip", "ALU Instr Flip" };
char const *l4sys_output_register_strings[] = { "Unknown", "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };

#define OUTPUT_CASE(OUTPUT) case L4SysProtoMsg::OUTPUT: return l4sys_output_result_strings[L4SysProtoMsg::OUTPUT];
std::string L4SysCampaign::output_result(L4SysProtoMsg_ResultType res) {
	switch (res) {
	OUTPUT_CASE(DONE);
	OUTPUT_CASE(INCOMPLETE);
	OUTPUT_CASE(TIMEOUT);
	OUTPUT_CASE(WRONG);
	OUTPUT_CASE(UNKNOWN);
	default:
		return l4sys_output_result_strings[0];
	}
}
#undef OUTPUT_CASE
#define OUTPUT_CASE(OUTPUT) case L4SysProtoMsg::OUTPUT: return l4sys_output_experiment_strings[L4SysProtoMsg::OUTPUT];
std::string L4SysCampaign::output_experiment(L4SysProtoMsg_ExperimentType res) {
	switch (res) {
	OUTPUT_CASE(GPRFLIP)
	OUTPUT_CASE(RATFLIP)
	OUTPUT_CASE(IDCFLIP)
	OUTPUT_CASE(ALUINSTR)
	default:
		return l4sys_output_experiment_strings[0];
	}
}
#undef OUTPUT_CASE
#define OUTPUT_CASE(OUTPUT) case L4SysProtoMsg::OUTPUT: return l4sys_output_register_strings[L4SysProtoMsg::OUTPUT];
std::string L4SysCampaign::output_register(L4SysProtoMsg_RegisterType res) {
	switch (res) {
	OUTPUT_CASE(EAX);
	OUTPUT_CASE(ECX);
	OUTPUT_CASE(EDX);
	OUTPUT_CASE(EBX);
	OUTPUT_CASE(ESP);
	OUTPUT_CASE(EBP);
	OUTPUT_CASE(ESI);
	OUTPUT_CASE(EDI);
	default:
		return l4sys_output_register_strings[0];
	}
}
#undef OUTPUT_CASE

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

	for (int i = 0; i < 1000; ++i) {
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
	for (int i = 0; i < 1000; ++i) {
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
	for (int i = 0; i < 1000; ++i) {
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
	for (int i = 0; i < 1000; ++i) {
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

		results << output_experiment(res->msg.exp_type()) << "," << hex << res->msg.injection_ip() << dec << ",";
		if (res->msg.has_register_offset())
			results << output_register(res->msg.register_offset());
		else
			results << "None";
		results	<< "," << res->msg.instr_offset() << "," << res->msg.bit_offset()
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
