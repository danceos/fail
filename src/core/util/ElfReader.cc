#include "ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include "Demangler.hpp"

namespace fail {

const std::string ELF::NOTFOUND = "[ELFReader] Function not found.";
static const ElfSymbol g_SymbolNotFound;

bool operator==(const std::string & str, const ElfSymbol & sym) {
	return sym.getName() == str;
}

bool operator==(guest_address_t address, const ElfSymbol & sym) {
	return sym.getAddress() == address;
}

std::ostream& operator<< (std::ostream &out, const ElfSymbol &symbol) {
	return (out << symbol.getName()
			<< " @ 0x" << std::hex << symbol.getAddress()
			<< " size " << std::dec << symbol.getSize());
}



ElfReader::ElfReader() : m_log("ElfReader", false) {
	// try to open elf file from environment variable
	char * elfpath = getenv("FAIL_ELF_PATH");
	if (elfpath == NULL) {
		m_log << "FAIL_ELF_PATH not set :(" << std::endl;
	} else {
		setup(elfpath);
	}
}

ElfReader::ElfReader(const char* path) : m_log("FAIL*Elfinfo", false) {
	setup(path);
}

void ElfReader::setup(const char* path) {
	// Try to open the ELF file
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		m_log << "Error: Could not open " << path << std::endl;
		return;
	}

	m_filename = std::string(path);

	// Evaluate headers
	Elf64_Ehdr ehdr;
	Elf64_Shdr sec_hdr;
	int num_hdrs;
	if (!read_ELF_file_header(fp, &ehdr)) {
		m_log << "Error: " << path << " is not an ELF file" << std::endl;
		return;
	}
	num_hdrs = ehdr.e_shnum;
	m_log << "Evaluating "
		<< (m_elfclass == ELFCLASS32 ? "32-bit" : "64-bit")
		<< " ELF file: " << path << std::endl;

	// Parse symbol table and generate internal map
	for (int i = 0; i < num_hdrs; ++i) {
		if (!read_ELF_section_header(fp, &ehdr, i, &sec_hdr)) {
			m_log << "Wrong section to read" << std::endl;
			continue;
		} else if (sec_hdr.sh_type != SHT_SYMTAB && sec_hdr.sh_type != SHT_DYNSYM) {
			continue;
		}
		process_symboltable(fp, &ehdr, i);
	}
	// Parse section information
	if (!read_ELF_section_header(fp, &ehdr, ehdr.e_shstrndx, &sec_hdr)) {
		m_log << "Error: reading section string table sect_num = " << ehdr.e_shstrndx << std::endl;
		return;
	}

	char *buff = (char*)malloc(sec_hdr.sh_size);
	if (!buff) {
		m_log << "Error: malloc failed to allocate buffer for shstrtab" << std::endl;
		return;
	}
	// seek to the offset in the file,
	fseek(fp, (off_t)sec_hdr.sh_offset, SEEK_SET);
	if (fread(buff, sec_hdr.sh_size, 1, fp) != 1) {
		printf("Couldn't read complete shstrtab\n");
		exit(0);
	}

	m_log << "Total number of sections: " << num_hdrs << std::endl;
	for (int i = 0; i < num_hdrs; ++i) {
		if (!read_ELF_section_header(fp, &ehdr, i, &sec_hdr)) {
			printf("Error: wrong Section to read\n");
		} else {
			process_section(&sec_hdr, buff);
		}
	}

	free(buff);
	fclose(fp);
}


int ElfReader::process_section(Elf64_Shdr *sect_hdr, char *sect_name_buff) {
	// Add section name, start address and size to list
	int idx=sect_hdr->sh_name;
	//  m_sections_map.push_back( sect_hdr->sh_addr, sect_hdr->sh_size, sect_name_buff+idx );
	m_sectiontable.push_back( ElfSymbol(sect_name_buff+idx, sect_hdr->sh_addr, sect_hdr->sh_size, ElfSymbol::SECTION)  );

	return 0;
}

static void Elf32to64_Sym(Elf32_Sym const *src, Elf64_Sym *dest)
{
	dest->st_name = src->st_name;
	dest->st_value = src->st_value;
	dest->st_size = src->st_size;
	dest->st_info = src->st_info;
	dest->st_other = src->st_other;
	dest->st_shndx = src->st_shndx;
}

