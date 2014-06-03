#ifndef __LRA_CAMPAIGN_HPP__
#define __LRA_CAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "lra_simple.pb.h"
#include "util/ElfReader.hpp"
#include <google/protobuf/descriptor.h>

class LraSimpleExperimentData : public fail::ExperimentData {
public:
	LraSimpleProtoMsg msg;
	LraSimpleExperimentData() : fail::ExperimentData(&msg) {}
};

class LraSimpleCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message()
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("LraSimpleProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
	virtual int expected_number_of_results(std::string variant, std::string benchmark) {
		return 1;
	}
};

#endif // __LRA_CAMPAIGN_HPP__
