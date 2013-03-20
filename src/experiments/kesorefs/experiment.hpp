#ifndef __KESO_REFS_EXPERIMENT_HPP__
  #define __KESO_REFS_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/Disassembler.hpp"
#include <string>

class KESOrefs : public fail::ExperimentFlow {
  fail::JobClient m_jc;
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
  KESOrefs() : m_log("KESOrefs", false), m_mm(fail::simulator.getMemoryManager()) {
  };
  bool run();
};

#endif // __KESO_REFS_EXPERIMENT_HPP__
