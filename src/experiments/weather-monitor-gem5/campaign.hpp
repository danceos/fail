#ifndef __WEATHERMONITOR_CAMPAIGN_HPP__
  #define __WEATHERMONITOR_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "weathermonitor.pb.h"

class WeatherMonitorExperimentData : public fail::ExperimentData {
public:
	WeathermonitorProtoMsg msg;
	WeatherMonitorExperimentData() : fail::ExperimentData(&msg) {}
};

class WeatherMonitorCampaign : public fail::Campaign {
public:
	virtual bool run();
};

#endif // __WEATHERMONITOR_CAMPAIGN_HPP__
