#include <sstream>
#include <iostream>
#include "RegisterImporter.hpp"
#include "util/Logger.hpp"

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif

using namespace fail;


static Logger LOG("RegisterImporter");

/**
 * Callback function that can be used to add command line options
 * to the campaign
 */
bool RegisterImporter::cb_commandline_init() {
	CommandLine &cmd = CommandLine::Inst();

	NO_GP = cmd.addOption("", "no-gp", Arg::None,
		"--no-gp \tRegisterImporter: do not inject general purpose registers");
	FLAGS = cmd.addOption("", "flags", Arg::None,
		"--flags \tRegisterImporter: inject flags register");
	IP    = cmd.addOption("", "ip", Arg::None,
		"--ip \tRegisterImporter: inject instruction pointer");
	NO_SPLIT = cmd.addOption("", "do-not-split", Arg::None,
		 "--do-not-split \tRegisterImporter: Do not split the registers into one byte chunks");

	return true;
}

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
bool RegisterImporter::addRegisterTrace(simtime_t curtime, instruction_count_t instr,
										Trace_Event &ev,
										const CapstoneToFailTranslator::reginfo_t &info,
										char access_type) {
	address_t from, to;
	int chunk_width;
	if (do_split_registers) {
		/* If we want to split the registers into one byte chunks (to
		   enable proper pruning, we use a one byte window register,
		   to determine beginning and end address */
		CapstoneToFailTranslator::reginfo_t one_byte_window = info;
		one_byte_window.width = 8;
		from = one_byte_window.toDataAddress();
		to   = one_byte_window.toDataAddress() + info.width / 8;
		chunk_width = 1; // One byte chunks
	} else {
		/* We trace whole registers */
		from = info.toDataAddress();
		to = from + 1; /* exactly one trace event per register access*/
		chunk_width = (info.width / 8);
	}

	// Iterate over all accessed bytes
	for (address_t data_address = from; data_address < to; ++data_address) {
		// skip events outside a possibly supplied memory map
		if (m_mm && !m_mm->isMatching(ev.ip())) {
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
		ev.set_width(chunk_width);
		ev.set_memaddr(data_address);
		ev.set_accesstype(access_type == 'R' ? ev.READ : ev.WRITE);
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


bool RegisterImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									   Trace_Event &ev) {
	if (!isDisassembled) {
		// Parse command line again, for jump-from and jump-to
		// operations
		CommandLine &cmd = CommandLine::Inst();
		if (!cmd.parse()) {
			std::cerr << "Error parsing arguments." << std::endl;
			return false;
		}
		do_gp = !cmd[NO_GP];
		do_flags = cmd[FLAGS];
		do_ip = cmd[IP];
		do_split_registers = !cmd[NO_SPLIT];

		// retrieve register IDs for general-purpose and flags register(s) for
		// the configured architecture
		fail::Architecture arch;
		m_ip_register_id =
				(*arch.getRegisterSetOfType(RT_IP)->begin())->getId();
		fail::UniformRegisterSet *regset;
		if (do_gp) {
			regset = arch.getRegisterSetOfType(RT_GP);
			for (fail::UniformRegisterSet::iterator it = regset->begin();
				 it != regset->end(); ++it) {
				m_register_ids.insert((*it)->getId());
			}
		}
		if (do_flags) {
			regset = arch.getRegisterSetOfType(RT_ST);
			for (fail::UniformRegisterSet::iterator it = regset->begin();
				 it != regset->end(); ++it) {
				m_register_ids.insert((*it)->getId());
			}
		}


		if (!m_elf) {
			LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
			return false;
		}
		disas.reset(new CapstoneDisassembler(m_elf));
		LOG << "Start to dissamble" << std::endl;
		disas->disassemble();
		LOG << "Get instr map" << std::endl;
		CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
		LOG << "instructions disassembled: " << std::dec << instr_map.size() << std::endl;
		m_ctof = disas->getTranslator();
		isDisassembled = true;
	}

	// instruction pointer is read + written at each instruction
	const CapstoneToFailTranslator::reginfo_t info_pc(m_ip_register_id);
	if (do_ip &&
		(!addRegisterTrace(curtime, instr, ev, info_pc, 'R') ||
		 !addRegisterTrace(curtime, instr, ev, info_pc, 'W'))) {
		return false;
	}

	const CapstoneDisassembler::InstrMap &instr_map = disas->getInstrMap();
	if (instr_map.find(ev.ip()) == instr_map.end()) {
		LOG << "Could not find instruction for IP " << std::hex << ev.ip()
			<< ", skipping" << std::endl;
		return true;
	}
	const CapstoneDisassembler::Instr &opcode = instr_map.at(ev.ip());
	//const MCRegisterInfo &reg_info = disas->getRegisterInfo();
//    LOG << std::hex  << "Address: " << opcode.address << " Opcode: " << opcode.opcode << std::endl;
//    std::string log_regs = "Uses: ";
	for (std::vector<CapstoneDisassembler::register_t>::const_iterator it = opcode.reg_uses.begin();
		 it != opcode.reg_uses.end(); ++it) {
//        log_regs += std::to_string(*it) + " ";
		const CapstoneToFailTranslator::reginfo_t &info = m_ctof->getFailRegisterInfo(*it);
		if (&info == &m_ctof->notfound) {
			// record failed translation, report later
			m_regnotfound[*it].count++;
			m_regnotfound[*it].address.insert(ev.ip());
			continue;
		}

		/* only proceed if we want to inject into this register */
		if (m_register_ids.find(info.id) == m_register_ids.end()) {
//            log_regs += "n ";
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'R')) {
			return false;
		}
	}

//    log_regs += "Defs: ";

	for (std::vector<CapstoneDisassembler::register_t>::const_iterator it = opcode.reg_defs.begin();
		 it != opcode.reg_defs.end(); ++it) {
//        log_regs += std::to_string(*it) + " ";
		const CapstoneToFailTranslator::reginfo_t &info = m_ctof->getFailRegisterInfo(*it);
		if (&info == &m_ctof->notfound) {
			// record failed translation, report later
			m_regnotfound[*it].count++;
			m_regnotfound[*it].address.insert(ev.ip());
			continue;
		}

		/* only proceed if we want to inject into this register */
		if (m_register_ids.find(info.id) == m_register_ids.end()) {
//            log_regs += "n ";
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'W'))
			return false;
	}
//    LOG << log_regs.c_str() << std::endl;

	return true;
}

