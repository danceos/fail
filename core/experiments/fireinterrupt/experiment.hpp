#ifndef __FIREINTERRUPT_EXPERIMENT_HPP__
  #define __FIREINTERRUPT_EXPERIMENT_HPP__

#include "controller/ExperimentFlow.hpp"

class fireinterruptExperiment : public fi::ExperimentFlow
{
	public:
		fireinterruptExperiment() { }
	
		bool run();
};

#endif // __FIREINTERRUPT_EXPERIMENT_HPP__
