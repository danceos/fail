#include <capstone/capstone.h>
#if CS_MAKE_VERSION(CS_API_MAJOR, CS_API_MINOR) < CS_MAKE_VERSION(4, 0)
#  error Need libcapstone >= 4.0
#endif

#include "CapstoneDisassembler.hpp"

using namespace fail;


CapstoneToFailTranslator *CapstoneDisassembler::getTranslator() {
	if (ctofail == 0) {
		switch (m_elf->m_machine) {
		case EM_386:
		case EM_X86_64:
			ctofail = new CapstoneToFailBochs();
			break;
		case EM_ARM:
			ctofail = new CapstoneToFailGem5();
			break;
		default:
			std::cerr << "ArchType "
				<< m_elf->m_machine
				<< " not supported\n";
			exit(1);
		}
	}
	return ctofail;
}

std::map<uint64_t, uint64_t> CapstoneDisassembler::get_symtab_map(uint64_t sect_addr, uint64_t sect_size) {
	// Make a list of all the symbols (virtual address, size) in this section.
	std::vector<std::pair<uint64_t, uint64_t> > symbols;
	for (ElfReader::container_t::const_iterator it = m_elf->sym_begin(); it != m_elf->sym_end(); ++it) {

//		std::cout << it->getSymbolType() << " " << it->getName() << " (" << it->getDemangledName() << ") " << std::hex << it->getAddress() << " " << std::dec << it->getSize() << "\n";
		if (it->getSymbolType() != STT_FUNC && it->getSymbolType() != STT_NOTYPE) {
			continue;
		}

		symbols.push_back(std::make_pair(it->getAddress(), it->getSize()));
#if 0
		std::cout << std::hex << it->getAddress() << "\t" << it->getSymbolType() << "\t" << it->getName().c_str() << "\t" << it->getSize() << std::endl;
#endif
	}

	// Sort the symbols by address, just in case they didn't come in that way.
	std::sort(symbols.begin(), symbols.end());

	std::map<uint64_t, uint64_t> symtab_map; // start (virtual address), size
	uint64_t start;
	uint64_t size;
	for (unsigned si = 0, se = symbols.size(); si != se; ++si) {
		start = symbols[si].first;
		size = symbols[si].second;
		// only admit symbols that start within this section
		if (start < sect_addr || sect_addr + sect_size <= start) {
			continue;
		}
		if (size == 0) {
			// The end is either the end of the section or the beginning of the next symbol.
			if (si == se - 1)
				// Last symbol? Span until section end.
				size = sect_addr + sect_size - start;
			else if (symbols[si + 1].first != start)
				// There is distance to the next symbol? Cover it.
				size = symbols[si + 1].first - start;
			else
				// This symbol has the same address as the next symbol. Skip it.
				continue;

			symbols[si].second = size;
		}
		// limit the symbol size to within this section
		if (start + size > sect_addr + sect_size) {
			size = sect_addr + sect_size - start;
		}
		symtab_map[symbols[si].first] = size;
	}
#if 0
	for (std::map<uint64_t, uint64_t>::iterator it=symtab_map.begin(); it!=symtab_map.end(); ++it)
		std::cout << std::hex << it->first << " => " << it->second << std::endl;
#endif
	return symtab_map;
}

