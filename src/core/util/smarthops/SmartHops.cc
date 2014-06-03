#include "SmartHops.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace fail {

#define COST_CHANGE 	1
#define COST_NO_CHANGE 	2


void SmartHops::init(const char *filename) {
	m_trace_reader.openTraceFile(filename);
}

void SmartHops::convertToIPM(std::vector<result_tuple > &result, unsigned costs, InjectionPointMessage &ipm) {

	ipm.Clear();
	// If checkpoint at beginning of hop-chain, add its id to HopChain
	std::vector<result_tuple >::iterator it_hop = result.begin();
	if (it_hop != result.end() && it_hop->first.second == ACCESS_CHECKPOINT) {
		ipm.set_checkpoint_id(it_hop->first.first);
		it_hop++;
	}

	if (result.size() > 0 && result.back().first.second != ACCESS_CHECKPOINT) {
		ipm.set_target_trace_position(result.back().second);
	} else {
		ipm.set_target_trace_position(0);
	}
	ipm.set_costs(costs);

	for (; it_hop != result.end();
		it_hop++) {
		InjectionPointMessage_Hops *hop = ipm.add_hops();
		hop->set_address(it_hop->first.first);

		enum InjectionPointMessage_Hops_AccessType at;
		switch (it_hop->first.second) {
		case ACCESS_NONE:
			at = InjectionPointMessage_Hops_AccessType_EXECUTE;
			break;
		case ACCESS_READ:
			at = InjectionPointMessage_Hops_AccessType_READ;
			break;
		case ACCESS_WRITE:
			at = InjectionPointMessage_Hops_AccessType_WRITE;
			break;
		case ACCESS_READORWRITE:
			m_log << "ReadOrWrite memory access event not yet"
			" covered" << std::endl;
			exit(-1);
			break;
		case ACCESS_CHECKPOINT:
			m_log << "Checkpoint not allowed after beginning of hop chain" << std::endl;
			exit(-1);
		}
		hop->set_accesstype(at);
	}
}

bool SmartHops::calculateFollowingHop(InjectionPointMessage &ip, unsigned instruction_offset) {

	if (instruction_offset == 0) {
		m_result.clear();
		m_costs = 0;
		convertToIPM(m_result, m_costs, ip);
		return true;
	}

	while (m_trace_pos < instruction_offset) {
		//m_log << "Calculating " << instruction_offset << std::endl;
		if (!m_trace_reader.getNextTraceEvents(m_trace_pos, m_trace_events)) {
			return false;
		}
		// Policy when multiple trace events: Choose the one with smallest last_pos
		trace_event_tuple_t *best_te = NULL;
		trace_pos_t last_pos = std::numeric_limits<trace_pos_t>::max();

		bool hop_found = false;
		bool checkpoint_forbidden = false;

		for (std::vector<trace_event_tuple_t >::iterator it_te = m_trace_events.begin();
						it_te != m_trace_events.end();
						it_te++) {

			// Don't use watchpoints?
			if (!m_use_watchpoints && it_te->second != ACCESS_NONE) {
				continue;
			}

			// Find last pos of current trace event
			std::map<trace_event_tuple_t, trace_pos_t>::iterator it_lp = m_last_positions.find(*it_te);

			// Not known? => Single hop, result calculation complete
			if (it_lp == m_last_positions.end()) {
				m_last_positions.insert(std::pair<trace_event_tuple_t, trace_pos_t>(*it_te, m_trace_pos));
				// New instruction => single hop
				if (!hop_found) {
					m_result.clear();
					m_result.push_back(result_tuple(*it_te, m_trace_pos));
					m_costs = COST_CHANGE;
					hop_found = true;
				}
			} else {
				if (it_lp->second < last_pos) {
					last_pos = it_lp->second;
					best_te = (trace_event_tuple_t*)&(it_lp->first);
				}
				it_lp->second = m_trace_pos;
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


		rit_last = m_result.rbegin();
		rit_pre_last = m_result.rbegin();
		rit_pre_last++;

		while (rit_last != m_result.rend()) {

			trace_pos_t left_pos;

			// Policy switch:
			// No weights 	=> 	last pos <= pos of rit_pre_last
			// Weights 		=> 	last pos < pos of rit_pre_last

			if ((rit_pre_last == m_result.rend())) {
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
					if ((*(rit_pre_last + m_rollback_thresh)).second < last_pos) {
						checkpoint_forbidden = true;
						break;
					}
				} else {
					break;
				}
			}

			left_pos = rit_pre_last->second;

			if ((!m_use_weights && !(last_pos <= left_pos))
					|| (m_use_weights && !(last_pos < left_pos))) {
				break;
			}

			// Hop a->b is not allowed if last pos of b is at a
			// but a is not b
			// e.g. instruction x with mem access at y
			// a is WP y, and b is BP x
			// OR a is BP x, and b is WP y but instruction
			// at position a has also memory access at y
			if (rit_pre_last != m_result.rend()) {
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
				m_costs = m_checkpoints[rit_last->first.first].first;

				m_result.clear();
				std::vector<result_tuple > *tmp = &(m_checkpoints[rit_last->first.first].second);
				m_result.insert(m_result.end(), tmp->begin(), tmp->end());

				rit_last = m_result.rbegin();
				rit_pre_last = m_result.rbegin();
				rit_pre_last++;

				// CP could be removed from m_checkpoints, but to do this
				// m_checkpoints needs to be a map (CP-ID is implicit as vector index)

				continue;
			}

			if (rit_pre_last->first == rit_last->first) {
				m_costs -= COST_NO_CHANGE;
			} else {
				m_costs -= COST_CHANGE;
			}

			// Delete element described by rit_last
			// Deletion of elements described by reverse iterator works like this
			m_result.erase((rit_last+1).base());
			rit_last = rit_pre_last;
			rit_pre_last++;
		}


		// Add costs for new hop
		if (*best_te == (m_result.back()).first) {
			m_costs += COST_NO_CHANGE;
		} else {
			m_costs += COST_CHANGE;
		}

		// Check if Checkpoint needed
		if (m_use_checkpoints && (m_costs > m_cp_thresh) && !checkpoint_forbidden) {
			checkpoint_tuple_t new_cp(m_costs, std::vector<result_tuple >(m_result));
			m_checkpoints.push_back(new_cp);
			m_result.clear();
			m_result.push_back(result_tuple(trace_event_tuple_t(m_next_cp_id++,
																ACCESS_CHECKPOINT),
											m_trace_pos));
			m_costs = m_cost_cp;
		} else {
			// Add new hop
			m_result.push_back(result_tuple(*best_te, m_trace_pos));
		}
	}
	InjectionPointMessage ipm;
	convertToIPM(m_result, m_costs, ip);
	return true;
}

} // end of namespace
