// #include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>

#include "../../src/core/util/smarthops/TraceReader.hpp"
#include "BasicAlgorithm.hpp"
#include "SmartAlgorithm.hpp"
#include "SimpleAlgorithm.hpp"
#include "ResultCollector.hpp"

#include "../../src/core/util/gzstream/gzstream.h"

#include "util/CommandLine.hpp"
#include "util/Logger.hpp"

#define STDOUT_CMD_STRING "-"

using namespace fail;

typedef enum {
	ALGO_SIMPLE,
	ALGO_SMART,
} algorithm_e;

bool g_use_weights;
bool g_use_watchpoints;
bool g_use_checkpoints;

unsigned int g_cp_thresh;
unsigned int g_cost_cp;
unsigned int g_rollback_thresh;

std::ofstream g_cp_ofstream;

Logger LOG("hop-calculator", false);

int main(int argc, char *argv[])
{
	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();

	CommandLine::option_handle UNKNOWN = cmd.addOption("", "", Arg::None, "USAGE: compute-hops [options]");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	CommandLine::option_handle ALGORITHM =
		cmd.addOption("a", "algorithm", Arg::Required,
			"-a,--algorithm \tHop algorithm (\"simple\"/\"smart\", default: \"smart\")");

	CommandLine::option_handle OUTPUT_MODE =
		cmd.addOption("m", "output-mode", Arg::Required,
			"-m,--output-mode \tOutput mode (\"results\"/\"costs\"/\"statistics\", default: \"results\")");

	CommandLine::option_handle TRACE_FILE =
		cmd.addOption("i", "input-file", Arg::Required,
			"-i,--input-file \tInput trace file path");

	CommandLine::option_handle OUTPUT_FILE =
		cmd.addOption("o", "output-file", Arg::Required,
			"-o,--output-file \tOutput file, default: stdout");

	CommandLine::option_handle OUTPUT_FILE_PROTOBUF =
		cmd.addOption("b", "protobuf-output", Arg::None,
			"-b,--protobuf-output \tOutput file will be created in protobuf (HopChain) format");

	CommandLine::option_handle USE_COSTS =
		cmd.addOption("c", "use-costs", Arg::None,
			"-c,--use-costs \tUse hop costs for calculations of Smart-Hopping algorithm");

	CommandLine::option_handle USE_WATCHPOINTS =
		cmd.addOption("w", "use-watchpoints", Arg::None,
			"-w,--use-watchpoints \tUse watchpoints as additional hop candidates");

	CommandLine::option_handle USE_CHECKPOINTS =
		cmd.addOption("", "use-checkpoints", Arg::None,
			"--use-checkpoints \tUse checkpoints to cap costs in case of long hop chains");

	CommandLine::option_handle CP_THRESH =
		cmd.addOption("", "cp-costs-threshold", Arg::Required,
			"--cp-costs-threshold \tIf costs of a hop-chain reach this threshold, a checkpoint will be created");

	CommandLine::option_handle CP_COSTS =
		cmd.addOption("", "cp-costs", Arg::Required,
			"--cp-costs \tThe costs for reloading a checkpoint into the system");

	CommandLine::option_handle CP_ROLLBACK_THRESH =
		cmd.addOption("", "cp-rollback-threshold", Arg::Required,
			"--cp-rollback-threshold \tMinial number of hops to roll back current solution beyond checkpoint. Must be smaller than ((cp_cost_thresh - cp_costs) / 2)");

	CommandLine::option_handle CHECKPOINT_OUTPUT_FILE =
		cmd.addOption("", "cp-output", Arg::Required,
			"--cp-output \tCheckpoint output file");

	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return 1;
	}

	if (cmd[HELP] || cmd[UNKNOWN] || cmd.parser()->nonOptionsCount() > 0) {
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

	if (cmd[USE_CHECKPOINTS]) {
		g_use_checkpoints = true;
	} else {
		g_use_checkpoints = false;
	}

	if (cmd[USE_WATCHPOINTS]) {
		g_use_watchpoints = true;
	} else {
		g_use_watchpoints = false;
	}

	if (cmd[USE_COSTS]) {
		g_use_weights = true;
	} else {
		g_use_weights = false;
	}

	algorithm_e algorithm;

	if (cmd[ALGORITHM].count() > 0) {
		std::string alg(cmd[ALGORITHM].first()->arg);
		if (alg == "simple") {
			algorithm = ALGO_SIMPLE;
		} else if (alg == "smart") {
			algorithm = ALGO_SMART;
		} else {
			exit(-1);
		}
	} else {
		// default
		algorithm = ALGO_SMART;
	}

	output_mode_e output_mode;

	if (cmd[OUTPUT_MODE].count() > 0) {
		std::string outp(cmd[OUTPUT_MODE].first()->arg);
		if (outp == "results") {
			output_mode = OUTPUT_RESULT;
		} else if (outp == "costs") {
			output_mode = OUTPUT_COSTS;
		} else if (outp == "statistics") {
			output_mode = OUTPUT_STATISTICS;
		} else {
			LOG << "Unkown output mode: " << outp << std::endl;
			exit(-1);
		}
	} else {
		// default
		output_mode = OUTPUT_RESULT;
	}

	fail::TraceReader trace;

	if (cmd[TRACE_FILE].count() > 0) {
		const char *filename = cmd[TRACE_FILE].first()->arg;
		if (!trace.openTraceFile(filename, 0)) {
			LOG << "Unable to open and parse input trace file " << filename << std::endl;
			return 1;
		}
	} else {
		LOG << "Input trace file path not defined" << std::endl;
		exit(-1);
	}

	std::ostream *outFile;

	if (cmd[OUTPUT_FILE].count() > 0) {
		std::string filename(cmd[OUTPUT_FILE].first()->arg);

		if (filename.compare(STDOUT_CMD_STRING) == 0) {
			outFile = &std::cout;
		} else {
			std::ofstream *of = new std::ofstream(filename.c_str());
			if (!of->is_open()) {
				LOG << "Unable to open output file " << filename << std::endl;
				exit(-1);
			}
			outFile = of;
		}
	} else {
		outFile = &std::cout;
	}

	if (cmd[OUTPUT_FILE_PROTOBUF] && outFile == &std::cout) {
		LOG << "If protobuf output format is selected, output file must be defined" << std::endl;
		exit(-1);
	}

	if (cmd[USE_COSTS] && !(algorithm == ALGO_SMART)) {
		LOG << "Using costs for calculations is only possible with Smart-Hopping algorithm" << std::endl;
		exit(-1);
	}

	if (cmd[CP_THRESH]) {
		if (cmd[CP_THRESH].count() != 1) {
			LOG << "Could not parse cp-costs-threshold" << std::endl;
			exit(-1);
		}
		g_cp_thresh = strtoul(cmd[CP_THRESH].first()->arg, NULL, 0);
	}

	if (cmd[CP_COSTS]) {
		if (cmd[CP_COSTS].count() != 1) {
			LOG << "Could not parse cp-costs" << std::endl;
			exit(-1);
		}
		g_cost_cp = strtoul(cmd[CP_COSTS].first()->arg, NULL, 0);
	}

	if (cmd[CP_ROLLBACK_THRESH]) {
		if (cmd[CP_ROLLBACK_THRESH].count()!=1) {
			LOG << "Could not parse cp-rollback-threshold" << std::endl;
			exit(-1);
		}
//		if (strcmp(argv[9],"max")==0) {
//			g_rollback_thresh = (g_cp_thresh - g_cost_cp - 5)/2;
//		}
		g_rollback_thresh = strtoul(cmd[CP_ROLLBACK_THRESH].first()->arg, NULL, 0);
	}

	if (cmd[USE_CHECKPOINTS] && !(cmd[CHECKPOINT_OUTPUT_FILE].count() > 0)) {
		LOG << "If checkpointing is enabled, --cp-output must be defined" << std::endl;
		exit(-1);
	}

	if (cmd[CHECKPOINT_OUTPUT_FILE].count() > 0) {
		std::string filename(cmd[CHECKPOINT_OUTPUT_FILE].first()->arg);
		g_cp_ofstream.open(filename.c_str());
		if (!g_cp_ofstream.is_open()) {
			LOG << "Unable to open cp_out_file " << filename << std::endl;
			exit(-1);
		}
	}

	if (cmd[USE_CHECKPOINTS] && (!cmd[CP_THRESH] ||
			!cmd[CP_COSTS]|| !cmd[CP_ROLLBACK_THRESH])) {
		LOG << "If using checkpointing is enabled, also cp-costs-threshold, "
				"cp-costs and cp-rollback-threshold must be defined" << std::endl;
		exit(-1);
	}

	if (!cmd[USE_CHECKPOINTS] && (cmd[CP_THRESH].count() ||
			(cmd[CP_COSTS].count() > 0) || (cmd[CP_ROLLBACK_THRESH].count() > 0))) {
		LOG << "If using checkpointing is disabled, cp-costs-threshold, "
				"cp-costs and cp-rollback-threshold must not be defined" << std::endl;
		exit(-1);
	}

	if (g_use_checkpoints && !(g_cp_thresh > g_cost_cp)) {
		LOG << "cp_cost_thresh needs to be bigger than cp_costs" << std::endl;
		exit(1);
	}

	if (g_use_checkpoints && !(g_rollback_thresh < ((g_cp_thresh - g_cost_cp) / 2))) {
		LOG << "cp_rollback_hop_thresh needs to be smaller than "
				"((cp_cost_thresh - cp_costs) / 2)" << std::endl;
		exit(1);
	}


	ogzstream zipstream;

	ResultCollector rc(*outFile, output_mode);

	if (cmd[OUTPUT_FILE_PROTOBUF]) {
		zipstream.open(cmd[OUTPUT_FILE].first()->arg);
		rc.setProtoOStream(new ProtoOStream(&zipstream));
	}

	BasicAlgorithm *algo;

	switch (algorithm) {
	case ALGO_SMART:
		algo = new SmartAlgorithm(&rc);
		break;
	case ALGO_SIMPLE:
		algo = new SimpleAlgorithm(&rc);
		break;
	default:
		break;
	}

	rc.startTimer();
	algo->calculateAllHops(trace);
	rc.stopTimer();

	rc.finish();

	// ToDo: close output file if not stdout
	if (outFile != &std::cout) {
		((std::ofstream*)outFile)->close();
		delete (std::ofstream*)outFile;
	}

	if (g_use_checkpoints) {
		g_cp_ofstream.close();
	}

	return 0;
}
