#ifndef __L4SYS_CAMPAIGN_HPP__
  #define __L4SYS_CAMPAIGN_HPP__

#include "cpn/Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include "l4sys.pb.h"

class L4SysExperimentData : public fail::ExperimentData {
public:
	L4SysProtoMsg msg;
	L4SysExperimentData() : fail::ExperimentData(&msg) {}
};

class L4SysCampaign : public fail::Campaign {
public:
	virtual bool run();
private:
	std::string output_result(L4SysProtoMsg_ResultType res);
	std::string output_experiment(L4SysProtoMsg_ExperimentType res);
	std::string output_register(L4SysProtoMsg_RegisterType res);
};

#endif // __L4SYS_CAMPAIGN_HPP__
