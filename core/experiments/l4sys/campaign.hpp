#ifndef __L4SYS_CAMPAIGN_HPP__
  #define __L4SYS_CAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "l4sys.pb.h"

class L4SysExperimentData : public fail::ExperimentData {
public:
	L4SysProtoMsg msg;
	L4SysExperimentData() : fail::ExperimentData(&msg) {}
};

class L4SysCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __L4SYS_CAMPAIGN_HPP__
