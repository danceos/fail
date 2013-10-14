#ifndef __FIREINTERRUPT_EXPERIMENT_HPP__
#define __FIREINTERRUPT_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"

class FireInterruptExperiment : public fail::ExperimentFlow {
public:
	FireInterruptExperiment() { }
	bool run();
};

#endif // __FIREINTERRUPT_EXPERIMENT_HPP__
