#ifndef __RAMPAGE_EXPERIMENT_HPP__
  #define __RAMPAGE_EXPERIMENT_HPP__
  
#include <string>

#include "sal/SALConfig.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"

class RAMpageExperimentData;

class RAMpageExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	fail::Logger m_log;
	std::string m_output;
	bool m_last_line_was_startingtestpass;
	fail::MemoryManager& m_mm;
	RAMpageExperimentData *m_param;

	void handleMemWrite(fail::address_t addr);
	bool handleIO(char c);
	void terminateExperiment(int resulttype);
public:
	RAMpageExperiment()
	: m_log("RAMpage"), m_last_line_was_startingtestpass(false),
	  m_mm(fail::simulator.getMemoryManager()) {}
	bool run();
};

#endif // __RAMPAGE_EXPERIMENT_HPP__
