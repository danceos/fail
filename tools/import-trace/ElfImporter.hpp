#ifndef __OBJDUMP_IMPORTER_H__
#define __OBJDUMP_IMPORTER_H__

#include "Importer.hpp"
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#include "util/CommandLine.hpp"

/**
	The ElfImporter is not a real trace importer, but we locate it
	into the import-trace utility, since here the infrastructure is
	already in place to import things related to an elf binary into
	the database.

	The ElfImporter calls objdump and dissassembles an elf binary
	and imports the results into the database
*/
class ElfImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

	fail::CommandLine::option_handle OBJDUMP;

	bool import_with_objdump(const std::string &objdump_binary);
	bool evaluate_objdump_line(const std::string &line);

	/* Imports a single instruction into the objdump table */
	bool import_instruction(fail::address_t addr, char opcode[16], int opcode_length,
								const std::string &instruction, const std::string &comment);

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									 Trace_Event &ev) { return true; }
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
									  Trace_Event &ev) { return true; }

	virtual void open_unused_ec_intervals() {
		/* empty, Memory Map has a no meaning in this importer */
	}

public:
	ElfImporter() : Importer() {};

	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool cb_commandline_init();
	virtual bool create_database();
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();
};

#endif
