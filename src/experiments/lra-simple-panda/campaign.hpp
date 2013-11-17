#ifndef __DCIAOCAMPAIGN_HPP__
#define __DCIAOCAMPAIGN_HPP__

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
};

#endif // __KESOREFCAMPAIGN_HPP__
