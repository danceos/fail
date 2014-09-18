#include <fstream>
#include <string>

#include "FormatConverter.hpp"
#include "Gem5Converter.hpp"
#include "DumpConverter.hpp"

#include "util/CommandLine.hpp"
#include "util/gzstream/gzstream.h"
#include "util/Logger.hpp"

using namespace fail;
using std::cin;
using std::endl;

static Logger LOG("convert-trace", true);

int main(int argc, char *argv[]) {
	CommandLine &cmd = CommandLine::Inst();
	CommandLine::option_handle UNKNOWN =
		cmd.addOption("", "", Arg::None, "usage: convert-trace -f dump|gem5 -t tracefile.tc");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
	CommandLine::option_handle FORMAT =
		cmd.addOption("f", "format", Arg::Required, "-f/--format FORMAT \tInput format (dump|gem5)");
	CommandLine::option_handle OUTFILE =
		cmd.addOption("t", "trace", Arg::Required, "-t/--trace FILE \tOutput file");
	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << endl;
		return 1;
	}

	if (cmd[HELP] || !cmd[OUTFILE] || !cmd[FORMAT] || cmd[UNKNOWN] || cmd.parser()->nonOptionsCount() != 0) {
		for (option::Option* opt = cmd[UNKNOWN]; opt; opt = opt->next()) {
			std::cerr << "Unknown option: " << opt->name << "\n";
		}
		for (int i = 0; i < cmd.parser()->nonOptionsCount(); ++i) {
			std::cerr << "Unknown non-option: " << cmd.parser()->nonOption(i) << "\n";
		}
		cmd.printUsage();
		if (cmd[HELP]) {
			exit(0);
		} else {
			exit(1);
		}
	}

	std::string format = cmd[FORMAT].first()->arg;
	std::string trace_file = cmd[OUTFILE].first()->arg;

	ogzstream gz_stream(trace_file.c_str());
	std::ostream *os = &gz_stream;
	ProtoOStream ps(os);

	FormatConverter *converter;
	if (format == "gem5") {
		converter = new Gem5Converter(cin, ps);
	} else if (format == "dump") {
		converter = new DumpConverter(cin, ps);
	} else {
		LOG << "unknown input format '" << format << "'" << endl;
		return 1;
	}

	if (!converter->convert()) {
		LOG << "converter failed" << endl;
		return 1;
	}
}
