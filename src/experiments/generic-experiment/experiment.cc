#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>


#include <stdlib.h>
#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include <string>
#include <vector>

#include "campaign.hpp"
#include "generic-experiment.pb.h"
#include "util/CommandLine.hpp"

using namespace std;
using namespace fail;


GenericExperiment::~GenericExperiment() {}

static GenericExperimentData space_for_param;
ExperimentData* GenericExperiment::cb_allocate_experiment_data() {
    return &space_for_param;
}

/**
 * Allocate a new result slot in the given experiment data
 */
google::protobuf::Message* GenericExperiment::cb_new_result(ExperimentData* data) {
    GenericExperimentData *param = static_cast<GenericExperimentData *>(data);
    GenericExperimentMessage_Result *result = param->msg.add_result();
    return result;
}

std::string loadFile(std::string filename)
{
	std::string data;
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in) {
		return data;
	}
	in.seekg(0, std::ios::end);
	data.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&data[0], data.size());
	in.close();
	return data;
}

void handleEvent(GenericExperimentMessage_Result& result,
				 GenericExperimentMessage_Result_ResultType restype,
				 unsigned int details) {
    cout << "Result details: "
		<< dec << restype << " " << hex << "0x" << details << endl;
    result.set_resulttype(restype);
    result.set_details(details);
}

void GenericExperiment::parseSymbols(const std::string &args, std::set<fail::BaseListener *> * into) {
	std::vector<std::string> elems;
	std::stringstream ss(args);
	std::string item;
	while (std::getline(ss, item, ',')) {
		bool is_optional = false;
		if (item.length() > 0 && item[0] == '?') {
			item.erase(item.begin());
			is_optional = true;
		}
		const ElfSymbol * symbol = &(m_elf->getSymbol(item));
		if (!symbol->isValid()) {
			if (is_optional) {
				m_log << "ELF Symbol not found, ignoring: " << item << endl;
				continue;
			} else {
				m_log << "ELF Symbol not found, aborting: " << item << endl;
				simulator.terminate(1);
			}
		}
		m_log << "Adding symbol " << item <<  " at 0x" << hex << symbol->getAddress() << endl;
		BPSingleListener *l = new BPSingleListener(symbol->getAddress());
		into->insert(l);
		end_markers.insert(l);
		listener_to_symbol[l] = symbol;
	}
}



