#ifndef SMART_ALGORITHM__HPP
#define SMART_ALGORITHM__HPP

#include <map>
#include <vector>

#include "TraceReader.hpp"
#include "comm/InjectionPointHopsMessage.pb.h"
#include "util/Logger.hpp"

namespace fail {

typedef std::pair<trace_event_tuple_t, trace_pos_t > result_tuple;

// 			       COSTS         NAVIGATIONAL RECORD
typedef std::pair<unsigned int, std::vector<result_tuple > > checkpoint_tuple_t;

/**
 * \class SmartHops
 *
 * Calculator for (optimal) hop chains on the basis of a simple cost model.
 * Currently hardcoded cost model for PandaBoard (horizontal hops cost
 * twice as much as diagonal hops).
 * Calculates stream based (only regarding the next trace event) and so
 * it can only calculate for ascending trace instruction offsets.
 */
class SmartHops {
public:

	// ToDo: If you want to use checkoints as additional hop-chain-element, you may switch
	// m_use_checkpoints to true, but there will be additional changes needed for this to fully work.
	// The sal must, for example, generate CPs at the given positions
	SmartHops() : m_trace_pos(0), m_costs(0), m_next_cp_id(0), m_log("SmartHops", false), m_use_watchpoints(true),
		m_use_weights(true), m_use_checkpoints(false), m_cp_thresh(0), m_cost_cp(0), m_rollback_thresh(0) {}

	/**
	 * Initializes the used TraceReader with given trace file path
	 * @param filename Path to the trace file
	 */
	void init(const char *filename);

	/**
	 * Initializes the used TraceReader with given trace file path
	 * @param filename Path to the trace file
	 * @returns \c true if calculation succeeded and \c false if it did not
	 */
	bool calculateFollowingHop(InjectionPointMessage &ip, unsigned instruction_offset);
private:

	/**
	 * Converts internal representation of a hop chain to a
	 * InjectionPointMessage. The delivered InjectionPointMessage is
	 * cleared before parsing.
	 * @param result Internal representation of a hop chain
	 * @param costs Costs of the hop chain (extracted from cost model)
	 * @param ipm InjectionPointMessage to which the hop chain is parsed
	 */
	void convertToIPM(std::vector<result_tuple > &result, unsigned costs, InjectionPointMessage &ipm);

	unsigned int m_trace_pos;
	unsigned int m_costs;

	TraceReader m_trace_reader;
	unsigned int m_next_cp_id;
	Logger m_log;

	bool m_use_watchpoints;
	bool m_use_weights;
	bool m_use_checkpoints;

	unsigned int m_cp_thresh;
	unsigned int m_cost_cp;

	// Must be smaller than ((COST_CP_THRESH - COST_CP) / 2)
	// If too high: Costs per dynamic instruction will approach
	// COST_CP_THRESH but number of CPs can be reduced drastically
	unsigned int m_rollback_thresh;


	std::map<trace_event_tuple_t, trace_pos_t> m_last_positions;
	std::vector<checkpoint_tuple_t > m_checkpoints;

	std::vector<trace_event_tuple_t > m_trace_events;
	std::vector<result_tuple > m_result;
};

} // end of namespace

#endif // SMART_ALGORITHM__HPP

