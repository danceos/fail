#include <algorithm>
#include <sstream>
#include "AdvancedMemoryImporter.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif
using namespace fail;

static fail::Logger LOG("AdvancedMemoryImporter");

std::string AdvancedMemoryImporter::database_additional_columns()
{
	return MemoryImporter::database_additional_columns() +
		"opcode INT UNSIGNED NULL, "
		"duration BIGINT UNSIGNED AS (time2 - time1 + 1) PERSISTENT, "
		"jumphistory INT UNSIGNED NULL, ";
}

void AdvancedMemoryImporter::database_insert_columns(std::string& sql, unsigned& num_columns)
{
	// FIXME upcall?
	sql = ", opcode, jumphistory";
	num_columns = 2;
}

//#include <google/protobuf/text_format.h>

bool AdvancedMemoryImporter::database_insert_data(Trace_Event &ev, std::stringstream& value_sql, unsigned num_columns, bool is_fake)
{
	// FIXME upcall?
	assert(num_columns == 2);
#if 0
	// sanity check
	if (!is_fake && delayed_entries.size() > 0 && ev.ip() != delayed_entries.front().ev.ip()) {
		std::string out;
		google::protobuf::TextFormat::PrintToString(ev, &out);
		std::cout << "ev: " << out << std::endl;
		google::protobuf::TextFormat::PrintToString(delayed_entries.front().ev, &out);
		std::cout << "delayed_entries.front.ev: " << out << std::endl;
	}
#endif
	assert(is_fake || delayed_entries.size() == 0 || ev.ip() == delayed_entries.front().ev.ip());
	if (is_fake) {
		value_sql << "NULL,NULL,";
	} else {
		value_sql << delayed_entries.front().opcode << ","
			<< m_cur_branchmask << ",";
	}
	return true;
}

void AdvancedMemoryImporter::insert_delayed_entries(bool finalizing)
{
	unsigned branchmask;
	unsigned last_branches_before = UINT_MAX;
	// If we don't know enough future, and there's a chance we'll learn more,
	// delay further.
	for (std::deque<DelayedTraceEntry>::iterator it = delayed_entries.begin();
	     it != delayed_entries.end() &&
	     (it->branches_before + BRANCH_WINDOW_SIZE <= branches_taken.size() ||
	      finalizing);
	     it = delayed_entries.erase(it)) {
		// determine branch decisions before / after this mem event
		if (it->branches_before != last_branches_before) {
			branchmask = 0;
			int pos = std::max(-(signed)BRANCH_WINDOW_SIZE, - (signed) it->branches_before);
			int maxpos = std::min((signed)BRANCH_WINDOW_SIZE,
				(signed)branches_taken.size() - (signed)it->branches_before);
			for (; pos < maxpos; ++pos) {
				branchmask |=
					((unsigned) branches_taken[it->branches_before + pos])
					<< (16 - pos - 1);
			}
			m_cur_branchmask = branchmask;
		}

		//LOG << "AdvancedMemoryImporter::insert_delayed_entries instr = " << it->instr << " data_address = " << it->ev.memaddr() << std::endl;

		// trigger INSERT
		// (will call back via database_insert_data() and ask for additional data)
		MemoryImporter::handle_mem_event(it->curtime, it->instr, it->ev);
	}

	// FIXME branches_taken could be shrunk here to stay within a bounded
	// memory footprint
}

bool AdvancedMemoryImporter::cb_initialize() {
	// Parse command line again, for jump-from and jump-to
	// operations
	CommandLine &cmd = CommandLine::Inst();
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return false;
	}

	if (!m_elf) {
		LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
		return false;
	}

	m_disassembler.reset(new Disassembler(m_elf));
	m_disassembler->disassemble();
	m_instr_map = m_disassembler->getInstrMap();

	return true;
}

bool AdvancedMemoryImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
	Trace_Event &ev)
{
	// Previous instruction was a branch, check and remember whether it was taken
	if (m_last_was_conditional_branch) {
		m_last_was_conditional_branch = false;
		branches_taken.push_back(ev.ip() != m_ip_jump_not_taken);
	}

	// Check whether we know enough branch-taken future to INSERT a few more
	// (delayed) trace entries
	insert_delayed_entries(false);


	const Disassembler::InstrMap::const_iterator it = m_instr_map->find(ev.ip());
	if (it == m_instr_map->end()) {
		LOG << "WARNING: Disassembler hasn't disassembled instruction at 0x"
			<< ev.ip() << std::endl;
		return true; // probably weird things will happen now
	}
	const Disassembler::Instr &opcode = it->second;

	/* Now we've got the opcode and know whether it's a conditional branch.  If
	 * it is, the next IP event will tell us whether it was taken or not. */
	if (opcode.conditional_branch) {
		m_last_was_conditional_branch = true;
		m_ip_jump_not_taken = opcode.address + opcode.length;
	}

	// IP events may need to be delayed, too, if the parent Importer draws any
	// information from them.  MemoryImporter does not, though.
	//return MemoryImporter::handle_ip_event(curtime, instr, ev);
	return true;
}

bool AdvancedMemoryImporter::handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
	Trace_Event &ev)
{
	const Disassembler::Instr &opcode = m_instr_map->at(ev.ip());

	DelayedTraceEntry entry = { curtime, instr, ev, opcode.opcode, (unsigned) branches_taken.size() };
	delayed_entries.push_back(entry);

	// delay upcall to handle_mem_event until we know enough future branch decisions
	return true;
}

bool AdvancedMemoryImporter::trace_end_reached()
{
	LOG << "inserting remaining trace events ..." << std::endl;
	// INSERT the remaining entries (with incomplete branch future)
	insert_delayed_entries(true);
	return true;
}
