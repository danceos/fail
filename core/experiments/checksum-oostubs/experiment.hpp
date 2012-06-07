#ifndef __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
  #define __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
  
#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class ChecksumOOStuBSExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	ChecksumOOStuBSExperiment() : m_jc("ios.cs.tu-dortmund.de") {}
	bool run();
};

#endif // __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
