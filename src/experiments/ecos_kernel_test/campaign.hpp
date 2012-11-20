#pragma once

#include <string>
#include <fstream>

#ifndef __puma
#include <boost/thread.hpp>
#endif

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "ecos_kernel_test.pb.h"
#include "util/MemoryMap.hpp"
#include "util/Logger.hpp"

class EcosKernelTestExperimentData : public fail::ExperimentData {
public:
	EcosKernelTestProtoMsg msg;
	EcosKernelTestExperimentData() : fail::ExperimentData(&msg) {}
};

class EcosKernelTestCampaign : public fail::Campaign {
	static const std::string dir_images;
	static const std::string dir_prerequisites;
	static const std::string dir_results;
	fail::Logger m_log;
	int count_exp, count_exp_jobs;
	int count_known, count_known_jobs;
	bool add_experiment_ec(const std::string& variant, const std::string& benchmark,
		fail::address_t data_address, int instr1, int instr2, fail::address_t instr_absolute);
	bool add_known_ec(const std::string& variant, const std::string& benchmark,
		fail::address_t data_address, int instr1, int instr2, fail::address_t instr_absolute);
	bool init_results();
	void add_result(const std::string& variant, const std::string& benchmark,
		int instr1, int instr2, fail::address_t instr2_absolute, fail::address_t ec_data_address,
		int bitnr, int bit_width, int resulttype, int ecos_test_result, fail::address_t latest_ip,
		int error_corrected, const std::string& details, float runtime);
	void finalize_results();
	void collect_results();
	bool check_available(const std::string& variant, const std::string& benchmark, fail::address_t data_address, int instr2);
	std::ofstream resultstream;
	typedef std::map<std::pair<const std::string, const std::string>, std::map<fail::address_t, std::set<int> > > AvailableResultMap;
	AvailableResultMap available_results;
#ifndef __puma
	boost::mutex m_result_mutex;
#endif
public:
	EcosKernelTestCampaign() : m_log("EcosKernelTest Campaign"),
		count_exp(0), count_exp_jobs(0), count_known(0), count_known_jobs(0) {}
	virtual bool run();
	static bool readMemoryMap(fail::MemoryMap &mm, char const * const filename);
	static bool writeTraceInfo(unsigned instr_counter, unsigned timeout, unsigned lowest_addr, unsigned highest_addr);
	static bool readTraceInfo(unsigned &instr_counter, unsigned &timeout, unsigned &lowest_addr, unsigned &highest_addr, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_memorymap(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_state(unsigned instr_offset, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_trace(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_traceinfo(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_results(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_elf(const std::string& variant = "", const std::string& benchmark = "");
};
