#ifndef __CHECKSUM_OOSTUBS_CAMPAIGN_HPP__
#define __CHECKSUM_OOSTUBS_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "checksum-oostubs.pb.h"

class ChecksumOOStuBSExperimentData : public fail::ExperimentData {
public:
	OOStuBSProtoMsg msg;
	ChecksumOOStuBSExperimentData() : fail::ExperimentData(&msg) {}
};

class ChecksumOOStuBSCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif  // __CHECKSUM_OOSTUBS_CAMPAIGN_HPP__
