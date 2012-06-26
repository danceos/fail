#ifndef __L4SYS_EXPERIMENT_HPP__
  #define __L4SYS_EXPERIMENT_HPP__

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
// not implemented yet
// #include "aluinstr.hpp"

class L4SysExperimentData;

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
	fail::BaseEvent* waitIOOrOther(bool clear_output);
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
	 * May be obsolete. Not supplying docu until I am sure whether I need to
	 */
	void readFromFileToVector(std::ifstream &file, std::vector<struct __trace_instr_type> &instr_list);
	/**
	 * Overwrites one Bochs instruction with another.
	 * @param dest the instruction to copy to
	 * @param src the instruction to copy from
	 */
	void changeBochsInstruction(bxInstruction_c *dest, bxInstruction_c *src);
};

#endif // __L4SYS_EXPERIMENT_HPP__
