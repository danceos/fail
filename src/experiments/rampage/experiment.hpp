#ifndef __RAMPAGE_EXPERIMENT_HPP__
  #define __RAMPAGE_EXPERIMENT_HPP__
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class RAMpageExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	bool run();
};

#endif // __RAMPAGE_EXPERIMENT_HPP__
