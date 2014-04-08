#ifndef __KESOGCCAMPAIGN_HPP__
  #define __KESOGCCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include <google/protobuf/descriptor.h>
#include "kesogc.pb.h"


class KesoGcExperimentData : public fail::ExperimentData {
public:
	KesoGcProtoMsg msg;
	KesoGcExperimentData() : fail::ExperimentData(&msg) {}
};


class KesoGcCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("KesoGcProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __KESOGCCAMPAIGN_HPP__
