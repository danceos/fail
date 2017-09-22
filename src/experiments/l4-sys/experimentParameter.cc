#include <fstream>
#include <iostream>
#include <list>
#include <stdlib.h>

#include "experiment.hpp"

#include "util/CommandLine.hpp"

using namespace std;
using namespace fail;

#define EXPERIMENT_CONF "experiment.conf"


static void parameterMissing(fail::Logger log, string c) {
	log << "Error: Missing config parameter (" << c << ")" << endl;
	simulator.terminate(1);
}

int L4SysExperiment::updateConfig(string parameter, string value) {

	bool replaced = false;

	std::list<std::string> buf;

	ifstream is(EXPERIMENT_CONF);
	
	if (is) {
		while(!is.eof()) {
			string tmp;
			getline(is, tmp);
			if( !tmp.compare(0, parameter.length() + 1, string(parameter + "=")) ) {
				buf.push_back(string(parameter + "=" + value) );
				replaced = true;
			} else {
				buf.push_back(tmp);
			}
		}
	} else {
		cerr << "Open config file failed" << endl;
		return -1;
	}
	is.close();

	ofstream os(EXPERIMENT_CONF);

	if (os) {
		if(!replaced) 
			os << parameter << "=" << value << endl;

		while(!buf.empty()) {
			os << buf.front() << endl;
			buf.pop_front();
		}
	} else {
		cerr << "Open config file for update failed" << endl;
		return -1;
	}

	return 0;
}

