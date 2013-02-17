#include <iostream>
#include <cstdlib>

#include "cpn/CampaignManager.hpp"
#include "campaign.hpp"

int main(int argc, char **argv)
{
	WeatherMonitorCampaign c;
	return !fail::campaignmanager.runCampaign(&c);
}
