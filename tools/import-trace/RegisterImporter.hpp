#ifndef __REGISTER_IMPORTER_H__
#define __REGISTER_IMPORTER_H__


#include "util/CommandLine.hpp"
#include "Importer.hpp"

#include "util/llvmdisassembler/LLVMDisassembler.hpp"


class RegisterImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

	bool addRegisterTrace(fail::simtime_t curtime, instruction_count_t instr,
						  Trace_Event &ev,
						  const fail::LLVMtoFailTranslator::reginfo_t &info,
						  char access_type);

	fail::CommandLine::option_handle NO_GP, FLAGS, IP, NO_SPLIT;
	bool do_gp, do_flags, do_ip, do_split_registers;

public:
	RegisterImporter() : Importer(), do_gp(true), do_flags(false), do_ip(false),
						 do_split_registers(true) {}
	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool cb_commandline_init();

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev) {
		/* ignore on purpose */
		return true;
	}

	virtual void open_unused_ec_intervals() {
		/* empty, Memory Map has a different meaning in this importer */
	}
};

#endif
