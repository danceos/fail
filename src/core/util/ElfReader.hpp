#ifndef __ELFREADER_HPP__
#define __ELFREADER_HPP__

#include <string>
#include <ostream>
#include <vector>
#include <map>
#include <elf.h>
#include "sal/SALConfig.hpp" // for ADDR_INV
#include "Logger.hpp"
#include "Demangler.hpp"
#include <string.h>
#include <cassert>

namespace fail {

struct ELF {
	static const std::string NOTFOUND;
};

class ElfSegment {
    private:
        guest_address_t m_paddress;
        guest_address_t m_vaddress;
        size_t m_size;
        unsigned m_flags;

    public:
        ElfSegment(Elf64_Phdr const * hdr): m_paddress(hdr->p_paddr), m_vaddress(hdr->p_vaddr), m_size(hdr->p_memsz), m_flags(hdr->p_flags) { }

        bool isReadable() const { return m_flags & PF_R; }
        bool isWriteable() const { return m_flags & PF_W; }
        bool isExecutable() const { return m_flags & PF_X; }
        guest_address_t getStart() const { assert(m_paddress == m_vaddress); return m_paddress; }
        guest_address_t getEnd() const { assert(m_paddress == m_vaddress); return m_paddress + m_size; }
        guest_address_t getSize() const { assert(m_paddress == m_vaddress); return m_size; }
        friend std::ostream& operator << (std::ostream&,const ElfSegment&);
};
inline std::ostream& operator<< (std::ostream &out, const ElfSegment &s) {
            return out << std::hex << std::showbase
                << "Elf Segment @ physical: " << s.m_paddress << " virtual: " << s.m_vaddress << " size: " << s.m_size << " (" << (s.isReadable() ? "R": " ") << (s.isWriteable() ? "W" : " ") << (s.isExecutable() ? "X" : " ") << ")"
                << std::dec << std::noshowbase;
        }

class ElfSymbol {
	std::string name;
	guest_address_t address;
	size_t size;
	int m_type;
	int m_symbol_type;

	public:
	enum { SECTION = 1, SYMBOL = 2, UNDEF = 3, };

	ElfSymbol(const std::string & name = ELF::NOTFOUND, guest_address_t addr = ADDR_INV,
		size_t size = -1, int type = UNDEF, int symbol_type = 0)
		: name(name), address(addr), size(size), m_type(type), m_symbol_type(symbol_type) {}

	const std::string& getName() const { return name; }
	std::string getDemangledName() const { return Demangler::demangle(name); }
	guest_address_t getAddress() const { return address; }
	size_t getSize() const { return size; }
	guest_address_t getStart() const { return getAddress(); } // alias
	guest_address_t getEnd() const { return address + size; }
	int getSymbolType() const { return m_symbol_type; }

	bool isSection() const { return m_type == SECTION; }
	bool isSymbol()  const { return m_type == SYMBOL; }
	bool isValid()   const { return name != ELF::NOTFOUND; }

	bool operator==(const std::string& rhs) const {
		if (rhs == name) {
			return true;
		}
		if ( rhs == Demangler::demangle(name) ) {
			return true;
		}

		return false;
	}

	bool operator==(const guest_address_t rhs) const {
		return rhs == address;
	}

	bool contains(guest_address_t ad) const {
		return (ad >= address) && (ad < address+size);
	}
};
/**
 * \fn
 * \relates ElfSymbol
 * overloaded stream operator for printing ElfSymbol
 */
std::ostream& operator<< (std::ostream &out, const ElfSymbol &symbol);

/**
 * \class ElfReader
 * Parses an ELF file and provides a list of symbol names
 * and corresponding addresses
 */

class ElfReader {
public:
	typedef ElfSymbol entry_t;
	typedef std::vector<entry_t> container_t;
	typedef container_t::const_iterator symbol_iterator;
	typedef container_t::const_iterator section_iterator;
    typedef ElfSegment seg_entry_t;
    typedef std::vector<seg_entry_t> seg_container_t;
    typedef seg_container_t::const_iterator segment_iterator;


