#pragma once

#include <string>
#include <vector>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "sal/SALConfig.hpp"

class EcosKernelTestExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
	fail::Logger log;
	std::string m_variant, m_benchmark;

	static const std::string dir_images;
	static const std::string dir_prerequisites;
public:
	EcosKernelTestExperiment() : log("eCos Kernel Test", false) {}
	bool run();

	void parseOptions();
	bool retrieveGuestAddresses(fail::guest_address_t addr_finish, fail::guest_address_t addr_data_start, fail::guest_address_t addr_data_end); // step 0
	bool establishState(fail::guest_address_t addr_entry, fail::guest_address_t addr_finish, fail::guest_address_t addr_errors_corrected); // step 1
	bool performTrace(fail::guest_address_t addr_entry, fail::guest_address_t addr_finish,
		fail::guest_address_t addr_testdata, fail::guest_address_t addr_testdata_size); // step 2
	bool faultInjection(); // step 3

	bool readELFSymbols(
		fail::guest_address_t& entry,
		fail::guest_address_t& finish,
		fail::guest_address_t& testdata,
		fail::guest_address_t& testdata_size,
		fail::guest_address_t& test_output,
		fail::guest_address_t& errors_corrected,
		fail::guest_address_t& panic,
		fail::guest_address_t& text_start,
		fail::guest_address_t& text_end,
		fail::guest_address_t& data_start,
		fail::guest_address_t& data_end);

	void handle_func_test_output(bool &test_failed, bool& test_passed);

	static bool writeTraceInfo(unsigned instr_counter, unsigned long long runtime, unsigned mem1_low, unsigned mem1_high, unsigned mem2_low, unsigned mem2_high, const std::string& variant = "", const std::string& benchmark = "");
	static bool readTraceInfo(unsigned &instr_counter, unsigned long long &runtime, unsigned &mem1_low, unsigned &mem1_high, unsigned &mem2_low, unsigned &mem2_high, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_memorymap(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_state(unsigned instr_offset, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_trace(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_traceinfo(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_elf(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_serial(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_memout(const std::string& variant = "", const std::string& benchmark = "");

	std::vector<char> loadFile(std::string filename);
	bool isMapped(fail::MemoryManager& mm, fail::guest_address_t start, unsigned len);
	std::vector<char> readFromMem(fail::guest_address_t addr_testdata, fail::guest_address_t addr_testdata_size);
};
