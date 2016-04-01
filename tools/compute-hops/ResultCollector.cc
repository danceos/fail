/*
 * StatisticsCollector.cc
 *
 *  Created on: 21.08.2013
 *      Author: lrade
 */

#include "ResultCollector.hpp"

#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

#include <vector>

#include "comm/InjectionPointHopsMessage.pb.h"

//#define MEASURE_MEM_USAGE

#ifdef MEASURE_MEM_USAGE
#include <proc/readproc.h>
#endif

using std::hex;
using std::dec;

char separator = ' ';

extern std::ofstream g_cp_ofstream;

extern fail::Logger LOG;

void
ResultCollector::setProtoOStream(ProtoOStream* protoOStream)
{
	ps = protoOStream;
}

unsigned int calculate_costs(std::vector<result_tuple >& res)
{
	std::vector<result_tuple>::iterator it = res.begin();
	std::vector<result_tuple>::iterator it_ahead = res.begin();
	it_ahead++;

	// Cost COST_CHANGE for first hop
	// Costs of all following hops are calculated only on basis of
	// difference between two consecutive hop_positions
	unsigned int costs = COST_CHANGE;

	while (it != res.end() && it_ahead != res.end()) {
		if (it->first == it_ahead->first) {
			costs += COST_NO_CHANGE;
		} else {
			costs += COST_CHANGE;
		}
		it_ahead++;
		it++;
	}

	return costs;
}

void
ResultCollector::addResult(std::vector<result_tuple >& res, unsigned int costs)
{
	m_result_size++;

	// Costs not calculated in hop calculator (e.g. dijkstra)? => Do it here
	if (costs == 0) {
		costs = calculate_costs(res);
	}

	if (m_output_mode == OUTPUT_COSTS) {
		m_ostream << m_res_count++ << separator << costs << '\n';
	} else if (m_output_mode == OUTPUT_RESULT) {
		if (ps) {
			InjectionPointMessage hc;
			hc.set_costs(costs);
			hc.set_target_trace_position(res.back().second);
			// If checkpoint at beginning of hop-chain, add its id to InjectionPointMessage
			std::vector<result_tuple >::iterator it_hop = res.begin();
			if (it_hop != res.end() && it_hop->first.second == ACCESS_CHECKPOINT) {
				hc.set_checkpoint_id(it_hop->first.first);
				it_hop++;
			}

			for (; it_hop != res.end();
				it_hop++) {
				InjectionPointMessage_Hops *hop = hc.add_hops();
				hop->set_address(it_hop->first.first);

				InjectionPointMessage::Hops::AccessType at;
				switch (it_hop->first.second) {
				case ACCESS_NONE:
					at = InjectionPointMessage::Hops::EXECUTE;
					break;
				case ACCESS_READ:
					at = InjectionPointMessage::Hops::READ;
					break;
				case ACCESS_WRITE:
					at = InjectionPointMessage::Hops::WRITE;
					break;
				case ACCESS_READORWRITE:
					LOG << "ReadOrWrite memory access event not yet"
					" covered" << std::endl;
					exit(-1);
					break;
				case ACCESS_CHECKPOINT:
					LOG << "Checkpoint not allowed after beginning of hop chain" << std::endl;
					exit(-1);
				default:
					LOG << "Unknown memory-access event" << std::endl;
					exit(-1);
				}
				hop->set_accesstype(at);
			}
			ps->writeMessage(&hc);
		} else {
			for (std::vector<result_tuple >::iterator it_hop = res.begin();
					it_hop != res.end();
					it_hop++) {
				address_t add = it_hop->first.first;
				mem_access_type_e mem_acc_type = it_hop->first.second;
				std::string prefix = (mem_acc_type == ACCESS_READ)?"R":((mem_acc_type == ACCESS_WRITE)?"W":((mem_acc_type == ACCESS_NONE)?"X":((mem_acc_type == ACCESS_CHECKPOINT)?"C":"")));
				m_ostream << prefix << hex << add << dec << separator;
			}

			if (res.size() > 0)
				m_ostream << '\n';
		}
	} else if (m_output_mode == OUTPUT_STATISTICS) {
		// Calculate mean
		// http://www.heikohoffmann.de/htmlthesis/node134.html
		// c(t+1) = c(t) + (1/(t+1))*(x - c(t))
		m_it_mean_costs = m_it_mean_costs + (1/(double)m_result_size)*((double)costs-m_it_mean_costs);

	}
}

void
ResultCollector::addCheckpoint(unsigned int pos)
{
	g_cp_ofstream << m_checkpoint_count++ << separator << pos << '\n';
}

void
ResultCollector::startTimer()
{
	m_timer.startTimer();
}

void
ResultCollector::stopTimer()
{
	m_timer.stopTimer();
}

void
ResultCollector::setMaxMemUsage()
{
#ifdef MEASURE_MEM_USAGE
	struct proc_t usage;
	look_up_our_self(&usage);
	m_mem_usage = usage.vsize;
#else
	m_mem_usage = 0;
#endif
}

void
ResultCollector::finish()
{
	// Print results if buffered

	// Print statistics
	if (m_output_mode == OUTPUT_STATISTICS) {

		m_ostream << m_result_size << separator << m_timer.getRuntimeAsDouble() << separator
				<< m_mem_usage << separator << m_it_mean_costs << separator << m_checkpoint_count << '\n';
	}
}
