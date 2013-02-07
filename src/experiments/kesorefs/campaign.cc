#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"

//#if COOL_FAULTSPACE_PRUNING
//#include "../plugins/tracing/TracingPlugin.hpp"
//char const * const trace_filename = "trace.pb";
//#endif

using namespace std;
using namespace fail;

char const * const results_csv = "kesorefs.csv";

bool KesoRefCampaign::run()
{
	Logger log("KesoRefCampaign");
  fail::ElfReader m_elf("/proj/mmtmp41/hoffmann/kesoeval/cdx_x86");

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

  ElfReader elf;
  address_t injip = elf.getAddressByName("c23_PersistentDetectorScopeEntry_m5_run");

  address_t rambase = elf.getAddressByName("__CIAO_APPDATA_cdx_det__heap");
  address_t ramend = rambase + 0x80000;
    cout << "bla: " << hex << ramend << endl;
  //address_t ramend = rambase + 4;

  log << "startup, injecting ram @ " << hex << rambase << endl;

	int count = 0;
	for (address_t ram_address = rambase; ram_address < ramend ; ram_address += 4) {
		for (int bit_offset = 23; bit_offset < 24; ++bit_offset) {
			KesoRefExperimentData *d = new KesoRefExperimentData;

      d->msg.set_pc_address(injip);
			d->msg.set_ram_address(ram_address);
			d->msg.set_bit_offset(bit_offset);

			campaignmanager.addParam(d);
			++count;
		}
	}
	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << dec << count << ")." << endl;

	// collect results
	KesoRefExperimentData *res;
	int rescount = 0;
	results << "injection_ip\tram_address\tbit_offset\tresulttype\toriginal_value\tdetails" << endl;
	while ((res = static_cast<KesoRefExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		results
     << "0x" << hex << res->msg.pc_address() << "\t"
		 << "0x" << hex << res->msg.ram_address() << "\t"
		 << dec << res->msg.bit_offset() << "\t"
		 << res->msg.resulttype() << "\t"
		 << res->msg.original_value() << "\t"
		 << res->msg.details() << "\n";
		delete res;
	}

	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
