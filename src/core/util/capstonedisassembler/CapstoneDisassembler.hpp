#ifndef __CAPSTONEDISASSEMBLER_HPP__
#define __CAPSTONEDISASSEMBLER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <limits.h>
#include <memory> // unique_ptr
#include <algorithm>

#include <elf.h>

#include "CapstoneToFailTranslator.hpp"
#include "CapstoneToFailBochs.hpp"
#include "CapstoneToFailGem5.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <fstream>

#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <sstream>
#include <map>

#include "util/ElfReader.hpp"

namespace fail {

class CapstoneDisassembler {

public:
	typedef uint16_t register_t;
	typedef unsigned int address_t;
	struct Instr {
		unsigned int opcode;
		unsigned int address;
		unsigned char length;
		bool conditional_branch;
		std::vector<register_t> reg_uses;
		std::vector<register_t> reg_defs;
	};

	typedef std::map<address_t, Instr> InstrMap;

private:
	std::unique_ptr<InstrMap> instrs;
	fail::CapstoneToFailTranslator *ctofail;
	fail::ElfReader *m_elf;

	static bool error(std::error_code ec) {
		if (!ec) return false;

		std::cerr << "DIS error: " << ec.message() << ".\n";
		return true;
	}

public:
	CapstoneDisassembler(fail::ElfReader *elf) : ctofail(0)  {
		this->m_elf = elf;
		this->instrs.reset(new InstrMap());
	}

	~CapstoneDisassembler() { delete ctofail; };

	InstrMap &getInstrMap() { return *instrs; };
	fail::CapstoneToFailTranslator *getTranslator();

	void disassemble();

private:
	int disassemble_section(Elf_Data *data, Elf32_Shdr *shdr, Elf64_Shdr *shdr64, std::map<uint64_t, uint64_t> symtab_map);
	std::map<uint64_t, uint64_t> get_symtab_map(uint64_t sect_addr, uint64_t sect_size);
};

}
#endif // __CAPSTONEDISASSEMBLER_HPP__
