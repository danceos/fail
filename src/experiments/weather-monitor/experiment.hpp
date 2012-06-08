#ifndef __WEATHERMONITOR_EXPERIMENT_HPP__
  #define __WEATHERMONITOR_EXPERIMENT_HPP__
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class WeatherMonitorExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	WeatherMonitorExperiment() : m_jc("ios.cs.tu-dortmund.de") {}
	bool run();
};

#endif // __WEATHERMONITOR_EXPERIMENT_HPP__
