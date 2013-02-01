#ifndef __KESO_REFS_EXPERIMENT_HPP__
  #define __KESO_REFS_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include <string>

class KESOrefs : public fail::ExperimentFlow {
  fail::JobClient m_jc;
  fail::ElfReader m_elf;
  fail::Logger m_log;
  fail::MemoryManager& m_mm;

  void printEIP();
  void setupExitBPs(const std::string&);
  void enableBPs();
  void clearExitBPs();
  void showStaticRefs();
  void injectStaticRefs(unsigned referenceoffset, unsigned bitpos);
public:
  KESOrefs() : m_log("KESOrefs", false), m_mm(fail::simulator.getMemoryManager()) {};
  bool run();
};

#endif // __KESO_REFS_EXPERIMENT_HPP__
