#include <iostream>
#include <fstream>
#include <sstream>
#include "comm/InjectionPointHopsMessage.pb.h"
#include "util/ProtoStream.hpp"
#include "../../src/core/util/Logger.hpp"
#include "../../src/core/util/gzstream/gzstream.h"
#include "util/CommandLine.hpp"

using namespace fail;
using std::stringstream;
using std::endl;
using std::cout;
using std::cerr;
using std::hex;
using std::dec;

Logger LOG("dump-hops", true);

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
		//LOG << "opened file " << input_file << " in GZip mode" << endl;
		return gz_stream;
	}

	normal_stream.seekg(0);

	//LOG << "opened file " << input_file << " in normal mode" << endl;
	return normal_stream;
}

int main(int argc, char *argv[])
{
	InjectionPointMessage ev;

	CommandLine &cmd = CommandLine::Inst();
	CommandLine::option_handle UNKNOWN =
		cmd.addOption("", "", Arg::None, "usage: dump-hops [options] hopfile.hp");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
//	CommandLine::option_handle STATS =
//		cmd.addOption("s", "stats", Arg::None,
//			"-s/--stats \tShow trace stats");

	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return 1;
	}

	if (cmd[HELP] || cmd[UNKNOWN] || cmd.parser()->nonOptionsCount() != 1) {
		for (option::Option* opt = cmd[UNKNOWN]; opt; opt = opt->next()) {
			std::cerr << "Unknown option: " << opt->name << "\n";
		}
		for (int i = 1; i < cmd.parser()->nonOptionsCount(); ++i) {
			std::cerr << "Unknown non-option: " << cmd.parser()->nonOption(i) << "\n";
		}
		cmd.printUsage();
		if (cmd[HELP]) {
			exit(0);
		} else {
			exit(1);
		}
	}

//	bool stats_only = cmd[STATS];
//	bool extended = cmd[EXTENDED_TRACE];

	std::ifstream normal_stream;
	igzstream gz_stream;
	ProtoIStream ps(&openStream(cmd.parser()->nonOption(0), normal_stream, gz_stream));

//	uint64_t stats_instr = 0, stats_reads = 0, stats_writes = 0, starttime = 0;

	while (ps.getNext(&ev)) {
		cout << "T" << ev.target_trace_position() ;

		if (ev.has_checkpoint_id()) {
			cout << " CP" << ev.checkpoint_id();
		}

		for (int i = 0; i < ev.hops_size(); i++) {
			const InjectionPointMessage_Hops &hops = ev.hops(i);
			cout << " " <<
				((hops.accesstype()==InjectionPointMessage_Hops_AccessType_EXECUTE)?"X":
						((hops.accesstype()==InjectionPointMessage_Hops_AccessType_READ)?"R":"W"))
						<< hex << hops.address() << dec;
		}
		cout << "\n";
	}

//	if (stats_only) {
//		cout << "#instructions: " << stats_instr << "\n"
//		     << "#memR:         " << stats_reads << "\n"
//		     << "#memW:         " << stats_writes << "\n"
//		     << "duration:      " << (acctime - starttime + 1) << endl;
//	}

	return 0;
}
