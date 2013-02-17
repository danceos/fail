#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <boost/timer.hpp>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"
#include "util/MemoryMap.hpp"

#include "ecc_region.hpp"

#include "../plugins/tracing/TracingPlugin.hpp"

//#define PRUNING_DEBUG_OUTPUT

using namespace std;
using namespace fail;

char const * const trace_filename = "trace.tc";
char const * const results_filename = "chksumoostubs.csv";

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
struct equivalence_class {
	address_t data_address;
	int instr1, instr2;
	address_t instr2_absolute; // FIXME we could record them all here
};

bool ChecksumOOStuBSCampaign::run()
{
	Logger log("ChecksumOOStuBS Campaign");

	// non-destructive: due to the CSV header we can always manually recover
	// from an accident (append mode)
	ofstream results(results_filename, ios::out | ios::app);
	if (!results.is_open()) {
		log << "failed to open " << results_filename << endl;
		return false;
	}

	log << "startup" << endl;

	boost::timer t;

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
	vector<equivalence_class> ecs_need_experiment;
	// set of equivalence classes that need no experiment, because we know
	// they'd be identical to the golden run
	vector<equivalence_class> ecs_no_effect;

#if 0
	equivalence_class current_ec;

	// map for efficient access when results come in
	map<ChecksumOOStuBSExperimentData *, unsigned> experiment_ecs;
	// experiment count
	int count = 0;

	// XXX do it the other way around: iterate over trace, search addresses
	//   -> one "open" EC for every address
	// for every injection address ...
	for (MemoryMap::iterator it = mm.begin(); it != mm.end(); ++it) {
		//cerr << ".";
		address_t data_address = *it;
		current_ec.instr1 = 0;
		int instr = 0;
		address_t instr_absolute = 0; // FIXME this one probably should also be recorded ...
		Trace_Event ev;
		ps.reset();

		// for every section in the trace between subsequent memory
		// accesses to that address ...
		while (ps.getNext(&ev) && instr < OOSTUBS_NUMINSTR) {
			// instruction events just get counted
			if (!ev.has_memaddr()) {
				// new instruction
				instr++;
				instr_absolute = ev.ip();
				continue;

			// skip accesses to other data
			// FIXME again, do it the other way around, and use mm.isMatching()!
			} else if (ev.memaddr() + ev.width() <= data_address
			        || ev.memaddr() > data_address) {
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
#ifdef PRUNING_DEBUG_OUTPUT
				cerr << dec << "EX " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif

				// instantly enqueue job: that way the job clients can already
				// start working in parallel
				ChecksumOOStuBSExperimentData *d = new ChecksumOOStuBSExperimentData;
				// we pick the rightmost instruction in that interval
				d->msg.set_instr_offset(current_ec.instr2);
				d->msg.set_instr_address(current_ec.instr2_absolute);
				d->msg.set_mem_addr(current_ec.data_address);

				// store index into ecs_need_experiment
				experiment_ecs[d] = ecs_need_experiment.size() - 1;

				campaignmanager.addParam(d);
				++count;
			} else if (ev.accesstype() == ev.WRITE) {
				// a sequence ending with WRITE: an
				// injection anywhere here would have
				// no effect.
				ecs_no_effect.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
				cerr << dec << "NE " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif
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
		current_ec.instr2_absolute = 0; // unknown
		current_ec.data_address = data_address;
		// zero-sized?  skip.
		if (current_ec.instr1 > current_ec.instr2) {
			continue;
		}
		// as the experiment ends, this byte is a "don't care":
		ecs_no_effect.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
		cerr << dec << "NE " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif
	}
#else
	// map for efficient access when results come in
	map<ChecksumOOStuBSExperimentData *, unsigned> experiment_ecs;
	// map for keeping one "open" EC for every address
	map<address_t, equivalence_class> open_ecs;
	// experiment count
	int count = 0;

	// instruction counter within trace
	int instr = 0;

	// fill open_ecs with one EC for every address
	for (MemoryMap::iterator it = mm.begin(); it != mm.end(); ++it) {
		open_ecs[*it].instr1 = instr;
	}

	// absolute address of current trace instruction
	address_t instr_absolute = 0; // FIXME this one probably should also be recorded ...

	Trace_Event ev;
	// for every event in the trace ...
	while (ps.getNext(&ev) && instr < OOSTUBS_NUMINSTR) {
		// instruction events just get counted
		if (!ev.has_memaddr()) {
			// new instruction
			instr++;
			instr_absolute = ev.ip();
			continue;
		}

		// for each single byte in this memory access ...
		for (address_t data_address = ev.memaddr(); data_address < ev.memaddr() + ev.width();
			++data_address) {
			// skip accesses to data outside our map of interesting addresses
			map<address_t, equivalence_class>::iterator current_ec_it;
			if ((current_ec_it = open_ecs.find(data_address)) == open_ecs.end()) {
				continue;
			}
			equivalence_class& current_ec = current_ec_it->second;

			// skip zero-sized intervals: these can occur when an instruction
			// accesses a memory location more than once (e.g., INC, CMPXCHG)
			if (current_ec.instr1 > instr) {
				continue;
			}

			// we now have an interval-terminating R/W event to the memaddr
			// we're currently looking at:

			// complete the equivalence interval
			current_ec.instr2 = instr;
			current_ec.instr2_absolute = instr_absolute;
			current_ec.data_address = data_address;

			if (ev.accesstype() == ev.READ) {
				// a sequence ending with READ: we need to do one experiment to
				// cover it completely
				ecs_need_experiment.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
				cerr << dec << "EX " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif

				// instantly enqueue job: that way the job clients can already
				// start working in parallel
				ChecksumOOStuBSExperimentData *d = new ChecksumOOStuBSExperimentData;
				// we pick the rightmost instruction in that interval
				d->msg.set_instr_offset(current_ec.instr2);
				d->msg.set_instr_address(current_ec.instr2_absolute);
				d->msg.set_mem_addr(current_ec.data_address);

				// store index into ecs_need_experiment
				experiment_ecs[d] = ecs_need_experiment.size() - 1;

				campaignmanager.addParam(d);
				++count;
			} else if (ev.accesstype() == ev.WRITE) {
				// a sequence ending with WRITE: an injection anywhere here
				// would have no effect.
				ecs_no_effect.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
				cerr << dec << "NE " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif
			} else {
				log << "WAT" << endl;
			}

			// next interval must start at next instruction; the aforementioned
			// skipping mechanism wouldn't work otherwise
			current_ec.instr1 = instr + 1;
		}
	}

	// close all open intervals (right end of the fault-space)
	for (map<address_t, equivalence_class>::iterator current_ec_it = open_ecs.begin();
	     current_ec_it != open_ecs.end(); ++current_ec_it) {
		address_t data_address = current_ec_it->first;
		equivalence_class& current_ec = current_ec_it->second;

		// Why -1?  In most cases it does not make sense to inject before the
		// very last instruction, as we won't execute it anymore.  This *only*
		// makes sense if we also inject into parts of the result vector.  This
		// is not the case in this experiment, and with -1 we'll get a result
		// comparable to the non-pruned campaign.
		// XXX still true for checksum-oostubs?

		current_ec.instr2 = instr - 1;
		current_ec.instr2_absolute = 0; // unknown
		current_ec.data_address = data_address;

		// zero-sized?  skip.
		if (current_ec.instr1 > current_ec.instr2) {
			continue;
		}

#if 0
		// the run continues after the FI window, so do this experiment
		// XXX this creates at least one experiment for *every* bit!
		//     fix: full trace, limited FI window
		ecs_need_experiment.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
		cerr << dec << "EX " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif

		// FIXME copy/paste, encapsulate this:
		// instantly enqueue job: that way the job clients can already start
		// working in parallel
		ChecksumOOStuBSExperimentData *d = new ChecksumOOStuBSExperimentData;
		// we pick the rightmost instruction in that interval
		d->msg.set_instr_offset(current_ec.instr2);
		//d->msg.set_instr_address(current_ec.instr2_absolute); // unknown!
		d->msg.set_mem_addr(current_ec.data_address);

		// store index into ecs_need_experiment
		experiment_ecs[d] = ecs_need_experiment.size() - 1;

		campaignmanager.addParam(d);
		++count;
#else
		// as the experiment ends, this byte is a "don't care":
		ecs_no_effect.push_back(current_ec);
#ifdef PRUNING_DEBUG_OUTPUT
		cerr << dec << "NE " << current_ec.instr1 << " " << current_ec.instr2 << " " << current_ec.data_address << "\n";
#endif
#endif
	}
	// conserve some memory
	open_ecs.clear();
#endif

	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	log << "equivalence classes generated:"
	    << " need_experiment = " << ecs_need_experiment.size()
	    << " no_effect = " << ecs_no_effect.size() << endl;

	// statistics
	unsigned long num_dumb_experiments = 0;
	for (vector<equivalence_class>::const_iterator it = ecs_need_experiment.begin();
	     it != ecs_need_experiment.end(); ++it) {
		num_dumb_experiments += (*it).instr2 - (*it).instr1 + 1;
	}
	for (vector<equivalence_class>::const_iterator it = ecs_no_effect.begin();
	     it != ecs_no_effect.end(); ++it) {
		num_dumb_experiments += (*it).instr2 - (*it).instr1 + 1;
	}
	log << "pruning: reduced " << num_dumb_experiments * 8 <<
	       " experiments to " << ecs_need_experiment.size() * 8 << endl;

	// CSV header
	results << "ec_instr1\tec_instr2\tec_instr2_absolute\tec_data_address\tbitnr\tbit_width\tresulttype\tresult0\tresult1\tresult2\tfinish_reached\tlatest_ip\terror_corrected\tdetails" << endl;

	// store no-effect "experiment" results
	for (vector<equivalence_class>::const_iterator it = ecs_no_effect.begin();
	     it != ecs_no_effect.end(); ++it) {
		results
		 << (*it).instr1 << "\t"
		 << (*it).instr2 << "\t"
		 << (*it).instr2_absolute << "\t" // incorrect in all but one case!
		 << (*it).data_address << "\t"
		 << "0\t" // this entry starts at bit 0 ...
		 << "8\t" // ... and is 8 bits wide
		 << "1\t"
		 << "99\t99\t99\t" // dummy value: we didn't do any real experiments
		 << "1\t"
		 << "99\t" // dummy value: we didn't do any real experiments
		 << "0\t\n";
	}

	// collect results
	ChecksumOOStuBSExperimentData *res;
	int rescount = 0;
	while ((res = static_cast<ChecksumOOStuBSExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		map<ChecksumOOStuBSExperimentData *, unsigned>::iterator it =
			experiment_ecs.find(res);
		if (it == experiment_ecs.end()) {
			results << "WTF, didn't find res!" << endl;
			log << "WTF, didn't find res!" << endl;
			continue;
		}
		equivalence_class &ec = ecs_need_experiment[it->second];

		// sanity check
		if (ec.instr2 != res->msg.instr_offset()) {
			results << "ec.instr2 != instr_offset" << endl;
			log << "ec.instr2 != instr_offset" << endl;
		}
		if (res->msg.result_size() != 8) {
			results << "result_size " << res->msg.result_size()
			        << " instr2 " << ec.instr2
			        << " data_address " << ec.data_address << endl;
			log << "result_size " << res->msg.result_size() << endl;
		}

		// one job contains 8 experiments
		for (int idx = 0; idx < res->msg.result_size(); ++idx) {
			//results << "ec_instr1\tec_instr2\tec_instr2_absolute\tec_data_address\tbitnr\tbit_width\tresulttype\tresult0\tresult1\tresult2\tfinish_reached\tlatest_ip\terror_corrected\tdetails" << endl;
			results
			// repeated for all single experiments:
			 << ec.instr1 << "\t"
			 << ec.instr2 << "\t"
			 << ec.instr2_absolute << "\t"
			 << ec.data_address << "\t"
			// individual results:
			 << res->msg.result(idx).bit_offset() << "\t"
			 << "1\t" // 1 bit wide
			 << res->msg.result(idx).resulttype() << "\t"
			 << res->msg.result(idx).resultdata(0) << "\t"
			 << res->msg.result(idx).resultdata(1) << "\t"
			 << res->msg.result(idx).resultdata(2) << "\t"
			 << res->msg.result(idx).finish_reached() << "\t"
			 << res->msg.result(idx).latest_ip() << "\t"
			 << res->msg.result(idx).error_corrected() << "\t"
			 << res->msg.result(idx).details() << "\n";
		}
		//delete res;	// currently racy if jobs are reassigned

	}
	results.close();
	log << "done.  sent " << count << " received " << rescount << endl;
	log << "elapsed: " << t.elapsed() << "s" << endl;

	return true;
}