bool ElfReader::process_symboltable(FILE *fp, Elf64_Ehdr const *ehdr, int sect_num) {

	Elf64_Shdr sect_hdr;
	Elf32_Sym mysym32;
	Elf64_Sym mysym;
	char *name_buf = 0;
	int num_sym, link, idx;
	off_t sym_data_offset;
	int sym_data_size;

	if (!read_ELF_section_header(fp, ehdr, sect_num, &sect_hdr)) {
		return false;
	}
	// we have to check to which strtab it is linked
	link = sect_hdr.sh_link;
	sym_data_offset = sect_hdr.sh_offset;
	sym_data_size = sect_hdr.sh_size;
	num_sym = sym_data_size / 
		(m_elfclass == ELFCLASS32 ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym));

	// read the corresponding strtab
	if (!read_ELF_section_header(fp, ehdr, link, &sect_hdr)) {
		return false;
	}
	// get the size of strtab in file and allocate a buffer
	name_buf = (char *) malloc(sect_hdr.sh_size);
	if (!name_buf) {
		return false;
	}
	// get the offset of strtab in file and seek to it
	fseek(fp, sect_hdr.sh_offset, SEEK_SET);
	// read all data from the section to the buffer.
	if (fread(name_buf, sect_hdr.sh_size, 1, fp) != 1) {
		return false;
	}
	// so we have the namebuf now seek to symtab data
	fseek(fp, sym_data_offset, SEEK_SET);

	for (int i = 0; i < num_sym; ++i) {
		int type = ELFCLASSNONE;
		if (m_elfclass == ELFCLASS32) {
			if (fread(&mysym32, sizeof(mysym32), 1, fp) != 1) {
				return false;
			}
			Elf32to64_Sym(&mysym32, &mysym);
			type = ELF32_ST_TYPE(mysym32.st_info);
		} else if (m_elfclass == ELFCLASS64) {
			if (fread(&mysym, sizeof(mysym), 1, fp) != 1) {
				return false;
			}
			type = ELF64_ST_TYPE(mysym.st_info);
		}
		idx = mysym.st_name;

		if (type != STT_SECTION && type != STT_FILE) {
			m_symboltable.push_back(
				ElfSymbol(name_buf + idx, mysym.st_value, mysym.st_size, ElfSymbol::SYMBOL, type));
		}
	}
	free(name_buf);
	return true;
}

static void Elf32to64_Ehdr(Elf32_Ehdr const *src, Elf64_Ehdr *dest)
{
	memcpy(dest->e_ident, src->e_ident, sizeof(dest->e_ident));
	dest->e_type = src->e_type;
	dest->e_machine = src->e_machine;
	dest->e_version = src->e_version;
	dest->e_entry = src->e_entry;
	dest->e_phoff = src->e_phoff;
	dest->e_shoff = src->e_shoff;
	dest->e_flags = src->e_flags;
	dest->e_ehsize = src->e_ehsize;
	dest->e_phentsize = src->e_phentsize;
	dest->e_phnum = src->e_phnum;
	dest->e_shentsize = src->e_shentsize;
	dest->e_shnum = src->e_shnum;
	dest->e_shstrndx = src->e_shstrndx;
}

bool ElfReader::read_ELF_file_header(FILE *fp, Elf64_Ehdr *filehdr)
{
	Elf32_Ehdr filehdr32;
	size_t ret;

	rewind(fp);
	ret = fread(&filehdr32, sizeof(filehdr32), 1, fp);

	if (ret != 1 ||
		strncmp((const char *)filehdr32.e_ident, "\177ELF", 4) != 0) {
		return false;
	}

	m_machine = filehdr32.e_machine;
	m_elfclass = filehdr32.e_ident[EI_CLASS];
	if (m_elfclass == ELFCLASS32) {
		Elf32to64_Ehdr(&filehdr32, filehdr);
	} else if (m_elfclass == ELFCLASS64) {
		rewind(fp);
		ret = fread(filehdr, sizeof(*filehdr), 1, fp);

		if (ret != 1) {
			return false;
		}
	} else {
		return false;
	}
	return true;
}

