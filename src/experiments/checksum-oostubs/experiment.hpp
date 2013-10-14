#ifndef __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
#define __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class ChecksumOOStuBSExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	bool run();
};

#endif // __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
