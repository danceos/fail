#include "jobserver/JobServer.hpp"
#include <iostream>
#include "experiments/AnExperiment.hpp"

#include <boost/thread.hpp>


fi::JobServer js;
using namespace std;

static const int nums = 30;

void exec_js(){
	js.waitForConnections();
	cout << "That's it.." << endl;
	
}



int main(int argc, char**argv){


	cout << "Testing Jobserver" << endl;
	boost::thread th(exec_js);
	
	AnExperimentData* datas[nums];

	for(int i = 1; i <= nums; i++){
	  datas[i] = new AnExperimentData;
	  datas[i]->setInput(i);
	  js.addExperiment(datas[i]);
	  usleep(100 * 1000); // 100 ms
	}
	js.setNoMoreExperiments();

	// test results.
	int f;
	int res = 0;
	int res2 = 0; 
	AnExperimentData * exp;
	for(int i = 1; i <= nums; i++){
		exp = static_cast<AnExperimentData*>( js.m_doneJobs.Dequeue() );
		f = exp->getOutput();
//		cout << ">>>>>>>>>>>>>>> Output: " << i << "^2 = " <<  f  << endl;
		res += f;
		res2 += (i*i);
		delete exp;
	}
	if (res == res2) {
		cout << "TEST SUCCESSFUL FINISHED! " << "[" << res << "==" << res2 << "]" << endl;
	}else{
		cout << "TEST FAILED!" << " [" << res << "!=" << res2 << "]" << endl;
	}
	cout << "thats all, waiting for server thread. " << endl;
	js.done();

	th.join();
	
	return 0;
}

