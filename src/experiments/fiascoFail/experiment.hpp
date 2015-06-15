#pragma once

#include <string>
#include <vector>

#include "util/Logger.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class FiascoFailExperiment : public fail::ExperimentFlow
{
private:
	fail::Logger log;
	fail::JobClient m_jc;
	std::string m_variant, m_benchmark;
	static const std::string dir_images;
	static const std::string dir_prerequisites;

	bool readTraceInfo(unsigned &instr_counter, unsigned long long &runtime, fail::guest_address_t &addr_finish,
		const std::string& variant, const std::string& benchmark);

	std::string filename_state(unsigned instr_offset, const std::string& variant, const std::string& benchmark);
	std::string filename_traceinfo(const std::string& variant, const std::string& benchmark);
	std::string filename_elf(const std::string& variant, const std::string& benchmark);
	std::string filename_serial(const std::string& variant, const std::string& benchmark);
	std::vector<char> loadFile(std::string filename);

	bool faultInjection();

public:
	FiascoFailExperiment() : log("FiascoFail", false){}
	bool run();
};
