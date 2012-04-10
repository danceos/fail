#include <iostream>
#include <cstdlib>

#include "controller/CampaignManager.hpp"
#include "experiments/checksum-oostubs/campaign.hpp"

int main(int argc, char **argv)
{
	ChecksumOOStuBSCampaign c;
	if (fi::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
