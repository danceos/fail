#pragma once

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "sal/SALConfig.hpp"

class EcosKernelTestExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	fail::Logger log;
	std::string m_variant, m_benchmark;
public:
	EcosKernelTestExperiment() : log("eCos Kernel Test", false) {}
	bool run();

	bool retrieveGuestAddresses(fail::guest_address_t addr_finish); // step 0
	bool establishState(fail::guest_address_t addr_entry, fail::guest_address_t addr_errors_corrected); // step 1
	bool performTrace(fail::guest_address_t addr_entry, fail::guest_address_t addr_finish); // step 2
	bool faultInjection(); // step 3

	bool readELFSymbols(
		fail::guest_address_t& entry,
		fail::guest_address_t& finish,
		fail::guest_address_t& test_output,
		fail::guest_address_t& errors_corrected,
		fail::guest_address_t& panic,
		fail::guest_address_t& text_start,
		fail::guest_address_t& text_end);
};
