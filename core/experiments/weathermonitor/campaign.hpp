#ifndef __WEATHERMONITOR_CAMPAIGN_HPP__
  #define __WEATHERMONITOR_CAMPAIGN_HPP__

#include "controller/Campaign.hpp"
#include "controller/ExperimentData.hpp"
#include "weathermonitor.pb.h"

class WeathermonitorExperimentData : public fail::ExperimentData {
public:
	WeathermonitorProtoMsg msg;
	WeathermonitorExperimentData() : fail::ExperimentData(&msg) {}
};

class WeathermonitorCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __WEATHERMONITOR_CAMPAIGN_HPP__
