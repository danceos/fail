#include <mysql/mysql.h>
#include <iostream>
#include <string>

#include "util/CommandLine.hpp"
#include "util/Logger.hpp"
static fail::Logger LOG("prune-trace", true);

using namespace fail;
using std::endl;

#include "Pruner.hpp"
#include "BasicPruner.hpp"

int main(int argc, char *argv[]) {
	std::string username, hostname, database, benchmark, variant;

	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();
	for (int i = 1; i < argc; ++i)
		cmd.add_args(argv[i]);

	CommandLine::option_handle IGNORE = cmd.addOption("", "", Arg::None, "USAGE: import-trace [options]");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help\t Print usage and exit");

	Database::cmdline_setup();

	CommandLine::option_handle VARIANT	 = cmd.addOption("v", "variant", Arg::Required,
	                                                     "-v/--variant\t Variant label (default: \"none\")");
	CommandLine::option_handle BENCHMARK = cmd.addOption("b", "benchmark", Arg::Required,
														 "-b/--benchmark\t Benchmark label (default: \"none\")\n");
	CommandLine::option_handle PRUNER	 = cmd.addOption("p", "prune-method", Arg::Required,
	                                                     "-p/--prune-method\t Which import method to use (default: basic)");

	if(!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	Pruner *pruner;
	if (cmd[PRUNER].count() > 0) {
		std::string imp(cmd[PRUNER].first()->arg);
		if (imp == "basic") {
			LOG << "Using BasicPruner" << endl;
			pruner = new BasicPruner();
		} else if (imp == "basic-left") {
			LOG << "Using BasicPruner (use left border, instr1)" << endl;
			pruner = new BasicPruner(true);

		} else {
			LOG << "Unkown import method: " << imp << endl;
			exit(-1);
		}

	} else {
		LOG << "Using BasicPruner" << endl;
		pruner = new BasicPruner();
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	Database *db = Database::cmdline_connect();

	if (cmd[VARIANT].count() > 0)
		variant = std::string(cmd[VARIANT].first()->arg);
	else
		variant = "none";

	if (cmd[BENCHMARK].count() > 0)
		benchmark = std::string(cmd[BENCHMARK].first()->arg);
	else
		benchmark = "none";

	if (!pruner->init(variant, benchmark, db)) {
		LOG << "pruner->init() failed" << endl;
		exit(-1);
	}

	////////////////////////////////////////////////////////////////
	// Do the actual import
	////////////////////////////////////////////////////////////////
	if (!pruner->create_database()) {
		LOG << "create_database() failed" << endl;
		exit(-1);
	}

	if (!pruner->clear_database()) {
		LOG << "clear_database() failed" << endl;
		exit(-1);
	}

	if (!pruner->prune_all()) {
		LOG << "prune_all() failed" << endl;
		exit(-1);
	}

	return 0;
}
