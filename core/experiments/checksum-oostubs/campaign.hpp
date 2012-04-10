#ifndef __CHECKSUM_OOSTUBS_CAMPAIGN_HPP__
#define __CHECKSUM_OOSTUBS_CAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "checksum-oostubs.pb.h"

class ChecksumOOStuBSExperimentData : public fi::ExperimentData {
public:
	OOStuBSProtoMsg msg;
	ChecksumOOStuBSExperimentData() : fi::ExperimentData(&msg) {}
};

class ChecksumOOStuBSCampaign : public fi::Campaign {
public:
	virtual bool run();
};

#endif 
