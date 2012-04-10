#include <iostream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "controller/CampaignManager.hpp"
#include "util/Logger.hpp"

using namespace fi;
using std::endl;

char const * const results_csv = "chksumoostubs.csv";

//TODO: generate new values for the updated experiment
const unsigned memoryMap[49][2] = {
{0x109134,	4},
{0x10913c,	4},
{0x109184,	4},
{0x1091cc,	1},
{0x109238,	256},
{0x109344,	4},
{0x109350,	4},
{0x109354,	4},
{0x109368,	1},
{0x109374,	4},
{0x109388,	1},
{0x109398,	4},
{0x1093a0,	4},
{0x1093b4,	4},
{0x1093b8,	4},
{0x1093c0,	4},
{0x1093d0,	4},
{0x1093dc,	4},
{0x1093e0,	4},
{0x1093e8,	1},
{0x1093f0,	4},
{0x1093f4,	4},
{0x10a460,	4},
{0x10a468,	4},
{0x10a470,	4},
{0x10a478,	4},
{0x10a480,	4},
{0x10a488,	4},
{0x10a494,	4},
{0x10a498,	4},
{0x10a4a8,	4},
{0x10a4ad,	1},
{0x10a4b4,	4},
{0x10a4b8,	4},
{0x10a4c8,	4},
{0x10a4cd,	1},
{0x10a4d4,	4},
{0x10a4d8,	4},
{0x10a4e8,	4},
{0x10a4ed,	1},
{0x10a4f4,	4},
{0x10a4f8,	4},
{0x10a500,	4},
{0x10d350,	4},
{0x10d358,	4},
{0x10d37c,	4},
{0x10d384,	4},
{0x10d3a8,	4},
{0x10d3b0,	4},
};



bool ChecksumOOStuBSCampaign::run()
{
	Logger log("ChecksumOOStuBS Campaign");

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

	unsigned count = 0;

	for(int member = 0; member < 49; ++member){ //TODO: 49 -> constant
		for (int bit_offset = 0; bit_offset < (memoryMap[member][1])*8; ++bit_offset) {
			for (int instr_offset = 0; instr_offset < OOSTUBS_NUMINSTR; ++instr_offset) {
				CoolChecksumExperimentData *d = new CoolChecksumExperimentData;
				/*
				d->msg.set_instr_offset(instr_offset);
				d->msg.set_mem_addr(memoryMap[member][0]);
				d->msg.set_bit_offset(bit_offset);
				*/
	  
				fi::campaignmanager.addParam(d);
				++count;
			}
		}
	}

	#if 0
	for (int bit_offset = 0; bit_offset < COOL_ECC_OBJUNDERTEST_SIZE*8; ++bit_offset) {
		for (int instr_offset = 0; instr_offset < OOSTUBS_NUMINSTR; ++instr_offset) {
			CoolChecksumExperimentData *d = new CoolChecksumExperimentData;
			d->msg.set_instr_offset(instr_offset);
			d->msg.set_mem_addr(0x0);
			d->msg.set_bit_offset(bit_offset);
	  
			fi::campaignmanager.addParam(d);
			++count;
		}
	}
	#endif
	
	fi::campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// collect results
	CoolChecksumExperimentData *res;
	int rescount = 0;
	results << "injection_ip\tinstr_offset\tinjection_bit\tresulttype\tresultdata\terror_corrected\tdetails" << endl;
	while ((res = static_cast<CoolChecksumExperimentData *>(fi::campaignmanager.getDone()))) {
		rescount++;

		/*
		results
		 << res->msg.injection_ip() << "\t"
		 << res->msg.instr_offset() << "\t"
		 << res->msg.mem_addr() << "\t"
		 << res->msg.bit_offset() << "\t"
		 << res->msg.resulttype() << "\t"
		 << res->msg.resultdata() << "\t"
		 << res->msg.error_corrected() << "\t"
		 << res->msg.details() << "\n";
		*/
		delete res;
	}
	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
