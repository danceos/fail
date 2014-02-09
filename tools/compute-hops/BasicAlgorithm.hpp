#ifndef __BASIC_ALGORITHM__HPP
#define __BASIC_ALGORITHM__HPP

#include "../../src/core/util/smarthops/TraceReader.hpp"
class ResultCollector;

class BasicAlgorithm {
public:
	/**
	 *
	 * @returns boolean value for calculation success
	 */
	virtual bool calculateAllHops(fail::TraceReader& trace) = 0;

	BasicAlgorithm(ResultCollector *rc) { m_resultCollector = rc; }
	virtual ~BasicAlgorithm() {}

protected:
	ResultCollector *m_resultCollector;
};

#endif // __BASIC_ALGORITHM__HPP
