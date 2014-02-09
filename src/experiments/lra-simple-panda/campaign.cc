#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/ElfReader.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;
using namespace google::protobuf;

void LraSimpleCampaign::cb_send_pilot(DatabaseCampaignMessage pilot)
{
	LraSimpleExperimentData *data = new LraSimpleExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	campaignmanager.addParam(data);
}
