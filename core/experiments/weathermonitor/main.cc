#include <iostream>
#include <cstdlib>

#include "controller/CampaignManager.hpp"
#include "experiments/checksum-oostubs/campaign.hpp"

int main(int argc, char **argv)
{
	WeathermonitorCampaign c;
	return !fi::campaignmanager.runCampaign(&c);
}
