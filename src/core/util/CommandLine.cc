#include "CommandLine.hpp"
#include <iostream>
#include <cstring>
#include <string>


namespace fail {
CommandLine CommandLine::m_instance;

void CommandLine::collect_args(argument_count &argc, argument_value &argv) {
	// Filter out all command line arguments that start with -Wf,
	for (int i = 0; i < argc; ++i) {
		if (strncmp(argv[i], "-Wf,", 4) == 0) {
			this->argv.push_back(argv[i] + 4);

			// also copy argv[argc], which equals 0
			for (int x = i + 1; x <= argc; ++x) {
				argv[x - 1] = argv[x];
			}
			i --;
			argc --;
		}

	}
}

CommandLine::option_handle CommandLine::addOption(
	const std::string &shortopt,
	const std::string &longopt,
	const option::CheckArg & check_arg,
	const std::string &help) {

	const unsigned int handle = this->options.size();
	option::Descriptor desc = {handle, 0, strdup(shortopt.c_str()),
		strdup(longopt.c_str()),
		check_arg, strdup(help.c_str())};
	this->options.push_back(desc);
	return handle;
}

bool CommandLine::parse() {
	// Terminate the descriptor list
	option::Descriptor desc = {0, 0, 0, 0, 0, 0};
	this->options.push_back(desc);

	// Copy argv to preserve original argument order
	// (for proper re-parsing after adding more options)
	argv_reordered = argv;

	// Generate the options stats (GNU mode)
	option::Stats stats(true, this->options.data(),
		argv_reordered.size(), argv_reordered.data());

	if (parsed_options)
		delete[] parsed_options;
	if (parsed_buffer)
		delete[] parsed_buffer;
	if (m_parser)
		delete m_parser;

	parsed_options = new option::Option[stats.options_max];
	parsed_buffer  = new option::Option[stats.buffer_max];

	m_parser = new option::Parser(true, this->options.data(),
		argv_reordered.size(), argv_reordered.data(),
		parsed_options, parsed_buffer);

	// Pop the terminating entry
	this->options.pop_back();

	return !m_parser->error();
}

} // end of namespace
