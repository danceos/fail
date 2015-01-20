#ifndef __KESOGCCAMPAIGN_HPP__
  #define __KESOGCCAMPAIGN_HPP__

#include "cpn/DatabaseCampaign.hpp"
#include <google/protobuf/descriptor.h>

class GenericExperimentCampaign : public fail::DatabaseCampaign {
	virtual const google::protobuf::Descriptor * cb_result_message() {
		return google::protobuf::DescriptorPool::generated_pool()
			->FindMessageTypeByName("GenericExperimentMessage");
	}

	virtual void cb_send_pilot(DatabaseCampaignMessage pilot);
};

#endif // __KESOGCCAMPAIGN_HPP__