bool RegisterImporter::trace_end_reached()
{
	// report failed LLVM -> FAIL* register mappings, if any
	if (m_regnotfound.empty()) {
		return true;
	}

	LOG << "WARNING: Some LLVM -> FAIL* register mappings failed during import, these will not be injected into:" << std::endl;
	for (auto it = m_regnotfound.cbegin(); it != m_regnotfound.cend(); ++it) {
		const CapstoneDisassembler::register_t id = it->first;
		const RegNotFound& rnf = it->second;
		LOG << "Capstone register " << std::dec << id
			/* << " \"" << disas->getRegisterInfo().getName(id) << "\": " */
			<< " seen " << rnf.count << " times in the trace" << std::endl;
		std::ostream& o = LOG << "   corresponding instruction addresses in ELF binary: " << std::hex;
		for (auto addr_it = rnf.address.cbegin(); addr_it != rnf.address.cend(); ++addr_it) {
			if (addr_it != rnf.address.cbegin()) {
				o << ", ";
			}
			o << "0x" << *addr_it;
		}
		o << std::endl;
	}

	return true;
}
#elif defined(BUILD_LLVM_DISASSEMBLER)
bool RegisterImporter::addRegisterTrace(simtime_t curtime, instruction_count_t instr,
										Trace_Event &ev,
										const LLVMtoFailTranslator::reginfo_t &info,
										char access_type) {
	address_t from, to;
	int chunk_width;
	if (do_split_registers) {
		/* If we want to split the registers into one byte chunks (to
		   enable proper pruning, we use a one byte window register,
		   to determine beginning and end address */
		LLVMtoFailTranslator::reginfo_t one_byte_window = info;
		one_byte_window.width = 8;
		from = one_byte_window.toDataAddress();
		to   = one_byte_window.toDataAddress() + info.width / 8;
		chunk_width = 1; // One byte chunks
	} else {
		/* We trace whole registers */
		from = info.toDataAddress();
		to = from + 1; /* exactly one trace event per register access*/
		chunk_width = (info.width / 8);
	}

	// Iterate over all accessed bytes
	for (address_t data_address = from; data_address < to; ++data_address) {
		// skip events outside a possibly supplied memory map
		if (m_mm && !m_mm->isMatching(ev.ip())) {
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
		ev.set_width(chunk_width);
		ev.set_memaddr(data_address);
		ev.set_accesstype(access_type == 'R' ? ev.READ : ev.WRITE);
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


bool RegisterImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									   Trace_Event &ev) {
	if (!binary) {
		// Parse command line again, for jump-from and jump-to
		// operations
		CommandLine &cmd = CommandLine::Inst();
		if (!cmd.parse()) {
			std::cerr << "Error parsing arguments." << std::endl;
			return false;
		}
		do_gp = !cmd[NO_GP];
		do_flags = cmd[FLAGS];
		do_ip = cmd[IP];
		do_split_registers = !cmd[NO_SPLIT];

		// retrieve register IDs for general-purpose and flags register(s) for
		// the configured architecture
		fail::Architecture arch;
		m_ip_register_id =
			(*arch.getRegisterSetOfType(RT_IP)->begin())->getId();
		fail::UniformRegisterSet *regset;
		if (do_gp) {
			regset = arch.getRegisterSetOfType(RT_GP);
			for (fail::UniformRegisterSet::iterator it = regset->begin();
				it != regset->end(); ++it) {
				m_register_ids.insert((*it)->getId());
			}
		}
		if (do_flags) {
			regset = arch.getRegisterSetOfType(RT_ST);
			for (fail::UniformRegisterSet::iterator it = regset->begin();
				it != regset->end(); ++it) {
				m_register_ids.insert((*it)->getId());
			}
		}

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
		LOG << "instructions disassembled: " << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;

		m_ltof = disas->getTranslator() ;
	}

	// instruction pointer is read + written at each instruction
	const LLVMtoFailTranslator::reginfo_t info_pc(m_ip_register_id);
	if (do_ip &&
		(!addRegisterTrace(curtime, instr, ev, info_pc, 'R') ||
		 !addRegisterTrace(curtime, instr, ev, info_pc, 'W'))) {
		return false;
	}

	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	if (instr_map.find(ev.ip()) == instr_map.end()) {
		LOG << "Could not find instruction for IP " << std::hex << ev.ip()
			<< ", skipping" << std::endl;
		return true;
	}
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());
	//const MCRegisterInfo &reg_info = disas->getRegisterInfo();
//    LOG << std::hex  << "Address: " << opcode.address << " Opcode: " << opcode.opcode << std::endl;
//    std::string log_regs = "Uses: ";

	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_uses.begin();
		 it != opcode.reg_uses.end(); ++it) {
//        log_regs += std::to_string(*it) + " ";
		const LLVMtoFailTranslator::reginfo_t &info = m_ltof->getFailRegisterInfo(*it);
		if (&info == &m_ltof->notfound) {
			// record failed translation, report later
			m_regnotfound[*it].count++;
			m_regnotfound[*it].address.insert(ev.ip());
			continue;
		}

		/* only proceed if we want to inject into this register */
		if (m_register_ids.find(info.id) == m_register_ids.end()) {
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'R')) {
			return false;
		}
	}

