#pragma once

#include <string>
#include <vector>

#include "util/Logger.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "sal/Listener.hpp"

class FiascoFailExperiment : public fail::ExperimentFlow
{
private:
	fail::Logger m_log;
	fail::JobClient m_jc;
	std::string m_variant, m_benchmark;
	void readGoldenRun(std::string &);
	fail::BaseListener* waitIOOrOther(bool);
	void parseOptions();
	bool faultInjection();

	std::string m_CurrentOutput;
	fail::guest_address_t endAddress;
	unsigned golden_run_instructions;
	unsigned long long golden_run_timer_ticks;
	fail::guest_address_t ecc_panic_address;
	fail::guest_address_t addr_errors_corrected;
	bool _golden_run;
	void goldenRun();

public:
	FiascoFailExperiment() : m_log("FiascoFail", false){}
	bool run();
};
