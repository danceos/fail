#include <sstream>
#include <iostream>
#include "InstructionImporter.hpp"
#include "util/Logger.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif
using namespace fail;


static Logger LOG("InstructionImporter");

bool InstructionImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
										  Trace_Event &ev) {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	if (!isDisassembled) {
		if (!m_elf) {
			LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
			return false;
		}

		disas.reset(new CapstoneDisassembler(m_elf));

		disas->disassemble();
		CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << instr_map.size() << std::endl;
		isDisassembled = true;
	}
	const CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
	if (instr_map.find(ev.ip()) == instr_map.end()) {
		LOG << "Could not find instruction for IP " << std::hex << ev.ip()
			<< ", skipping" << std::endl;
		return true;
	}
	const CapstoneDisassembler::Instr &opcode = instr_map.at(ev.ip());
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
		ObjectFile *obj = llvm::dyn_cast<ObjectFile>(binary);
		disas.reset(new LLVMDisassembler(obj));
#endif
		disas->disassemble();
		LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;
	}

	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());
#endif

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

		// pass through potentially available extended trace information
		ev.set_accesstype(ev.READ); // instruction fetch is always a read
		ev.set_memaddr(data_address);
		ev.set_width(1); // exactly one byte
		if (!add_trace_event(left_margin, right_margin, ev)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}

		// next interval must start at next instruction; the aforementioned
		// skipping mechanism wouldn't work otherwise
		newOpenEC(data_address, curtime + 1, instr + 1, ev.ip());
	}

	return true;
}
