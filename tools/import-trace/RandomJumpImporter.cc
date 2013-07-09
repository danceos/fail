#include <sstream>
#include <iostream>
#include "util/Logger.hpp"
#include "RandomJumpImporter.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;
using namespace std;

static Logger LOG("RandomJumpImporter");

/**
 * Callback function that can be used to add command line options
 * to the campaign
 */
bool RandomJumpImporter::cb_commandline_init() {
	CommandLine &cmd = CommandLine::Inst();

	FROM = cmd.addOption("", "jump-from", Arg::Required,
						 "--jump-from\t RandomJump: Which addresses should be jumped from\n");
	TO   = cmd.addOption("", "jump-to", Arg::Required,
						 "--jump-to\t RandomJump: Where to jump (a memory map>\n");
	return true;
}

bool RandomJumpImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									   const Trace_Event &ev) {
	if (!binary) {
		// Parse command line again, for jump-from and jump-to
		// operations
		CommandLine &cmd = CommandLine::Inst();
		if (!cmd.parse()) {
			std::cerr << "Error parsing arguments." << std::endl;
			return false;
		}

		// Read FROM memory file
		if (cmd[FROM].count() > 0) {
			m_mm_from = new MemoryMap();
			for (option::Option *o = cmd[FROM]; o; o = o->next()) {
				if (!m_mm_from->readFromFile(o->arg)) {
					LOG << "failed to load memorymap " << o->arg << endl;
					return false;
				}
			}
		}

		if (cmd[TO].count() > 0) {
			m_mm_to = new MemoryMap();
			for (option::Option *o = cmd[TO]; o; o = o->next()) {
				if (!m_mm_to->readFromFile(o->arg)) {
					LOG << "failed to load memorymap " << o->arg << endl;
					return false;
				}
			}
		} else {
			LOG << "Please give at least one --jump-to memory map" << endl;
			return false;
		}

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
		LOG << "instructions disassembled: " << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;

		/* Collect all addresses we want to jump to */
		for (LLVMDisassembler::InstrMap::const_iterator instr = instr_map.begin();
			 instr != instr_map.end(); ++instr) {
			if (m_mm_to->isMatching(instr->first)) {
				m_jump_to_addresses.push_back(instr->first);
			}
		}
		LOG << "we will jump to " << m_jump_to_addresses.size() << " addresses" << endl;
	}


	// skip events that are outside the memory map. -m instruction map
	if (m_mm && !m_mm->isMatching(ev.ip())) {
		return  true;
	}

	// skip events that are outside the --jump-from memory map.
	if (!m_mm_from->isMatching(ev.ip())) {
		return  true;
	}


	for (std::vector<guest_address_t>::const_iterator it = m_jump_to_addresses.begin();
		 it != m_jump_to_addresses.end(); ++it) {
		guest_address_t to_addr = *it;
		/* Do not add a jump to the same instruction */
		if (to_addr == ev.ip())
			continue;

		margin_info_t margin;
		margin.time = curtime;
		margin.dyninstr = instr; // !< The current instruction
		margin.ip = ev.ip();

		// we now have an interval-terminating R/W event to the memaddr
		// we're currently looking at; the EC is defined by
		// data_address, dynamic instruction start/end, the absolute PC at
		// the end, and time start/end
		access_info_t access;
		access.access_type  = 'R'; // instruction fetch is always a read
		access.data_address = to_addr;
		access.data_width    = 4; // exactly one byte
		if (!add_trace_event(margin, margin, access)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}
	}

	return true;
}
