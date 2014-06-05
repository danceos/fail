#ifndef __EXPERIMENT_INFO_HPP__
#define __EXPERIMENT_INFO_HPP__

#include "comm/ExperimentData.hpp"
#include "generic-experiment.pb.h"

class GenericExperimentData : public fail::ExperimentData {
public:
	GenericExperimentMessage msg;
	GenericExperimentData() : fail::ExperimentData(&msg) {}
};



#endif // __EXPERIMENT_INFO_HPP__
