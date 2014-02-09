#include "SmartAlgorithm.hpp"

#include <algorithm>
#include <limits>

#include "ResultCollector.hpp"

extern bool g_use_weights;
extern bool g_use_watchpoints;
extern bool g_use_checkpoints;

extern unsigned int g_cp_thresh;
extern unsigned int g_cost_cp;

// Must be smaller than ((COST_CP_THRESH - COST_CP) / 2)
// If too high: Costs per dynamic instruction will approach
// COST_CP_THRESH but number of CPs can be reduced drastically
extern unsigned int g_rollback_thresh;

extern fail::Logger LOG;

bool SmartAlgorithm::calculateAllHops(TraceReader& trace)
{
	unsigned int trace_pos = 0;
	unsigned int costs = 0;

	std::vector<trace_event_tuple_t > trace_events;
	std::vector<result_tuple > result;

	while (trace.getNextTraceEvents(trace_pos, trace_events)) {
		// Policy when multiple trace events: Choose the one with smallest last_pos
		trace_event_tuple_t *best_te = NULL;
		trace_pos_t last_pos = std::numeric_limits<trace_pos_t>::max();

		bool hop_found = false;
		bool checkpoint_forbidden = false;

		for (std::vector<trace_event_tuple_t >::iterator it_te = trace_events.begin();
						it_te != trace_events.end();
						it_te++) {

			// Don't use watchpoints?
			if (!g_use_watchpoints && it_te->second != ACCESS_NONE) {
				continue;
			}

			// Find last pos of current trace event
			std::map<trace_event_tuple_t, trace_pos_t>::iterator it_lp = m_last_positions.find(*it_te);

			// Not known? => Single hop, result calculation complete
			if (it_lp == m_last_positions.end()) {
				m_last_positions.insert(std::pair<trace_event_tuple_t, trace_pos_t>(*it_te, trace_pos));
				// New instruction => single hop
				if (!hop_found) {
					result.clear();
					result.push_back(result_tuple(*it_te, trace_pos));
					costs = COST_CHANGE;
					m_resultCollector->addResult(result, costs);
					hop_found = true;
				}
			} else {
				if (it_lp->second < last_pos) {
					last_pos = it_lp->second;
					best_te = (trace_event_tuple_t*)&(it_lp->first);
				}
				it_lp->second = trace_pos;
			}
		}

		if (hop_found) {
			continue;
		}

		// Deletion of unnecessary hops

		/*
		 *   |----------------|
		 *   |pos bigger than |
		 *   |last pos of new | --> delete -->-|
		 *   |trace_event?    |                |
		 *   |----------------|                |
		 *         ^                           |
		 *         |                           v
		 * A   B   A                           C       <<< Last result
		 *         ^                           ^
		 *      Reverse                     Reverse
		 *      iterator                    iterator
		 *    rit_pre_last                  rit_last
		 *
		 */

		std::vector<result_tuple >::reverse_iterator rit_pre_last, rit_last, end;


		rit_last = result.rbegin();
		rit_pre_last = result.rbegin();
		rit_pre_last++;

		while (rit_last != result.rend()) {

			trace_pos_t left_pos;

			// Policy switch:
			// No weights 	=> 	last pos <= pos of rit_pre_last
			// Weights 		=> 	last pos < pos of rit_pre_last

			if ((rit_pre_last == result.rend())) {
				// First node in hop list is Checkpoint? => Look at result before CP-Creation
				if (rit_last->first.second == ACCESS_CHECKPOINT) {
					rit_pre_last = (m_checkpoints[rit_last->first.first]).second.rbegin();

					// This should never happen:
					if (rit_pre_last == (m_checkpoints[rit_last->first.first]).second.rend()) {
						break;
					}

					// Don't allow deletion of CP, if afterwards
					// too few of the olf hops would be deleted.
					// (past before to be deleted CP)

					// Should not exceed vector boundaries, if
					// g_rollback_thresh*2 < (g_cp_thresh - g_cost_cp)
					if ((*(rit_pre_last + g_rollback_thresh)).second < last_pos) {
						checkpoint_forbidden = true;
						break;
					}
				} else {
					break;
				}
			}

			left_pos = rit_pre_last->second;

			if ((!g_use_weights && !(last_pos <= left_pos))
					|| (g_use_weights && !(last_pos < left_pos))) {
				break;
			}

			// Hop a->b is not allowed if last pos of b is at a
			// but a is not b
			// e.g. instruction x with mem access at y
			// a is WP y, and b is BP x
			// OR a is BP x, and b is WP y but instruction
			// at position a has also memory access at y
			if (rit_pre_last != result.rend()) {
				if ((last_pos == rit_pre_last->second) &&
						rit_pre_last->first != *best_te) {
					break;
				}
			}

			// Right hop will be deleted
			// => Recalculate costs

			// If to be deleted hop is a checkpoint:
			// There will be no past in the trace
			// Reconfigure result and iterators
			if (rit_last->first.second == ACCESS_CHECKPOINT) {
				costs = m_checkpoints[rit_last->first.first].first;

				result.clear();
				std::vector<result_tuple > *tmp = &(m_checkpoints[rit_last->first.first].second);
				result.insert(result.end(), tmp->begin(), tmp->end());

				rit_last = result.rbegin();
				rit_pre_last = result.rbegin();
				rit_pre_last++;

				// CP could be removed from m_checkpoints, but to do this
				// m_checkpoints needs to be a map (CP-ID is implicit as vector index)

				continue;
			}

			if (rit_pre_last->first == rit_last->first) {
				costs -= COST_NO_CHANGE;
			} else {
				costs -= COST_CHANGE;
			}

			// Delete element described by rit_last
			// Deletion of elements described by reverse iterator works like this
			result.erase((rit_last+1).base());
			rit_last = rit_pre_last;
			rit_pre_last++;
		}


		// Add costs for new hop
		if (*best_te == (result.back()).first) {
			costs += COST_NO_CHANGE;
		} else {
			costs += COST_CHANGE;
		}

		// Check if Checkpoint needed
		if (g_use_checkpoints && (costs > g_cp_thresh) && !checkpoint_forbidden) {
			checkpoint_tuple_t new_cp(costs, std::vector<result_tuple >(result));
			m_checkpoints.push_back(new_cp);
			result.clear();
			result.push_back(result_tuple(trace_event_tuple_t(m_next_cp_id++,
																ACCESS_CHECKPOINT),
											trace_pos));
			costs = g_cost_cp;

			m_resultCollector->addCheckpoint(trace_pos);
		} else {
			// Add new hop
			result.push_back(result_tuple(*best_te, trace_pos));
		}

		m_resultCollector->addResult(result, costs);
	}
	// MAX MEM USAGE
	m_resultCollector->setMaxMemUsage();

	return true;
}
