#ifndef __KESOGCCAMPAIGN_HPP__
  #define __KESOGCCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include <google/protobuf/descriptor.h>
#include "vezs.pb.h"


class VEZSExperimentData : public fail::ExperimentData {
public:
	VEZSProtoMsg msg;
	VEZSExperimentData() : fail::ExperimentData(&msg) {}
};


class VEZSCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("VEZSProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __KESOGCCAMPAIGN_HPP__
