#include <iostream>
#include <fstream>
#include <string>
#include "comm/TracePlugin.pb.h"
#include "util/ProtoStream.hpp"
#include "../../src/core/util/Logger.hpp"
#include "../../src/core/util/gzstream/gzstream.h"
#include "util/CommandLine.hpp"

using namespace fail;
using std::string;
using std::endl;
using std::cout;
using std::cerr;
using std::hex;
using std::dec;

Logger LOG("dump-trace", true);

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
	Trace_Event ev;

	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "usage: dump-trace [options] tracefile.tc");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
	CommandLine::option_handle STATS =
		cmd.addOption("s", "stats", Arg::None,
			"-s/--stats \tShow trace stats");

	for (int i = 1; i < argc; ++i) {
		cmd.add_args(argv[i]);
	}
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return 1;
	}

	if (cmd[HELP] || cmd.parser()->nonOptionsCount() != 1) {
		cmd.printUsage();
		if (cmd[HELP]) {
			exit(0);
		} else {
			exit(1);
		}
	}

	bool stats_only = false;
	if (cmd[STATS]) {
		stats_only = true;
	}

	std::ifstream normal_stream;
	igzstream gz_stream;
	ProtoIStream ps(&openStream(cmd.parser()->nonOption(0), normal_stream, gz_stream));

	uint64_t acctime = 0;
	uint64_t stats_instr = 0, stats_reads = 0, stats_writes = 0, starttime = 0;

	while (ps.getNext(&ev)) {
		if (ev.has_time_delta()) {
			if (!acctime) {
				starttime = ev.time_delta();
			}
			acctime += ev.time_delta();
		}
		if (!ev.has_memaddr()) {
			++stats_instr;
			if (!stats_only) {
				cout << "IP " << hex << ev.ip() << dec << " t=" << acctime << "\n";
			}
		} else {
			string ext = ""; // FIXME: use stringstream?
			if (ev.has_trace_ext() && !stats_only) {
				const Trace_Event_Extended& temp_ext = ev.trace_ext();
				ext = " DATA ";
				ext += temp_ext.data();
				int i;
				for (i = 0 ; i < temp_ext.registers_size() ; i++) {
					const Trace_Event_Extended_Registers& temp_reg = temp_ext.registers(i);
					ext += "REG: " ;
					ext += temp_reg.id();
					ext += " ";
					ext += temp_reg.value();
					ext += "=";
					ext += temp_reg.value_deref();
				}
				if (temp_ext.stack_size() > 0 ) {
					ext += " STACK: ";
					for (i = 0 ; i< temp_ext.stack_size() ; i++) {
						const Trace_Event_Extended_Stack& temp_stack = temp_ext.stack(i);
						ext += temp_stack.value();
					}
				}
			}
			if (!stats_only) {
				cout << "MEM "
				     << (ev.accesstype() == Trace_Event_AccessType_READ ? "R" : "W") << " "
				     << ev.memaddr()
				     << dec << " width " << ev.width()
				     << hex << " IP " << ev.ip()
				     << dec << " t=" << acctime
				     << ext << "\n";
			}
			stats_reads += (ev.accesstype() == Trace_Event_AccessType_READ);
			stats_writes += (ev.accesstype() == Trace_Event_AccessType_WRITE);
		}
	}

	if (stats_only) {
		cout << "#instructions: " << stats_instr << "\n"
		     << "#memR:         " << stats_reads << "\n"
		     << "#memW:         " << stats_writes << "\n"
		     << "duration:      " << (acctime - starttime + 1) << endl;
	}

	return 0;
}
