#include <iostream>
#include <cstdlib>

#include "cpn/CampaignManager.hpp"
#include "MHTestCampaign.hpp"

using namespace std;

int main(int argc, char**argv)
{
	int paramcount = 0;
	if (argc == 2)
		paramcount = atoi(argv[1]);
	else
		paramcount = 10;
	cout << "Running MHTestCampaign [" << paramcount <<  " parameter sets]" << endl;

	MHTestCampaign mhc(paramcount);
	fail::campaignmanager.runCampaign(&mhc);

	cout << "Campaign complete." << endl;

	return 0;
}
