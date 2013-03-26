#ifndef __DCIAOCAMPAIGN_HPP__
#define __DCIAOCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "dciao_kernel.pb.h"
#include "util/ElfReader.hpp"
#include <google/protobuf/descriptor.h>

class DCIAOKernelExperimentData : public fail::ExperimentData {
public:
	DCIAOKernelProtoMsg msg;
	DCIAOKernelExperimentData() : fail::ExperimentData(&msg) {}
};

class DCIAOKernelCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("DCIAOKernelProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __KESOREFCAMPAIGN_HPP__
