#include "util/CommandLine.hpp"
#include "util/Database.hpp"
#include "util/ElfReader.hpp"
#include "util/MemoryMap.hpp"
#include "util/gzstream/gzstream.h"
#include "util/Logger.hpp"
#include <fstream>
#include <string>
#include "MemoryImporter.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
#include "InstructionImporter.hpp"
#include "RegisterImporter.hpp"
#include "RandomJumpImporter.hpp"
#include "AdvancedMemoryImporter.hpp"
#include "ElfImporter.hpp"
#endif


using namespace fail;
using std::cerr;
using std::endl;
using std::cout;

static Logger LOG("import-trace", true);

std::istream& openStream(const char *input_file,
	std::ifstream& normal_stream, igzstream& gz_stream) {
	normal_stream.open(input_file);
	if (!normal_stream) {
		LOG << "couldn't open " << input_file << endl;
		exit(-1);
	}
	unsigned char b1, b2;
	normal_stream >> b1 >> b2;

	if (b1 == 0x1f && b2 == 0x8b) {
		normal_stream.close();
		gz_stream.open(input_file);
		if (!gz_stream) {
			LOG << "couldn't open " << input_file << endl;
			exit(-1);
		}
		LOG << "opened file " << input_file << " in GZip mode" << endl;
		return gz_stream;
	}

	normal_stream.seekg(0);

	LOG << "opened file " << input_file << " in normal mode" << endl;
	return normal_stream;
}

