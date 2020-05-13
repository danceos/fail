#include <sstream>
#include <iostream>
#include "util/Logger.hpp"
#include "RandomJumpImporter.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif
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
		"--jump-from \tRandomJump: Which addresses should be jumped from (a memory map; may be used more than once)");
	TO   = cmd.addOption("", "jump-to", Arg::Required,
		"--jump-to \tRandomJump: Where to jump (a memory map; may be used more than once)");
	return true;
}

bool RandomJumpImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									   Trace_Event &ev) {
	if (!binary) {
		// Parse command line again, for jump-from and jump-to
		// operations
		CommandLine &cmd = CommandLine::Inst();
		if (!cmd.parse()) {
			std::cerr << "Error parsing arguments." << std::endl;
			return false;
		}

		// Read FROM memory map(s)
		if (cmd[FROM]) {
			m_mm_from = new MemoryMap();
			for (option::Option *o = cmd[FROM]; o; o = o->next()) {
				if (!m_mm_from->readFromFile(o->arg)) {
					LOG << "failed to load memorymap " << o->arg << endl;
					return false;
				}
			}
		}

		// Read TO memory map(s)
		if (cmd[TO]) {
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

		if (!m_elf) {
			LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
			return false;
		}
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
		disas.reset(new CapstoneDisassembler(m_elf));

		disas->disassemble();
		CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << instr_map.size() <<  std::endl;

		/* Collect all addresses we want to jump to */
		for (CapstoneDisassembler::InstrMap::const_iterator instr = instr_map.begin();
			 instr != instr_map.end(); ++instr) {
			if (m_mm_to && m_mm_to->isMatching(instr->first)) {
				m_jump_to_addresses.push_back(instr->first);
			}
		}
		binary = true;
#elif defined(BUILD_LLVM_DISASSEMBLER)
		/* Disassemble the binary if necessary */
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllDisassemblers();

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
		LOG << "instructions disassembled: " << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;

		/* Collect all addresses we want to jump to */
		for (LLVMDisassembler::InstrMap::const_iterator instr = instr_map.begin();
			 instr != instr_map.end(); ++instr) {
			if (m_mm_to && m_mm_to->isMatching(instr->first)) {
				m_jump_to_addresses.push_back(instr->first);
			}
		}
#endif
		LOG << "we will jump to " << m_jump_to_addresses.size() << " addresses" << endl;
	}


	// skip events that are outside the memory map. -m instruction map
	if (m_mm && !m_mm->isMatching(ev.ip())) {
		return true;
	}

	// skip events that are outside the --jump-from memory map.
	if (m_mm_from && !m_mm_from->isMatching(ev.ip())) {
		return true;
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

		// pass through potentially available extended trace information
		ev.set_accesstype(ev.READ); // instruction fetch is always a read
		ev.set_memaddr(to_addr);
		ev.set_width(4); // FIXME arbitrary, use Instr.length instead?
		if (!add_trace_event(margin, margin, ev)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}
	}

	return true;
}
