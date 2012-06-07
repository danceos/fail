#ifndef __COOLCAMPAIGN_HPP__
  #define __COOLCAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "coolchecksum.pb.h"

class CoolChecksumExperimentData : public fail::ExperimentData {
public:
	CoolChecksumProtoMsg msg;
	CoolChecksumExperimentData() : fail::ExperimentData(&msg) {}
};

class CoolChecksumCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __COOLCAMPAIGN_HPP__
