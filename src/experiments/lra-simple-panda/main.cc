#include <iostream>
#include <cstdlib>

#include "cpn/CampaignManager.hpp"
#include "util/CommandLine.hpp"
#include "campaign.hpp"

int main(int argc, char **argv)
{
	fail::CommandLine &cmd = fail::CommandLine::Inst();
	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}

	LraSimpleCampaign c;
	if (fail::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
