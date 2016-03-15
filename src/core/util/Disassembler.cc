#include "Disassembler.hpp"
#include <cstdlib>
#include "pstream.h"
#include <sstream>
#include <iostream>
#include <vector>
#include <utility>
#ifndef __puma
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#endif
#include <stdlib.h>

namespace fail {

const std::string DISASSEMBLER::FAILED = "[Disassembler] Disassemble failed.";

Disassembler::Disassembler() : m_log("FAIL*Disassembler", false) {  }

int Disassembler::init() {
	// try to open elf file from environment variable
	char * elfpath = getenv("FAIL_ELF_PATH");
	if (elfpath == NULL) {
		m_log << "FAIL_ELF_PATH not set :(" << std::endl;
		exit(EXIT_FAILURE);
	} else {
		return init(elfpath);
	}
}

int Disassembler::init(const char* path) {
	// Disassemble ELF
#ifndef __puma
	std::string command = std::string(ARCH_TOOL_PREFIX) +  std::string("objdump -d ") + std::string(path);
	m_log << "Executing: " << command << std::endl;
	redi::ipstream objdump( command );
	std::string str;
	while (std::getline(objdump, str)) {
		evaluate(str);
	}

	objdump.close();
	if (objdump.rdbuf()->exited()) {
		int ex = objdump.rdbuf()->status();
		if (ex != 0) {
			m_code.clear();
			m_log << "Could not disassemble!" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	m_log << "disassembled " << m_code.size() << " lines." << std::endl;
#endif
	return m_code.size();
}

std::ostream& operator <<(std::ostream & os, const fail::Instruction & i) {
#ifndef __puma
	std::ios::fmtflags f(os.flags()); // save ostream state
	os << std::hex << ((int)(i.address)) << "\t" << i.opcode << "\t" << i.instruction << "\t" << i.comment;
	os.flags(f); // restore ostream state
#endif
	return os;
}

void Disassembler::evaluate(const std::string& line) {
#ifndef __puma
	// Only read in real code lines:
	// Code lines start with a leading whitespace! (hopefully in each objdump implementation!)
	if (line.size() > 0 && isspace(line[0])) {
		// a line looks like: 800156c:\tdd14          \tble.n   8001598 <_ZN2hw3hal7T32Term8PutBlockEPci+0x30>
		boost::regex expr("\\s+([A-Fa-f0-9]+):\\t(.*?)\\t(.+?)(;.*)?$");
		boost::smatch res;
		if (boost::regex_search(line, res, expr)) {
			std::string address = res[1];
			std::stringstream ss;
			ss << std::hex << address;
			address_t addr = 0;
			ss >> addr;
			ss.clear();
			ss.str("");

			std::string opcode = res[2];
			// delete trailing/leading whitespaces
			boost::trim(opcode);
			// delete inner whitespaces and merge nibbles
			opcode.erase(std::remove(opcode.begin(), opcode.end(), ' '), opcode.end());
			ss << std::hex << opcode;
			unsigned opc = 0;
			ss >> opc;

			std::string instruction = res[3];
			boost::trim(instruction);
			std::string comment = res[4];
			boost::trim(comment);

			m_code.insert(std::make_pair(addr, Instruction(addr, opc, instruction, comment)));
		}
	}
#endif
}

static Instruction g_InstructionNotFound;
const Instruction & Disassembler::disassemble(address_t address) const {
	InstructionMap_t::const_iterator it = m_code.find(address);
	if (it == m_code.end()) {
		return g_InstructionNotFound;
	} else {
		return it->second;
	}
}

} // end of namespace

