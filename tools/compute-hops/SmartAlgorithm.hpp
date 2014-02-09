#ifndef SMART_ALGORITHM__HPP
#define SMART_ALGORITHM__HPP

#include <map>
#include <vector>

#include "BasicAlgorithm.hpp"
#include "../../src/core/util/smarthops/TraceReader.hpp"
#include "ResultCollector.hpp"

class ResultCollector;

using namespace fail;

// 			       COSTS         NAVIGATIONAL RECORD
typedef std::pair<unsigned int, std::vector<result_tuple > > checkpoint_tuple_t;

class SmartAlgorithm : public BasicAlgorithm {
public:

	SmartAlgorithm(ResultCollector *rc) : BasicAlgorithm(rc), m_next_cp_id(0) {}
	virtual ~SmartAlgorithm() {}

	bool calculateAllHops(TraceReader& trace);

private:

	bool
	occurenceInPosIntervall(unsigned int intervall_low,
							unsigned int intervall_high,
							address_t add,
							mem_access_type_e acc);

	std::map<trace_event_tuple_t, trace_pos_t> m_last_positions;
	std::vector<checkpoint_tuple_t > m_checkpoints;
	unsigned int m_next_cp_id;
};

#endif // SMART_ALGORITHM__HPP

