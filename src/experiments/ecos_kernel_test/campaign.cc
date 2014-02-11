#include "campaign.hpp"
#include "cpn/CampaignManager.hpp"

void EcosKernelTestCampaign::cb_send_pilot(DatabaseCampaignMessage pilot)
{
	EcosKernelTestExperimentData *data = new EcosKernelTestExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	fail::campaignmanager.addParam(data);
}
