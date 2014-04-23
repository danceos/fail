#ifndef __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
  #define __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"

#include "util/Disassembler.hpp"
#include "util/ElfReader.hpp"

class VEZSExperiment : public fail::ExperimentFlow {
    fail::JobClient *m_jc;
    fail::MemoryManager& m_mm;
    fail::ElfReader m_elf;

	unsigned injectBitFlip(fail::address_t data_address, unsigned data_width, unsigned bitpos);
	void redecodeCurrentInstruction();

public:
    VEZSExperiment() : m_mm(fail::simulator.getMemoryManager()) {
        char *server_host = getenv("FAIL_SERVER_HOST");
        if(server_host != NULL){
            this->m_jc = new fail::JobClient(std::string(server_host));
        } else {
            this->m_jc = new fail::JobClient();
        }
    };

    bool run();
};

#endif // __CHECKSUM_OOSTUBS_EXPERIMENT_HPP__
