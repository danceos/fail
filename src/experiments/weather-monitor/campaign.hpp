#ifndef __WEATHERMONITOR_CAMPAIGN_HPP__
#define __WEATHERMONITOR_CAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "weathermonitor.pb.h"
#include <google/protobuf/descriptor.h>

class WeatherMonitorExperimentData : public fail::ExperimentData {
public:
	WeathermonitorProtoMsg msg;
	WeatherMonitorExperimentData() : fail::ExperimentData(&msg) {}
};

class WeatherMonitorCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message()
	{
		return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("WeathermonitorProtoMsg");
	}

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __WEATHERMONITOR_CAMPAIGN_HPP__
