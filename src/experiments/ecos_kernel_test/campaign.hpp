#pragma once

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
public:
	virtual bool run();
	static bool readMemoryMap(fail::MemoryMap &mm, char const * const filename);
	static bool writeTraceInfo(unsigned instr_counter, unsigned timeout, unsigned lowest_addr, unsigned highest_addr);
	static bool readTraceInfo(unsigned &instr_counter, unsigned &timeout, unsigned &lowest_addr, unsigned &highest_addr);
};

