#ifndef __COOLCAMPAIGN_HPP__
#define __COOLCAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "coolchecksum.pb.h"

class CoolChecksumExperimentData : public fi::ExperimentData {
public:
	CoolChecksumProtoMsg msg;
	CoolChecksumExperimentData() : fi::ExperimentData(&msg) {}
};


class CoolChecksumCampaign : public fi::Campaign {
public:
	virtual bool run();
};

#endif 
