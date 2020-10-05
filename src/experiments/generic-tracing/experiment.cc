#include <iostream>
#include <fstream>

#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Listener.hpp"
#include "experiment.hpp"
#include "util/CommandLine.hpp"
#include "util/gzstream/gzstream.h"
#include <limits>

// You need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"
// You need to have the serialoutput plugin enabled for this
#include "../plugins/serialoutput/SerialOutputLogger.hpp"


/*
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
*/

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_SAVE)
  #error This experiment needs: breakpoints, and save. Enable these in the configuration.
#endif

void  GenericTracing::parseOptions() {
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>\n\n");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	CommandLine::option_handle ELF_FILE = cmd.addOption("", "elf-file", Arg::Required,
		"--elf-file FILE \tELF binary file (default: $FAIL_ELF_PATH)");
	CommandLine::option_handle START_SYMBOL = cmd.addOption("s", "start-symbol", Arg::Required,
		"-s,--start-symbol SYMBOL \tELF symbol to start tracing (default: main)");
	CommandLine::option_handle STOP_SYMBOL	= cmd.addOption("e", "end-symbol", Arg::Required,
		"-e,--end-symbol SYMBOL \tELF symbol to end tracing");
	// only there for backwards compatibility; remove at some point
	CommandLine::option_handle SAVE_SYMBOL	= cmd.addOption("S", "save-symbol", Arg::Required,
		"-S,--save-symbol SYMBOL \tELF symbol to save the state of the machine "
		"(exists for backward compatibility, must be identical to --start-symbol if used)\n");
	CommandLine::option_handle STATE_FILE	= cmd.addOption("f", "state-file", Arg::Required,
		"-f,--state-file FILE \tFile/dir to save the state to (default: state). "
		"Use /dev/null if no state is required");
	CommandLine::option_handle TRACE_FILE	= cmd.addOption("t", "trace-file", Arg::Required,
		"-t,--trace-file FILE \tFile to save the execution trace to (default: trace.pb)\n");

	CommandLine::option_handle RESTORE = cmd.addOption("", "restore", Arg::None,
		"--restore \tRestore to the saved state of the machine immediately after saving (default: off). "
		"This option is needed when the state is used by other experiments that depend on the "
		"trace, which slighty differs without a restore.");
	CommandLine::option_handle FULL_TRACE = cmd.addOption("", "full-trace", Arg::None,
		"--full-trace \tDo a full trace (more data, default: off)");
	CommandLine::option_handle MEM_SYMBOL	= cmd.addOption("m", "memory-symbol", Arg::Required,
		"-m,--memory-symbol SYMBOL \tELF symbol to trace accesses to "
		"(default: all mem read/writes are traced; may be used more than once)");
	CommandLine::option_handle MEM_REGION	= cmd.addOption("M", "memory-region", Arg::Required,
		"-M,--memory-region R \trestrict memory region which is traced"
		" (Possible formats: 0x<address>, 0x<address>:0x<address>, 0x<address>:<length>)");
	CommandLine::option_handle START_ADDRESS = cmd.addOption("B", "start-address", Arg::Required,
		"-B,--start-address ADDRESS \tStart Address to start tracing");
	CommandLine::option_handle STOP_ADDRESS = cmd.addOption("E", "end-address",Arg::Required,
		"-E,--end-address ADDRESS \tEnd Address to end tracing");
	CommandLine::option_handle SERIAL_FILE = cmd.addOption("", "serial-file", Arg::Required,
		"--serial-file FILE \tSave the serial output to file");
	CommandLine::option_handle SERIAL_PORT = cmd.addOption("", "serial-port", Arg::Required,
		"--serial-port PORT \tI/O port to expect output on (default: 0x3f8, i.e. x86's serial COM1)");
	CommandLine::option_handle E9_FILE = cmd.addOption("", "e9-file", Arg::Required,
		"--e9-file FILE \tShorthand for --serial-file FILE --serial-port 0xe9");
	CommandLine::option_handle CHECK_BOUNDS = cmd.addOption("", "check-bounds", Arg::None,
		"--check-bounds \tWhether or not to enable outerspace and text segment checkers which are used in the experiment stage during tracing. If these trip, something is wrong with your architecture implementation.");

	if (!cmd.parse()) {
		cerr << "Error parsing arguments." << endl;
		exit(-1);
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	if (cmd[ELF_FILE]) {
		elf_file = cmd[ELF_FILE].first()->arg;
		m_elf = new ElfReader(elf_file.c_str());
	} else {
		char *elfpath = getenv("FAIL_ELF_PATH");
		if (elfpath == NULL) {
			m_elf = NULL;
		} else {
			elf_file = elfpath;
			m_elf = new ElfReader(elf_file.c_str());
		}
	}

	if (cmd[START_SYMBOL] && m_elf != NULL) {
		start_symbol = cmd[START_SYMBOL].first()->arg;
		const ElfSymbol& symbol = m_elf->getSymbol(start_symbol);
		if (!symbol.isValid()) {
			m_log << "Start symbol '" << start_symbol << "' not found." << std::endl;
			exit(EXIT_FAILURE);
		}
		start_address = symbol.getAddress();
	} else if (cmd[START_ADDRESS]) {
		start_address = strtoul(cmd[START_ADDRESS].first()->arg, NULL, 16);
	} else if (m_elf == NULL) {
		m_log << "FAIL_ELF_PATH not set or no start address given :( (alternative: --elf-file, --start-address)" << std::endl;
		exit(EXIT_FAILURE);
	} else {
		start_symbol = "main";
		const ElfSymbol& symbol = m_elf->getSymbol(start_symbol);
		if (!symbol.isValid()) {
			m_log << "Start symbol '" << start_symbol << "' not found." << std::endl;
			exit(EXIT_FAILURE);
		}
		start_address = symbol.getAddress();
	}

	if (cmd[STOP_SYMBOL] && m_elf != NULL) {
		stop_symbol = std::string(cmd[STOP_SYMBOL].first()->arg);
		const ElfSymbol& symbol = m_elf->getSymbol(stop_symbol);
		if (!symbol.isValid()) {
			m_log << "Stop symbol '" << stop_symbol << "' not found." << std::endl;
			exit(EXIT_FAILURE);
		}
		stop_address = symbol.getAddress();
	} else if (cmd[STOP_ADDRESS]) {
		stop_address = strtoul(cmd[STOP_ADDRESS].first()->arg, NULL, 16);
	} else {
		m_log << "You have to give an end symbol or an end address (-e,--end-symbol, --end-address)!" << std::endl;
		exit(EXIT_FAILURE);
	}

	// only there for backwards compatibility; remove at some point
	if (cmd[SAVE_SYMBOL]) {
		if (std::string(cmd[SAVE_SYMBOL].first()->arg) != start_symbol) {
			m_log << "Save symbol (-S,--save-symbol) must be identical to start symbol (-s,--start-symbol)!" << std::endl;
			exit(EXIT_FAILURE);
		} else {
			m_log << "(Using -S,--save-symbol is deprecated)" << std::endl;
		}
	}

	if (cmd[STATE_FILE]) {
		state_file = std::string(cmd[STATE_FILE].first()->arg);
	} else {
		state_file = "state";
	}

	if (cmd[TRACE_FILE]) {
		trace_file = std::string(cmd[TRACE_FILE].first()->arg);
	} else {
		trace_file = "trace.pb";
	}

	use_memory_map = false;

	if (cmd[MEM_SYMBOL]) {
		use_memory_map = true;
		option::Option *opt = cmd[MEM_SYMBOL].first();

		while (opt != 0) {
			const ElfSymbol& symbol = m_elf->getSymbol(opt->arg);
			if (!symbol.isValid()) {
				m_log << "Symbol '" << start_symbol << "' not found." << std::endl;
				exit(EXIT_FAILURE);
			}

			m_log << "Adding '" << opt->arg << "' == 0x" << std::hex << symbol.getAddress()
				  << "+" << std::dec << symbol.getSize() << " to trace map" << std::endl;
			traced_memory_map.add(symbol.getAddress(), symbol.getSize());

			opt = opt->next();
		}
	}

	if (cmd[MEM_REGION]) {
		use_memory_map = true;
		option::Option *opt = cmd[MEM_REGION].first();

		while (opt != 0) {
			char *endptr;
			guest_address_t begin = strtoul(opt->arg, &endptr, 16);
			guest_address_t size;
			if (endptr == opt->arg) {
				m_log << "Couldn't parse " << opt->arg << std::endl;
				exit(-1);
			}

			char delim = *endptr;
			if (delim == 0) {
				size = 1;
			} else if (delim == ':') {
				char *p = endptr +1;
				size = strtoul(p, &endptr, 16) - begin;
				if (p == endptr || *endptr != 0) {
					m_log << "Couldn't parse " << opt->arg << std::endl;
					exit(-1);
				}
			} else if (delim == '+') {
				char *p = endptr +1;
				size = strtoul(p, &endptr, 10);
				if (p == endptr || *endptr != 0) {
					m_log << "Couldn't parse " << opt->arg << std::endl;
					exit(-1);
				}
			} else {
				m_log << "Couldn't parse " << opt->arg << std::endl;
				exit(-1);
			}

			traced_memory_map.add(begin, size);

			m_log << "Adding " << opt->arg << " 0x" << std::hex << begin
				  << "+" << std::dec << size << " to trace map" << std::endl;

			opt = opt->next();
		}
	}

	if (cmd[RESTORE]) {
		this->restore = true;
	}
	if (cmd[FULL_TRACE]) {
		this->full_trace = true;
	}

	if (cmd[SERIAL_PORT]) {
		option::Option *opt = cmd[SERIAL_PORT].first();
		char *endptr;
		serial_port = strtoul(opt->arg, &endptr, 16);
		if (endptr == opt->arg) {
			m_log << "Couldn't parse " << opt->arg << std::endl;
			exit(-1);
		}
		m_log << "serial port: 0x" << std::hex << serial_port << std::endl;
	} else {
		serial_port = 0x3f8;
	}

	if (cmd[SERIAL_FILE]) {
		serial_file = std::string(cmd[SERIAL_FILE].first()->arg);
		m_log << "serial file: " << serial_file << std::endl;
	}

	if (cmd[E9_FILE]) {
		serial_file = std::string(cmd[E9_FILE].first()->arg);
		serial_port = 0xe9;
		m_log << "port E9 output is written to: " << serial_file << std::endl;
	}

	if(cmd[CHECK_BOUNDS]) {
		this->check_bounds = true;
		m_log << "enabled bounds sanity check" << std::endl;
	}


	if (m_elf != NULL) {
		m_log << "start/save symbol: " << start_symbol << " 0x" << std::hex << start_address << std::endl;
		m_log << "stop symbol: "  << stop_symbol  << " 0x" << std::hex << stop_address << std::endl;
	} else {
		m_log << "start address: 0x" << std::hex << start_address << std::endl;
		m_log << "stop address: 0x" << std::hex << stop_address << std::endl;
	}

	m_log << "state file: "	  << state_file		  << std::endl;
	m_log << "trace file: "	  << trace_file		  << std::endl;
	m_log << "full-trace: "	  << this->full_trace << std::endl;
}

