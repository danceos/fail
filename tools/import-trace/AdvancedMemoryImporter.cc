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

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	if (!isDisassembled) {
		if (!m_elf) {
			LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
			return false;
		}

		disas.reset(new CapstoneDisassembler(m_elf));

		disas->disassemble();
		CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << std::dec << instr_map.size() << std::endl;
#if 0
		for (CapstoneDisassembler::InstrMap::const_iterator it = instr_map.begin();
			it != instr_map.end(); ++it) {
			LOG << "DIS " << std::hex << it->second.address << " " << (int) it->second.length << std::endl;
		}
#endif
	}

	const CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const CapstoneDisassembler::InstrMap::const_iterator it = instr_map.find(ev.ip());
	if (it == instr_map.end()) {
		LOG << "WARNING: CapstoneDisassembler hasn't disassembled instruction at 0x"
			<< ev.ip() << " -- are you using Capstone < 4.0?" << std::endl;
		return true; // probably weird things will happen now
	}
	const CapstoneDisassembler::Instr &opcode = it->second;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	if (!binary) {
		/* Disassemble the binary if necessary */
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllDisassemblers();

		if (!m_elf) {
			LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
			return false;
		}

		Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(m_elf->getFilename());
		if (!BinaryOrErr) {
			std::string Buf;
			raw_string_ostream OS(Buf);
			logAllUnhandledErrors(std::move(BinaryOrErr.takeError()), OS, "");
			OS.flush();
			LOG << m_elf->getFilename() << "': " << Buf << ".\n";
			return false;
		}
		binary = BinaryOrErr.get().getBinary();

// necessary due to an AspectC++ bug triggered by LLVM 3.3's dyn_cast()
#ifndef __puma
		ObjectFile *obj = dyn_cast<ObjectFile>(binary);
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
#endif

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
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	const CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const CapstoneDisassembler::Instr &opcode = instr_map.at(ev.ip());
#elif defined(BUILD_LLVM_DISASSEMBLER)
	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());
#endif
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
