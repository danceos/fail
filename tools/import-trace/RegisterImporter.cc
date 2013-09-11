#include <sstream>
#include <iostream>
#include "RegisterImporter.hpp"
#include "util/Logger.hpp"

using namespace llvm;
using namespace llvm::object;
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
<<<<<<< HEAD
		access_info_t access;
		access.access_type	= access_type; // instruction fetch is always a read
		access.data_address = data_address;
		access.data_width	 = chunk_width;
		if (!add_trace_event(left_margin, right_margin, access)) {
=======

		// pass through potentially available extended trace information
		ev.set_width(1); // exactly one byte
		ev.set_memaddr(data_address);
		ev.set_accesstype(access_type == 'R' ? ev.READ : ev.WRITE);
		if (!add_trace_event(left_margin, right_margin, ev)) {
>>>>>>> dcd2c021a5ac91d38187d397914e5f51e2fc8819
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
<<<<<<< HEAD

		// Read FROM memory file
		if (cmd[NO_GP].count() > 0) {
			do_gp = false;
		}
		if (cmd[FLAGS].count() > 0) {
			do_flags = true;
		}
		if (cmd[IP].count() > 0) {
			do_ip = true;
		}
		if (cmd[NO_SPLIT].count() > 0) {
			do_split_registers = false;
		}

=======
		do_gp = !cmd[NO_GP];
		do_flags = cmd[FLAGS];
		do_ip = cmd[IP];
>>>>>>> dcd2c021a5ac91d38187d397914e5f51e2fc8819

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
	}

	const LLVMDisassembler::InstrMap &instr_map = disas->getInstrMap();
	if (instr_map.find(ev.ip()) == instr_map.end()) {
		LOG << "Could not find instruction for IP: " << std::hex << ev.ip() << std::endl;
		return false;
	}
	const LLVMDisassembler::Instr &opcode = instr_map.at(ev.ip());
	//const MCRegisterInfo &reg_info = disas->getRegisterInfo();

	fail::LLVMtoFailTranslator & ltof = disas->getTranslator() ;

	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_uses.begin();
		 it != opcode.reg_uses.end(); ++it) {
		const LLVMtoFailTranslator::reginfo_t &info = ltof.getFailRegisterID(*it);
		if (&info == &ltof.notfound) {
			LOG << "Could not find a mapping for LLVM input register #" << std::dec << *it
			    << " at IP " << std::hex << ev.ip()
			    << ", skipping" << std::endl;
			continue;
		}

		/* if not tracing flags, but flags register -> ignore it
		   if not tracing gp, but ! flags -> ignore it*/
		if (info.id == RID_FLAGS && !do_flags)
			continue;
		else if (!do_gp)
			continue;

		if (!addRegisterTrace(curtime, instr, ev, info, 'R')) {
			return false;
		}
	}

	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_defs.begin();
		 it != opcode.reg_defs.end(); ++it) {
		const LLVMtoFailTranslator::reginfo_t &info = ltof.getFailRegisterID(*it);
		if (&info == &ltof.notfound) {
			LOG << "Could not find a mapping for LLVM output register #" << std::dec << *it
			    << " at IP " << std::hex << ev.ip()
			    << ", skipping" << std::endl;
			continue;
		}

		/* if not tracing flags, but flags register -> ignore it
		   if not tracing gp, but ! flags -> ignore it*/
		if (info.id == RID_FLAGS && !do_flags)
			continue;
		else if (!do_gp)
			continue;

		if (!addRegisterTrace(curtime, instr, ev, info, 'W'))
			return false;
	}

	const LLVMtoFailTranslator::reginfo_t info_pc(RID_PC);
	if (do_ip) {
		if (!addRegisterTrace(curtime, instr, ev, info_pc, 'R'))
			return false;
		if (!addRegisterTrace(curtime, instr, ev, info_pc, 'W'))
			return false;
	}

	return true;
}
