#ifndef __DCIAOCAMPAIGN_HPP__
#define __DCIAOCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "erika-tester.pb.h"
#include "util/ElfReader.hpp"
#include <google/protobuf/descriptor.h>

class ErikaTesterExperimentData : public fail::ExperimentData {
public:
	ErikaTesterProtoMsg msg;
	ErikaTesterExperimentData() : fail::ExperimentData(&msg) {}
};

class ErikaTesterCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("ErikaTesterProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
	virtual int expected_number_of_results(std::string variant, std::string benchmark) { 
		if (benchmark.find("jump") != std::string::npos)
			return 1;
		else
			return 8;
	}

};

#endif // __KESOREFCAMPAIGN_HPP__
