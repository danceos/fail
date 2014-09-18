#include <iostream>
#include <fstream>
#include <sstream>
#include "comm/TracePlugin.pb.h"
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
	CommandLine::option_handle UNKNOWN =
		cmd.addOption("", "", Arg::None, "usage: dump-trace [options] tracefile.tc");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
	CommandLine::option_handle STATS =
		cmd.addOption("s", "stats", Arg::None,
			"-s/--stats \tShow trace stats");
	CommandLine::option_handle EXTENDED_TRACE =
		cmd.addOption("", "extended-trace", Arg::None,
			"--extended-trace \tDump extended trace information if available");

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

	bool stats_only = cmd[STATS];
	bool extended = cmd[EXTENDED_TRACE];

	std::ifstream normal_stream;
	igzstream gz_stream;
	ProtoIStream ps(&openStream(cmd.parser()->nonOption(0), normal_stream, gz_stream));

	uint64_t acctime = 0;
	uint64_t stats_instr = 0, stats_reads = 0, stats_writes = 0;
	uint64_t stats_read_b = 0, stats_write_b = 0, starttime = 0;

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
			stringstream ext;
			if (ev.has_trace_ext() && !stats_only && extended) {
				const Trace_Event_Extended& temp_ext = ev.trace_ext();
				ext << " DATA " << std::hex;
				ext << (uint64_t) temp_ext.data();
				for (int i = 0; i < temp_ext.registers_size(); i++) {
					const Trace_Event_Extended_Registers& temp_reg = temp_ext.registers(i);
					ext << " REG "
					    << (unsigned) temp_reg.id() << " = ";
					if (temp_reg.has_value()) {
						ext << (uint32_t) temp_reg.value();
					} else {
						ext << "??";
					}
					ext << " -> ";
					if (temp_reg.has_value_deref()) {
						ext << (uint32_t) temp_reg.value_deref();
					} else {
						ext << "??";
					}
				}
				if (temp_ext.stack_size() > 0 ) {
					ext << " STACK:";
					for (int i = 0; i < temp_ext.stack_size(); i++) {
						const Trace_Event_Extended_Stack& temp_stack = temp_ext.stack(i);
						ext << " " << (uint32_t) temp_stack.value();
					}
				}
			}
			if (!stats_only) {
				cout << "MEM "
				     << (ev.accesstype() == Trace_Event_AccessType_READ ? "R" : "W") << " "
				     << hex << ev.memaddr()
				     << dec << " width " << ev.width()
				     << hex << " IP " << ev.ip()
				     << dec << " t=" << acctime
				     << ext.str() << "\n";
			}
			if (ev.accesstype() == Trace_Event_AccessType_READ) {
				stats_reads++;
				stats_read_b += ev.width();
			} else {
				stats_writes++;
				stats_write_b += ev.width();
			}
		}
	}

	if (stats_only) {
		cout << "#instructions: " << stats_instr << "\n"
			 << "#memR:         " << stats_reads << "\n"
			 << "#memR_bytes    " << stats_read_b << "\n"
			 << "#memW:         " << stats_writes << "\n"
			 << "#memW_bytes    " << stats_write_b << "\n"
			 << "duration:      " << (acctime - starttime + 1) << endl;
	}

	return 0;
}
