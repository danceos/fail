#ifndef __TESTEXPERIMENT_HPP__
#define __TESTEXPERIMENT_HPP__
  
#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class MHTestExperiment : public fi::ExperimentFlow {
  fi::JobClient m_jc;
  public: 
    MHTestExperiment(){};
    ~MHTestExperiment(){};
    bool run();
};

  
  
#endif 