	/**
	 * Constructor.
	 * @param path Path to the ELF file.
	 */
	ElfReader(const char* path);

	/**
	 * Constructor.
	 * @note The path is guessed from a FAIL_ELF_PATH environment variable
	 */
	ElfReader();

	/**
	 * Print the list of available mangled symbols
	 * @note This includes both C and C++ symbols
	 */
	void printMangled();

	/**
	 * Print a list of demangled symbols.
	 */
	void printDemangled();

	/**
	 * Print the list of all available sections.
     * FIXME: This prints sections _AND_ segments by default, it should be fixed once
     *        all usages have been evaluated.
	 */
	void printSections();

    /**
     * Print List of all available segments
     */
    void printSegments();

	/**
	 * Get symbol by address
	 * @param address Address within range of the symbol
	 * @return The according symbol name if symbol.address <= address < symbol.address + symbol.size , else g_SymbolNotFound
	 */
	const ElfSymbol& getSymbol(guest_address_t address);

	/**
	 * Get symbol by name
	 * @param address Name of the symbol
	 * @return The according symbol name if section was found, else g_SymbolNotFound
	 */
	const ElfSymbol& getSymbol( const std::string& name );

	/**
	 * Get section by address
	 * @param address An address to search for a section containing that address.
	 * @return The according section name if section was found, else g_SymbolNotFound
	 */
	const ElfSymbol& getSection(guest_address_t address);

	/**
	 * Get section by name
	 * @param name The name of the section
	 * @return The according section if section was found, else g_SymbolNotFound
	 */
	const ElfSymbol& getSection( const std::string& name );

	/**
	 * Get symboltable iterator. Derefences to a ElfSymbol
	 * @return iterator
	 */
	container_t::const_iterator sym_begin() { return m_symboltable.begin(); }
	container_t::const_iterator sym_end() { return m_symboltable.end(); }

	/**
	 * Get section iterator. Derefences to a ElfSymbol
	 * @return iterator
	 */
	container_t::const_iterator sec_begin() { return m_sectiontable.begin(); }
	container_t::const_iterator sec_end() { return m_sectiontable.end(); }



    /**
     * Get segment iterator. Dereferences to an ElfSymbol
     * @return iterator
     */
    seg_container_t::const_iterator seg_begin() { return m_segmenttable.begin(); }
    seg_container_t::const_iterator seg_end() { return m_segmenttable.end(); }

    /**
     * Return minimal and maximal valid address in ELF, this uses the ELF segment headers
     * and assumes that there are no holes in the ELFs memory layout.
     */
    guest_address_t getMinimumAddress();
    guest_address_t getMaximumAddress();

	const std::string & getFilename() { return m_filename; }

private:
	Logger m_log;
	std::string m_filename;
	int m_elfclass;

	void setup(const char*);
	bool process_symboltable(FILE *fp, Elf64_Ehdr const *ehdr, int sect_num);
	int process_section(Elf64_Shdr *sect_hdr, char *sect_name_buff);
    void process_segment(Elf64_Phdr *seg_hdr);

	// Returns true if it finds a valid ELF header.  Stores ELFCLASS32 or 64 in m_elfclass.
	bool read_ELF_file_header(FILE *fp, Elf64_Ehdr *ehdr);
	// Returns true if it finds a valid ELF section header.
	bool read_ELF_section_header(FILE *fp, Elf64_Ehdr const *filehdr, int sect_num, Elf64_Shdr *sect_hdr);
    // Return true if it finds a valid ELF program header
    bool read_ELF_segment_header(FILE* fp, Elf64_Ehdr const *filehdr, unsigned seg_num, Elf64_Phdr* seg_hdr);

	container_t m_symboltable;
	container_t m_sectiontable;
    seg_container_t m_segmenttable;

	guest_address_t getAddress(const std::string& name);
	std::string getName(guest_address_t address);

};

} // end-of-namespace fail

#endif //__ELFREADER_HPP__
