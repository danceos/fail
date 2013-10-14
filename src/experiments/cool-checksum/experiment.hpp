#ifndef __COOLEXPERIMENT_HPP__
#define __COOLEXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class CoolChecksumExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	bool run();
};

#endif // __COOLEXPERIMENT_HPP__