void L4SysExperiment::parseOptions(L4SysConfig &conf) {
	CommandLine &cmd = CommandLine::Inst();

	ifstream fin(EXPERIMENT_CONF);

	//Currently we don't delete the parameterstring to avoid deling ptr in the cmd
	//Nevertheless, we shouldn't need them, so deleting should be safe

	//We interpret the file content as parameters to our main function, so we don't
	//need an own parser. Program parameters appear earlier in the parameter list
	//and will overwrite the parameters from the config file.
	while(fin.good()) {
		string buffer;
		string prefix("--");

		getline(fin, buffer, '\n');

		if( !(strncmp(buffer.c_str(),"#",1) == 0) && 
		    !(strncmp(buffer.c_str()," ",1) == 0 ) &&
		    !(strncmp(buffer.c_str(),"",1) == 0 ) ) {
			string t(prefix + buffer);
			//Workarround, if we just alloc t.length some of the arguments values 
			//disappear
			char *c = (char *)  malloc(t.length() * 2);
			strcpy(c, t.c_str());
			cmd.add_args( c );
		}
	}

	cout << "end of config file" << endl;

	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");
	CommandLine::option_handle STEP =
		cmd.addOption("s", "step", Arg::Optional, "-s,--step \tSpecify preparation step, without this argumnt fail-client start in experiment mode (cr3: get CR3, cc: Create Checkpoint, it: collect instruction trace, gr: golden run, all: do the whole preparation)");

	CommandLine::option_handle OPT_MAX_INSTR_BYTES = 
		cmd.addOption("", "max_instr_bytes", Arg::Optional, "--max_instr_bytes \t define MAX_INSTR_BYTES");
	CommandLine::option_handle OPT_ADDRESS_SPACE = 
		cmd.addOption("", "address_space", Arg::Optional, "--address_space \t define L4SYS_ADDRESS_SPACE");
	CommandLine::option_handle OPT_ADDRESS_SPACE_TRACE = 
		cmd.addOption("", "address_space_trace", Arg::Optional, "--address_space_trace \t define L4SYS_ADDRESS_SPACE_TRACE");
	CommandLine::option_handle OPT_FUNC_ENTRY = 
		cmd.addOption("", "func_entry", Arg::Optional, "--func_entry \t define L4SYS_FUNC_ENTRY");
	CommandLine::option_handle OPT_FUNC_EXIT = 
		cmd.addOption("", "func_exit", Arg::Optional, "--func_exit \t define L4SYS_FUNC_EXIT");
	CommandLine::option_handle OPT_FILTER_ENTRY = 
		cmd.addOption("", "filter_entry", Arg::Optional, "--filter_entry \t define L4SYS_FILTER_ENTRY");
	CommandLine::option_handle OPT_FILTER_EXIT = 
		cmd.addOption("", "filter_exit", Arg::Optional, "--filter_exit \t define L4SYS_FILTER_EXIT");
	CommandLine::option_handle OPT_BREAK_BLINK = 
		cmd.addOption("", "break_blink", Arg::Optional, "--break_blink \t define L4SYS_BREAK_BLINK");
	CommandLine::option_handle OPT_BREAK_LONGJMP = 
		cmd.addOption("", "break_longjmp", Arg::Optional, "--break_longjmp \t define L4SYS_BREAK_LONGJMP");
	CommandLine::option_handle OPT_BREAK_EXIT = 
		cmd.addOption("", "break_exit", Arg::Optional, "--break_exit \t define L4SYS_BREAK_EXIT");
	CommandLine::option_handle OPT_NUMINSTR = 
		cmd.addOption("", "numinstr", Arg::Optional, "--numinstr \t define L4SYS_NUMINSTR");
	CommandLine::option_handle OPT_TOTINSTR = 
		cmd.addOption("", "totinstr", Arg::Optional, "--totinstr \t define L4SYS_TOTINSTR");
	CommandLine::option_handle OPT_EMUL_IPS = 
		cmd.addOption("", "emul_ips", Arg::Optional, "--emul_ips \t define L4SYS_BOCHS_IPS");
	CommandLine::option_handle OPT_STATE_FOLDER = 
		cmd.addOption("", "state_folder", Arg::Optional, "--state_folder \t define L4SYS_STATE_FOLDER");
	CommandLine::option_handle OPT_INSTRUCTION_LIST = 
		cmd.addOption("", "instruction_list", Arg::Optional, "--instruction_list \t define L4SYS_INSTRUCTION_LIST");
	CommandLine::option_handle OPT_CORRECT_OUTPUT = 
		cmd.addOption("", "golden_run", Arg::Optional, "--correct_output \t define L4SYS_CORRECT_OUTPUT");
	CommandLine::option_handle OPT_FILTER = 
		cmd.addOption("", "filter", Arg::Optional, "--filter \t define L4SYS_FILTER");
	CommandLine::option_handle OPT_TRACE = 
		cmd.addOption("", "trace", Arg::Optional, "--trace \t define outputfile for trace (default trace.pb)");
	CommandLine::option_handle OPT_CAMPAIN_SERVER = 
		cmd.addOption("", "campain_server", Arg::Optional, "--campain_server \t specify the hostname of the campain server (default localhost)");

	
	if (!cmd.parse()) { 
		cerr << "Error parsing arguments." << endl;
		simulator.terminate(1);
	} else if (cmd[HELP]) {
		cmd.printUsage();
		simulator.terminate(0);
	}

	if (cmd[OPT_MAX_INSTR_BYTES]) {
		conf.max_instr_bytes = strtol(cmd[OPT_MAX_INSTR_BYTES].arg, NULL, 10);
		log << "max_instr_bytes: "<< dec << conf.max_instr_bytes << endl;
	} else {
		 parameterMissing(log, "max_instr_bytes");
	}

	if (cmd[OPT_ADDRESS_SPACE]) {
		conf.address_space = strtol(cmd[OPT_ADDRESS_SPACE].arg, NULL, 16);
		log << "address_space: "<< hex << conf.address_space << endl;

		//Confi address_space=0 means no address space filtering
		if (conf.address_space == 0)
			conf.address_space = fail::ANY_ADDR;
	} else {
		conf.address_space = fail::ANY_ADDR;
	}

	if (cmd[OPT_ADDRESS_SPACE_TRACE]) {
		conf.address_space_trace = strtol(cmd[OPT_ADDRESS_SPACE_TRACE].arg, NULL, 16);
		if( conf.address_space_trace == 0 )
			conf.address_space_trace = conf.address_space;
		else
			log << "address_space_trace: "<< hex << conf.address_space_trace << endl;
	} else {
			conf.address_space_trace = conf.address_space;
	}


	if (conf.address_space_trace == 0)
		conf.address_space_trace = fail::ANY_ADDR;

	if (cmd[OPT_FUNC_ENTRY]) {
		conf.func_entry = strtol(cmd[OPT_FUNC_ENTRY].arg, NULL, 16);
		log << "func_entry: "<< hex << conf.func_entry << endl;
	} else{
		parameterMissing(log, "func_entry");
	}

	if (cmd[OPT_FUNC_EXIT]) {
		conf.func_exit = strtol(cmd[OPT_FUNC_EXIT].arg, NULL, 16);
		log << "func_exit: "<< hex << conf.func_exit << endl;
	} else {
		parameterMissing(log, "func_exit");
	}

	if (cmd[OPT_FILTER_ENTRY]) {
		conf.filter_entry = strtol(cmd[OPT_FILTER_ENTRY].arg, NULL, 16);
	 	log << "filter_entry: "<< hex << conf.filter_entry << endl;
	} else {
		 conf.filter_entry = conf.func_entry;
	}

	if (cmd[OPT_FILTER_EXIT]) {
		conf.filter_exit = strtol(cmd[OPT_FILTER_EXIT].arg, NULL, 16);
		log << "filter_exit: "<< hex << conf.filter_exit << endl;
	} else {
		conf.filter_exit = conf.func_exit;
	}

	if (cmd[OPT_BREAK_BLINK]) {
		conf.break_blink = strtol(cmd[OPT_BREAK_BLINK].arg, NULL, 16);
		log << "break_blink: "<< hex << conf.break_blink << endl;
	} else {
		conf.break_blink = 0;
	}

	if (cmd[OPT_BREAK_LONGJMP]) {
		conf.break_longjmp = strtol(cmd[OPT_BREAK_LONGJMP].arg, NULL, 16);
		log << "break_longjmp: "<< hex << conf.break_longjmp << endl;
	} else {
		conf.break_longjmp = 0;
	}

	if (cmd[OPT_BREAK_EXIT]) {
		conf.break_exit = strtol(cmd[OPT_BREAK_EXIT].arg, NULL, 16);
		log << "break_exit: "<< hex << conf.break_exit << endl;
	} else {
		conf.break_exit = 0;
	}

	if (cmd[OPT_NUMINSTR]) {
		conf.numinstr = strtol(cmd[OPT_NUMINSTR].arg, NULL, 10);
		log << "numinstr: "<< dec << conf.numinstr << endl;
	} else {
		parameterMissing(log, "numinstr");
	}

	if (cmd[OPT_TOTINSTR]) {
		conf.totinstr = strtol(cmd[OPT_TOTINSTR].arg, NULL, 10);
		log << "totinstr: "<< dec << conf.totinstr << endl;
	} else {
		parameterMissing(log, "totinstr");
	}

	if (cmd[OPT_EMUL_IPS]) {
		conf.emul_ips = strtol(cmd[OPT_EMUL_IPS].arg, NULL, 10);
		log << "emul_ips = " << dec << conf.emul_ips << endl;
	} else {
		parameterMissing(log, "emul_ips");
	}

	if (cmd[OPT_STATE_FOLDER]) {
		conf.state_folder = std::string(cmd[OPT_STATE_FOLDER].arg);
		log << "state_folder: "<< conf.state_folder << endl;
	} else {
		conf.state_folder = "l4sys.state";
	}

	if (cmd[OPT_INSTRUCTION_LIST]) {
		conf.instruction_list = std::string(cmd[OPT_INSTRUCTION_LIST].arg);
		log << "instruction_list: "<< conf.instruction_list << endl;
	} else {
		conf.instruction_list = "ip.list";
	}

	if (cmd[OPT_CORRECT_OUTPUT]) {
		conf.golden_run = std::string(cmd[OPT_CORRECT_OUTPUT].arg);
		log << "golden_run: "<< conf.golden_run << endl;
	} else {
		conf.golden_run = "golden.out";
	}

	if (cmd[OPT_FILTER]) {
		conf.filter = std::string(cmd[OPT_FILTER].arg);
		log << "filter: "<< conf.filter << endl;
	} else {
		 conf.filter = "filter.list";
	}

	if (cmd[OPT_TRACE]) {
		conf.trace = std::string(cmd[OPT_TRACE].arg);
	 	log << "trace: "<< conf.trace << endl;
	} else {
		conf.trace = "trace.pb";
	}

	if (cmd[OPT_CAMPAIN_SERVER]) {
		conf.campain_server = std::string(cmd[OPT_CAMPAIN_SERVER].arg);
	 	log << "campain_server: "<< conf.campain_server << endl;
	} else {
		conf.campain_server = "localhost";
	}

	if (cmd[STEP]) {
		if (!std::string("cr3").compare(cmd[STEP].arg)  ) {
			log << "calculate cr3" << endl;
			conf.step = L4SysConfig::GET_CR3;
		} else if (!std::string("cc").compare(cmd[STEP].arg) ) { 
			log << "Create Checkpoint" << endl;
			conf.step = L4SysConfig::CREATE_CHECKPOINT;
		} else if (!std::string("it").compare(cmd[STEP].arg) ) { 
			log << "collect instruction trace" << endl;
			conf.step = L4SysConfig::COLLECT_INSTR_TRACE;
		} else if (!std::string("gr").compare(cmd[STEP].arg) ) {  
			log << "golden run" << endl;
			conf.step = L4SysConfig::GOLDEN_RUN;
		} else if (!std::string("all").compare(cmd[STEP].arg) ) {  
			log << "do all preparation steps" << endl;
			conf.step = L4SysConfig::FULL_PREPARATION;
		} else {
			cerr << "Wrong argument for option '--step'" << endl;
			simulator.terminate(1);
		}
	} else {
		conf.step = L4SysConfig::NO_PREP;
	}
}
