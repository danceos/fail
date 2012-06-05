#include <iostream>
#include <vector>
#include <map>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "controller/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"
#include "util/MemoryMap.hpp"

#include "ecc_region.hpp"

#include "plugins/tracing/TracingPlugin.hpp"
char const * const trace_filename = "trace.pb";

using namespace fi;
using std::endl;

char const * const results_csv = "chksumoostubs.csv";

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
struct equivalence_class {
	sal::address_t data_address;
	int instr1, instr2;
	sal::address_t instr2_absolute; // FIXME we could record them all here
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

	// load trace
	ifstream tracef(trace_filename);
	if (tracef.fail()) {
		log << "couldn't open " << trace_filename << endl;
		return false;
	}
	ProtoIStream ps(&tracef);

	// a map of addresses of ECC protected objects
	MemoryMap mm;
	for (unsigned i = 0; i < sizeof(memoryMap)/sizeof(*memoryMap); ++i) {
		mm.add(memoryMap[i][0], memoryMap[i][1]);
	}

	// set of equivalence classes that need one (rather: eight, one for
	// each bit in that byte) experiment to determine them all
	std::vector<equivalence_class> ecs_need_experiment;
	// set of equivalence classes that need no experiment, because we know
	// they'd be identical to the golden run
	std::vector<equivalence_class> ecs_no_effect;

	equivalence_class current_ec;

	// map for efficient access when results come in
	std::map<ChecksumOOStuBSExperimentData *, unsigned> experiment_ecs;
	// experiment count
	int count = 0;

	// XXX do it the other way around: iterate over trace, search addresses
	// for every injection address ...
	for (MemoryMap::iterator it = mm.begin(); it != mm.end(); ++it) {
		std::cerr << ".";
		sal::address_t data_address = *it;
		current_ec.instr1 = 0;
		int instr = 0;
		sal::address_t instr_absolute = 0; // FIXME this one probably should also be recorded ...
		Trace_Event ev;
		ps.reset();

		// for every section in the trace between subsequent memory
		// accesses to that address ...
		// XXX reorganizing the trace for efficient seeks could speed this up
		while(ps.getNext(&ev)) {
			// instruction events just get counted
			if (!ev.has_memaddr()) {
				// new instruction
				instr++;
				instr_absolute = ev.ip();
				continue;

			// skip accesses to other data
			} else if (ev.memaddr() != data_address) {
				continue;

			// skip zero-sized intervals: these can
			// occur when an instruction accesses a
			// memory location more than once
			// (e.g., INC, CMPXCHG)
			} else if (current_ec.instr1 > instr) {
				continue;
			}

			// we now have an interval-terminating R/W
			// event to the memaddr we're currently looking
			// at:

			// complete the equivalence interval
			current_ec.instr2 = instr;
			current_ec.instr2_absolute = instr_absolute;
			current_ec.data_address = data_address;

			if (ev.accesstype() == ev.READ) {
				// a sequence ending with READ: we need
				// to do one experiment to cover it
				// completely
				ecs_need_experiment.push_back(current_ec);

				// instantly enqueue jobs: that way the job clients can already
				// start working in parallel
				for (int bitnr = 0; bitnr < 8; ++bitnr) {
					ChecksumOOStuBSExperimentData *d = new ChecksumOOStuBSExperimentData;
					// we pick the rightmost instruction in that interval
					d->msg.set_instr_offset(current_ec.instr2);
					d->msg.set_instr_address(current_ec.instr2_absolute);
					d->msg.set_mem_addr(current_ec.data_address);
					d->msg.set_bit_offset(bitnr);

					// store index into ecs_need_experiment
					experiment_ecs[d] = ecs_need_experiment.size() - 1;

					fi::campaignmanager.addParam(d);
					++count;
				}
			} else if (ev.accesstype() == ev.WRITE) {
				// a sequence ending with WRITE: an
				// injection anywhere here would have
				// no effect.
				ecs_no_effect.push_back(current_ec);
			} else {
				log << "WAT" << endl;
			}

			// next interval must start at next
			// instruction; the aforementioned
			// skipping mechanism wouldn't work
			// otherwise
			current_ec.instr1 = instr + 1;
		}

		// close the last interval:
		// Why -1?  In most cases it does not make sense to inject before the
		// very last instruction, as we won't execute it anymore.  This *only*
		// makes sense if we also inject into parts of the result vector.  This
		// is not the case in this experiment, and with -1 we'll get a
		// result comparable to the non-pruned campaign.
		// XXX still true for checksum-oostubs?
		current_ec.instr2 = instr - 1;
		current_ec.instr2_absolute = 0; // won't be used
		current_ec.data_address = data_address;
		// zero-sized?  skip.
		if (current_ec.instr1 > current_ec.instr2) {
			continue;
		}
		// as the experiment ends, this byte is a "don't care":
		ecs_no_effect.push_back(current_ec);
	}

