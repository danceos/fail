#ifndef __COOLEXPERIMENT_HPP__
#define __COOLEXPERIMENT_HPP__
  
#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class CoolChecksumExperiment : public fi::ExperimentFlow {
	fi::JobClient m_jc;
public:
	CoolChecksumExperiment() : m_jc("ios.cs.tu-dortmund.de") {}
	bool run();
};

#endif 
