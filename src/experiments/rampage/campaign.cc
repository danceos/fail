#include <iostream>
#include <fstream>
#include <set>

#include "cpn/CampaignManager.hpp"
#include "campaign.hpp"
#include "experimentInfo.hpp"

using namespace std;
using namespace fail;

char const * const results_filename = "rampage.csv";

bool RAMpageCampaign::run()
{
	// read already existing results
	bool file_exists = false;
	set<uint64_t> existing_results;
	ifstream oldresults(results_filename, ios::in);
	if (oldresults.is_open()) {
		char buf[16*1024];
		uint64_t addr;
		int count = 0;
		m_log << "scanning existing results ..." << endl;
		file_exists = true;
		while (oldresults.getline(buf, sizeof(buf)).good()) {
			stringstream ss;
			ss << buf;
			ss >> addr;
			if (ss.fail()) {
				continue;
			}
			++count;
			if (!existing_results.insert(addr).second) {
				m_log << "duplicate: " << addr << endl;
			}
		}
		m_log << "found " << dec << count << " existing results" << endl;
		oldresults.close();
	}

	// non-destructive: due to the CSV header we can always manually recover
	// from an accident (append mode)
	ofstream results(results_filename, ios::out | ios::app);
	if (!results.is_open()) {
		m_log << "failed to open " << results_filename << endl;
		return false;
	}
	// only write CSV header if file didn't exist before
	if (!file_exists) {
		results << "addr\tbit1\tbit2\terrortype\tlocal_timeout\tglobal_timeout\tmem_written\tresulttype\terror_detected_pfn\texperiment_time\tdetails" << endl;
	}

	// count address bits needed for memory size
	unsigned address_bits = 0;
	for (uint64_t i = 1; i < MEM_SIZE; i <<= 1) {
		++address_bits;
	}
	m_log << dec << MEM_SIZE << "b mem needs "
	      << address_bits << " address bits" << endl;

	// systematically march through the fault space
	for (uint64_t n = 0; n < 1024*256; ++n) {
		uint64_t addr = reverse_bits(n) >> (64 - address_bits);
		if (addr >= MEM_SIZE ||
		    existing_results.find(addr) != existing_results.end()) {
			continue;
		}

		RAMpageExperimentData *d = new RAMpageExperimentData;
		d->msg.set_mem_addr(addr);
		d->msg.set_mem_bit(4);
		d->msg.set_errortype(d->msg.ERROR_STUCK_AT_1);
		//d->msg.set_empty_passes(2);
		d->msg.set_empty_passes(4);
		//d->msg.set_local_timeout(1000000*60*10); // 10m
		//d->msg.set_global_timeout(1000000*60*50); // 50m
		//d->msg.set_local_timeout(1000000*60*20); // 20m
		//d->msg.set_global_timeout(1000000*60*90); // 90m
		//d->msg.set_local_timeout(1000000*60*20); // 20m
		//d->msg.set_global_timeout(1000000*60*120); // 120m
		d->msg.set_local_timeout(1000000ULL*60*30); // 30m
		d->msg.set_global_timeout(1000000ULL*60*120); // 120m
		campaignmanager.addParam(d);
	}
	campaignmanager.noMoreParameters();

	// collect results
	RAMpageExperimentData *res;
	while ((res = static_cast<RAMpageExperimentData *>(campaignmanager.getDone()))) {
		results
			<< res->msg.mem_addr() << "\t"
			<< res->msg.mem_bit() << "\t"
			<< res->msg.mem_coupled_bit() << "\t"
			<< res->msg.errortype() << "\t"
			<< res->msg.local_timeout() << "\t"
			<< res->msg.global_timeout() << "\t"
			<< res->msg.mem_written() << "\t"
			<< res->msg.resulttype() << "\t"
			<< res->msg.error_detected_pfn() << "\t"
			<< res->msg.experiment_time() << "\t"
			<< res->msg.details() << endl;
	}
	results.close();
	return true;
}

uint64_t RAMpageCampaign::reverse_bits(uint64_t v)
{
	// http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
	uint64_t r = v; // r will be reversed bits of v; first get LSB of v
	int s = sizeof(v) * CHAR_BIT - 1; // extra shift needed at end

	for (v >>= 1; v; v >>= 1)
	{
		r <<= 1;
		r |= v & 1;
		s--;
	}
	r <<= s; // shift when v's highest bits are zero
	return r;
}
