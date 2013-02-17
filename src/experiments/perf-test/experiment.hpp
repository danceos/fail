#ifndef __PERF_TEST_EXPERIMENT_HPP__
  #define __PERF_TEST_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"

class PerfTestExperiment : public fail::ExperimentFlow {
public:
	PerfTestExperiment() { }

	bool run();
};

#endif // __PERF_TEST_EXPERIMENT_HPP__
