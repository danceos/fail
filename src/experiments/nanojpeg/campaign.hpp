#ifndef __NANOJPEG_CAMPAIGN_HPP__
  #define __NANOJPEG_CAMPAIGN_HPP__

#include <fstream>
#include <map>
#include <utility>

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "util/Logger.hpp"
#include "sal/x86/X86Architecture.hpp"
#include "nanojpeg.pb.h"

class NanoJPEGExperimentData : public fail::ExperimentData {
public:
	NanoJPEGProtoMsg msg;
	NanoJPEGExperimentData() : fail::ExperimentData(&msg) {}
};

class NanoJPEGCampaign : public fail::Campaign {
	fail::Logger m_log;
	int count_exp, count_exp_jobs;
	int count_known, count_known_jobs;
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
	uint64_t check_available(unsigned instr_offset,
		fail::GPRegisterId register_id, uint64_t bitmask);
	std::ofstream resultstream;
	std::map<std::pair<unsigned, int>, uint64_t> available_results;
public:
	NanoJPEGCampaign() : m_log("nJPEG Campaign"),
		count_exp(0), count_exp_jobs(0), count_known(0), count_known_jobs(0) {}
	virtual bool run();
};

#endif // __NANOJPEG_CAMPAIGN_HPP__
