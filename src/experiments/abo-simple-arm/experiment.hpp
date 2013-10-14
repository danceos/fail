#ifndef __ABO_SIMPLE_ARM_EXPERIMENT_HPP__
#define __ABO_SIMPLE_ARM_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class ABOSimpleARMExperiment : public fail::ExperimentFlow {
public:
	ABOSimpleARMExperiment() { }
	bool run();
};

#endif // __ABO_SIMPLE_ARM_EXPERIMENT_HPP__
