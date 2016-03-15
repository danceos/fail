#ifndef __L4SYS_EXPERIMENT_HPP__
  #define __L4SYS_EXPERIMENT_HPP__

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "sal/Listener.hpp"

//#include "l4sys.pb.h"
struct L4SysProtoMsg_Result;

class L4SysExperimentData;

/**
 * A data type containing an instruction trace entry.
 */
typedef struct TraceInstrType {
	fail::address_t trigger_addr; //!< the instruction pointer of the observed instruction
	/**
	 * counts how often this instruction has been called
	 * if you want to call exactly this instruction, e.g. in a loop,
	 * you need to ignore the breakpoint \c bp_counter - 1 times
	 */
	unsigned bp_counter;
} TraceInstr;

class L4SysExperiment : public fail::ExperimentFlow {
private:
	class L4SysConfig;
	fail::JobClient m_jc; //!< the job client connecting to the campaign server
	fail::Logger log; //<! the logger
	L4SysExperimentData *param; //<! the parameter set currently in use by the client
	std::string currentOutput; //<! output for the current experiment run
public:
	L4SysExperiment();
	~L4SysExperiment();
	bool run();
protected:
	/**
	 * Frees all attached resources and terminates the simulator.
	 * @param reason the exit reason, i.e. exit code, passed on to simulator::terminate
	 */
	void terminate(int reason);
	/**
	 * Frees all resources allocated by this object.
	 * This function is called by terminate as well as the destructor.
	 */
	inline void destroy();
private:
	/**
	 * Sanitises the output string of the serial device monitored.
	 * @param a string containing special ASCII characters
	 * @returns a byte-stuffed version of the given string
	 */
	std::string sanitised(const std::string &in_str);
	/**
	 * Waits for events and simultaneously logs output from the serial console
	 * @param clear_output if true, the output logged so far is deleted, thus the buffer is reset (cleared)
	 * @returns the event returned by waitAny, as long as it did not log output
	 */
	fail::BaseListener* waitIOOrOther(bool clear_output);
	/**
	 * Calculates the address where Bochs will read the current instruction from.
	 * This code is copied from various Bochs methods and should be reviewed as
	 * soon as a new Bochs version is introduced.
	 * @returns a pointer to the memory region containing the current Bochs instruction
	 */
	const Bit8u *calculateInstructionAddress();
	/**
	 * A function necessary for Bochs internal address translation
	 * @returns a value for Bochs' eipBiased variable
	 */
	Bit32u eipBiased();
	/**
	 * Calculate the timeout of the current workload in milliseconds.
	 */
	unsigned calculateTimeout(unsigned instr_left, unsigned ips);
	/**
	 * Send back the experiment parameter set with a description of the error.
	 */
	void terminateWithError(std::string details, int reason, L4SysProtoMsg_Result*);
	/**
	 * Run until reaching the entry point of the experiment and save 
	 * state.
	 */
	void runToStart(fail::BPSingleListener *bp);
	/**
	 * Collect list of executed instructions, considering instruction
	 * filtering if configured.
	 */
	void collectInstructionTrace(fail::BPSingleListener* bp);
	/**
	 * Perform the golden run.
	 */
	void goldenRun(fail::BPSingleListener* bp);

	/**
	 * Doing fault injection expiriments.
	 */
	void doExperiments(fail::BPSingleListener *bp);

	/**
	 * Check that all required setup has been done before an experiment run.
	 */
	void validatePrerequisites(std::string state, std::string output);

	/**
	 * Load job parameters for an experiment.
	 */
	void getJobParameters();

	fail::BaseListener* afterInjection(L4SysProtoMsg_Result* res);

	/**
	 * Read the golden run output into the target string.
	 */
	void readGoldenRun(std::string& target, std::string golden_run);

	/*
	 * Prepare memory experiment. Creates a breakpoint to run until the
	 * injection location.
	 */
	fail::BPSingleListener* prepareMemoryExperiment(int ip, int offset, int dataAddress);

	/*
	 * Prepare register experiment. Creates a breakpoint to run until the
	 * injection location.
	 */
	fail::BPSingleListener* prepareRegisterExperiment(int ip, int offset, int dataAddress);
	
	/**
	 * Perform memory bit flip at (address, bit).
	 */
	bool doMemoryInjection(int address, int bit);
	
	/**
	 * Perform register bit flip in the specified (register, bit)
	 * combination.
	 */
	void doRegisterInjection(int regDesc, int bit);

	void setupFilteredBreakpoint(fail::BPSingleListener* bp, int instOffset, std::string instr_list);

	/**
	 * Updates a parameter of the config file or adds a new one
	 * if the parameter was not specified.
	 */
	int updateConfig(std::string parameter, std::string value);

	/**
	 * Adds the value of the config file to the parameter list and 
	 * parse the parameter list. This function makes use of the
	 * CommandLine Parser from the FAIL* framework.
	 */
	void parseOptions(L4SysConfig&);

	/**
	 * Configuration Setup.
	 */
	class L4SysConfig {
		public:
			unsigned long int max_instr_bytes;
			unsigned long int address_space;
			unsigned long int address_space_trace;
			unsigned long int func_entry;
			unsigned long int func_exit;
			unsigned long int filter_entry;
			unsigned long int filter_exit;
			unsigned long int break_blink;
			unsigned long int break_longjmp;
			unsigned long int break_exit;
			unsigned long int filter_instructions;
			unsigned long int emul_ips;
			std::string state_folder;
			std::string instruction_list;
			std::string alu_instructions;
			std::string golden_run;
			std::string filter;
			std::string trace;
			std::string campain_server;

			unsigned long int numinstr;
			unsigned long int totinstr;

			enum {NO_PREP, GET_CR3, CREATE_CHECKPOINT, COLLECT_INSTR_TRACE, GOLDEN_RUN, FULL_PREPARATION} step;
	};

private:
	L4SysConfig conf;
};


#endif // __L4SYS_EXPERIMENT_HPP__