int CapstoneDisassembler::disassemble_section(Elf_Data *data, Elf32_Shdr *shdr32, Elf64_Shdr *shdr64, std::map<uint64_t, uint64_t> symtab_map) {
#if 0
	std::cout << std::dec << "bit: " << m_elf->m_elfclass << " 32: "<< ELFCLASS32 << " 64: " << ELFCLASS64 << " arch: " << m_elf->m_machine << " arm:" << EM_ARM << " x86: " << EM_386 << " x86_64: "<< EM_X86_64 << std::endl;
#endif
	csh handle;
	cs_insn *insn;
	size_t count, j;
	cs_regs regs_read, regs_write;
	uint8_t read_count, write_count, i;

	cs_opt_skipdata skipdata = {
			.mnemonic = "db",
	};

	// Arm may not work, because thumb is a problem
	if (m_elf->m_machine == EM_386) {
		if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
			return -1;
	} else if(m_elf->m_machine == EM_X86_64) {
		if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
			return -1;
	} else if (m_elf->m_machine == EM_ARM) {
		if (m_elf->m_elfclass == ELFCLASS32) {
			if (cs_open(CS_ARCH_ARM, CS_MODE_ARM, &handle) != CS_ERR_OK)
				return -1;
		} else {
			if (cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle) != CS_ERR_OK)
				return -1;
		}
	}

	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
	cs_option(handle, CS_OPT_SKIPDATA, CS_OPT_ON);
	cs_option(handle, CS_OPT_SKIPDATA_SETUP, (size_t)&skipdata);

	int global_count = 0;

	for (std::map<uint64_t, uint64_t>::iterator it=symtab_map.begin(); it!=symtab_map.end(); ++it) {

		if (m_elf->m_elfclass == ELFCLASS32) {
			count = cs_disasm(handle, (uint8_t *) data->d_buf + it->first - shdr32->sh_addr,
							  it->second, it->first, 0, &insn);
		} else {
			count = cs_disasm(handle, (uint8_t *) data->d_buf + it->first - shdr64->sh_addr,
							  it->second, it->first, 0, &insn);
		}

		if (count > 0) {
			for (j = 0; j < count; j++) {
				unsigned int opcode = 0;
				if (m_elf->m_machine == EM_386 || m_elf->m_machine == EM_X86_64) {
					if (insn[j].detail) { // NULL if insn is broken
						opcode = (insn[j].detail->x86.opcode[3] << 24) | (insn[j].detail->x86.opcode[2] << 16) |
						  (insn[j].detail->x86.opcode[1] << 8) | insn[j].detail->x86.opcode[0];
					}
				} else if (m_elf->m_machine == EM_ARM) {
					// placeholder
					opcode = 0;
				}

				// Print assembly
#if 0
				printf("%s\t%s\n", insn[j].mnemonic, insn[j].op_str);
				printf("Opcode: %x\t Address: %lx\t Size: %d\n", opcode, insn[j].address, insn[j].size);
#endif
				// Print all registers accessed by this instruction.
				if (cs_regs_access(handle, &insn[j], regs_read, &read_count, regs_write, &write_count) == CS_ERR_OK) {
					Instr instr;
					instr.opcode = opcode;
					instr.length = insn[j].size;
					instr.address = insn[j].address;
					// FIXME could not find a functionality in capstone
					instr.conditional_branch = false;

					if (read_count > 0) {
//                        printf("\n\tRegisters read:");
						for (i = 0; i < read_count; i++) {
							instr.reg_uses.push_back(regs_read[i]);
//                            printf(" %s, %d |", cs_reg_name(handle, regs_read[i]), regs_read[i]);
						}
//                        printf("\n");
					}

					if (write_count > 0) {
//                        printf("\n\tRegisters modified:");
						for (i = 0; i < write_count; i++) {
							instr.reg_defs.push_back(regs_write[i]);
//                            printf(" %s, %d |", cs_reg_name(handle, regs_write[i]), regs_write[i]);
						}
//                        printf("\n");
					}
					(*instrs)[instr.address] = instr;
				}
//                printf("________________________________________________\n");
			}
			global_count += count;

			cs_free(insn, count);
		} else
			std::cerr << "ERROR: Failed to disassemble given code!" << std::endl;
	}
//    printf("len_instr_map: %d\n", instrs->size());
	cs_close(&handle);
//    printf("%d instructions\n", global_count);
	return 0;
}

