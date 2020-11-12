#include <sstream>
#include <iostream>
#include <map>
#include "RegisterImporter.hpp"
#include "util/Logger.hpp"
#include "sal/sail/registers.hpp"

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

	return true;
}


bool RegisterImporter::addRegisterTrace(simtime_t curtime, instruction_count_t instr,
										Trace_Event &ev,
                                        size_t register_id,
										Importer::access_t type) {
    if(m_mm && m_mm->isMatching(ev.ip())) return true;

    auto area = dynamic_cast<register_area*>(m_fsp.get_area("register").get());
    assert(area != nullptr && "RegisterImporter failed to get a RegisterArea from the fault space description");

    std::map<uint8_t, std::vector<std::unique_ptr<util::fsp::element>>> mask_to_elems;
    if(do_inject_regs) {
        mask_to_elems = area->encode(register_id);
    }
#ifndef __acweaving
    if(do_inject_tags) {
        // FIXME: this is highly specific to the Sail architecture
        // where typeof(get_arch()) == typeof(get_state())
#ifdef BUILD_RISCV_CHERI
        auto cpu = dynamic_cast<ConcreteCPU&>(m_arch);
        auto tag_register = cpu.get_tag_reg();
        size_t index = tag_register.get_index(register_id);
        unsigned byte = index / 8;
        unsigned mask = (1u << (index - byte*8));
        mask_to_elems[mask].push_back(area->encode(tag_register.getId(),byte));

        LOG << "injecting tag bit @ idx " << index
            <<" in tag register (name=" << tag_register.getName() << ",id=" << tag_register.getId() << ")"
            << " (byte="<< byte << ",mask="<< mask << ")"
            << std::endl;
#endif
    }
#endif
    for(auto& p: mask_to_elems) {
        bool succ = add_faultspace_elements(curtime, instr, std::move(p.second), p.first, type, ev);
        if(!succ) return false;
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
        do_inject_tags = (m_memtype == fail::memory_type::tag) || (m_memtype == fail::ANY_MEMORY);
        do_inject_regs = (m_memtype == fail::memory_type::ram) || (m_memtype == fail::ANY_MEMORY);

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
		LOG << "instructions disassembled: " << std::dec << instr_map.size() << " Triple: " << disas->GetTriple() <<  std::endl;

		m_ltof = disas->getTranslator() ;
	}

	// instruction pointer is read + written at each instruction
	if (do_ip &&
		(!addRegisterTrace(curtime, instr, ev, m_ip_register_id, access_t::READ) ||
		 !addRegisterTrace(curtime, instr, ev, m_ip_register_id, access_t::WRITE))) {
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
		LOG << "working on instruction @ IP " << std::hex << ev.ip()
		 << std::endl;

	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_uses.begin();
		 it != opcode.reg_uses.end(); ++it) {
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
        LOG << std::hex << std::showbase
            << "[IP=" << ev.ip() << "] "
            << "use: " << disas->getRegisterInfo().getName(*it)
            << std::dec << std::noshowbase << std::endl;

		if (!addRegisterTrace(curtime, instr, ev, info.id, access_t::READ)) {
			return false;
		}
	}

	for (std::vector<LLVMDisassembler::register_t>::const_iterator it = opcode.reg_defs.begin();
		 it != opcode.reg_defs.end(); ++it) {
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

        LOG << std::hex << std::showbase
            << "[IP=" << ev.ip() << "] "
            << "def: " << disas->getRegisterInfo().getName(*it)
            << std::dec << std::noshowbase << std::endl;

		if (!addRegisterTrace(curtime, instr, ev, info.id, access_t::WRITE))
			return false;
	}

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
