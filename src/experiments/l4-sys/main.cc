#include <iostream>
#include <cstdlib>

#include "cpn/CampaignManager.hpp"
#include "campaign.hpp"
#include "util/CommandLine.hpp"  

int main(int argc, char **argv)
{

	L4SysCampaign c;

	fail::CommandLine &cmd = fail::CommandLine::Inst();

	for (int i = 1; i < argc; ++i) {
		if(strncmp(argv[i], "--type=", 7) == 0) 
			c.type = std::string(std::string(argv[i]), 7);
		else	
			cmd.add_args(argv[i]);
	}

	if (c.type.empty()) {
		std::cerr << "You have to specify FI type" << std::endl;
		exit(-1);
	} 

	if(!c.type.compare("mem")) {
		std::cout << "We will do memory FI" << std::endl;
	} else if(!c.type.compare("reg")) {
		std::cout << "We will do register FI" << std::endl;
	} else {
		std::cout << "Specified FI-type not supported" << std::endl;
		exit(-1);
	}

	if (fail::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
