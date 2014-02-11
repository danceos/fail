#pragma once

#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "experimentInfo.hpp"
#include "ecos_kernel_test.pb.h"
//#include <google/protobuf/descriptor.h>

class EcosKernelTestExperimentData : public fail::ExperimentData {
public:
	EcosKernelTestProtoMsg msg;
	EcosKernelTestExperimentData() : fail::ExperimentData(&msg) {}
};

class EcosKernelTestCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor *cb_result_message()
	{
		return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("EcosKernelTestProtoMsg");
	}

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);

	virtual int expected_number_of_results(std::string variant, std::string benchmark)
	{
#if ECOS_FAULTMODEL_BURST
		return 1;
#else
		return 8;
#endif
	}
};
