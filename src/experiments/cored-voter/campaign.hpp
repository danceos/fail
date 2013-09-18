#ifndef __DCIAOCAMPAIGN_HPP__
#define __DCIAOCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "cored_voter.pb.h"
#include "util/ElfReader.hpp"
#include <google/protobuf/descriptor.h>

class CoredVoterExperimentData : public fail::ExperimentData {
public:
	CoredVoterProtoMsg msg;
	CoredVoterExperimentData() : fail::ExperimentData(&msg) {}
};

class CoredVoterCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("CoredVoterProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
	virtual int expected_number_of_results(std::string variant, std::string benchmark) {
		if (benchmark.find("random-2") != std::string::npos)
			return 248; // 32 * 31 / 4
		else if (benchmark.find("random") != std::string::npos)
			/* This number was choosen arbitrarily. It should be high
			   enough to get at least 5000 experiments per qtrace
			   event. But since we're doing Monte Carlo it does not
			   matter whether we reach this number. */
			return 5000;
		else if (benchmark.find("jump") != std::string::npos)
			return 1;
		else
			return 8;
	}

};

#endif // __KESOREFCAMPAIGN_HPP__
