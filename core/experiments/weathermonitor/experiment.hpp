#ifndef __WEATHERMONITOR_EXPERIMENT_HPP__
#define __WEATHERMONITOR_EXPERIMENT_HPP__
  
#include "controller/ExperimentFlow.hpp"
#include "jobserver/JobClient.hpp"

class WeathermonitorExperiment : public fi::ExperimentFlow {
	fi::JobClient m_jc;
public:
	WeathermonitorExperiment() : m_jc("ios.cs.tu-dortmund.de") {}
	bool run();
};

#endif 
