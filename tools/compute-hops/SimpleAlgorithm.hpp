#ifndef SIMPLE_ALGORITHM__HPP
#define SIMPLE_ALGORITHM__HPP

#include <map>

#include "BasicAlgorithm.hpp"
#include "../../src/core/util/smarthops/TraceReader.hpp"

using namespace fail;

class ResultCollector;

class SimpleAlgorithm : public BasicAlgorithm {
public:
	SimpleAlgorithm(ResultCollector *rc) : BasicAlgorithm(rc) {}
	virtual ~SimpleAlgorithm() {}

	bool calculateAllHops(TraceReader& trace);

private:
	// Count occurences
	//map<trace_event_tuple_t, vector<trace_pos_t> > m_occurences;
	std::map<trace_event_tuple_t, unsigned long> m_occurences;

	std::map<trace_event_tuple_t, unsigned long> m_occ_cp;
};

#endif // SIMPLE_ALGORITHM__HPP
