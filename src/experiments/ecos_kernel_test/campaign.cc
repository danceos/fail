#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/ProtoStream.hpp"
#include "util/MemoryMap.hpp"

#include "../plugins/tracing/TracingPlugin.hpp"

//#define PRUNING_DEBUG_OUTPUT

using namespace std;
using namespace fail;

const std::string EcosKernelTestCampaign::dir_images("images");
const std::string EcosKernelTestCampaign::dir_prerequisites("prerequisites");
const std::string EcosKernelTestCampaign::dir_results("results");

bool EcosKernelTestCampaign::readMemoryMap(fail::MemoryMap &mm, char const * const filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cout << "failed to open " << filename << endl;
		return false;
	}

	string buf;
	unsigned guest_addr, guest_len;
	unsigned count = 0;

	while (getline(file, buf)) {
		stringstream ss(buf, ios::in);
		ss >> guest_addr >> guest_len;
		mm.add(guest_addr, guest_len);
		count++;
	}
	file.close();
	assert(count > 0);
	return (count > 0);
}

bool EcosKernelTestCampaign::writeTraceInfo(unsigned instr_counter, unsigned timeout, unsigned lowest_addr, unsigned highest_addr) {
	ofstream ti(filename_traceinfo().c_str(), ios::out);
	if (!ti.is_open()) {
		cout << "failed to open " << filename_traceinfo() << endl;
		return false;
	}
	ti << instr_counter << endl << timeout << endl << lowest_addr << endl << highest_addr << endl;
	ti.flush();
	ti.close();
	return true;
}

bool EcosKernelTestCampaign::readTraceInfo(unsigned &instr_counter, unsigned &timeout, unsigned &lowest_addr, unsigned &highest_addr,
	const std::string& variant, const std::string& benchmark) {
	ifstream file(filename_traceinfo(variant, benchmark).c_str());
	if (!file.is_open()) {
		cout << "failed to open " << filename_traceinfo(variant, benchmark) << endl;
		return false;
	}

	string buf;
	unsigned count = 0;

	while (getline(file, buf)) {
		stringstream ss(buf, ios::in);
		switch(count) {
			case 0:
				ss >> instr_counter;
				break;
			case 1:
				ss >> timeout;
				break;
			case 2:
				ss >> lowest_addr;
				break;
			case 3:
				ss >> highest_addr;
				break;
		}
		count++;
	}
	file.close();
	assert(count == 4);
	return (count == 4);
}

std::string EcosKernelTestCampaign::filename_memorymap(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "memorymap.txt";
	}
	return "memorymap.txt";
}

std::string EcosKernelTestCampaign::filename_state(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "state";
	}
	return "state";
}

std::string EcosKernelTestCampaign::filename_trace(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "trace.tc";
	}
	return "trace.tc";
}

std::string EcosKernelTestCampaign::filename_traceinfo(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "traceinfo.txt";
	}
	return "traceinfo.txt";
}

std::string EcosKernelTestCampaign::filename_results(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_results + "/" + variant + "-" + benchmark + "-" + "results.csv";
	}
	return "results.csv";
}

std::string EcosKernelTestCampaign::filename_elf(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_images + "/" + variant + "/" + benchmark + ".elf";
	}
	return "kernel.elf";
}

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
typedef std::map<address_t, int> AddrLastaccessMap;

char const *variants[] = {
	"bitmap_vanilla",
	"bitmap_SUM+DMR",
	"bitmap_CRC",
	"bitmap_CRC+DMR",
	"bitmap_TMR",
	// "bitmap_Hamming"
	0
};

char const *benchmarks[] = {
	"bin_sem0", "bin_sem1", "bin_sem2", "bin_sem3", "clock1", "clockcnv",
	"clocktruth", "cnt_sem1", "except1", "flag1", "kill", "mqueue1", "mutex1",
	"mutex2", "mutex3", "release", "sched1", "sync2", "sync3", "thread0",
	"thread1", "thread2",
	0
};