//    log_regs += "Defs: ";
	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_defs.begin();
		 it != opcode.reg_defs.end(); ++it) {
//        log_regs += std::to_string(*it) + " ";
		const LLVMtoFailTranslator::reginfo_t &info = m_ltof->getFailRegisterInfo(*it);
		if (&info == &m_ltof->notfound) {
			// record failed translation, report later
			m_regnotfound[*it].count++;
			m_regnotfound[*it].address.insert(ev.ip());
			continue;
		}

		/* only proceed if we want to inject into this register */
		if (m_register_ids.find(info.id) == m_register_ids.end()) {
//            log_regs += "n ";
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'W'))
			return false;
	}
//    LOG << log_regs.c_str() << std::endl;

	return true;
}

bool RegisterImporter::trace_end_reached()
{
	// report failed LLVM -> FAIL* register mappings, if any
	if (m_regnotfound.empty()) {
		return true;
	}

	LOG << "WARNING: Some LLVM -> FAIL* register mappings failed during import, these will not be injected into:" << std::endl;
	for (auto it = m_regnotfound.cbegin(); it != m_regnotfound.cend(); ++it) {
		const LLVMDisassembler::register_t id = it->first;
		const RegNotFound& rnf = it->second;
		LOG << "LLVM register " << std::dec << id
			<< " \"" << disas->getRegisterInfo().getName(id) << "\": "
			<< "seen " << rnf.count << " times in the trace" << std::endl;
		std::ostream& o = LOG << "   corresponding instruction addresses in ELF binary: " << std::hex;
		for (auto addr_it = rnf.address.cbegin(); addr_it != rnf.address.cend(); ++addr_it) {
			if (addr_it != rnf.address.cbegin()) {
				o << ", ";
			}
			o << "0x" << *addr_it;
		}
		o << std::endl;
	}

	return true;
}
#endif
