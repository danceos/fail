#ifndef __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
  #define __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class LRASimplePandaExperiment : public fail::ExperimentFlow {
public:
	LRASimplePandaExperiment() { }
	bool run();
};

#endif // __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