static void Elf32to64_Shdr(Elf32_Shdr const *src, Elf64_Shdr *dest)
{
	dest->sh_name = src->sh_name;
	dest->sh_type = src->sh_type;
	dest->sh_flags = src->sh_flags;
	dest->sh_addr = src->sh_addr;
	dest->sh_offset = src->sh_offset;
	dest->sh_size = src->sh_size;
	dest->sh_link = src->sh_link;
	dest->sh_info = src->sh_info;
	dest->sh_addralign = src->sh_addralign;
	dest->sh_entsize = src->sh_entsize;
}

bool ElfReader::read_ELF_section_header(FILE *fp, Elf64_Ehdr const *filehdr, int section, Elf64_Shdr *sect_hdr)
{
	int numsect = filehdr->e_shnum;
	off_t sect_hdr_off;
	Elf32_Shdr sect_hdr32;

	if (numsect < section || section < 0) {
		return false;
	}
	sect_hdr_off = filehdr->e_shoff;
	sect_hdr_off += filehdr->e_shentsize * section;
	fseek(fp, (off_t) sect_hdr_off, SEEK_SET);
	if (m_elfclass == ELFCLASS32) {
		int ret = fread(&sect_hdr32, sizeof(sect_hdr32), 1, fp);
		if (ret != 1) {
			return false;
		}

		Elf32to64_Shdr(&sect_hdr32, sect_hdr);
	} else if (m_elfclass == ELFCLASS64) {
		int ret = fread(sect_hdr, sizeof(*sect_hdr), 1, fp);
		if (ret != 1) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

const ElfSymbol& ElfReader::getSymbol(guest_address_t address) {
	for (container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it) {
		if (it->contains(address)) {
			return *it;
		}
	}

	return g_SymbolNotFound;
}

// Symbol search
const ElfSymbol& ElfReader::getSymbol( const std::string& name ) {
	container_t::const_iterator it;
	// Fist, try to find as mangled symbol
	it = std::find(m_symboltable.begin(), m_symboltable.end(), name);
	if (it != m_symboltable.end()) {
		return *it;
	}

	// Then, try to find as demangled symbol
	std::string dname = Demangler::demangle(name);
	if (dname == Demangler::DEMANGLE_FAILED) {
		return g_SymbolNotFound;
	}

	it = std::find(m_symboltable.begin(), m_symboltable.end(), dname);
	if (it != m_symboltable.end()) {
		return *it;
	}

	return g_SymbolNotFound;
}

// Section search
const ElfSymbol& ElfReader::getSection(guest_address_t address) {
	for (container_t::const_iterator it = m_sectiontable.begin(); it != m_sectiontable.end(); ++it) {
		if (it->contains(address)) {
			return *it;
		}
	}
	return g_SymbolNotFound;
}

const ElfSymbol& ElfReader::getSection( const std::string& name ) {
	for (container_t::const_iterator it = m_sectiontable.begin(); it !=m_sectiontable.end(); ++it) {
		if (it->getName() == name) {
			return *it;
		}
	}
	return g_SymbolNotFound;
}

// "Pretty" Print
void ElfReader::printDemangled() {
	m_log << "Demangled: " << std::endl;
	for (container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it) {
		std::string str = Demangler::demangle(it->getName());
		if (str == Demangler::DEMANGLE_FAILED) {
			str = it->getName();
		}
		m_log << "0x"  << std::hex << it->getAddress()  << "\t" << str.c_str() << "\t"  << it->getSize() << std::endl;
	}
}

void ElfReader::printMangled() {
	for (container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it) {
		m_log  << "0x" << std::hex << it->getAddress() << "\t" << it->getName().c_str() << "\t" << it->getSize() << std::endl;
	}
}

void ElfReader::printSections() {
	for (container_t::const_iterator it = m_sectiontable.begin(); it !=m_sectiontable.end(); ++it) {
		m_log  << "0x"  << it->getAddress() << "\t" << it->getName().c_str() << "\t" << it->getSize() << std::endl;
	}
}

} // end-of-namespace fail
