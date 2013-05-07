#ifndef __KESOREFCAMPAIGN_HPP__
  #define __KESOREFCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include <google/protobuf/descriptor.h>
#include "kesoref.pb.h"


class KesoRefExperimentData : public fail::ExperimentData {
public:
	KesoRefProtoMsg msg;
	KesoRefExperimentData() : fail::ExperimentData(&msg) {}
};


class KesoRefCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() 
	{ return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("KesoRefProtoMsg"); }

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __KESOREFCAMPAIGN_HPP__
