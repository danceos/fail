#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "comm/DatabaseCampaignMessage.pb.h"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"

#include "../plugins/tracing/TracingPlugin.hpp"

//#define PRUNING_DEBUG_OUTPUT

using namespace std;
using namespace fail;

using namespace google::protobuf;

void WeatherMonitorCampaign::cb_send_pilot(DatabaseCampaignMessage pilot)
{
	WeatherMonitorExperimentData *data = new WeatherMonitorExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	campaignmanager.addParam(data);
}
