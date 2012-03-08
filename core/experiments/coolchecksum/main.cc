#include <iostream>
#include <cstdlib>

#include "controller/CampaignManager.hpp"
#include "experiments/coolchecksum/campaign.hpp"

int main(int argc, char **argv)
{
	CoolChecksumCampaign c;
	if (fi::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
