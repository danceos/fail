#pragma once
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class EcosKernelTestExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	EcosKernelTestExperiment() : m_jc("ios.cs.tu-dortmund.de") {}
	bool run();
};

