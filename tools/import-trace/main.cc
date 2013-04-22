#include "util/optionparser/optionparser.h"
#include "util/optionparser/optionparser_ext.hpp"
#include "util/CommandLine.hpp"
#include "util/Database.hpp"
#include "util/ElfReader.hpp"
#include "util/gzstream/gzstream.h"
#include "util/Logger.hpp"
#include <fstream>
#include <string>

#include "BasicImporter.hpp"
#include "DCiAOKernelImporter.hpp"


using namespace fail;
using std::cerr;
using std::endl;
using std::cout;

static Logger LOG("import-trace", true);

ProtoIStream openProtoStream(std::string input_file) {
	std::ifstream *gz_test_ptr = new std::ifstream(input_file.c_str()), &gz_test = *gz_test_ptr;
	if (!gz_test) {
		LOG << "couldn't open " << input_file << endl;
		exit(-1);
	}
	unsigned char b1, b2;
	gz_test >> b1 >> b2;

	if (b1 == 0x1f && b2 == 0x8b) {
		igzstream *tracef = new igzstream(input_file.c_str());
		if (!tracef) {
			LOG << "couldn't open " << input_file << endl;
			exit(-1);
		}
		LOG << "opened file " << input_file << " in GZip mode" << endl;
		delete gz_test_ptr;
		ProtoIStream ps(tracef);
		return ps;
	}

	LOG << "opened file " << input_file << " in normal mode" << endl;
	ProtoIStream ps(gz_test_ptr);
	return ps;
}

int main(int argc, char *argv[]) {
	std::string trace_file, username, hostname, database, benchmark;
	std::string variant, importer_args;
	ElfReader *elf_file = 0;

	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();
	for (int i = 1; i < argc; ++i)
		cmd.add_args(argv[i]);

	cmd.addOption("", "", Arg::None, "USAGE: import-trace [options]");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h/--help\t Print usage and exit");
	CommandLine::option_handle TRACE_FILE	= cmd.addOption("t", "trace-file", Arg::Required,
															"-t/--trace-file\t File to load the execution trace from\n");

	// setup the database command line options
	Database::cmdline_setup();

	CommandLine::option_handle VARIANT	 = cmd.addOption("v", "variant", Arg::Required,
															"-v/--variant\t Variant label (default: \"none\")");
	CommandLine::option_handle BENCHMARK = cmd.addOption("b", "benchmark", Arg::Required,
														 "-b/--benchmark\t Benchmark label (default: \"none\")\n");
	CommandLine::option_handle IMPORTER	  = cmd.addOption("i", "importer", Arg::Required,
															"-i/--importer\t Which import method to use (default: BasicImporter)");
	CommandLine::option_handle IMPORTER_ARGS	  = cmd.addOption("I", "importer-args", Arg::Required,
															"-I/--importer-args\t Which import method to use (default: "")");
	CommandLine::option_handle ELF_FILE  = cmd.addOption("e", "elf-file", Arg::Required,
														 "-e/--elf-file\t ELF File (default: UNSET)");


	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	Importer *importer;

	if (cmd[IMPORTER].count() > 0) {
		std::string imp(cmd[IMPORTER].first()->arg);
		if (imp == "BasicImporter") {
			LOG << "Using BasicImporter" << endl;
			importer = new BasicImporter();
		} else if (imp == "DCiAOKernelImporter") {
			LOG << "Using DCiAOKernelImporter" << endl;
			importer = new DCiAOKernelImporter();
		} else {
			LOG << "Unkown import method: " << imp << endl;
			exit(-1);
		}

	} else {
		LOG << "Using BasicImporter" << endl;
		importer = new BasicImporter();
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	if (cmd[TRACE_FILE].count() > 0)
		trace_file = std::string(cmd[TRACE_FILE].first()->arg);
	else
		trace_file = "trace.pb";

	ProtoIStream ps = openProtoStream(trace_file);
	Database *db = Database::cmdline_connect();

	if (cmd[VARIANT].count() > 0)
		variant = std::string(cmd[VARIANT].first()->arg);
	else
		variant = "none";

	if (cmd[BENCHMARK].count() > 0)
		benchmark = std::string(cmd[BENCHMARK].first()->arg);
	else
		benchmark = "none";

	if (cmd[IMPORTER_ARGS].count() > 0)
		importer_args = std::string(cmd[IMPORTER_ARGS].first()->arg);

	if (cmd[ELF_FILE].count() > 0) {
		elf_file = new ElfReader(cmd[ELF_FILE].first()->arg);
	}



	if (!importer->init(variant, benchmark, db)) {
		LOG << "importer->init() failed" << endl;
		exit(-1);
	}
	importer->set_elf_file(elf_file);

	////////////////////////////////////////////////////////////////
	// Do the actual import
	////////////////////////////////////////////////////////////////

	if (!importer->create_database()) {
		LOG << "create_database() failed" << endl;
		exit(-1);
	}

	if (!importer->clear_database()) {
		LOG << "clear_database() failed" << endl;
		exit(-1);
	}

	if (!importer->copy_to_database(ps)) {
		LOG << "copy_to_database() failed" << endl;
		exit(-1);
	}
}
