/*
 * ResultCollector.hpp
 *
 *  Created on: 23.08.2013
 *      Author: Lars Rademacher
 */

#ifndef RESULTCOLLECTOR_HPP_
#define RESULTCOLLECTOR_HPP_

#include <vector>
#include <fstream>
#include <iostream>

#include "../../src/core/util/smarthops/TraceReader.hpp"

#include "util/WallclockTimer.hpp"

using namespace fail;

typedef unsigned int trace_pos_t;

typedef std::pair<trace_event_tuple_t, trace_pos_t > result_tuple;

typedef enum {
	OUTPUT_RESULT		,
	OUTPUT_COSTS		,
	OUTPUT_STATISTICS	,
} output_mode_e;

using fail::ProtoOStream;

#define COST_CHANGE 	1
#define COST_NO_CHANGE 	2

class ResultCollector {
public:

	ResultCollector(std::ostream& out_stream, output_mode_e output_mode) :
		m_ostream(out_stream),
		m_res_count(1),
		m_output_mode(output_mode),
		m_mem_usage(0),
		m_result_size(0),
		m_checkpoint_count(1),
		m_it_mean_costs(0),
		m_max_costs(0),
		ps(0) {}

	void
	addResult(std::vector<result_tuple >& res, unsigned int costs);

	void
	addCheckpoint(unsigned int pos);

	void
	startTimer();

	void
	stopTimer();

	void
	setMaxMemUsage();

	void
	setProtoOStream(ProtoOStream* protoOStream);

	// Prints buffered results on output stream
	void
	finish();

private:

	std::ostream& m_ostream;
	unsigned int m_res_count;
	output_mode_e m_output_mode;
	unsigned long m_mem_usage;
	unsigned int m_result_size;
	unsigned int m_checkpoint_count;

	double m_it_mean_costs;
	unsigned int m_max_costs;

	fail::WallclockTimer m_timer;

	fail::ProtoOStream *ps;
};

#endif /* STATISTICSCOLLECTOR_HPP_ */
