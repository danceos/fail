#pragma once
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"

class EcosKernelTestExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	fail::Logger log;
public:
	EcosKernelTestExperiment() : m_jc("ios.cs.tu-dortmund.de"), log("eCos Kernel Test", false) {}
	bool run();

	bool retrieveGuestAddresses(); // step 0
	bool establishState(); // step 1
	bool performTrace(); // step 2
	bool faultInjection(); // step 3
};

