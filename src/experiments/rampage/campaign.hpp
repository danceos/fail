#ifndef __RAMPAGE_CAMPAIGN_HPP__
  #define __RAMPAGE_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "rampage.pb.h"

class RAMpageExperimentData : public fail::ExperimentData {
public:
	RAMpageProtoMsg msg;
	RAMpageExperimentData() : fail::ExperimentData(&msg) {}
};

class RAMpageCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __RAMPAGE_CAMPAIGN_HPP__
