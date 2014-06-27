#ifndef __WEATHERMONITOR_EXPERIMENT_HPP__
#define __WEATHERMONITOR_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

#include "util/Logger.hpp"
#include "util/ElfReader.hpp"

#include "sal/Listener.hpp"
#include "sal/SALConfig.hpp"

class WeatherMonitorExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	std::string m_variant, m_benchmark;
    static const std::string dir_images;
    static const std::string dir_prerequisites;
	fail::Logger LOG;
	fail::BPSingleListener bp;

	std::string filename_elf(const std::string& variant, const std::string& benchmark);
	std::string filename_state(const std::string& variant, const std::string& benchmark);
	std::string filename_trace(const std::string& variant, const std::string& benchmark);
	std::string filename_traceinfo(const std::string& variant, const std::string& benchmark);
	bool writeTraceInfo(unsigned numinstr_tracing, unsigned numinstr_after);
	bool readTraceInfo(unsigned& numinstr_tracing, unsigned& numinstr_after);
	bool readElfSymbols(fail::guest_address_t& entry, fail::guest_address_t& text_start,
						fail::guest_address_t& text_end, fail::guest_address_t& data_start,
						fail::guest_address_t& data_end, fail::guest_address_t& wait_begin,
						fail::guest_address_t& wait_end, fail::guest_address_t& vptr_panic);
	bool establishState(fail::guest_address_t& entry);
	bool performTrace(fail::guest_address_t& entry, fail::guest_address_t& data_start,
						fail::guest_address_t& data_end, fail::guest_address_t& wait_end);
	bool faultInjection();

public:
	WeatherMonitorExperiment() : LOG("Weathermonitor", false) {}
	bool run();
	void parseOptions(void);
};

#endif // __WEATHERMONITOR_EXPERIMENT_HPP__
