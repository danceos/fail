#ifndef __L4SYS_EXPERIMENT_HPP__
  #define __L4SYS_EXPERIMENT_HPP__

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"

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

typedef std::vector<TraceInstr> TraceVector;

class L4SysExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	L4SysExperiment() : m_jc("localhost") {}
	bool run();
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
	 * Parses a raw instruction into a bxInstruction_c structure.
	 * This simple version of the function is taken from Bochs
	 * where it is currently disabled due to the TRACE_CACHE option,
	 * and has been modified to fit the needs of instruction modification.
	 * @param instance a pointer to the current Bochs CPU
	 * @param instr a pointer to the address the instruction is fetched from
	 * @param iStorage an outgoing value which contains the parsed instruction
	 * @returns \a false if the instruction continued on the following page in memory
	 */
	bx_bool fetchInstruction(BX_CPU_C *instance, const Bit8u *instr, bxInstruction_c *iStorage);
	/**
	 * Write out the injection parameters to the given logger.
	 * @param log A reference to the Logger object
	 * @param param The experiment parameter object to log data from
	 */
	void logInjection(fail::Logger &log, const L4SysExperimentData &param);
	/**
	 * Proceeds by one single instruction.
	 */
	void singleStep();
	/**
	 * Injects a new instruction into the Bochs instruction stream and restores the previous one
	 * @param oldInstr address of the instruction to be replaced
	 * @param newInstr address of the instruction to replace it with
	 */
	void injectInstruction(bxInstruction_c *oldInstr, bxInstruction_c *newInstr);
	/**
	 * Calculate the timeout of the current workload in milliseconds.
	 */
	unsigned calculateTimeout(unsigned instr_left);
};

#endif // __L4SYS_EXPERIMENT_HPP__
