#include <iostream>

#include "experiment.hpp"
#include "experimentInfo.hpp"

#include "util/CommandLine.hpp"

using namespace std;
using namespace fail;

void L4SysExperiment::parseOptions(L4SysConfig &conf) {
	CommandLine &cmd = CommandLine::Inst();

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
	CommandLine::option_handle OPT_FILTER_INSTRUCTIONS = 
		cmd.addOption("", "filter_instructions", Arg::Optional, "--filter_instructions \t define L4SYS_FILTER_INSTRUCTIONS");
	CommandLine::option_handle OPT_NUMINSTR = 
		cmd.addOption("", "numinstr", Arg::Optional, "--numinstr \t define L4SYS_NUMINSTR");
	CommandLine::option_handle OPT_TOTINSTR = 
		cmd.addOption("", "totinstr", Arg::Optional, "--totinstr \t define L4SYS_TOTINSTR");
	CommandLine::option_handle OPT_EMUL_IPS = 
		cmd.addOption("", "bochs_ips", Arg::Optional, "--bochs_ips \t define L4SYS_BOCHS_IPS");
	CommandLine::option_handle OPT_STATE_FOLDER = 
		cmd.addOption("", "state_folder", Arg::Optional, "--state_folder \t define L4SYS_STATE_FOLDER");
	CommandLine::option_handle OPT_INSTRUCTION_LIST = 
		cmd.addOption("", "instruction_list", Arg::Optional, "--instruction_list \t define L4SYS_INSTRUCTION_LIST");
	CommandLine::option_handle OPT_ALU_INSTRUCTIONS = 
		cmd.addOption("", "alu_instructions", Arg::Optional, "--alu_instructions \t define L4SYS_ALU_INSTRUCTIONS");
	CommandLine::option_handle OPT_CORRECT_OUTPUT = 
		cmd.addOption("", "golden_run", Arg::Optional, "--correct_output \t define L4SYS_CORRECT_OUTPUT");
	CommandLine::option_handle OPT_FILTER = 
		cmd.addOption("", "filter", Arg::Optional, "--filter \t define L4SYS_FILTER");
	CommandLine::option_handle OPT_TRACE = 
		cmd.addOption("", "trace", Arg::Optional, "--trace \t define outputfile for trace (default trace.pb)");
	
	if (!cmd.parse()) { 
		cerr << "Error parsing arguments." << endl;
		simulator.terminate(1);
	} else if (cmd[HELP]) {
		cmd.printUsage();
		simulator.terminate(0);
	}

	if (cmd[OPT_MAX_INSTR_BYTES]) {
		 conf.max_instr_bytes = strtol(cmd[OPT_MAX_INSTR_BYTES].arg, NULL, 16);
	} else {
		 conf.max_instr_bytes = MAX_INSTR_BYTES;
	}

	if (cmd[OPT_ADDRESS_SPACE]) {
		 conf.address_space = strtol(cmd[OPT_ADDRESS_SPACE].arg, NULL, 16);
	} else {
		 conf.address_space = L4SYS_ADDRESS_SPACE;
	}

	if (cmd[OPT_ADDRESS_SPACE_TRACE]) {
		 conf.address_space_trace = strtol(cmd[OPT_ADDRESS_SPACE_TRACE].arg, NULL, 16);
	} else {
		 conf.address_space_trace = L4SYS_ADDRESS_SPACE_TRACE;
	}

	if (cmd[OPT_FUNC_ENTRY]) {
		 conf.func_entry = strtol(cmd[OPT_FUNC_ENTRY].arg, NULL, 16);
	} else {
		 conf.func_entry = L4SYS_FUNC_ENTRY;
	}

	if (cmd[OPT_FUNC_EXIT]) {
		 conf.func_exit = strtol(cmd[OPT_FUNC_EXIT].arg, NULL, 16);
	} else {
		 conf.func_exit = L4SYS_FUNC_EXIT;
	}

	if (cmd[OPT_FILTER_ENTRY]) {
		 conf.filter_entry = strtol(cmd[OPT_FILTER_ENTRY].arg, NULL, 16);
	} else {
		 conf.filter_entry = L4SYS_FILTER_ENTRY;
	}

	if (cmd[OPT_FILTER_EXIT]) {
	 conf.filter_exit = strtol(cmd[OPT_FILTER_EXIT].arg, NULL, 16);
	} else {
	 conf.filter_exit = L4SYS_FILTER_EXIT;
	}

	if (cmd[OPT_BREAK_BLINK]) {
	 conf.break_blink = strtol(cmd[OPT_BREAK_BLINK].arg, NULL, 16);
	} else {
	 conf.break_blink = L4SYS_BREAK_BLINK;
	}

	if (cmd[OPT_BREAK_LONGJMP]) {
	 conf.break_longjmp = strtol(cmd[OPT_BREAK_LONGJMP].arg, NULL, 16);
	} else {
	 conf.break_longjmp = L4SYS_BREAK_LONGJMP;
	}

	if (cmd[OPT_BREAK_EXIT]) {
	 conf.break_exit = strtol(cmd[OPT_BREAK_EXIT].arg, NULL, 16);
	} else {
	 conf.break_exit = L4SYS_BREAK_EXIT;
	}

	if (cmd[OPT_FILTER_INSTRUCTIONS]) {
	 conf.filter_instructions = strtol(cmd[OPT_FILTER_INSTRUCTIONS].arg, NULL, 16);
	} else {
	 conf.filter_instructions = L4SYS_FILTER_INSTRUCTIONS;
	}

	if (cmd[OPT_NUMINSTR]) {
	 conf.numinstr = strtol(cmd[OPT_NUMINSTR].arg, NULL, 16);
	} else {
	 conf.numinstr = L4SYS_NUMINSTR;
	}

	if (cmd[OPT_TOTINSTR]) {
	 conf.totinstr = strtol(cmd[OPT_TOTINSTR].arg, NULL, 16);
	} else {
	 conf.totinstr = L4SYS_TOTINSTR;
	}

	if (cmd[OPT_EMUL_IPS]) {
	 conf.emul_ips = strtol(cmd[OPT_EMUL_IPS].arg, NULL, 16);
	} else {
	 conf.emul_ips = L4SYS_BOCHS_IPS;
	}

	if (cmd[OPT_STATE_FOLDER]) {
	 conf.state_folder = std::string(cmd[OPT_STATE_FOLDER].arg);
	} else {
	 conf.state_folder = L4SYS_STATE_FOLDER;
	}

	if (cmd[OPT_INSTRUCTION_LIST]) {
	 conf.instruction_list = std::string(cmd[OPT_INSTRUCTION_LIST].arg);
	} else {
	 conf.instruction_list = L4SYS_INSTRUCTION_LIST;
	}

	if (cmd[OPT_ALU_INSTRUCTIONS]) {
		 conf.alu_instructions = std::string(cmd[OPT_ALU_INSTRUCTIONS].arg);
	} else {
		 conf.alu_instructions = L4SYS_ALU_INSTRUCTIONS;
	}

	if (cmd[OPT_CORRECT_OUTPUT]) {
		 conf.golden_run = std::string(cmd[OPT_CORRECT_OUTPUT].arg);
	} else {
		 conf.golden_run = L4SYS_CORRECT_OUTPUT;
	}

	if (cmd[OPT_FILTER]) {
		 conf.filter = std::string(cmd[OPT_FILTER].arg);
	} else {
		 conf.filter = L4SYS_FILTER;
	}

	if (cmd[OPT_TRACE]) {
		 conf.trace = std::string(cmd[OPT_TRACE].arg);
	} else {
		 conf.trace = std::string("trace.pb");
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
