#pragma once

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "experimentInfo.hpp"
#include "fiascofail.pb.h"
//#include <google/protobuf/descriptor.h>

class FiascoFailExperimentData : public fail::ExperimentData
{
public:
	FiascofailProtoMsg msg;
	FiascoFailExperimentData() : fail::ExperimentData(&msg) {}
};

class FiascoFailCampaign : public fail::DatabaseCampaign
{
	virtual const google::protobuf::Descriptor * cb_result_message()
	{
		return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("FiascofailProtoMsg");
	}
	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);

	virtual int expected_number_of_results(std::string variant, std::string benchmark)
	{
#if FIASCO_FAULTMODEL_BURST
		return 1;
#else
		return 8;
#endif
	}
};
