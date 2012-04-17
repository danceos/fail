#ifndef __WEATHERMONITOR_CAMPAIGN_HPP__
#define __WEATHERMONITOR_CAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "weathermonitor.pb.h"

class WeathermonitorExperimentData : public fi::ExperimentData {
public:
	WeathermonitorProtoMsg msg;
	WeathermonitorExperimentData() : fi::ExperimentData(&msg) {}
};

class WeathermonitorCampaign : public fi::Campaign {
public:
	virtual bool run();
};

#endif 