bool EcosKernelTestCampaign::run()
{
	m_log << "startup" << endl;

	if (!init_results()) {
		return false;
	}

	for (int variant_nr = 0; variants[variant_nr]; ++variant_nr) {
		char const *variant = variants[variant_nr];
		for (int benchmark_nr = 0; benchmarks[benchmark_nr]; ++benchmark_nr) {
			char const *benchmark = benchmarks[benchmark_nr];

			// local copies of experiment/job count (to calculate differences)
			int local_count_exp = count_exp, local_count_exp_jobs = count_exp_jobs,
				local_count_known = count_known, local_count_known_jobs = count_known_jobs;

			// load trace
			ifstream tracef(filename_trace(variant, benchmark).c_str());
			if (tracef.fail()) {
				m_log << "couldn't open " << filename_trace(variant, benchmark) << endl;
				return false;
			}
			ProtoIStream ps(&tracef);

			// read trace info
			unsigned instr_counter, estimated_timeout, lowest_addr, highest_addr;
			EcosKernelTestCampaign::readTraceInfo(instr_counter,
				estimated_timeout, lowest_addr, highest_addr, variant, benchmark);

			// a map of addresses of ECC protected objects
			MemoryMap mm;
			EcosKernelTestCampaign::readMemoryMap(mm,
				filename_memorymap(variant, benchmark).c_str());

			// map for keeping one "open" EC for every address
			// (maps injection data address => equivalence class)
			AddrLastaccessMap open_ecs;

			// instruction counter within trace
			unsigned instr = 0;
			// "rightmost" instr where we did a FI experiment
			unsigned instr_rightmost = 0;

			// fill open_ecs with one EC for every address
			for (MemoryMap::iterator it = mm.begin(); it != mm.end(); ++it) {
				open_ecs[*it] = instr;
			}

			// absolute address of current trace instruction
			address_t instr_absolute = 0;

			Trace_Event ev;
			// for every event in the trace ...
			while (ps.getNext(&ev) && instr < instr_counter) {
				// instruction events just get counted
				if (!ev.has_memaddr()) {
					// new instruction
					instr++;
					instr_absolute = ev.ip();
					continue;
				}

				// for each single byte in this memory access ...
				for (address_t data_address = ev.memaddr();
					data_address < ev.memaddr() + ev.width();
					++data_address) {
					// skip accesses to data outside our map of interesting addresses
					AddrLastaccessMap::iterator lastuse_it;
					if ((lastuse_it = open_ecs.find(data_address)) == open_ecs.end()) {
						continue;
					}
					int instr1 = lastuse_it->second;
					int instr2 = instr;

					// skip zero-sized intervals: these can occur when an instruction
					// accesses a memory location more than once (e.g., INC, CMPXCHG)
					if (instr1 > instr2) {
						continue;
					}

					// we now have an interval-terminating R/W event to the memaddr
					// we're currently looking at; the EC is defined by
					// data_address [instr1, instr2] (instr_absolute)

					if (ev.accesstype() == ev.READ) {
						// a sequence ending with READ: we need to do one experiment to
						// cover it completely
						add_experiment_ec(variant, benchmark, data_address, instr1, instr2, instr_absolute);
						instr_rightmost = instr2;
					} else if (ev.accesstype() == ev.WRITE) {
						// a sequence ending with WRITE: an injection anywhere here
						// would have no effect.
						add_known_ec(variant, benchmark, data_address, instr1, instr2, instr_absolute);
					} else {
						m_log << "WAT" << endl;
					}

					// next interval must start at next instruction; the aforementioned
					// skipping mechanism wouldn't work otherwise
					lastuse_it->second = instr2 + 1;
				}
			}

			// close all open intervals (right end of the fault-space)
			for (AddrLastaccessMap::iterator lastuse_it = open_ecs.begin();
				 lastuse_it != open_ecs.end(); ++lastuse_it) {
				address_t data_address = lastuse_it->first;
				int instr1 = lastuse_it->second;

#if 0
				// Why -1?  In most cases it does not make sense to inject before the
				// very last instruction, as we won't execute it anymore.  This *only*
				// makes sense if we also inject into parts of the result vector.  This
				// is not the case in this experiment, and with -1 we'll get a result
				// comparable to the non-pruned campaign.
				int instr2 = instr - 1;
#else
				// EcosKernelTestCampaign only variant: fault space ends with the last FI experiment
				int instr2 = instr_rightmost;
#endif
				int instr_absolute = 0; // unknown

				// zero-sized?  skip.
				if (instr1 > instr2) {
					continue;
				}

#if 0
				// the run continues after the FI window, so do this experiment
				// XXX this creates at least one experiment for *every* bit!
				//     fix: full trace, limited FI window

				ecs_need_experiment.push_back(current_ec);
				add_experiment_ec(variant, benchmark, data_address, instr1, instr2, instr_absolute);
#else
				// as the experiment ends, this byte is a "don't care":
				add_known_ec(variant, benchmark, data_address, instr1, instr2, instr_absolute);
#endif
			}
			// conserve some memory
			open_ecs.clear();

			// progress report
			m_log << variant << "/" << benchmark
			      << " exp " << (count_exp - local_count_exp) << " (" << (count_exp_jobs - local_count_exp_jobs) << " jobs)"
				  << " known " << (count_known - local_count_known) << " (" << (count_known_jobs - local_count_known_jobs) << " jobs)"
				  << " faultspace cutoff @ " << instr_rightmost << " out of " << instr
				  << endl;
		}
	}

	available_results.clear();

	campaignmanager.noMoreParameters();
	m_log << "total"
		  << " exp " << count_exp << " (" << count_exp_jobs << " jobs)"
		  << " known " << count_known << " (" << count_known_jobs << " jobs)"
		  << endl;

	// collect results
	EcosKernelTestExperimentData *res;
	while ((res = static_cast<EcosKernelTestExperimentData *>(campaignmanager.getDone()))) {
		// sanity check
		if (res->msg.result_size() != 8) {
			m_log << "wtf, result_size = " << res->msg.result_size() << endl;
			continue;
		}

		EcosKernelTestProtoMsg_Result const *prev_singleres = 0;
		int first_bit = 0, bit_width = 0;

		// one job contains 8 experiments
		for (int idx = 0; idx < res->msg.result_size(); ++idx) {
			EcosKernelTestProtoMsg_Result const *cur_singleres = &res->msg.result(idx);
			if (!prev_singleres) {
				prev_singleres = cur_singleres;
				first_bit = cur_singleres->bit_offset();
				bit_width = 1;
				continue;
			}
			// compatible?  merge.
			if (cur_singleres->bit_offset() == first_bit + bit_width // neighbor?
			 && prev_singleres->resulttype() == cur_singleres->resulttype()
			 && prev_singleres->latest_ip() == cur_singleres->latest_ip()
			 && prev_singleres->ecos_test_result() == cur_singleres->ecos_test_result()
			 && prev_singleres->error_corrected() == cur_singleres->error_corrected()
			 && prev_singleres->details() == cur_singleres->details()) {
				bit_width++;
				continue;
			}
			add_result(res->msg.variant(), res->msg.benchmark(), res->msg.instr1_offset(),
				res->msg.instr2_offset(), res->msg.instr2_address(), res->msg.mem_addr(),
				first_bit, bit_width, prev_singleres->resulttype(), prev_singleres->ecos_test_result(),
				prev_singleres->latest_ip(), prev_singleres->error_corrected(), prev_singleres->details(),
				res->msg.runtime() * bit_width / 8.0);
			prev_singleres = cur_singleres;
			first_bit = cur_singleres->bit_offset();
			bit_width = 1;
		}
		add_result(res->msg.variant(), res->msg.benchmark(), res->msg.instr1_offset(),
			res->msg.instr2_offset(), res->msg.instr2_address(), res->msg.mem_addr(),
			first_bit, bit_width, prev_singleres->resulttype(), prev_singleres->ecos_test_result(),
			prev_singleres->latest_ip(), prev_singleres->error_corrected(), prev_singleres->details(),
			res->msg.runtime() * bit_width / 8.0);
		delete res;
	}
	finalize_results();
	m_log << "done." << endl;

	return true;
}

