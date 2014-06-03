#include "InjectionPoint.hpp"
#include "util/smarthops/SmartHops.hpp"

#include <algorithm>
#include <limits>

namespace fail {

InjectionPointHops::~InjectionPointHops() {
	if (m_initialized) {
		delete m_sa;
	}
}

void InjectionPointHops::init()
{
	m_sa = new SmartHops();

	char * elfpath = getenv("FAIL_TRACE_PATH");
	if (elfpath == NULL) {
		m_log << "FAIL_TRACE_PATH not set :(" << std::endl;
		exit(-1);
	} else {
		m_sa->init((const char*) elfpath);
	}

	m_initialized = true;
}

/*
 * if (!it->has_costs()) {
		m_log << "FATAL ERROR: Costs must be delivered in order to calculate minimum costs" << endl;
	}
 */

void InjectionPointHops::parseFromInjectionInstr(unsigned instr1, unsigned instr2) {
	if (!m_initialized) {
		init();
	}

	// clear results older than instr1, as the input needs to be sorted by instr1, these
	// results won't be needed anymore
	std::vector<InjectionPointMessage>::iterator it = m_results.begin(), delete_iterator = m_results.end();
	for (; it != m_results.end(); it++) {
		if (!it->has_target_trace_position()) {
			m_log << "FATAL ERROR: Target trace offset must be delivered in order to calculate minimum costs" << std::endl;
			m_log << m_results.size() << std::endl;
			exit(0);
		}
		if (it->target_trace_position() < instr1) {
			delete_iterator = it;
		}
	}

	// Delete as chunk
	if (delete_iterator != m_results.end()) {
		m_results.erase(m_results.begin(), delete_iterator);
		m_curr_instr1 = instr1;
	}

	// Calculate next needed results
	while ((long)instr2 > m_curr_instr2) {
		// if instr1 is bigger than nex instr2, we can skip instructions
		// And current instr1 will be newly defined
		unsigned new_curr_instr2;
		if ((long)instr1 > m_curr_instr2) {
			m_curr_instr1 = instr1;
			new_curr_instr2 = instr1;
		} else {
			new_curr_instr2 = m_curr_instr2 + 1;
		}

		InjectionPointMessage m;
		if (!m_sa->calculateFollowingHop(m, new_curr_instr2)) {
			m_log << "FATAL ERROR: Trace does not contain enough instructions (no instruction with offset "
					<< new_curr_instr2 << ")" << std::endl;
			exit(-1);
		}

		m_results.push_back(m);
		m_curr_instr2 = new_curr_instr2;
	}

	// Choose minimum
	InjectionPointMessage *min_cost_msg;
	uint32_t min_costs = std::numeric_limits<uint32_t>::max();

	std::vector<InjectionPointMessage>::iterator search, search_end;
	search = m_results.begin() + (instr1 - m_curr_instr1);
	search_end = m_results.begin() + (instr2 - m_curr_instr1);

	// Single-instruction eqivalence class
	if (search == search_end) {
		m_ip = *search;
	} else {
		for (;search != search_end; search++) {
			if (!search->has_costs()) {
				m_log << "FATAL ERROR: Costs must be delivered in order to calculate minimum costs" << std::endl;
				exit(-1);
			}
			if (search->costs() < min_costs) {
				min_cost_msg = &(*search);
				min_costs = search->costs();
			}
		}
		m_ip = *min_cost_msg;
	}
}

} /* namespace fail */