bool GenericTracing::run()
{
	parseOptions();

	fail::MemAccessListener l_mem_text(fail::MemAccessEvent::MEM_WRITE);
	fail::MemAccessListener l_mem_outerspace(fail::MemAccessEvent::MEM_READWRITE);
	fail::MemAccessListener l_mem_lowerspace(fail::MemAccessEvent::MEM_READWRITE);

	if(check_bounds) { using std::numeric_limits;
		{
			auto bounds = m_elf->getTextSegmentBounds();
			m_log << "Catch writes to text segment from "
				  << hex << bounds.first << " to " << bounds.second << std::endl;
			l_mem_text.setWatchAddress(bounds.first);
			l_mem_text.setWatchWidth(bounds.second - bounds.first);
			// FIXME: l_mem_text.setWatchMemoryType(memory_type::ram);
		}
		{
			auto bounds = m_elf->getValidAddressBounds();
			m_log << "Catch writes to upper outerspace from " << hex << bounds.second << std::endl;
			l_mem_outerspace.setWatchAddress(bounds.second);
			l_mem_outerspace.setWatchWidth(numeric_limits<guest_address_t>::max() - bounds.second);
			// FIXME: l_mem_outerspace.setWatchMemoryType(memory_type::ram);
			m_log << "Catch writes to lower outer-space below " << hex << bounds.first << std::endl;
			l_mem_lowerspace.setWatchAddress(numeric_limits<guest_address_t>::min());
			l_mem_lowerspace.setWatchWidth(bounds.first - numeric_limits<guest_address_t>::min());
			// FIXME: l_mem_lowerspace.setWatchMemoryType(memory_type::ram);
		}
	}


	BPSingleListener l_start_symbol(start_address);
	BPSingleListener l_stop_symbol(stop_address);

	////////////////////////////////////////////////////////////////
	// STEP 1: run until interesting function starts, save, and start tracing
	simulator.addListenerAndResume(&l_start_symbol);

	m_log << start_symbol << " reached, save ..." << std::endl;
	simulator.save(state_file);

	if (restore) {
		m_log << "restoring clean state ..." << std::endl;
		simulator.restore(state_file);
	}

	m_log << "... and start tracing" << std::endl;

	// restrict memory access logging to injection target
	TracingPlugin tp;
	tp.setFullTrace(this->full_trace);

	if (use_memory_map) {
		m_log << "Use restricted memory map for tracing" << std::endl;
		tp.restrictMemoryAddresses(&traced_memory_map);
	}

	ogzstream of(trace_file.c_str());
	if (of.bad()) {
		m_log << "Couldn't open trace file: " << trace_file << std::endl;
		exit(-1);
	}
	tp.setTraceFile(&of);

	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// record serial output
	SerialOutputLogger sol(serial_port);
	if (serial_file != "") {
		simulator.addFlow(&sol);
	}

	////////////////////////////////////////////////////////////////
	// Step 2: Continue to the stop point (and/or watch for unexpected events)
	if(check_bounds) {
		simulator.addListener(&l_mem_text);
		simulator.addListener(&l_mem_outerspace);
		simulator.addListener(&l_mem_lowerspace);
	}

	simulator.addListener(&l_stop_symbol);
	fail::BaseListener* listener = simulator.resume();

	if (check_bounds) {
		if(listener == &l_mem_text) {
			m_log << "sanity check: .text write detected. aborting!" << std::endl
				  << "invalid access at: 0x"  << std::hex << l_mem_text.getTriggerAddress() << std::endl;
			return false;
		}
		if(listener == &l_mem_outerspace) {
			m_log << "sanity check: write outside above valid elf detected. aborting!" << std::endl
				  << " invalid access at: 0x" << std::hex << l_mem_outerspace.getTriggerAddress()
				//<< " memory type watched: " << l_mem_outerspace.getWatchMemoryType()
				//<< " memory type trigger: " << l_mem_outerspace.getMemoryType()
				  << " access type trigger: " << (l_mem_outerspace.getTriggerAccessType() == MemAccessEvent::MEM_READ ? "R" : "W")
				//<< " data accessed: " << l_mem_outerspace.getAccessedData()
				  << std::endl;
			return false;
		}
		if(listener == &l_mem_lowerspace) {
			m_log << "sanity check: write outside below valid elf detected. aborting!" << std::endl
				  << " invalid access at: 0x" << std::hex << l_mem_lowerspace.getTriggerAddress()
				//<< " memory type watched: " << l_mem_lowerspace.getWatchMemoryType()
				//<< " memory type trigger: " << l_mem_lowerspace.getMemoryType()
				  << " access type trigger: " << (l_mem_lowerspace.getTriggerAccessType() == MemAccessEvent::MEM_READ ? "R" : "W")
				// << " data accessed: " << l_mem_lowerspace.getAccessedData()
				  << std::endl;
			return false;
		}
	}
	////////////////////////////////////////////////////////////////
	// Step 3: tear down the tracing

	simulator.removeFlow(&tp);
	// serialize trace to file
	if (of.fail()) {
		m_log << "failed to write " << trace_file << std::endl;
		return false;
	}
	of.close();

	if (serial_file != "") {
		simulator.removeFlow(&sol);
		ofstream of_serial(serial_file.c_str(), ios::out|ios::binary);
		if (!of_serial.fail()) {
			of_serial << sol.getOutput();
		} else {
			m_log << "failed to write " << serial_file << endl;
		}
		of_serial.close();
	}

	simulator.clearListeners();
	simulator.terminate();

	return true;
}