bool GenericExperiment::cb_start_experiment() {
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>\n");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	CommandLine::option_handle ELF_FILE = cmd.addOption("", "elf-file", Arg::Required,
		"--elf-file FILE \tELF-binary file (default: $FAIL_ELF_PATH)");

	CommandLine::option_handle STATE_DIR = cmd.addOption("", "state-dir", Arg::Required,
		"--state-dir DIR \tPath to the state directory");

	// catch any trap
	CommandLine::option_handle TRAP = cmd.addOption("", "trap", Arg::None,
		"--trap \tCatch traps");
	CommandLine::option_handle WRITE_MEM_TEXT = cmd.addOption("", "catch-write-textsegment", Arg::None,
		"--catch-write-textsegment \tCatch writes to the text segment");

	CommandLine::option_handle WRITE_MEM_OUTERSPACE
		= cmd.addOption("", "catch-write-outerspace", Arg::None,
		"--catch-write-outerspace \tCatch writes to the outerspace");

	CommandLine::option_handle TIMEOUT = cmd.addOption("", "timeout", Arg::Required,
		"--timeout TIME \tExperiment timeout in uS");

	CommandLine::option_handle SERIAL_FILE = cmd.addOption("", "serial-file", Arg::Required,
		"--serial-file FILE \tGolden-run serial output recording to check against");
	CommandLine::option_handle SERIAL_PORT = cmd.addOption("", "serial-port", Arg::Required,
		"--serial-port PORT \tI/O port to expect output on (default: 0x3f8, i.e. x86's serial COM1)");
	CommandLine::option_handle E9_FILE = cmd.addOption("", "e9-file", Arg::Required,
		"--e9-file FILE \tShorthand for --serial-file FILE --serial-port 0xe9");


	std::map<std::string, CommandLine::option_handle> option_handles;
	for (std::map<std::string, ListenerSet *>::iterator it = end_marker_groups.begin();
		 it != end_marker_groups.end(); ++it) {
		CommandLine::option_handle handle =
			cmd.addOption("", it->first, Arg::Required,
				"--" + it->first + " S1,S2,... \tSymbol list (prefix non-essential symbols with '?')");
		option_handles[it->first] = handle;
	}

	if (!cmd.parse()) {
		cerr << "Error parsing arguments." << endl;
		exit(1);
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	if (cmd[ELF_FILE]) {
		elf_file = cmd[ELF_FILE].first()->arg;
		m_elf = new ElfReader(elf_file.c_str());
		m_log << "ELF file specified: " << elf_file << std::endl;
	} else {
		char *elfpath = getenv("FAIL_ELF_PATH");
		if (elfpath == NULL) {
			m_elf = NULL;
		} else {
			elf_file = elfpath;
			m_elf = new ElfReader(elf_file.c_str());
			m_log << "ELF file via environment variable: " << elf_file << std::endl;
		}
	}
	if (m_elf == NULL) {
		m_log << "ERROR: no FAIL_ELF_PATH set or --elf-file given. exiting!" << std::endl;
		exit(1);
	}

	address_t minimal_ip = INT_MAX; // Every address is lower
	address_t maximal_ip = 0;
	address_t minimal_data = 0x100000; // 1 Mbyte
	address_t maximal_data = 0;

	for (ElfReader::section_iterator it = m_elf->sec_begin();
		 it != m_elf->sec_end(); ++it) {
		const ElfSymbol &symbol = *it;
		std::string prefix(".text");
		if (symbol.getName().compare(0, prefix.size(), prefix) == 0) {
			minimal_ip = std::min(minimal_ip, symbol.getStart());
			maximal_ip = std::max(maximal_ip, symbol.getEnd());
		} else {
			minimal_data = std::min(minimal_data, symbol.getStart());
			maximal_data = std::max(maximal_data, symbol.getEnd());
		}
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

	std::string serial_file;
	if (cmd[SERIAL_FILE]) {
		serial_file = std::string(cmd[SERIAL_FILE].first()->arg);
		serial_enabled = true;
		m_log << "serial file: " << serial_file << std::endl;
	}

	if (cmd[E9_FILE]) {
		serial_file = std::string(cmd[E9_FILE].first()->arg);
		serial_port = 0xe9;
		serial_enabled = true;
		m_log << "port E9 output is monitored and compared to: " << serial_file << std::endl;
	}

	if (serial_enabled) {
		serial_goldenrun = loadFile(serial_file);
		sol.setPort(serial_port);

		// Limit the serial-output logger buffer to prevent overly large memory
		// consumption in case the target system ends up, e.g., in an endless
		// loop.  "+ 1" to be able to detect the case when the target system
		// makes a correct output but faultily adds extra characters
		// afterwards.
		sol.setLimit(serial_goldenrun.size() + 1);
	}

	if (cmd[WRITE_MEM_TEXT]) {
		m_log << "Catch writes to text segment from " << hex << minimal_ip << " to " << maximal_ip << std::endl;
		enabled_mem_text = true;

		l_mem_text.setWatchAddress(minimal_ip);
		l_mem_text.setTriggerAccessType(MemAccessEvent::MEM_WRITE);
		l_mem_text.setWatchWidth(maximal_ip - minimal_ip);
	}

	if (cmd[WRITE_MEM_OUTERSPACE]) {
		m_log << "Catch writes to outerspace from " << hex << " from " << maximal_data << std::endl;
		enabled_mem_outerspace = true;

		l_mem_outerspace.setWatchAddress(maximal_data);
		l_mem_outerspace.setTriggerAccessType(MemAccessEvent::MEM_WRITE);
		l_mem_outerspace.setWatchWidth(0xfffffff0 - maximal_data);
	}

	if (cmd[TRAP]) {
		m_log << "Catch all traps" << endl;
		enabled_trap = true;
	}

	if (cmd[STATE_DIR]) {
		std::string value(cmd[STATE_DIR].first()->arg);
		m_state_dir = value;
		m_log << "Set state dir to " << value << endl;
	}

	if (cmd[TIMEOUT]) {
		std::string value(cmd[TIMEOUT].first()->arg);
		std::stringstream ss(value);
		ss >> m_Timeout;
		if (ss.bad()) {
			m_log << "Could not parse --timeout argument" << endl;
			return false; // Initialization failed
		}
		l_timeout.setTimeout(m_Timeout);
		enabled_timeout = true;
		m_log << "Enabled Experiment Timeout of " << dec << m_Timeout << " microseconds" << endl;
	}

	for (std::map<std::string, CommandLine::option_handle>::iterator it = option_handles.begin();
		 it != option_handles.end(); ++it) {
		if (cmd[option_handles[it->first]]) {
			option::Option *opt = cmd[option_handles[it->first]].first();
			while (opt != 0) {
				parseSymbols(std::string(opt->arg), end_marker_groups[it->first]);
				opt = opt->next();
			}
		}
	}

	return true; // Everything OK
}


bool GenericExperiment::cb_before_fast_forward()
{
	if (serial_enabled) {
		// output may already appear *before* FI
		simulator.addFlow(&sol);
	}
	return true;
}

bool GenericExperiment::cb_before_resume() {
	if (enabled_trap)
		simulator.addListener(&l_trap);

	if (enabled_mem_text)
		simulator.addListener(&l_mem_text);

	if (enabled_mem_outerspace) {
		std::cout << "enabled mem outerspace " << endl;
		simulator.addListener(&l_mem_outerspace);
	}

	if (enabled_timeout)
		simulator.addListener(&l_timeout);

	for (std::set<BaseListener *>::iterator it = end_markers.begin();
		 it != end_markers.end(); ++it) {
		simulator.addListener(*it);
	}

	return true; // everything OK
}

void GenericExperiment::cb_after_resume(fail::BaseListener *event) {
	GenericExperimentMessage_Result * result = static_cast<GenericExperimentMessage_Result *>(this->get_current_result());

	// Record the crash time
	result->set_crash_time(simulator.getTimerTicks());

	if (event == &l_timeout) {
		handleEvent(*result, result->TIMEOUT, m_Timeout);
	}  else if (event == &l_trap) {
		handleEvent(*result, result->TRAP, l_trap.getTriggerNumber());
	} else if (event == &l_mem_text) {
		handleEvent(*result, result->WRITE_TEXTSEGMENT,
					l_mem_text.getTriggerAddress());

	} else if (event == &l_mem_outerspace){
		handleEvent(*result, result->WRITE_OUTERSPACE,
					l_mem_outerspace.getTriggerAddress());

		//////////////////////////////////////////////////
		// End Marker Groups
		//////////////////////////////////////////////////
	} else if (OK_marker.find(event) != OK_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->OK_MARKER, symbol->getAddress());

		// check experiment's data for SDC
		if (serial_enabled) {
			// compare golden run to experiment
			std::string serial_experiment = sol.getOutput();
			if ( ! (serial_experiment.size() == serial_goldenrun.size()
					&& equal(serial_experiment.begin(), serial_experiment.end(), serial_goldenrun.begin())) ) {
				handleEvent(*result, result->SDC, 0);
			}
		}

	} else if (FAIL_marker.find(event) != FAIL_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->FAIL_MARKER, symbol->getAddress());

	} else if (DETECTED_marker.find(event) != DETECTED_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->DETECTED_MARKER, symbol->getAddress());

	} else if (GROUP1_marker.find(event) != GROUP1_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->GROUP1_MARKER, symbol->getAddress());

	} else if (GROUP2_marker.find(event) != GROUP2_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->GROUP2_MARKER, symbol->getAddress());

	} else if (GROUP3_marker.find(event) != GROUP3_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->GROUP3_MARKER, symbol->getAddress());

	} else if (GROUP4_marker.find(event) != GROUP4_marker.end()) {
		const ElfSymbol *symbol = listener_to_symbol[event];
		handleEvent(*result, result->GROUP4_MARKER, symbol->getAddress());

	} else {
		handleEvent(*result, result->UNKNOWN, 0);
	}

	// remove and reset serial logger even if this run was not "OK"
	if (serial_enabled) {
		simulator.removeFlow(&sol);
		sol.resetOutput();
	}
}
