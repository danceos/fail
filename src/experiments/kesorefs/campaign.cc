#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"


using namespace std;
using namespace fail;
using namespace google::protobuf;

void KesoRefCampaign::cb_send_pilot(DatabaseCampaignMessage pilot) {
	KesoRefExperimentData *data = new KesoRefExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	campaignmanager.addParam(data);
}
