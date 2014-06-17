#ifndef __L4SYS_CAMPAIGN_HPP__
#define __L4SYS_CAMPAIGN_HPP__


#include "cpn/DatabaseCampaign.hpp"
#include "comm/ExperimentData.hpp"
#include "l4sys.pb.h"
#include <google/protobuf/descriptor.h>

#include "util/Logger.hpp" 

#include <string>

class L4SysExperimentData : public fail::ExperimentData {
public:
	L4SysProtoMsg msg;
	L4SysExperimentData() : fail::ExperimentData(&msg) {}
};

class L4SysCampaign : public fail::DatabaseCampaign {
    virtual const google::protobuf::Descriptor * cb_result_message() 
    { return google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("L4SysProtoMsg"); }

    virtual void cb_send_pilot(DatabaseCampaignMessage pilot);

		fail::Logger log; //<! the logger

public:
		std::string type;
};

#endif // __L4SYS_CAMPAIGN_HPP__
