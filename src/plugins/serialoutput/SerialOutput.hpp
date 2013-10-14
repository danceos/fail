#ifndef __SERIAL_OUTPUT_HPP__
#define __SERIAL_OUTPUT_HPP__

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "config/FailConfig.hpp"

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_IOPORT)
	#warning The serialoutput plugin may (depending on its use) need ioport event. Enable these in the cmake configuration tool.
#endif

/**
 * \class SerialOutput
 *
 * \brief Plugin to record ioport traffic.
 */
class SerialOutput : public fail::ExperimentFlow
{

private:
	bool m_out; //!< Defines the direction of the listener.
	unsigned m_port; //!< the port the listener is listening on
	std::string m_output; //!< contains the traffic of ioport

public:
	/**
	 * Constructor of SerialOutput.
	 *
	 * @param port the port the listener is listening on
	 * @param out Defines the direction of the listener.
	 * \arg \c true Output on the given port is captured. This is default.
	 * \arg \c false Input on the given port is captured.
	 */
	SerialOutput(unsigned port, bool out = true) : m_out(out), m_port(port) { }
	bool run();
	/**
	 * Resets the output variable which contains the traffic of
	 * ioport.
	 */
	void resetOutput();
	/**
	 * Returns the output variable.
	 */
	std::string getOutput();
};

#endif // __SERIAL_OUTPUT_HPP__
