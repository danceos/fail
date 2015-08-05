#ifndef __CORED_TESTER_EXPERIMENT_HPP__
#define __CORED_TESTER_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include <vector>
#include <string>
#include "util/llvmdisassembler/LLVMtoFailTranslator.hpp"

class dOSEKTester : public fail::ExperimentFlow {
public:

private:
	fail::Logger m_log;
	fail::MemoryManager& m_mm;
	fail::ElfReader m_elf;
	fail::LLVMtoFailTranslator* m_ltof;

	unsigned injectBitFlip(fail::address_t data_address, unsigned data_width, unsigned bitpos);
	void redecodeCurrentInstruction();
	const fail::ElfSymbol& getELFSymbol(const std::string name);

public:
	dOSEKTester() : m_log("dOSEKTester", false), m_mm(fail::simulator.getMemoryManager()) {}

	bool run();
};

#endif
