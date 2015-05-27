#ifndef __ERIKA_TRACING_HPP__
#define __ERIKA_TRACING_HPP__

#include "efw/ExperimentFlow.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/MemoryMap.hpp"
#include <string>
#include <vector>



class ErikaTracing : public fail::ExperimentFlow {
	std::string start_symbol;
	std::string stop_symbol;
	std::string save_symbol;

	std::string state_file;
	std::string trace_file;
	std::string elf_file;

	bool use_memory_map;
	fail::MemoryMap traced_memory_map;

	bool full_trace;

	fail::Logger m_log;
	fail::ElfReader *m_elf;

public:
	void parseOptions();
	bool run();

	ErikaTracing() : full_trace(false), m_log("ErikaTracing", false) {};
};

#endif // __ERIKA_TRACING_HPP__