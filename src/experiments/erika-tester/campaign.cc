#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;
using namespace google::protobuf;

void ErikaTesterCampaign::cb_send_pilot(DatabaseCampaignMessage pilot) {
	ErikaTesterExperimentData *data = new ErikaTesterExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	campaignmanager.addParam(data);
}

