#include <iostream>

#include "DataRetrievalExperiment.hpp"
#include "../SAL/SALInst.hpp"
#include "../controller/Event.hpp"
#include "ExperimentDataExample/FaultCoverageExperiment.pb.h"

using std::cout;
using std::endl;
using std::hex;

#define MEMTEST86_BREAKPOINT 0x4EDC

bool DataRetrievalExperiment::run()
{
	cout << "[getExperimentDataExperiment] Experiment start." << endl;

	// Breakpoint address for Memtest86:
	fi::BPEvent mainbp(MEMTEST86_BREAKPOINT);
	sal::simulator.addEventAndWait(&mainbp);
	cout << "[getExperimentDataExperiment] Breakpoint reached." << endl;
	
	FaultCoverageExperimentData* test = NULL;
	cout << "[getExperimentDataExperiment] Getting ExperimentData (FaultCoverageExperiment)..." << endl;
	test = sal::simulator.getExperimentData<FaultCoverageExperimentData>();
	cout << "[getExperimentDataExperiment] Content of ExperimentData (FaultCoverageExperiment):" << endl;

	if(test->has_data_name())
		cout << "Name: "<< test->data_name() << endl;
	// m_instrptr1 augeben
	cout << "m_instrptr1: " <<  hex << test->m_instrptr1() << endl;
	// m_instrptr2 augeben
	cout << "m_instrptr2: " << hex << test->m_instrptr2() << endl;

	sal::simulator.clearEvents(this);
	return true; // experiment successful
}