int main(int argc, char *argv[]) {
	std::string trace_file, variant, benchmark;
	ElfReader *elf_file = 0;
	MemoryMap *memorymap = 0;

	// Manually fill the command line option parser
	CommandLine &cmd = CommandLine::Inst();
	for (int i = 1; i < argc; ++i)
		cmd.add_args(argv[i]);

	cmd.addOption("", "", Arg::None, "USAGE: import-trace [options]");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
	CommandLine::option_handle TRACE_FILE =
		cmd.addOption("t", "trace-file", Arg::Required,
			"-t/--trace-file \tFile to load the execution trace from\n");

	// setup the database command line options
	Database::cmdline_setup();

	CommandLine::option_handle VARIANT =
		cmd.addOption("v", "variant", Arg::Required,
			"-v/--variant \tVariant label (default: \"none\")");
	CommandLine::option_handle BENCHMARK =
		cmd.addOption("b", "benchmark", Arg::Required,
			"-b/--benchmark \tBenchmark label (default: \"none\")\n");
	CommandLine::option_handle IMPORTER =
		cmd.addOption("i", "importer", Arg::Required,
			"-i/--importer \tWhich import method to use (default: MemoryImporter)");
	CommandLine::option_handle ELF_FILE =
		cmd.addOption("e", "elf-file", Arg::Required,
			"-e/--elf-file \tELF File (default: UNSET)");
	CommandLine::option_handle MEMORYMAP =
		cmd.addOption("m", "memorymap", Arg::Required,
			"-m/--memorymap \tMemory map to intersect with trace (may be used more than once; default: UNSET)");
	CommandLine::option_handle NO_DELETE =
		cmd.addOption("", "no-delete", Arg::None,
			"--no-delete \tAssume there are no DB entries for this variant/benchmark, don't issue a DELETE");
	CommandLine::option_handle NO_WRITE_ECS =
		cmd.addOption("", "no-write-ecs", Arg::None,
			"--no-write-ecs \tDo not import any write ECs into the database; "
			"results in a perforated fault space and is OK if you only use absolute failure numbers");
	CommandLine::option_handle EXTENDED_TRACE =
		cmd.addOption("", "extended-trace", Arg::None,
			"--extended-trace \tImport extended trace information if available");

	// variant 1: care (synthetic Rs)
	// variant 2: don't care (synthetic Ws)
	CommandLine::option_handle FAULTSPACE_RIGHTMARGIN =
		cmd.addOption("", "faultspace-rightmargin", Arg::Required,
			"--faultspace-rightmargin \tMemory access type (R or W) to "
			"complete fault space at the right margin "
			"(default: W -- don't care)");
	// (don't) cutoff at first R
	// (don't) cutoff at last R
	//CommandLine::option_handle FAULTSPACE_CUTOFF =
	//	cmd.addOption("", "faultspace-cutoff-end", Arg::Required,
	//		"--faultspace-cutoff-end \tCut off fault space end (no, lastr) "
	//		"(default: no)");

	CommandLine::option_handle ENABLE_SANITYCHECKS =
		cmd.addOption("", "enable-sanitychecks", Arg::None,
			"--enable-sanitychecks \tEnable sanity checks "
			"(in case something looks fishy) "
			"(default: disabled)");

	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	Importer *importer;

	if (cmd[IMPORTER]) {
		std::string imp(cmd[IMPORTER].first()->arg);
		if (imp == "BasicImporter" || imp == "MemoryImporter" || imp == "memory" || imp == "mem") {
			imp = "MemoryImporter";
			importer = new MemoryImporter();
#ifdef BUILD_LLVM_DISASSEMBLER
		} else if (imp == "InstructionImporter" || imp == "code") {
			imp = "InstructionImporter";
			importer = new InstructionImporter();

		} else if (imp == "RegisterImporter" || imp == "regs") {
			imp = "RegisterImporter";
			importer = new RegisterImporter();

		} else if (imp == "RandomJumpImporter") {
			importer = new RandomJumpImporter();
		} else if (imp == "AdvancedMemoryImporter") {
			importer = new AdvancedMemoryImporter();
		} else if (imp == "ObjdumpImporter" || imp == "objdump" || imp == "ElfImporter") {
			importer = new ElfImporter();
#endif
		} else {
			LOG << "Unkown import method: " << imp << endl;
			exit(-1);
		}
		LOG << "Using " << imp << endl;

	} else {
		LOG << "Using MemoryImporter" << endl;
		importer = new MemoryImporter();
	}

	if (importer && !(importer->cb_commandline_init())) {
		std::cerr << "Cannot call importers command line initialization!" << std::endl;
		exit(-1);
	}
	// Since the importer might have added command line options, we need to
	// reparse all arguments.
	cmd.parse();

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	if (cmd[TRACE_FILE]) {
		trace_file = std::string(cmd[TRACE_FILE].first()->arg);
	} else {
		trace_file = "trace.pb";
	}

	std::ifstream normal_stream;
	igzstream gz_stream;
	ProtoIStream ps(&openStream(trace_file.c_str(), normal_stream, gz_stream));
	Database *db = Database::cmdline_connect();

	if (cmd[VARIANT]) {
		variant = std::string(cmd[VARIANT].first()->arg);
	} else {
		variant = "none";
	}

	if (cmd[BENCHMARK]) {
		benchmark = std::string(cmd[BENCHMARK].first()->arg);
	} else {
		benchmark = "none";
	}

	if (cmd[ELF_FILE]) {
		elf_file = new ElfReader(cmd[ELF_FILE].first()->arg);
	}
	importer->set_elf(elf_file);

	if (cmd[MEMORYMAP]) {
		memorymap = new MemoryMap();
		for (option::Option *o = cmd[MEMORYMAP]; o; o = o->next()) {
			if (!memorymap->readFromFile(o->arg)) {
				LOG << "failed to load memorymap " << o->arg << endl;
			}
		}
	}
	importer->set_memorymap(memorymap);

	if (cmd[FAULTSPACE_RIGHTMARGIN]) {
		std::string rightmargin(cmd[FAULTSPACE_RIGHTMARGIN].first()->arg);
		if (rightmargin == "W") {
			importer->set_faultspace_rightmargin('W');
		} else if (rightmargin == "R") {
			importer->set_faultspace_rightmargin('R');
		} else {
			LOG << "unknown memory access type '" << rightmargin << "', using default" << endl;
			importer->set_faultspace_rightmargin('W');
		}
	} else {
		importer->set_faultspace_rightmargin('W');
	}

	importer->set_sanitychecks(cmd[ENABLE_SANITYCHECKS]);
	importer->set_extended_trace(cmd[EXTENDED_TRACE]);
	importer->set_import_write_ecs(!cmd[NO_WRITE_ECS]);

	if (!importer->init(variant, benchmark, db)) {
		LOG << "importer->init() failed" << endl;
		exit(-1);
	}

	////////////////////////////////////////////////////////////////
	// Do the actual import
	////////////////////////////////////////////////////////////////

	if (!importer->create_database()) {
		LOG << "create_database() failed" << endl;
		exit(-1);
	}

	if (!cmd[NO_DELETE] && !importer->clear_database()) {
		LOG << "clear_database() failed" << endl;
		exit(-1);
	}

	if (!importer->copy_to_database(ps)) {
		LOG << "copy_to_database() failed" << endl;
		exit(-1);
	}
}
