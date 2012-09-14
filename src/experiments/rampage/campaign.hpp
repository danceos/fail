#ifndef __RAMPAGE_CAMPAIGN_HPP__
  #define __RAMPAGE_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "util/Logger.hpp"
#include "rampage.pb.h"

class RAMpageExperimentData : public fail::ExperimentData {
public:
	RAMpageProtoMsg msg;
	RAMpageExperimentData() : fail::ExperimentData(&msg) {}
};

class RAMpageCampaign : public fail::Campaign {
	fail::Logger m_log;

	static uint64_t reverse_bits(uint64_t v);
public:
	RAMpageCampaign() : m_log("RAMpage Campaign") {}
	virtual bool run();
};

#endif // __RAMPAGE_CAMPAIGN_HPP__
