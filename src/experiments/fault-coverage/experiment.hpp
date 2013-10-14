#ifndef __FAULT_COVERAGE_EXPERIMENT_HPP__
#define __FAULT_COVERAGE_EXPERIMENT_HPP__

#include <iostream>
#include <fstream>

#include "config/FailConfig.hpp"
#include "efw/ExperimentFlow.hpp"

#define INST_ADDR_FUNC_START  0x4ae6
#define INST_ADDR_FUNC_END    0x4be6

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_TRAP) || \
    !defined(CONFIG_SR_RESTORE) || !defined(CONFIG_SR_SAVE)
  #error At least one of the following configuration dependencies are not satisfied: \
         breakpoints, traps, save/restore. Enable these in the configuration.
#endif

class FaultCoverageExperiment : public fail::ExperimentFlow {
public:
	bool run();
};

#endif // __FAULT_COVERAGE_EXPERIMENT_HPP__
