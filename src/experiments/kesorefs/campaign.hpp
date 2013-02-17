#ifndef __KESOREFCAMPAIGN_HPP__
  #define __KESOREFCAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "kesoref.pb.h"

class KesoRefExperimentData : public fail::ExperimentData {
public:
	KesoRefProtoMsg msg;
	KesoRefExperimentData() : fail::ExperimentData(&msg) {}
};

class KesoRefCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __KESOREFCAMPAIGN_HPP__
