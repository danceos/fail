#ifndef __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
  #define __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
  
#include "efw/ExperimentFlow.hpp"
#include "cpn/InjectionPoint.hpp"

using namespace fail;

class LRASimplePandaExperiment : public ExperimentFlow {
public:
	LRASimplePandaExperiment() : ExperimentFlow() {}
	bool run();
	void navigateToInjectionPoint(ConcreteInjectionPoint &ip);
	// void navigateToInjectionPoint(ConcreteInjectionPoint &ip, std::ostream &log);
};

#endif // __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
