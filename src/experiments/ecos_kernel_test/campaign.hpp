#pragma once

#include <string>

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "ecos_kernel_test.pb.h"
#include "util/MemoryMap.hpp"

class EcosKernelTestExperimentData : public fail::ExperimentData {
public:
	EcosKernelTestProtoMsg msg;
	EcosKernelTestExperimentData() : fail::ExperimentData(&msg) {}
};

class EcosKernelTestCampaign : public fail::Campaign {
	static const std::string dir_images;
	static const std::string dir_prerequisites;
	static const std::string dir_results;
public:
	virtual bool run();
	static bool readMemoryMap(fail::MemoryMap &mm, char const * const filename);
	static bool writeTraceInfo(unsigned instr_counter, unsigned timeout, unsigned lowest_addr, unsigned highest_addr);
	static bool readTraceInfo(unsigned &instr_counter, unsigned &timeout, unsigned &lowest_addr, unsigned &highest_addr);
	static std::string filename_memorymap(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_state(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_trace(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_traceinfo(const std::string& variant = "", const std::string& benchmark = "");
	static std::string filename_results(const std::string& variant, const std::string& benchmark);
};

