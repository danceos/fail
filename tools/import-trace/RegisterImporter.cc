#include <sstream>
#include <iostream>
#include <map>
#include "RegisterImporter.hpp"
#include "sal/Register.hpp"
#include "util/Logger.hpp"

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

bool RegisterImporter::cb_initialize(void) {
	CommandLine &cmd = CommandLine::Inst();
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return false;
	}

	if (!m_elf) {
		LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
		return false;
	}

	// Depending on the command line interface, we fill up
	// m_register_ids, which is used to filter all def/used registers
	// during trace consumption.
	fail::Architecture *arch = m_area->get_arch();
	if (cmd[IP]) {
		m_import_ip    = true;
		Register *pc  = *(arch->getRegisterSetOfType(RT_IP)->begin());
		// Depending on the ELF Class of the imported binary, we
		// select the width of the PC register to be injected on
		// the --ip command-line switch.
		unsigned pc_width = m_elf->m_elfclass == ELFCLASS64 ? 64 : 32;
		m_ip_register = RegisterView(pc->getId(), pc_width);
		m_register_ids.insert(pc->getId());
	}

	if (!cmd[NO_GP]) {
		auto regset = arch->getRegisterSetOfType(RT_GP);
		assert(regset != nullptr && "No register was marked as general purpose register");
		for (Register * reg : *regset) {
			m_register_ids.insert(reg->getId());
		}
	}

	if (cmd[FLAGS]) {
		auto regset = arch->getRegisterSetOfType(RT_ST);
		assert(regset != nullptr && "Architecture has no flags registers");
		for (Register * reg : *regset) {
			m_register_ids.insert(reg->getId());
		}
	}


	m_disassembler.reset(new Disassembler(m_elf));
	m_disassembler->disassemble();
	m_instr_map  = m_disassembler->getInstrMap();
	m_translator = m_disassembler->getTranslator();

	return true;
}

bool RegisterImporter::handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
									   Trace_Event &ev) {
	// If the instruction pointer is not included in the memory map, ignore ir.
	if(m_mm && m_mm->isMatching(ev.ip()))
		return true;

	// instruction pointer is read + written at each instruction
	if (m_import_ip) {
		if (!addRegisterTrace(curtime, instr, ev, m_ip_register, 'R') ||
			!addRegisterTrace(curtime, instr, ev, m_ip_register, 'W')) {
			return false;
		}
	}

	if (m_instr_map->find(ev.ip()) == m_instr_map->end()) {
		LOG << "Could not find instruction for IP " << std::hex << ev.ip()
			<< ", skipping" << std::endl;
		return true;
	}

	const Disassembler::Instr &opcode = m_instr_map->at(ev.ip());
	LOG << "working on instruction @ IP " << std::hex << ev.ip() << std::endl;

	for (Disassembler::register_t reg : opcode.reg_uses) {
		const RegisterView &info = m_translator->getFailRegisterInfo(reg);
		if (&info == &m_translator->notfound) {
			// record failed translation, report later
			m_regnotfound[reg].count++;
			m_regnotfound[reg].address.insert(ev.ip());
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'R')) {
			return false;
		}
	}

	for (Disassembler::register_t reg : opcode.reg_defs)  {
		const RegisterView &info = m_translator->getFailRegisterInfo(reg);
		if (&info == &m_translator->notfound) {
			// record failed translation, report later
			m_regnotfound[reg].count++;
			m_regnotfound[reg].address.insert(ev.ip());
			continue;
		}

		if (!addRegisterTrace(curtime, instr, ev, info, 'W'))
			return false;
	}

	return true;
}

bool RegisterImporter::addRegisterTrace(simtime_t curtime, instruction_count_t instr,
										Trace_Event &ev,
										RegisterView view,
										char access_type) {
	/* only proceed if we want to inject into this register */
	if (m_register_ids.find(view.id) == m_register_ids.end())
		return true;

	Architecture *arch = m_area->get_arch();
	Register * reg = arch->getRegister(view.id);

	if (view.width == -1) {
		view.width = reg->getWidth();
	}

	LOG << std::hex << "[IP=0x" << ev.ip() << "] " << reg->getName() << std::dec
		<< " access: "  << access_type
		<< " offset: " << (int) view.offset
		<< " width: "  << (int) view.width
		<< std::endl;

	for (auto access : m_area->translate(view)) {
		bool succ = add_faultspace_element(curtime, instr,
										   access.first, access.second,
										   access_type, ev);
		if (!succ) return false;
	}
	return true;
}

bool RegisterImporter::trace_end_reached()
{
	// report failed Dissassembler -> FAIL* register mappings, if any
	if (m_regnotfound.empty()) {
		return true;
	}

	LOG << "WARNING: Some Disassembler -> FAIL* register mappings failed during import, these will not be injected into:" << std::endl;
	for (auto it = m_regnotfound.cbegin(); it != m_regnotfound.cend(); ++it) {
		const Disassembler::register_t id = it->first;
		const RegNotFound& rnf = it->second;
		LOG << "Disassembler register " << std::dec << id
			<< " \"" << m_disassembler->getRegisterName(id) << "\": "
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


