#ifndef __HSC_SIMPLE_EXPERIMENT_HPP__
#define __HSC_SIMPLE_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"

class HSCSimpleExperiment : public fail::ExperimentFlow {
public:
	HSCSimpleExperiment() { }

	bool run();
};

#endif // __HSC_SIMPLE_EXPERIMENT_HPP__
