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

	fail::guest_address_t start_address;
	fail::guest_address_t stop_address;

	std::string state_file;
	std::string trace_file;
	std::string elf_file;

	bool use_memory_map;
	fail::MemoryMap traced_memory_map;

	bool restore;
	bool full_trace;
	bool check_bounds;

	fail::guest_address_t serial_port;
	std::string serial_file;

	fail::Logger m_log;
	fail::ElfReader *m_elf;
	
	bool enabled_trap;

public:
	void parseOptions();
	bool run();

	GenericTracing() : restore(false),
		full_trace(false), check_bounds(false), m_log("GenericTracing", false),
		enabled_trap(false)
		{}
};

#endif // __TRACING_TEST_HPP__
