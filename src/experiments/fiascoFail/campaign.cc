#include "campaign.hpp"
#include "cpn/CampaignManager.hpp"

void FiascoFailCampaign::cb_send_pilot(DatabaseCampaignMessage pilot)
{
	FiascoFailExperimentData *data = new FiascoFailExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	fail::campaignmanager.addParam(data);
}
