#include <mysql/mysql.h>
#include <iostream>
#include <string>

#include "util/CommandLine.hpp"
#include "util/Logger.hpp"
#include "util/AliasedRegistry.hpp"

static fail::Logger LOG("prune-trace", true);

using namespace fail;
using std::endl;

#include "Pruner.hpp"
#include "BasicPruner.hpp"
#include "FESamplingPruner.hpp"

int main(int argc, char *argv[]) {
	std::string username, hostname, database;

	// register possible Pruners
	AliasedRegistry registry;
	BasicPruner basicpruner;
	registry.add(&basicpruner);
	BasicPrunerLeft basicprunerleft;
	registry.add(&basicprunerleft);
	FESamplingPruner fesamplingpruner;
	registry.add(&fesamplingpruner);

	std::string pruners = registry.getPrimeAliasesCSV();

	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();
	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}

	CommandLine::option_handle UNKNOWN =
		cmd.addOption("", "", Arg::None, "USAGE: prune-trace [options]");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	Database::cmdline_setup();

	CommandLine::option_handle VARIANT =
		cmd.addOption("v", "variant", Arg::Required,
			"-v/--variant \tVariant label (default: \"%\"; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle VARIANT_EXCLUDE =
		cmd.addOption("", "variant-exclude", Arg::Required,
			"--variant-exclude \tVariant to exclude (default: UNSET; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle BENCHMARK =
		cmd.addOption("b", "benchmark", Arg::Required,
			"-b/--benchmark \tBenchmark label (default: \"%\"; use % and _ as wildcard characters; may be used more than once)");
	CommandLine::option_handle BENCHMARK_EXCLUDE =
		cmd.addOption("", "benchmark-exclude", Arg::Required,
			"--benchmark-exclude \tBenchmark to exclude (default: UNSET; use % and _ as wildcard characters; may be used more than once)");
	std::string pruner_help = "-p/--prune-method \tWhich pruning method to use (default: basic); available pruning methods: " + pruners;
	CommandLine::option_handle PRUNER =
		cmd.addOption("p", "prune-method", Arg::Required, pruner_help);
	CommandLine::option_handle NO_DELETE =
		cmd.addOption("", "no-delete", Arg::None,
			"--no-delete \tAssume there are no DB entries for this variant/benchmark, don't issue a DELETE");
	CommandLine::option_handle OVERWRITE =
		cmd.addOption("", "overwrite", Arg::None,
			"--overwrite \tOverwrite already existing pruning data (the default is to skip variants with existing entries)");

	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	Pruner *pruner;
	std::string pruner_name = "BasicPruner";
	if (cmd[PRUNER]) {
		pruner_name = cmd[PRUNER].first()->arg;
	}

	// try and get the according pruner object; die on failure
	if ((pruner = (Pruner *)registry.get(pruner_name)) == 0) {
		if (pruner_name != "?" ) {
			std::cerr << "Unknown pruning method: " << pruner_name << std::endl;
		}
		std::cerr << "Available pruning methods: " << pruners << std::endl;
		exit(-1);
	}
	registry.getPrimeAlias(pruner, pruner_name);
	LOG << "Using " << pruner_name << endl;

	if (!(pruner->commandline_init())) {
		std::cerr << "Pruner's commandline initialization failed" << std::endl;
		exit(-1);
	}
	// Since the pruner might have added command line options, we need to
	// reparse all arguments.
	cmd.parse();

	if (cmd[HELP] || cmd[UNKNOWN] || cmd.parser()->nonOptionsCount() > 0) {
		for (option::Option* opt = cmd[UNKNOWN]; opt; opt = opt->next()) {
			std::cerr << "Unknown option: " << opt->name << "\n";
		}
		for (int i = 0; i < cmd.parser()->nonOptionsCount(); ++i) {
			std::cerr << "Unknown non-option: " << cmd.parser()->nonOption(i) << "\n";
		}
		cmd.printUsage();
		exit(!cmd[HELP]);
	}

	Database *db = Database::cmdline_connect();
	pruner->set_db(db);

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
	if (variants.size() == 0) {
		variants.push_back("%");
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
	if (benchmarks.size() == 0) {
		benchmarks.push_back("%");
	}

	if (!pruner->create_database()) {
		LOG << "pruner->create_database() failed" << endl;
		exit(-1);
	}

	if (!pruner->init(variants, variants_exclude, benchmarks, benchmarks_exclude, cmd[OVERWRITE])) {
		LOG << "pruner->init() failed" << endl;
		exit(-1);
	}

	////////////////////////////////////////////////////////////////
	// Do the actual pruning
	////////////////////////////////////////////////////////////////
	if (!cmd[NO_DELETE] && cmd[OVERWRITE] && !pruner->clear_database()) {
		LOG << "clear_database() failed" << endl;
		exit(-1);
	}

	if (!pruner->prune_all()) {
		LOG << "prune_all() failed" << endl;
		exit(-1);
	}

	return 0;
}
