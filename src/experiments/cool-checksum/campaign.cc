#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"

#if COOL_FAULTSPACE_PRUNING
#include "../plugins/tracing/TracingPlugin.hpp"
char const * const trace_filename = "trace.pb";
#endif

using namespace std;
using namespace fail;

char const * const results_csv = "coolcampaign.csv";

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
struct equivalence_class {
	unsigned byte_offset;
	int instr1, instr2;
	address_t instr2_absolute; // FIXME we could record them all here
};

bool CoolChecksumCampaign::run()
{
	Logger log("CoolChecksumCampaign");

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

#if !COOL_FAULTSPACE_PRUNING
	int count = 0;
	for (int bit_offset = 0; bit_offset < COOL_ECC_OBJUNDERTEST_SIZE*8; ++bit_offset) {
		for (int instr_offset = 0; instr_offset < COOL_ECC_NUMINSTR; ++instr_offset) {
			CoolChecksumExperimentData *d = new CoolChecksumExperimentData;
			d->msg.set_instr_offset(instr_offset);
			d->msg.set_bit_offset(bit_offset);

			campaignmanager.addParam(d);
			++count;
		}
	}
	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// collect results
	CoolChecksumExperimentData *res;
	int rescount = 0;
	results << "injection_ip\tinstr_offset\tinjection_bit\tresulttype\tresultdata\terror_corrected\tdetails" << endl;
	while ((res = static_cast<CoolChecksumExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		results
		 << res->msg.injection_ip() << "\t"
		 << res->msg.instr_offset() << "\t"
		 << res->msg.bit_offset() << "\t"
		 << res->msg.resulttype() << "\t"
		 << res->msg.resultdata() << "\t"
		 << res->msg.error_corrected() << "\t"
		 << res->msg.details() << "\n";
		delete res;
	}
#else
	// load trace
	ifstream tracef(trace_filename);
	if (tracef.fail()) {
		log << "couldn't open " << trace_filename << endl;
		return false;
	}
	ProtoIStream ps(&tracef);

	// set of equivalence classes that need one (rather: eight, one for
	// each bit in that byte) experiment to determine them all
	vector<equivalence_class> ecs_need_experiment;
	// set of equivalence classes that need no experiment, because we know
	// they'd be identical to the golden run
	vector<equivalence_class> ecs_no_effect;

	equivalence_class current_ec;

	// for every injection address ...
	// XXX in more complex cases we'll need to iterate over a MemoryMap here
	for (unsigned byte_offset = 0; byte_offset < COOL_ECC_OBJUNDERTEST_SIZE; ++byte_offset) {

		current_ec.instr1 = 0;
		// for every section in the trace between subsequent memory
		// accesses to that address ...
		// XXX reorganizing the trace for efficient seeks could speed this up
		int instr = 0;
		address_t instr_absolute = 0; // FIXME this one probably should also be recorded ...
		Trace_Event ev;
		ps.reset();

		while (ps.getNext(&ev)) {
			// only count instruction events
			if (!ev.has_memaddr()) {
				// new instruction
				instr++;
				instr_absolute = ev.ip();
				continue;

			// skip accesses to other data
			} else if (ev.memaddr() != byte_offset + COOL_ECC_OBJUNDERTEST) {
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
			current_ec.byte_offset = byte_offset;

			if (ev.accesstype() == ev.READ) {
				// a sequence ending with READ: we need
				// to do one experiment to cover it
				// completely
				ecs_need_experiment.push_back(current_ec);
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
		current_ec.instr2 = instr - 1;
		current_ec.instr2_absolute = 0; // won't be used
		current_ec.byte_offset = byte_offset;
		// zero-sized?  skip.
		if (current_ec.instr1 > current_ec.instr2) {
			continue;
		}
		// as the experiment ends, this byte is a "don't care":
		ecs_no_effect.push_back(current_ec);
	}

	log << "equivalence classes generated:"
	    << " need_experiment = " << ecs_need_experiment.size()
	    << " no_effect = " << ecs_no_effect.size() << endl;

	// statistics
	int num_dumb_experiments = 0;
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

	// map for efficient access when results come in
	map<CoolChecksumExperimentData *, equivalence_class *> experiment_ecs;
	int count = 0;
	for (vector<equivalence_class>::iterator it = ecs_need_experiment.begin();
	     it != ecs_need_experiment.end(); ++it) {
		for (int bitnr = 0; bitnr < 8; ++bitnr) {
			CoolChecksumExperimentData *d = new CoolChecksumExperimentData;
			// we pick the rightmost instruction in that interval
			d->msg.set_instr_offset((*it).instr2);
			d->msg.set_instr_address((*it).instr2_absolute);
			d->msg.set_bit_offset((*it).byte_offset * 8 + bitnr);

			experiment_ecs[d] = &(*it);

			campaignmanager.addParam(d);
			++count;
		}
	}
	campaignmanager.noMoreParameters();
	log << "done enqueueing parameter sets (" << count << ")." << endl;

	// CSV header
	results << "injection_ip\tinstr_offset\tinjection_bit\tresulttype\tresultdata\terror_corrected\tdetails" << endl;

	// store no-effect "experiment" results
	// (for comparison reasons; we'll store that more compactly later)
	for (vector<equivalence_class>::const_iterator it = ecs_no_effect.begin();
	     it != ecs_no_effect.end(); ++it) {
		for (int bitnr = 0; bitnr < 8; ++bitnr) {
			for (int instr = (*it).instr1; instr <= (*it).instr2; ++instr) {
				results
				 << (*it).instr2_absolute << "\t" // incorrect in all but one case!
				 << instr << "\t"
				 << ((*it).byte_offset * 8 + bitnr) << "\t"
				 << "1" << "\t"
				 << "45" << "\t"
				 << "0" << "\t"
				 << "" << "\n";
			}
		}
	}

	// collect results
	CoolChecksumExperimentData *res;
	int rescount = 0;
	while ((res = static_cast<CoolChecksumExperimentData *>(campaignmanager.getDone()))) {
		rescount++;

		equivalence_class *ec = experiment_ecs[res];

		// sanity check
		if (ec->instr2 != res->msg.instr_offset()) {
			results << "WTF" << endl;
			log << "WTF" << endl;
			delete res;
			continue;
		}

		// explode equivalence class to single "experiments"
		// (for comparison reasons; we'll store that more compactly later)
		for (int instr = ec->instr1; instr <= ec->instr2; ++instr) {
				results
				 << res->msg.injection_ip() << "\t" // incorrect in all but one case!
				 << instr << "\t"
				 << res->msg.bit_offset() << "\t"
				 << res->msg.resulttype() << "\t"
				 << res->msg.resultdata() << "\t"
				 << res->msg.error_corrected() << "\t"
				 << res->msg.details() << "\n";
		}
		delete res;
	}
#endif
	log << "done.  sent " << count << " received " << rescount << endl;
	results.close();

	return true;
}
