#pragma once

#include <string>
#include <fstream>
#include <mysql/mysql.h>

#ifndef __puma
#include <boost/thread.hpp>
#endif

#include "util/Database.hpp"
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
	fail::Database *db;
	fail::Database *db_recv;
	int fspmethod_id;

	static const std::string dir_images;
	static const std::string dir_prerequisites;
	std::string m_result_table;
	fail::Logger m_log;
	void add_result(unsigned pilot_id,
		int instr2, fail::address_t instr2_absolute, fail::address_t ec_data_address,
		int bitnr, int bit_width, int resulttype, int ecos_test_result, fail::address_t latest_ip,
		int error_corrected, const std::string& details, float runtime);
	void collect_results();
public:
	EcosKernelTestCampaign() : m_log("EcosKernelTest Campaign") {}
	virtual bool run();
	static bool readMemoryMap(fail::MemoryMap &mm, char const * const filename);
	static bool writeTraceInfo(unsigned instr_counter, unsigned timeout, unsigned mem1_low, unsigned mem1_high, unsigned mem2_low, unsigned mem2_high, const std::string& variant = "", const std::string& benchmark = "");
	static bool readTraceInfo(unsigned &instr_counter, unsigned &timeout, unsigned &mem1_low, unsigned &mem1_high, unsigned &mem2_low, unsigned &mem2_high, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_memorymap(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_state(unsigned instr_offset, const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_trace(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_traceinfo(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_elf(const std::string& variant = "", const std::string& benchmark = "");
};
