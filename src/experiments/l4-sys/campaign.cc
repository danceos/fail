#include "campaign.hpp"
#include "experiment.hpp"
#include "cpn/CampaignManager.hpp"

void L4SysCampaign::cb_send_pilot(DatabaseCampaignMessage p)
{
	L4SysExperimentData *d = new L4SysExperimentData;
	d->msg.mutable_fsppilot()->CopyFrom(p);

	if(!type.compare("mem")) {
		d->msg.set_exp_type(d->msg.MEM);
	} else if(!type.compare("reg")) {
		d->msg.set_exp_type(d->msg.GPRFLIP);
	} else {
		log << "Specified FI-type not supported" << std::endl;
		exit(-1);
	}

	fail::campaignmanager.addParam(d);
}
