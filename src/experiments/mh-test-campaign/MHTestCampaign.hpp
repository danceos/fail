#ifndef __MH_TEST_CAMPAIGN_HPP__
  #define __MH_TEST_CAMPAIGN_HPP__

#include <cpn/Campaign.hpp>
#include "comm/ExperimentData.hpp"
#include "MHTest.pb.h"

class MHExperimentData : public fail::ExperimentData {
public:
	MHTestData msg;
	MHExperimentData() : fail::ExperimentData(&msg) { }
};

class MHTestCampaign : public fail::Campaign {
private:
	int m_parameter_count; 
public:
	MHTestCampaign(int parametercount) : m_parameter_count(parametercount) { }
	bool run();
};

#endif // __MH_TEST_CAMPAIGN_HPP__
