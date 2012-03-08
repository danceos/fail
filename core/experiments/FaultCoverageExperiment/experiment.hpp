#ifndef __FAULTCOVERAGE_EXPERIMENT_HPP__
  #define __FAULTCOVERAGE_EXPERIMENT_HPP__

#include <iostream>
#include <fstream>

#include "AspectConfig.hpp"
#include "controller/ExperimentFlow.hpp"

#define INST_ADDR_FUNC_START  0x4ae6		
#define INST_ADDR_FUNC_END    0x4be6

/*
// Check if aspect dependencies are satisfied:
#if CONFIG_EVENT_CPULOOP != 1 || CONFIG_EVENT_TRAP != 1 || \
    CONFIG_SR_RESTORE != 1 || CONFIG_SR_SAVE != 1
  #error At least one of the following aspect-dependencies are not satisfied: \
         cpu loop, traps, save/restore. Enable aspects first (see AspectConfig.hpp)!
#endif
//   This is disabled because the AspectConfig.hpp-header disables
//   all aspects on default.
*/
using namespace fi;

class FaultCoverageExperiment : public ExperimentFlow
{
	public:
		bool run();
};

#endif // __FAULTCOVERAGE_EXPERIMENT_HPP__
 
