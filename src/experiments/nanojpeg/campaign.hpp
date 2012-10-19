#ifndef __NANOJPEG_CAMPAIGN_HPP__
  #define __NANOJPEG_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "util/Logger.hpp"
#include "nanojpeg.pb.h"

class NanoJPEGExperimentData : public fail::ExperimentData {
public:
	NanoJPEGProtoMsg msg;
	NanoJPEGExperimentData() : fail::ExperimentData(&msg) {}
};

class NanoJPEGCampaign : public fail::Campaign {
	fail::Logger m_log;
public:
	NanoJPEGCampaign() : m_log("nJPEG Campaign") {}
	virtual bool run();
};

#endif // __NANOJPEG_CAMPAIGN_HPP__
