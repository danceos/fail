#ifndef __KESO_GC_EXPERIMENT_HPP__
  #define __KESO_GC_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/Disassembler.hpp"
#include <string>
#include <stdlib.h>


class KESOgc : public fail::ExperimentFlow {
  fail::JobClient *m_jc;
  fail::Logger m_log;
  fail::MemoryManager& m_mm;
  fail::ElfReader m_elf;
  fail::Disassembler m_dis;

  void printEIP();
  void setupExitBPs(const std::string&);
  void enableBPs();
  void clearExitBPs();
  void showStaticRefs();

  unsigned injectBitFlip(fail::address_t data_address, unsigned bitpos);

public:
  KESOgc() : m_log("KESOgc", false), m_mm(fail::simulator.getMemoryManager()) {

	char *server_host = getenv("FAIL_SERVER_HOST");
	if(server_host != NULL){
		this->m_jc = new fail::JobClient(std::string(server_host));
	} else {
		this->m_jc = new fail::JobClient();
	}

  };
  bool run();
};

#endif // __KESO_GC_EXPERIMENT_HPP__
