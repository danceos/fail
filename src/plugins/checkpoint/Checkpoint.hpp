#ifndef __Checkpoint_HPP__
#define __Checkpoint_HPP__

#include <string>
#include <vector>
#include <utility>
#include <unistd.h>
#include "efw/ExperimentFlow.hpp"
#include "config/FailConfig.hpp"
#include "util/Logger.hpp"
#include <fstream>
#include "util/ElfReader.hpp"

/**
 * @class Checkpoint
 * @brief Listens to a memory location and outputs instruction pointer, written value,
 * timestamp and SHA1 hash of memory regions to file on each write access from SUT
 */
class Checkpoint : public fail::ExperimentFlow
{
public:
	typedef std::pair<fail::address_t,bool> indirectable_address_t; //!< fixed address or pointer to address
	typedef std::pair<indirectable_address_t,indirectable_address_t> address_range; //!< contiguous memory region
	typedef std::vector<address_range> range_vector; //!< vector of memory regions

	enum check_result {
		IDENTICAL,
		DIFFERENT_IP,
		DIFFERENT_VALUE,
		DIFFERENT_DIGEST,
		INVALID
	};

private:
	const fail::ElfSymbol m_symbol; //!< target memory symbol triggering checkpoints
	const range_vector m_check_ranges; //!< address ranges to checksum
	const bool m_checking; //!< checking mode (when false: tracing mode)
	std::string m_file; //!< the input/output filename
	fail::Logger m_log; //!< debug output
	std::ofstream m_ostream; //!< outputfile stream
	std::ifstream m_istream; //!< inputfile stream
	unsigned m_count;

	// In this map, we save the contents of stackpointer-variables to
	// beginning/end of region markers. We allow this to make those
	// stackpointer-variables injectable.
	std::map<fail::address_t, fail::address_t> cached_stackpointer_variables;

public:
	/**
	 * Construct Checkpoint Logger in tracing (output) mode.
	 *
	 * @param symbol The global memory address the plugin listens to
	 * @param check_ranges Address ranges which are included in the saved checksum
	 * @param outputfile The path to the file to write the checkpoints
	 */
	Checkpoint(const fail::ElfSymbol & symbol,
	           const std::vector<address_range> check_ranges,
	           const std::string& outputfile) :
			m_symbol(symbol), m_check_ranges(check_ranges), m_checking(false),
			m_file(outputfile) , m_log("CPLogger", false), m_count(0)
	{
		m_ostream.open(m_file.c_str() );
		if (!m_ostream.is_open()) {
			m_log << "Could not open " << m_file.c_str() << " for writing." << std::endl;
		}
	}

	/**
	 * Construct Checkpoint Logger in checking (input) mode.
	 *
	 * @param symbol The global memory address the plugin listens to
	 * @param check_ranges Address ranges which are compared to the saved checksum
	 * @param inputfile The path to the file to read in checkpoints
	 */
	Checkpoint(const std::vector<address_range> check_ranges,
	           const std::string& inputfile) :
			m_check_ranges(check_ranges), m_checking(true), m_file(inputfile),
			m_log("CPLogger", false), m_count(0)
	{
		m_istream.open(m_file.c_str() );
		if (!m_istream.is_open()) {
			m_log << "Could not open " << m_file.c_str() << " for reading." << std::endl;
		}
	}

	//! How many checkpoints have been triggered so far
	unsigned getCount() const {
		return m_count;
	}

	//! Start plugin control flow for tracing mode. Do not call in checking mode!
	bool run();

	/**
	 * Perform a check against saved checkpoints. Call this method in experiment when
	 * a MemWriteBreakpoint on symbol triggers.
	 *
	 * @param symbol (memory) symbol which triggered checkpoint
	 * @param ip instruction pointer which triggered checkpoint
	 * @return check result
	 */
	check_result check(const fail::ElfSymbol symbol, fail::address_t ip);

	/**
	 * Checks whether the given address is currently (according to the
	 * machine state) within the checked ranges.
	 *
	 * @param addr address to check
	 * @return addr is in checked ranges
	 */
	bool in_range(fail::address_t addr);

	/**
	 * Cache a memory address, which is to be dereferenced by the checkpoint plugin.
	 *
	 * @param address of the stackpointer variable
	 * @param value of the stackpointer variable
	 */
	void cache_stackpointer_variable(fail::address_t addr, fail::address_t value) {
		cached_stackpointer_variables[addr] = value;
	}

	void uncache_stackpointer_variable(fail::address_t addr) {
		cached_stackpointer_variables.erase(addr);
	}

private:
	//! calulate checksum over memory regions
	void checksum(uint8_t (&Message_Digest)[20]);

	//! extract checkpoint information from simulation
	void checkpoint(const fail::ElfSymbol symbol,
	                uint32_t &value,
	                fail::simtime_t &simtime,
	                std::string &digest_str);

	//! save checkpoint to file
	void save_checkpoint(fail::address_t ip);

	//! get value of indirectable_address_t
	fail::address_t resolve_address(const indirectable_address_t &addr);
};

#endif // __Checkpoint_HPP__
