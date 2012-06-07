#ifndef __MH_TEST_EXPERIMENT_HPP__
  #define __MH_TEST_EXPERIMENT_HPP__
  
#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class MHTestExperiment : public fail::ExperimentFlow {
private:
	fail::JobClient m_jc;
public: 
	MHTestExperiment() { }
	~MHTestExperiment() { }

	bool run();
};

#endif // __MH_TEST_EXPERIMENT_HPP__
