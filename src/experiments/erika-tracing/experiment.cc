#include <iostream>
#include <fstream>

#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Listener.hpp"
#include "experiment.hpp"
#include "util/CommandLine.hpp"
#include "util/gzstream/gzstream.h"

// required (enabled) plugins
#include "../plugins/tracing/TracingPlugin.hpp"
#include "../plugins/randomgenerator/RandomGenerator.hpp"
#include "../plugins/checkpoint/Checkpoint.hpp"

using namespace std;
using namespace fail;

void  ErikaTracing::parseOptions() {
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>\n\n");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");


	CommandLine::option_handle ELF_FILE = cmd.addOption("", "elf-file", Arg::Required,
															"--elf-file \tELF Binary File (default: $FAIL_ELF_PATH)");
	CommandLine::option_handle START_SYMBOL = cmd.addOption("s", "start-symbol", Arg::Required,
															"-s,--start-symbol \tELF symbol to start tracing (default: main)");
	CommandLine::option_handle STOP_SYMBOL	= cmd.addOption("e", "end-symbol", Arg::Required,
															"-e,--end-symbol \tELF symbol to end tracing");
	CommandLine::option_handle SAVE_SYMBOL	= cmd.addOption("S", "save-symbol", Arg::Required,
															"-S,--save-symbol \tELF symbol to save the state of the machine (default: main)\n");
	CommandLine::option_handle STATE_FILE	= cmd.addOption("f", "state-file", Arg::Required,
															"-f,--state-file \tFile/dir to save the state to (default: state)");
	CommandLine::option_handle TRACE_FILE	= cmd.addOption("t", "trace-file", Arg::Required,
															"-t,--trace-file \tFile to save the execution trace to (default: trace.pb)\n");

	CommandLine::option_handle FULL_TRACE = cmd.addOption("", "full-trace", Arg::None, "--full-trace \tDo a full trace (more data, default: off)");
	CommandLine::option_handle MEM_SYMBOL	= cmd.addOption("m", "memory-symbol", Arg::Required,
															"-m,--memory-symbol \tELF symbol(s) to trace accesses (default: all mem read/writes are traced)");
	CommandLine::option_handle MEM_REGION	= cmd.addOption("M", "memory-region", Arg::Required,
															"-M,--memory-region \trestrict memory region which is traced"
															" (Possible formats: 0x<address>, 0x<address>:0x<address>, 0x<address>:<length>)");

	if (!cmd.parse()) {
		cerr << "Error parsing arguments." << endl;
		exit(-1);
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	if (cmd[ELF_FILE].count() > 0)
		elf_file = cmd[ELF_FILE].first()->arg;
	else {
		char * elfpath = getenv("FAIL_ELF_PATH");
		if (elfpath == NULL) {
			m_log << "FAIL_ELF_PATH not set :( (alternative: --elf-file) " << std::endl;
			exit(-1);
		}

		elf_file = elfpath;
	}
	m_elf = new ElfReader(elf_file.c_str());

	if (cmd[START_SYMBOL].count() > 0)
		start_symbol = cmd[START_SYMBOL].first()->arg;
	else
		start_symbol = "main";

	if (cmd[STOP_SYMBOL].count() > 0)
		stop_symbol = std::string(cmd[STOP_SYMBOL].first()->arg);
	else {
		m_log << "You have to give an end symbol (-e,--end-symbol)!" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (cmd[SAVE_SYMBOL].count() > 0)
		save_symbol = std::string(cmd[SAVE_SYMBOL].first()->arg);
	else
		save_symbol = "main";

	if (cmd[STATE_FILE].count() > 0)
		state_file = std::string(cmd[STATE_FILE].first()->arg);
	else
		state_file = "state";

	if (cmd[TRACE_FILE].count() > 0)
		trace_file = std::string(cmd[TRACE_FILE].first()->arg);
	else
		trace_file = "trace.pb";

	use_memory_map = false;

	if (cmd[MEM_SYMBOL].count() > 0) {
		use_memory_map = true;
		option::Option *opt = cmd[MEM_SYMBOL].first();

		while (opt != 0) {
			const ElfSymbol &symbol = m_elf->getSymbol(opt->arg);
			assert(symbol.isValid());

			m_log << "Adding '" << opt->arg << "' == 0x" << std::hex << symbol.getAddress()
				  << "+" << std::dec << symbol.getSize() << " to trace map" << std::endl;
			traced_memory_map.add(symbol.getAddress(), symbol.getSize());

			opt = opt->next();
		}
	}

	if (cmd[MEM_REGION].count() > 0) {
		use_memory_map = true;
		option::Option *opt = cmd[MEM_REGION].first();

		while (opt != 0) {
			char *endptr;
			guest_address_t begin = strtol(opt->arg, &endptr, 16);
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
				size = strtol(p, &endptr, 16) - begin;
				if (p == endptr || *endptr != 0) {
					m_log << "Couldn't parse " << opt->arg << std::endl;
					exit(-1);
				}
			} else if (delim == '+') {
				char *p = endptr +1;
				size = strtol(p, &endptr, 10);
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

	if (cmd[FULL_TRACE]) {
		this->full_trace = true;
	}

	assert(m_elf->getSymbol(start_symbol).isValid());
	assert(m_elf->getSymbol(stop_symbol).isValid());
	assert(m_elf->getSymbol(save_symbol).isValid());

	m_log << "start symbol: " << start_symbol << " 0x" << std::hex << m_elf->getSymbol(start_symbol).getAddress() << std::endl;
	m_log << "save symbol: "  << save_symbol  << " 0x" << std::hex << m_elf->getSymbol(save_symbol).getAddress() << std::endl;
	m_log << "stop symbol: "  << stop_symbol  << " 0x" << std::hex << m_elf->getSymbol(stop_symbol).getAddress() << std::endl;

	m_log << "state file: "	  << state_file		  << std::endl;
	m_log << "trace file: "	  << trace_file		  << std::endl;
	m_log << "full-trace: "	  << this->full_trace << std::endl;


}

bool ErikaTracing::run()
{
	parseOptions();

	BPSingleListener l_start_symbol(m_elf->getSymbol(start_symbol).getAddress());
	BPSingleListener l_save_symbol (m_elf->getSymbol(save_symbol).getAddress());
	BPSingleListener l_stop_symbol (m_elf->getSymbol(stop_symbol).getAddress());


	////////////////////////////////////////////////////////////////
	// STEP 1: run until interesting function starts, start the tracing
	simulator.addListenerAndResume(&l_start_symbol);
	m_log << start_symbol << " reached, start tracing" << std::endl;

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

	////////////////////////////////////////////////////////////////
	// STEP 2: continue to the save point, and save state
	if (start_symbol != save_symbol) {
		simulator.addListenerAndResume(&l_save_symbol);
	}
	m_log << start_symbol << " reached, save state" << std::endl;
	simulator.save(state_file);


	////////////////////////////////////////////////////////////////
	// Step 3: add plugins
	// symbol to trigger checkpoints
	const ElfSymbol &s_fail_trace = m_elf->getSymbol("fail_trace");
	const ElfSymbol &s_tos = m_elf->getSymbol("EE_x86_system_tos");

	Checkpoint *cpoint;
	if(s_fail_trace.isValid() && s_tos.isValid()) {
		Checkpoint::range_vector check_ranges;

		const ElfSymbol &s_os_stack = m_elf->getSymbol("os_stack");
		assert (s_os_stack.isValid());
		m_log << "found task stack: " << "os_stack" << std::endl;

		Checkpoint::indirectable_address_t start = std::make_pair(s_tos.getAddress() + 0*4, true);
		Checkpoint::indirectable_address_t end = std::make_pair(s_os_stack.getEnd(), false);
		check_ranges.push_back(std::make_pair(start, end));

		for(int i=1; i<255; i++) {
			stringstream stackname;
			stackname << "EE_x86_stack_";
			stackname << i;
			const ElfSymbol &s_stack = m_elf->getSymbol(stackname.str());
			if(!s_stack.isValid()) {
				m_log << "found " << std::dec << (i-1) << " task stacks" << std::endl;
				break;
			}

			m_log << "found task stack: " << stackname.str() << std::endl;

			Checkpoint::indirectable_address_t start = std::make_pair(s_tos.getAddress() + i*4, true);
			Checkpoint::indirectable_address_t end = std::make_pair(s_stack.getEnd(), false);
			check_ranges.push_back(std::make_pair(start, end));
		}

		cpoint = new Checkpoint(s_fail_trace, check_ranges, "checkpoint.trace");
		simulator.addFlow(cpoint);
	} else {
		m_log << "Checkpoint plugin NOT added to simulation" << std::endl;
	}

	// symbol to read random values from
	const ElfSymbol &s_random_source = m_elf->getSymbol("random_source");
	RandomGenerator *rgen;
	if (s_random_source.isValid()) {
		const unsigned seed = 12342;
		rgen = new RandomGenerator(s_random_source, seed);
		simulator.addFlow(rgen);
	} else {
		m_log << "Randomgenerator plugin NOT added to simulation" << std::endl;
	}


	////////////////////////////////////////////////////////////////
	// Step 4: Continue to the stop point
	simulator.addListener(&l_stop_symbol);
	simulator.resume();

	////////////////////////////////////////////////////////////////
	// Step 5: tear down the tracing

	simulator.removeFlow(&tp);
	// serialize trace to file
	if (of.fail()) {
		m_log << "failed to write " << trace_file << std::endl;
		return false;
	}
	of.close();

	simulator.clearListeners();
	simulator.terminate();

	return true;
}
