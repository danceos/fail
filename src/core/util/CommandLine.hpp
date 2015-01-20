#ifndef __COMMAND_LINE_HPP__
#define __COMMAND_LINE_HPP__

#include <vector>
#include <string>
#include <assert.h>

// Ignore warnings regarding const removal, we don't use this part of
// optionparser's interface anyways.  (Works with GCC 4.6 and above, throws a
// warning with earlier versions.  The reset to "warning" is necessary for
// those earlier versions.)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include "optionparser/optionparser.h"
#include "optionparser/optionparser_ext.hpp"
#pragma GCC diagnostic warning "-Wcast-qual"
#pragma GCC diagnostic pop

namespace fail {

/**
 * @class CommandLine
 * @brief Implements a command line interface, that filters the
 * simulators command line. It is a Singleton.
 */
class CommandLine {
public:
	typedef int argument_count;
	typedef char **argument_value;
private:
	static CommandLine m_instance;

	std::vector<const char *> argv, argv_reordered;
	std::vector<option::Descriptor> options;
	option::Option *parsed_options, *parsed_buffer;
	option::Parser *m_parser;
public:
	/// Handle for accessing the parsed data of an option
	typedef int option_handle;

	/**
	 * Singleton accessor
	 *
	 * @return reference to the CommandLine singleton object
	 */
	static CommandLine &Inst() { return m_instance; }

	/**
	 * Called by the simulator to filter all fail arguments from
	 * argc, argv
	 */
	void collect_args(argument_count &, argument_value &);

	/**
	 * Add a argument manually
	 */
	void add_args(const char *value) { argv.push_back(value); }

	/**
	 * Add a option to the command line interface of the fail-client
	 *
	 * @param shortopt e.g "m" for -m
	 * @param longopt e.g. "memory-region" for --memory-region=
	 * @param check_arg argument is required.
	 * @param help help text to be printed for -h
	 *
	 * @return return handle to option
	 */
	option_handle addOption(const std::string &shortopt,
		const std::string &longopt,
		const option::CheckArg & check_arg,
		const std::string &help);

	/**
	 *
	 * do the actual parsing, called by the experiment
	 *
	 * @return true on success
	 */
	bool parse();

	/**
	 * Accessor for the command line option objects
	 *
	 * @param handle option handle
	 *
	 * @return reference to the option parser object
	 */
	option::Option &operator[](option_handle handle) {
		assert(parsed_options != 0);
		assert(handle >= 0 && handle < (int)options.size());
		return parsed_options[handle];
	}
	/**
	 * Print help message.
	 */
	void printUsage() {
		int columns = getenv("COLUMNS")? atoi(getenv("COLUMNS")) : 80;
		option::printUsage(fwrite, stdout, options.data(), columns);
	}
	/**
	 * Return the internal option::Parser object for further usage.
	 */
	option::Parser *parser() {
		return m_parser;
	}
};

} // end of namespace

#endif
