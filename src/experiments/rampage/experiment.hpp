#ifndef __RAMPAGE_EXPERIMENT_HPP__
  #define __RAMPAGE_EXPERIMENT_HPP__
  
#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"

class RAMpageExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	fail::Logger m_log;
	std::string m_output;
	bool last_line_was_startingtestpass;

	bool handleIO(char c);
public:
	RAMpageExperiment() : m_log("RAMpage"), last_line_was_startingtestpass(false) {}
	bool run();
};

#endif // __RAMPAGE_EXPERIMENT_HPP__
