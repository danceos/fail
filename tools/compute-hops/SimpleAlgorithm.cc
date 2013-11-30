#include "SimpleAlgorithm.hpp"

#include <limits>
//#include <utility>
#include <cmath>
#include <algorithm>

#include "../../src/core/util/smarthops/TraceReader.hpp"
#include "ResultCollector.hpp"

using namespace std;
using namespace fail;

// Idea for CPs
// for each new trace_event: costs > THRESH?
//   => Use latest CP
//      Costs still > THRESH?
//		=> New CP

// Additional Map, which tracks down, how often a instruction occured
// since creation of the latest CP
// => MEM USAGE x 2

extern unsigned int g_cp_thresh;
extern unsigned int g_cost_cp;

extern bool g_use_watchpoints;
extern bool g_use_checkpoints;
extern fail::Logger LOG;

bool SimpleAlgorithm::calculateAllHops(TraceReader& trace)
{
	trace_pos_t trace_pos;
	vector<trace_event_tuple_t > trace_events;

	vector<result_tuple > empty_result;

	// Parse all executed instructions
	while (trace.getNextTraceEvents(trace_pos, trace_events)) {

		// Policy when multiple trace events: Choose the most rare one
		trace_event_tuple_t *candidate = NULL;
		//vector<trace_pos_t> *cand_pos = NULL;
		unsigned long num_hops = numeric_limits<unsigned long>::max();

		for (vector<trace_event_tuple_t >::iterator it_te = trace_events.begin();
								it_te != trace_events.end();
								it_te++) {
			// Don't use watchpoints?
			if (!g_use_watchpoints && (it_te->second != ACCESS_NONE)) {
				continue;
			}

			// Find last pos of current trace event
			//map<trace_event_tuple_t, vector<trace_pos_t> >::iterator it_oc = m_occurences.find(*it_te);
			map<trace_event_tuple_t, unsigned long >::iterator it_oc = m_occurences.find(*it_te);

			// Not known? => Add
			if (it_oc == m_occurences.end()) {
				//m_occurences.insert(pair<trace_event_tuple_t, vector<trace_pos_t > >(*it_te, vector<trace_pos_t>(1, trace_pos)));
				m_occurences.insert(pair<trace_event_tuple_t, unsigned long >(*it_te, 1));
				num_hops = 1;
				it_oc = m_occurences.find(*it_te);
				candidate = (trace_event_tuple_t*)&(it_oc->first);
				//cand_pos = (vector<trace_pos_t>*)&(it_oc->second);
			} else {
				it_oc->second++;
				if (it_oc->second < num_hops) {
					candidate = (trace_event_tuple_t*)&(it_oc->first);
					//cand_pos = (vector<trace_pos_t>*)&(it_oc->second);
					num_hops = it_oc->second;
				}
			}

			// Do the same thing for the occ_cp map
			map<trace_event_tuple_t, unsigned long >::iterator it_oc_cp = m_occ_cp.find(*it_te);

			// Not known? => Add
			if (it_oc_cp == m_occ_cp.end()) {
				m_occ_cp.insert(pair<trace_event_tuple_t, unsigned long >(*it_te, 1));
			} else {
				it_oc_cp->second++;
			}

		}

		if (candidate == NULL ) { //|| cand_pos == NULL
			cout << "Error while calculating" << endl;
			exit(1);
		}

		/*vector<result_tuple > res;
		for (vector<trace_pos_t>::iterator it_res = cand_pos->begin();
				it_res != cand_pos->end();
				it_res++) {
			res.push_back(result_tuple(*candidate, *it_res));
		}*/

		unsigned int costs = 1 + 2*(num_hops - 1);

		// Costs too high?
		if (g_use_checkpoints && costs > g_cp_thresh) {
			// costs acceptable with usage of latest CP?
			map<trace_event_tuple_t, unsigned long >::iterator occ_cp_find = m_occ_cp.find(*candidate);
			if (occ_cp_find == m_occ_cp.end()) {
				cout << "Error in data structure!"<< endl;
				exit(1);
			}
			costs = 1 + 2*(occ_cp_find->second - 1) + g_cost_cp;
			// Costs still too hight? Add new CP
			if (costs > g_cp_thresh) {
				m_resultCollector->addCheckpoint(trace_pos);
				m_occ_cp.clear();
				costs = g_cost_cp;
			}
		}

		m_resultCollector->addResult(empty_result, costs);
	}

	// MAX MEM USAGE
	m_resultCollector->setMaxMemUsage();

	return true;
}
