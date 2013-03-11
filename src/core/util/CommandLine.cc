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
                this->argv.push_back(std::string(argv[i] + 4));

                for (int x = i + 1; x < argc; ++x) {
                    argv[x - 1] = argv[x];
                }
                i --;
                argc --;
            }

        }
    }

    CommandLine::option_handle CommandLine::addOption(const std::string &shortopt,
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

        // Build an argv array
        std::vector<const char *> tmp_argv;
        int argc = this->argv.size();
        for (unsigned i = 0; i < this->argv.size(); ++i)
            tmp_argv.push_back(this->argv[i].c_str());

        // Generate the options stats
        option::Stats stats(this->options.data(), argc, tmp_argv.data());

        if (parsed_options)
            delete[] parsed_options;
        if (parsed_buffer)
            delete[] parsed_buffer;

        parsed_options = new option::Option[stats.options_max];
        parsed_buffer  = new option::Option[stats.buffer_max];

        option::Parser parse(this->options.data(), argc, tmp_argv.data(),
                             parsed_options, parsed_buffer);


        // Pop the terminating entry
        this->options.pop_back();

        return !parse.error();
    }


} // end of namespace
