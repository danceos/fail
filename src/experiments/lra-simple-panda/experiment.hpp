#ifndef __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
#define __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "cpn/InjectionPoint.hpp"
#include "util/ElfReader.hpp"

using namespace fail;

class LRASimplePandaExperiment : public ExperimentFlow {
private:
	fail::ElfReader *elfReader;
public:
	LRASimplePandaExperiment() : ExperimentFlow() {}
	bool run();
	bool navigateToInjectionPoint(ConcreteInjectionPoint &ip);
	// void navigateToInjectionPoint(ConcreteInjectionPoint &ip, std::ostream &log);
};

#endif // __LRA_SIMPLE_PANDA_EXPERIMENT_HPP__
