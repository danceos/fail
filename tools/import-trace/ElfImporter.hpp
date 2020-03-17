#ifndef __OBJDUMP_IMPORTER_H__
#define __OBJDUMP_IMPORTER_H__

#include <string>
#include <list>
#include "libdwarf.h"
#include "libelf.h"

#include "Importer.hpp"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#include "util/capstonedisassembler/CapstoneDisassembler.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif

#include "util/CommandLine.hpp"
#include "util/DwarfReader.hpp"

/**
	The ElfImporter is not a real trace importer, but we locate it
	into the import-trace utility, since here the infrastructure is
	already in place to import things related to an ELF binary into
	the database.

	The ElfImporter calls objdump to dissassemble an ELF binary, and
	imports the results into the database.

	In addition, debugging information can be imported: If the --sources
	option is set, the source files will be imported into the database.
	Only the files that were actually used in the ELF binary will be imported.
	Additionally, the line number table of the ELF binary will be imported
	into the database.
*/
class ElfImporter : public Importer {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	std::unique_ptr<fail::CapstoneDisassembler> disas;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	std::unique_ptr<fail::LLVMDisassembler> disas;
#endif

	fail::CommandLine::option_handle OBJDUMP;
	fail::CommandLine::option_handle SOURCECODE;
	fail::CommandLine::option_handle DEBUGINFO;
	fail::DwarfReader dwReader;

	bool import_with_objdump(const std::string &objdump_binary);
	bool evaluate_objdump_line(const std::string &line);

	/* Imports a single instruction into the objdump table */
	bool import_instruction(fail::address_t addr, char opcode[16], int opcode_length,
		const std::string &instruction, const std::string &comment);

	bool import_source_files(const std::string& elf_filename, std::list<std::string>& filenames);
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
	ElfImporter() : Importer() {}

	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool cb_commandline_init();
	virtual bool create_database();
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("ElfImporter");
		aliases->push_back("ObjdumpImporter");
		aliases->push_back("objdump");
	}
};

#endif
