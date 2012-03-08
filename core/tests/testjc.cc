#include <iostream>
#include "jobserver/JobClient.hpp"

#include "experiments/AnExperiment.hpp"


using namespace std;
using namespace fi;
int main(int argc, char** argv){
    
	int portno;
	JobClient* jc;
	cout << "JobClient" << endl;
	if(argc == 1){
	    jc = new JobClient();
	}else if(argc == 3){
	    portno = atoi(argv[2]);
	    jc = new JobClient(argv[1], portno);
	}else{
		cerr << "usage: " << argv[0] << " hostname port" << endl;
		return 1;
	}

	AnExperimentData exp;

	while(1){
	if(jc->getExperimentData(exp)){
	    /// Do some work.
	    cout << "Got data: " << exp.getInput() << endl;
	    int result =  exp.getInput() * exp.getInput();
	    usleep(500 * 1000); // 500 ms
	    /// Send back.
	    exp.setOutput(result);
	    jc->sendResult(exp);
	  }else{
	    cout << "No (more) data for me :(" << endl;
	    break;
	  }
	}
	delete jc;

}