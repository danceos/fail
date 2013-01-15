#ifndef __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
  #define __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
  
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"

class VEZSExperiment : public fail::ExperimentFlow {

  fail::JobClient m_jc;
  fail::ElfReader m_elf;
  fail::Logger m_log;

  void printEIP();

public:
  VEZSExperiment() : m_log("VEZS-example", false) {};
	bool run();
};

#endif // __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