bool EcosKernelTestCampaign::add_experiment_ec(const std::string& variant, const std::string& benchmark,
	address_t data_address, int instr1, int instr2, address_t instr_absolute)
{
	if (check_available(variant, benchmark, data_address, instr2)) {
		return false;
	}

	count_exp_jobs++;
	count_exp += 8;

	// enqueue job
#if 1
	EcosKernelTestExperimentData *d = new EcosKernelTestExperimentData;
	d->msg.set_variant(variant);
	d->msg.set_benchmark(benchmark);
	d->msg.set_instr1_offset(instr1);
	d->msg.set_instr2_offset(instr2);
	d->msg.set_instr2_address(instr_absolute);
	d->msg.set_mem_addr(data_address);
	campaignmanager.addParam(d);
#endif

	return true;
}

bool EcosKernelTestCampaign::add_known_ec(const std::string& variant, const std::string& benchmark,
	address_t data_address, int instr1, int instr2, address_t instr_absolute)
{
	if (check_available(variant, benchmark, data_address, instr2)) {
		return false;
	}

	count_known_jobs++;
	count_known += 8;

#if 1
	add_result(variant, benchmark, instr1, instr2, instr_absolute, data_address,
		0, 8, // bitnr, bit_width
		1, // resulttype
		1, // ecos_test_result
		99, // latest_ip
		0, // error_corrected
		"", // details
		0 // runtime
	);
#endif
	return true;
}

