#ifndef __puma
#include <sstream>
#include <iostream>
#include "InstructionImporter.hpp"
#include "util/Logger.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;


static Logger LOG("InstructionImporter");

bool InstructionImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
										  const Trace_Event &ev) {
	if (!binary) {
		/* Disassemble the binary if necessary */
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllDisassemblers();

		if (error_code ec = createBinary(m_elf->getFilename(), binary)) {
			LOG << m_elf->getFilename() << "': " << ec.message() << ".\n";
			return false;
		}

		ObjectFile *obj = dyn_cast<ObjectFile>(binary.get());

		disas.reset(new LLVMDisassembler(obj));
		disas->disassemble();
		LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;
	}

	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());

	address_t from = ev.ip(), to = ev.ip() + opcode.length;

	// Iterate over all accessed bytes
	for (address_t data_address = from; data_address < to; ++data_address) {
		// skip events outside a possibly supplied memory map
		if (m_mm && !m_mm->isMatching(data_address)) {
			continue;
		}
		margin_info_t left_margin = getOpenEC(data_address);
		margin_info_t right_margin;
		right_margin.time = curtime;
		right_margin.dyninstr = instr; // !< The current instruction
		right_margin.ip = ev.ip();

		// skip zero-sized intervals: these can occur when an instruction
		// accesses a memory location more than once (e.g., INC, CMPXCHG)
		if (left_margin.dyninstr > right_margin.dyninstr) {
			continue;
		}

		// we now have an interval-terminating R/W event to the memaddr
		// we're currently looking at; the EC is defined by
		// data_address, dynamic instruction start/end, the absolute PC at
		// the end, and time start/end
		access_info_t access;
		access.access_type  = 'R'; // instruction fetch is always a read
		access.data_address = data_address;
		access.data_width    = 1; // exactly one byte
		if (!add_trace_event(left_margin, right_margin, access)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}

		// next interval must start at next instruction; the aforementioned
		// skipping mechanism wouldn't work otherwise
		newOpenEC(data_address, curtime + 1, instr + 1, ev.ip());
	}

	return true;
}


#endif // !__puma
