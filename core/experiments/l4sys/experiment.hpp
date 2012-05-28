#ifndef __COOLEXPERIMENT_HPP__
#define __COOLEXPERIMENT_HPP__

#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class L4SysExperiment : public fi::ExperimentFlow {
	fi::JobClient m_jc;
public:
	L4SysExperiment() : m_jc("localhost") {}
	bool run();
private:
	std::string sanitised(std::string in_str);
	fi::BaseEvent* waitGuestOrOther(bool clear_output);
};

#endif 
