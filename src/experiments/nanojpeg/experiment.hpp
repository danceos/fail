#ifndef __NANOJPEG_EXPERIMENT_HPP__
#define __NANOJPEG_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class NanoJPEGExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	bool run();
};

#endif // __NANOJPEG_EXPERIMENT_HPP__
