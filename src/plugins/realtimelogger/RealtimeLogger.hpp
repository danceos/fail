#ifndef __REALTIMELOGGER_HPP__
#define __REALTIMELOGGER_HPP__

#include <string>
#include <unistd.h>
#include "efw/ExperimentFlow.hpp"
#include "config/FailConfig.hpp"
#include "util/Logger.hpp"
#include <fstream>
#include "util/ElfReader.hpp"

/**
 * @class RealtimeLogger
 * @brief Listens to a memory location and outputs content on write access
 * from SUT to file
 */
class RealtimeLogger : public fail::ExperimentFlow
{
private:
	const fail::ElfSymbol m_symbol; //!< the target's memory symbol the plugin is listening on
	std::string m_outputfile; //!< the output filename
	fail::Logger m_log; //!< debug output
	std::ofstream m_ostream; //!< Outputfile stream

public:
	/**
	 * Constructor of RealtimeLogger.
	 *
	 * @param symbol The global memory address the plugin listens to
	 * @param outputfile The path to the file to write the output to
	 */
	RealtimeLogger( const fail::ElfSymbol & symbol, const std::string& outputfile ) :  m_symbol(symbol), m_outputfile(outputfile) , m_log("RTLogger", false) {
		m_ostream.open(m_outputfile.c_str() );
		if (!m_ostream.is_open()) {
			m_log << "Could not open " << m_outputfile.c_str() << " for writing." << std::endl;
		}
	}

	bool run();

private:
	/**
	 * Handle the memory event
	 */
	void handleEvent(fail::simtime_t simtime, uint32_t value);
};

#endif // __REALTIMELOGGER_HPP__
