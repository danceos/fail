#ifndef __NANOJPEG_CAMPAIGN_HPP__
  #define __NANOJPEG_CAMPAIGN_HPP__

#include <fstream>

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "util/Logger.hpp"
#include "sal/bochs/BochsRegisterIDs.hpp"
#include "nanojpeg.pb.h"

class NanoJPEGExperimentData : public fail::ExperimentData {
public:
	NanoJPEGProtoMsg msg;
	NanoJPEGExperimentData() : fail::ExperimentData(&msg) {}
};

class NanoJPEGCampaign : public fail::Campaign {
	fail::Logger m_log;
	int add_experiment_ec(unsigned instr_ecstart, unsigned instr_offset,
		unsigned instr_address, fail::GPRegisterId register_id, uint64_t bitmask);
	int add_known_ec(unsigned instr_ecstart, unsigned instr_offset,
		unsigned instr_address, fail::GPRegisterId register_id, uint64_t bitmask);
	bool init_results();
	void add_result(unsigned instr_ecstart, unsigned instr_offset,
		uint32_t instr_address, fail::GPRegisterId register_id,
		unsigned timeout, uint32_t injection_ip, uint64_t bitmask,
		int resulttype, uint32_t latest_ip, float psnr, char const *details);
	void finalize_results();
	std::ofstream resultstream;
public:
	NanoJPEGCampaign() : m_log("nJPEG Campaign") {}
	virtual bool run();
};

#endif // __NANOJPEG_CAMPAIGN_HPP__
