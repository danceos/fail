#include <iostream>

#include "MHTestCampaign.hpp"
#include <cpn/CampaignManager.hpp>

using namespace std;
using namespace fail;

bool MHTestCampaign::run()
{
	MHExperimentData* datas[m_parameter_count];
	cout << "[MHTestCampaign] Adding " << m_parameter_count << " values." << endl;
	for (int i = 1; i <= m_parameter_count; i++) {
		datas[i] = new MHExperimentData;
		datas[i]->msg.set_input(i);

		campaignmanager.addParam(datas[i]);
		usleep(100 * 1000); // 100 ms
	}
	campaignmanager.noMoreParameters();
	// test results.
	int f;
	int res = 0;
	int res2 = 0; 
	MHExperimentData * exp;
	for (int i = 1; i <= m_parameter_count; i++) {
		exp = static_cast<MHExperimentData*>(campaignmanager.getDone());
		f = exp->msg.output();
//		cout << ">>>>>>>>>>>>>>> Output: " << i << "^2 = " <<  f  << endl;
		res += f;
		res2 += (i*i);
		delete exp;
	}
	if (res == res2) {
		cout << "TEST SUCCESSFUL FINISHED! " << "[" << res << "==" << res2 << "]" << endl;
	} else {
		cout << "TEST FAILED!" << " [" << res << "!=" << res2 << "]" << endl;
	}
	cout << "thats all... " << endl;
	
	return true;  
}