	fi::campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	log << "equivalence classes generated:"
	    << " need_experiment = " << ecs_need_experiment.size()
	    << " no_effect = " << ecs_no_effect.size() << endl;

	// statistics
	unsigned long num_dumb_experiments = 0;
	for (std::vector<equivalence_class>::const_iterator it = ecs_need_experiment.begin();
	     it != ecs_need_experiment.end(); ++it) {
		num_dumb_experiments += (*it).instr2 - (*it).instr1 + 1;
	}
	for (std::vector<equivalence_class>::const_iterator it = ecs_no_effect.begin();
	     it != ecs_no_effect.end(); ++it) {
		num_dumb_experiments += (*it).instr2 - (*it).instr1 + 1;
	}
	log << "pruning: reduced " << num_dumb_experiments * 8 <<
	       " experiments to " << ecs_need_experiment.size() * 8 << endl;

	// CSV header
	results << "ec_instr1\tec_instr2\tec_instr2_absolute\tec_data_address\tbitnr\tresulttype\tresult0\tresult1\tresult2\tfinish_reached\tlatest_ip\terror_corrected\tdetails" << endl;

	// store no-effect "experiment" results
	for (std::vector<equivalence_class>::const_iterator it = ecs_no_effect.begin();
	     it != ecs_no_effect.end(); ++it) {
		results
		 << (*it).instr1 << "\t"
		 << (*it).instr2 << "\t"
		 << (*it).instr2_absolute << "\t" // incorrect in all but one case!
		 << (*it).data_address << "\t"
		 << "99\t" // dummy value: we didn't do any real experiments
		 << "1\t"
		 << "99\t99\t99\t"
		 << "1\t"
		 << "99\t"
		 << "0\t\n";
	}

	// collect results
	ChecksumOOStuBSExperimentData *res;
	int rescount = 0;
	while ((res = static_cast<ChecksumOOStuBSExperimentData *>(fi::campaignmanager.getDone()))) {
		rescount++;

		std::map<ChecksumOOStuBSExperimentData *, unsigned>::iterator it =
			experiment_ecs.find(res);
		if (it == experiment_ecs.end()) {
			results << "WTF, didn't find res!" << endl;
			log << "WTF, didn't find res!" << endl;
			continue;
		}
		equivalence_class &ec = ecs_need_experiment[it->second];

		// sanity check
		if (ec.instr2 != res->msg.instr_offset()) {
			results << "WTF" << endl;
			log << "WTF" << endl;
			//delete res;	// currently racy if jobs are reassigned
		}

		results
		 << ec.instr1 << "\t"
		 << ec.instr2 << "\t"
		 << ec.instr2_absolute << "\t" // incorrect in all but one case!
		 << ec.data_address << "\t"
		 << res->msg.bit_offset() << "\t"
		 << res->msg.resulttype() << "\t"
		 << res->msg.resultdata(0) << "\t"
		 << res->msg.resultdata(1) << "\t"
		 << res->msg.resultdata(2) << "\t"
		 << res->msg.finish_reached() << "\t"
		 << res->msg.latest_ip() << "\t"
		 << res->msg.error_corrected() << "\t"
		 << res->msg.details() << "\n";
		//delete res;	// currently racy if jobs are reassigned
	}
	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
