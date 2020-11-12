#include <sstream>
#include <iostream>
#include "InstructionImporter.hpp"
#include "util/Logger.hpp"

using namespace llvm;
using namespace llvm::object;
using namespace fail;


static Logger LOG("InstructionImporter");

bool InstructionImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
										  Trace_Event &ev) {
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

	address_t from = ev.ip(), to = ev.ip() + std::ceil(opcode.length/8.0);
    assert(std::ceil(opcode.length/8.0) == std::floor(opcode.length/8.0) && "Instruction importer can't handle instruction length which don't have byte aligned length");

    auto filter = [this] (address_t a) -> bool { return !(this->m_mm && !this->m_mm->isMatching(a)); };
    auto area = dynamic_cast<memory_area*>(m_fsp.get_area("memory0").get());
    assert(area != nullptr && "InstructionImporter failed to get a MemoryArea from the fault space description");
    access_t acc = ev.accesstype() == ev.READ ? access_t::READ : access_t::WRITE;
    auto elems = area->encode<address_t>(from, to, filter);
    return add_faultspace_elements(curtime, instr, std::move(elems), 0xFF, acc, ev);
}
