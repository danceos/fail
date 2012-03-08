#ifndef __TESTCAMPAIGN_HPP__
#define __TESTCAMPAIGN_HPP__
  
 
#include <controller/Campaign.hpp>
#include "controller/ExperimentData.hpp"
#include <experiments/MHTestCampaign/MHTest.pb.h>

using namespace fi;

class MHExperimentData : public ExperimentData {
  public:
    MHTestData msg;
  public:
    MHExperimentData() : ExperimentData(&msg){  };
};


class MHTestCampaign : public Campaign {
    int m_parameter_count; 
  public:
    MHTestCampaign(int parametercount) : m_parameter_count(parametercount){};
    virtual bool run();
};

  
#endif 