void CapstoneDisassembler::disassemble() {
	int fd;       // File descriptor for the executable ELF file
	char *section_name;
	size_t shstrndx;
	Elf *e;           // ELF struct
	Elf_Data *data = 0;
	Elf_Scn *scn;     // Section index struct
	Elf32_Shdr *shdr32 = 0;     // Section struct 32 Bit
	Elf64_Shdr *shdr64 = 0;     // Section struct 64 Bit

	if (elf_version(EV_CURRENT) == EV_NONE)
		std::cerr << "ELF library initialization failed" << std::endl;

	if ((fd = open(m_elf->getFilename().c_str(), O_RDONLY, 0)) < 0)
		std::cerr << "open " << m_elf->getFilename().c_str() << " failed" << std::endl;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		std::cerr << "elf_begin() failed" << std::endl;

	if (elf_kind(e) != ELF_K_ELF) {
		std::cerr << m_elf->getFilename().c_str() << " is not an Elf object" << std::endl;
	}

	if (elf_getshdrstrndx(e, &shstrndx) != 0)
		std::cerr << "elf_getshdrstrndx() failed" << std::endl;
	scn = NULL;

	// Loop over all sections in the ELF object

	while ((scn = elf_nextscn(e, scn)) != NULL) {
		// Given an Elf Scn pointer, retrieve the associated section header
		if (m_elf->m_elfclass == ELFCLASS32) {
			if ((shdr32 = elf32_getshdr(scn)) == NULL)
				std::cerr << "getshdr() failed" << std::endl;

			// Retrieve the name of the section
			if ((section_name = elf_strptr(e, shstrndx, shdr32->sh_name)) == NULL)
				std::cerr << "elf_strptr() failed" << std::endl;

			if (!strcmp(section_name, ".text")) {
				if ((data = elf_rawdata(scn, data)) == NULL) {
					std::cerr << "No section data available" << std::endl;
				}
#if 0
				printf("Section name: %s\n", section_name);
				printf("sh_offset: %x\n", shdr32->sh_offset);
				printf("sh_type: %x\n", shdr32->sh_type);
				printf("sh_flags: %x\n", shdr32->sh_flags);
				printf("sh_addr: %x\n", shdr32->sh_addr);
				printf("sh_size: %x\n", shdr32->sh_size);
				printf("sh_link: %x\n", shdr32->sh_link);
				printf("sh_info: %x\n", shdr32->sh_info);
				printf("sh_addralign: %x\n", shdr32->sh_addralign);
				printf("sh_entsize: %x\n", shdr32->sh_entsize);
				printf("data: %x\n", data);
				printf("buf: %x\n", data->d_buf);
				printf("size: %d\n", data->d_size);
#endif
				break;
			}
		}
		else {
			if ((shdr64 = elf64_getshdr(scn)) == NULL)
				std::cerr << "getshdr() failed" << std::endl;

			// Retrieve the name of the section
			if ((section_name = elf_strptr(e, shstrndx, shdr64->sh_name)) == NULL)
				std::cerr << "elf_strptr() failed" << std::endl;

			if (!strcmp(section_name, ".text")) {
				if ((data = elf_rawdata(scn, data)) == NULL) {
					std::cerr << "No section data availible" << std::endl;
				}
#if 0
				printf("Section name: %s\n", section_name);
				printf("sh_offset: %lx\n", shdr64->sh_offset);
				printf("sh_type: %lx\n", shdr64->sh_type);
				printf("sh_flags: %lx\n", shdr64->sh_flags);
				printf("sh_addr: %lx\n", shdr64->sh_addr);
				printf("sh_size: %lx\n", shdr64->sh_size);
				printf("sh_link: %lx\n", shdr64->sh_link);
				printf("sh_info: %lx\n", shdr64->sh_info);
				printf("sh_addralign: %lx\n", shdr64->sh_addralign);
				printf("sh_entsize: %lx\n", shdr64->sh_entsize);
				printf("data: %lx\n", data);
				printf("buf: %lx\n", data->d_buf);
				printf("size: %d\n", data->d_size);
#endif
				break;
			}
		}
	}
	std::map<uint64_t, uint64_t> symtab_map;
	if (m_elf->m_elfclass == ELFCLASS32) {
		symtab_map = get_symtab_map(shdr32->sh_addr, shdr32->sh_size);
	} else if (m_elf->m_elfclass == ELFCLASS64) {
		symtab_map = get_symtab_map(shdr64->sh_addr, shdr64->sh_size);
	}

	disassemble_section(data, shdr32, shdr64, symtab_map);

	elf_end(e);
	close(fd);
}
