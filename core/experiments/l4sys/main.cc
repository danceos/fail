#include <iostream>
#include <cstdlib>

#include "controller/CampaignManager.hpp"
#include "experiments/l4sys/campaign.hpp"

int main(int argc, char **argv)
{
	L4SysCampaign c;
	if (fi::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
