#include <algorithm>
#include "AdvancedMemoryImporter.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;

static fail::Logger LOG("AdvancedMemoryImporter");

std::string AdvancedMemoryImporter::database_additional_columns()
{
	return MemoryImporter::database_additional_columns() +
		"opcode INT UNSIGNED NULL, "
		"duration BIGINT UNSIGNED AS (time2 - time1 + 1) PERSISTENT, "
		"jumphistory INT UNSIGNED NULL, ";
}

bool AdvancedMemoryImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
	Trace_Event &ev)
{
	// Previous instruction was a branch, check and remember whether it was taken
	if (m_last_was_conditional_branch) {
		m_last_was_conditional_branch = false;
		branches_taken.push_back(ev.ip() != m_ip_jump_not_taken);
	}

	if (!binary) {
		/* Disassemble the binary if necessary */
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllDisassemblers();

		if (error_code ec = createBinary(m_elf->getFilename(), binary)) {
			LOG << m_elf->getFilename() << "': " << ec.message() << ".\n";
			return false;
		}

// necessary due to an AspectC++ bug triggered by LLVM 3.3's dyn_cast()
#ifndef __puma
		ObjectFile *obj = dyn_cast<ObjectFile>(binary.get());
		disas.reset(new LLVMDisassembler(obj));
#endif
		disas->disassemble();
		LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << std::dec << instr_map.size() << " Triple: " << disas->GetTriple() << std::endl;
#if 0
		for (LLVMDisassembler::InstrMap::const_iterator it = instr_map.begin();
			it != instr_map.end(); ++it) {
			LOG << "DIS " << std::hex << it->second.address << " " << (int) it->second.length << std::endl;
		}
#endif
	}

	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const LLVMDisassembler::InstrMap::const_iterator it = instr_map.find(ev.ip());
	if (it == instr_map.end()) {
		LOG << "WARNING: LLVMDisassembler hasn't disassembled instruction at 0x"
		    << ev.ip() << " -- are you using LLVM < 3.3?" << std::endl;
		return true; // probably weird things will happen now
	}
	const LLVMDisassembler::Instr &opcode = it->second;

	/* Now we've got the opcode and know whether it's a conditional branch.  If
	 * it is, the next IP event will tell us whether it was taken or not. */
	if (opcode.conditional_branch) {
		m_last_was_conditional_branch = true;
		m_ip_jump_not_taken = opcode.address + opcode.length;
	}

	return MemoryImporter::handle_ip_event(curtime, instr, ev);
}

bool AdvancedMemoryImporter::handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
	Trace_Event &ev)
{
	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());
	TraceEntry entry = { instr, ev.memaddr(), ev.width(), opcode.opcode, branches_taken.size() };
	update_entries.push_back(entry);

	return MemoryImporter::handle_mem_event(curtime, instr, ev);
}

bool AdvancedMemoryImporter::finalize()
{
	LOG << "adding opcodes and jump history to trace events ..." << std::endl;

	MYSQL_STMT *stmt = 0;
	std::stringstream sql;
	sql << "UPDATE trace SET opcode = ?, jumphistory = ? "
	       "WHERE variant_id = " << m_variant_id << " AND data_address BETWEEN ? AND ? AND instr2 = ?";
	stmt = mysql_stmt_init(db->getHandle());
	if (mysql_stmt_prepare(stmt, sql.str().c_str(), sql.str().length())) {
	LOG << "query '" << sql.str() << "' failed: " << mysql_error(db->getHandle()) << std::endl;
		return false;
	}
	MYSQL_BIND bind[5];

	unsigned rowcount = 0, rowcount_blocks = 0;
	for (std::vector<TraceEntry>::iterator it = update_entries.begin();
	     it != update_entries.end(); ++it) {
		// determine branche decisions before / after this mem event
		unsigned branchmask = 0;
		int pos = std::max(-16, - (signed) it->branches_before);
		int maxpos = std::min((unsigned) 16, branches_taken.size() - it->branches_before);
		for (; pos < maxpos; ++pos) {
			branchmask |=
				((unsigned) branches_taken[it->branches_before + pos])
				<< (16 - pos - 1);
		}

		memset(bind, 0, sizeof(bind));
		for (unsigned i = 0; i < sizeof(bind)/sizeof(*bind); ++i) {
			bind[i].buffer_type = MYSQL_TYPE_LONG;
			bind[i].is_unsigned = 1;
		}
		bind[0].buffer = &it->opcode;
		bind[1].buffer = &branchmask;
		bind[2].buffer = &it->data_address;
		unsigned rightmargin = it->data_address + it->data_width - 1;
		bind[3].buffer = &rightmargin;
		bind[4].buffer = &it->instr2;

		if (mysql_stmt_bind_param(stmt, bind)) {
			LOG << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
			return false;
		} else if (mysql_stmt_execute(stmt)) {
			LOG << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
			return false;
		}
		rowcount += mysql_stmt_affected_rows(stmt);

		if (rowcount >= rowcount_blocks + 10000) {
			LOG << "Updated " << rowcount << " trace events" << std::endl;
			rowcount_blocks += 10000;
		}
	}
	LOG << "Updated " << rowcount << " trace events.  Done." << std::endl;

	return true;
}
