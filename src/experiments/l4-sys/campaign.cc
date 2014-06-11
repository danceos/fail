#include "campaign.hpp"
#include "experiment.hpp"
#include "cpn/CampaignManager.hpp"

void L4SysCampaign::cb_send_pilot(DatabaseCampaignMessage p)
{
	L4SysExperimentData *d = new L4SysExperimentData;
	d->msg.mutable_fsppilot()->CopyFrom(p);
	//d->msg.set_exp_type(d->msg.GPRFLIP);
	d->msg.set_exp_type(d->msg.MEM);
	fail::campaignmanager.addParam(d);
}
