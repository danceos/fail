#ifndef __COOLCAMPAIGN_HPP__
#define __COOLCAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "l4sys.pb.h"

class L4SysExperimentData : public fi::ExperimentData {
public:
	L4SysProtoMsg msg;
	L4SysExperimentData() : fi::ExperimentData(&msg) {}
};


class L4SysCampaign : public fi::Campaign {
public:
	virtual bool run();
};

#endif 
