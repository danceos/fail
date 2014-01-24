#ifndef __OBJDUMP_IMPORTER_H__
#define __OBJDUMP_IMPORTER_H__

#include <string>
#include <list>
#include "libdwarf.h"
#include "libelf.h"

#include "Importer.hpp"
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#include "util/CommandLine.hpp"
#include "util/DwarfReader.hpp"

/**
	The ElfImporter is not a real trace importer, but we locate it
	into the import-trace utility, since here the infrastructure is
	already in place to import things related to an elf binary into
	the database.

	The ElfImporter calls objdump and dissassembles an elf binary
	and imports the results into the database

	In addition, debugging information can be imported:

	If the --sources option is set, the source files will be imported
	into the database. Only the files that were actually used in the
	elf binary will be imported.

	If the --debug option is set, the line number table of the elf binary will
	be imported into the database. The information will be stored in the
	"mapping" table.
*/
class ElfImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

	fail::CommandLine::option_handle OBJDUMP;
	fail::CommandLine::option_handle SOURCECODE;
	fail::CommandLine::option_handle DEBUGINFO;
	fail::DwarfReader dwReader;

	bool import_with_objdump(const std::string &objdump_binary);
	bool evaluate_objdump_line(const std::string &line);

	/* Imports a single instruction into the objdump table */
	bool import_instruction(fail::address_t addr, char opcode[16], int opcode_length,
								const std::string &instruction, const std::string &comment);

	bool import_source_files(const std::string& fileName,std::list<std::string>& lines);
	bool import_source_code(std::string fileName);
	bool import_mapping(std::string fileName);

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
