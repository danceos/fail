#ifndef __GENERIC_TRACING_HPP__
#define __GENERIC_TRACING_HPP__

#include "efw/ExperimentFlow.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "util/MemoryMap.hpp"
#include <string>
#include <vector>

class GenericTracing : public fail::ExperimentFlow {
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

	GenericTracing() : full_trace(false), m_log("GenericTracing", false) {}
};

#endif // __TRACING_TEST_HPP__
