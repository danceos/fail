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
	std::string username, hostname, database;

	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();
	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}

	cmd.addOption("", "", Arg::None, "USAGE: import-trace [options]");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	Database::cmdline_setup();

	CommandLine::option_handle VARIANT =
		cmd.addOption("v", "variant", Arg::Required,
			"-v/--variant \tVariant label (default: \"none\"; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle VARIANT_EXCLUDE =
		cmd.addOption("", "variant-exclude", Arg::Required,
			"--variant-exclude \tVariant to exclude (default: UNSET; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle BENCHMARK =
		cmd.addOption("b", "benchmark", Arg::Required,
			"-b/--benchmark \tBenchmark label (default: \"none\"; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle BENCHMARK_EXCLUDE =
		cmd.addOption("", "benchmark-exclude", Arg::Required,
			"--benchmark-exclude \tBenchmark to exclude (default: UNSET; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle PRUNER =
		cmd.addOption("p", "prune-method", Arg::Required,
			"-p/--prune-method \tWhich import method to use (default: basic)");
	CommandLine::option_handle NO_DELETE =
		cmd.addOption("", "no-delete", Arg::None,
			"--no-delete \tAssume there are no DB entries for this variant/benchmark, don't issue a DELETE");

	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	Pruner *pruner;
	if (cmd[PRUNER]) {
		std::string imp(cmd[PRUNER].first()->arg);
		if (imp == "BasicPruner" || imp == "basic") {
			LOG << "Using BasicPruner" << endl;
			pruner = new BasicPruner();
		} else if (imp == "BasicPrunerLeft" || imp == "basic-left") {
			LOG << "Using BasicPruner (use left border, instr1)" << endl;
			pruner = new BasicPruner(true);

		} else {
			LOG << "Unknown pruning method: " << imp << endl;
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

	std::vector<std::string> variants, benchmarks, variants_exclude, benchmarks_exclude;
	if (cmd[VARIANT]) {
		for (option::Option *o = cmd[VARIANT]; o; o = o->next()) {
			variants.push_back(std::string(o->arg));
		}
	}

	if (cmd[VARIANT_EXCLUDE]) {
		for (option::Option *o = cmd[VARIANT_EXCLUDE]; o; o = o->next()) {
			variants_exclude.push_back(std::string(o->arg));
		}
	}

	// fallback
	if (variants.size() == 0 && variants_exclude.size() == 0) {
		variants.push_back(std::string("none"));
	}

	if (cmd[BENCHMARK]) {
		for (option::Option *o = cmd[BENCHMARK]; o; o = o->next()) {
			benchmarks.push_back(std::string(o->arg));
		}
	}

	if (cmd[BENCHMARK_EXCLUDE]) {
		for (option::Option *o = cmd[BENCHMARK_EXCLUDE]; o; o = o->next()) {
			benchmarks_exclude.push_back(std::string(o->arg));
		}
	}

	// fallback
	if (benchmarks.size() == 0 && benchmarks_exclude.size() == 0) {
		benchmarks.push_back(std::string("none"));
	}

	if (!pruner->init(db, variants, variants_exclude, benchmarks, benchmarks_exclude)) {
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

	if (!cmd[NO_DELETE] && !pruner->clear_database()) {
		LOG << "clear_database() failed" << endl;
		exit(-1);
	}

	if (!pruner->prune_all()) {
		LOG << "prune_all() failed" << endl;
		exit(-1);
	}

	return 0;
}