bool EcosKernelTestCampaign::init_results()
{
	// read already existing results
	bool file_exists = false;
	ifstream oldresults(filename_results().c_str(), ios::in);
	if (oldresults.is_open()) {
		file_exists = true;
		char buf[16*1024];
		std::string variant, benchmark;
		unsigned ignore;
		int instr2;
		address_t data_address;
		int bit_width;
		int rowcount = 0;
		int expcount = 0;
		m_log << "scanning existing results ..." << endl;
		while (oldresults.getline(buf, sizeof(buf)).good()) {
			stringstream ss;
			ss << buf;
			ss >> hex >> variant >> benchmark >> ignore >> instr2 >> ignore
			   >> data_address >> ignore >> bit_width;
			if (ss.fail()) {
				continue;
			}
			++rowcount;
			expcount += bit_width;
			// TODO: sanity check (duplicates?)
			available_results
				[AvailableResultMap::key_type(variant, benchmark)]
				[data_address].insert(instr2);
		}
		m_log << "found " << dec << expcount << " existing experiment results ("
		      << rowcount << " CSV rows)" << endl;
		oldresults.close();
	}

	// non-destructive: due to the CSV header we can always manually recover
	// from an accident (append mode)
	resultstream.open(filename_results().c_str(), ios::out | ios::app);
	if (!resultstream.is_open()) {
		m_log << "failed to open " << filename_results() << endl;
		return false;
	}
	// only write CSV header if file didn't exist before
	if (!file_exists) {
		resultstream << "variant\tbenchmark\tec_instr1\tec_instr2\t"
		                "ec_instr2_absolute\tec_data_address\tbitnr\tbit_width\t"
						"resulttype\tecos_test_result\tlatest_ip\t"
						"error_corrected\tdetails\truntime" << endl;
	}
	return true;
}

bool EcosKernelTestCampaign::check_available(const std::string& variant, const std::string& benchmark,
	address_t data_address, int instr2)
{
	AvailableResultMap::const_iterator it_variant =
		available_results.find(AvailableResultMap::key_type(variant, benchmark));
	if (it_variant == available_results.end()) {
		return false;
	}
	AvailableResultMap::mapped_type::const_iterator it_address =
		it_variant->second.find(data_address);
	if (it_address == it_variant->second.end()) {
		return false;
	}
	AvailableResultMap::mapped_type::mapped_type::const_iterator it_instr =
		it_address->second.find(instr2);
	if (it_instr == it_address->second.end()) {
		return false;
	}
	return true;
}

void EcosKernelTestCampaign::add_result(const std::string& variant, const std::string& benchmark,
	int instr1, int instr2, address_t instr2_absolute, address_t ec_data_address,
	int bitnr, int bit_width, int resulttype, int ecos_test_result, address_t latest_ip,
	int error_corrected, const std::string& details, float runtime)
{
	resultstream << hex
		<< variant << "\t"
		<< benchmark << "\t"
		<< instr1 << "\t"
		<< instr2 << "\t"
		<< instr2_absolute << "\t"
		<< ec_data_address << "\t"
		<< bitnr << "\t"
		<< bit_width << "\t"
		<< resulttype << "\t"
		<< ecos_test_result << "\t"
		<< latest_ip << "\t"
		<< error_corrected << "\t"
		<< details << "\t"
		<< runtime << "\n";
	//resultstream.flush(); // for debugging purposes
}

void EcosKernelTestCampaign::finalize_results()
{
	resultstream.close();
}
