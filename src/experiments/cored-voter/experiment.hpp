#ifndef __CORED_VOTER_EXPERIMENT_HPP__
#define __CORED_VOTER_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include <vector>
#include <string>

class CoredVoter : public fail::ExperimentFlow {
public:

private:
	fail::JobClient m_jc;
	fail::Logger m_log;
	fail::MemoryManager& m_mm;
	fail::ElfReader m_elf;


	unsigned injectBitFlip(fail::address_t data_address, unsigned data_width, unsigned bitpos);
	void redecodeCurrentInstruction();

public:
	CoredVoter() : m_log("CoredVoter", false), m_mm(fail::simulator.getMemoryManager()) {}

	bool run();
};

#endif 
