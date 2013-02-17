#ifndef __ELFREADER_HPP__
#define __ELFREADER_HPP__

#include <string>

#ifndef __puma
#include <boost/bimap.hpp>
#endif

#include <ostream>
#include "sal/SALConfig.hpp" // for ADDR_INV
#include "Logger.hpp"
#include "elfinfo/elfinfo.h"
#include <vector>
#include <map>

namespace fail {

  /*
   * Helper struct for section information
   */
  struct SectionsMap {
    typedef std::pair<guest_address_t, size_t> address_pair_t;
    typedef std::vector< std::pair< address_pair_t, std::string> > container ;
    container section_to_name;

    void push_back(guest_address_t start, size_t size, std::string name)
    {
      section_to_name.push_back(std::make_pair(std::make_pair(start, size), name));
    }

    address_pair_t find_range_by(std::string name){
      for(container::iterator it = section_to_name.begin(), end = section_to_name.end(); it != end; ++it){
        container::value_type pair_pair_string = *it;
        typedef container::value_type::first_type section_type;

        if(pair_pair_string.second == name){
          return pair_pair_string.first;
        }
      }
      return std::make_pair(ADDR_INV, 0);
    }

    std::string find_name_by(guest_address_t address){
      for(container::iterator it = section_to_name.begin(), end = section_to_name.end(); it != end; ++it){
        container::value_type pair_pair_string = *it;
        typedef container::value_type::first_type section_type;
        section_type section = pair_pair_string.first;
        if(address >= section.first && address < section.first + section.second){
          return pair_pair_string.second;
        }
      }
      return std::string("SECTION_NOT_FOUND");
    }


  };


  /**
   * \class ElfReader
   * Parses an ELF file and provides a list of symbol names
   * and corresponding addresses
   */

  class ElfReader {

    public:

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
       * Get guest address by symbol name.
       * Both mangled an demangled symbols are searched.
       * @param name The symbol name as string
       * @return The according address if found, else ADDR_INV
       */
      guest_address_t getAddressByName(const std::string& name) ;

      /**
       * Get demangled symbol name associated to an address
       * This is interesting when checking instruction pointers.
       * @param name The address of a symbol (or around a symbol -> instruction pointer)
       * @return The according address if found, else ElfReader::NOTFOUND
       */
      std::string getNameByAddress(guest_address_t address) ;

      /**
       * Get the mangled symbol name associated to an address

       * @param name The address of a symbol (or around a symbol -> instruction pointer)
       * @return The according address if found, else ElfReader::NOTFOUND
       */
      std::string getMangledNameByAddress(guest_address_t address) ;

      /**
       * Get the demangled symbol name associated to an address
       * Note the the demangled name is simplified, not showing any types!
       * @param name The address of a symbol (or around a symbol -> instruction pointer)
       * @return The according address if found, else ElfReader::NOTFOUND
       */
      std::string getDemangledNameByAddress(guest_address_t address) ;

      /**
       * Print the list of available mangled symbols
       * @note This includes both C and C++ symbols
       */
       void printMangled();

      /**
       * Print the list of all available demangled symbols
       * @note These are only C++ symbols.
       */
       void printDemangled();

       //! Default string, if symbol is not found
       static const std::string NOTFOUND;

      /**
       * Get the name of a section
       * @param address The address of the section
       * @return The according section name if section was found, else SECTION_NOT_FOUND
       */
       std::string getSection(guest_address_t address);

      /**
       * Get the start address of a section
       * @param name The name of the section
       * @return The according section start if section was found, else ADDR_INV
       */
       guest_address_t getSectionStart(const std::string& name);

      /**
       * Get the end address of a section
       * @param name The name of the section
       * @return The according section end if section was found, else ADDR_INV
       */
       guest_address_t getSectionEnd(const std::string& name);

      /**
       * Get the size of a section
       * @param name The name of the section
       * @return The according section sizh if section was found, else ADDR_INV
       */
       guest_address_t getSectionSize(const std::string& name);

    private:
      Logger m_log;

      void setup(const char*);
      int process_symboltable(int sect_num, FILE* fp);
      int process_section(Elf32_Shdr *sect_hdr, char* sect_name_buff);

      fail::SectionsMap m_sections_map;

#ifndef __puma
      typedef boost::bimap< std::string, guest_address_t > bimap_t;
      typedef bimap_t::value_type entry;

      bimap_t m_bimap_mangled;
      bimap_t m_bimap_demangled;

      template < typename MapType >
        void print_map(const MapType  & m){
          typedef typename MapType::const_iterator const_iterator;
          for( const_iterator iter = m.begin(), iend = m.end(); iter != iend; ++iter )
          {
            m_log << std::hex  <<  iter->first << " \t "<< std::hex << iter->second << std::endl;
          }
        }

      guest_address_t getAddress(const bimap_t& map, const std::string& name);
      std::string getName(const bimap_t& map, guest_address_t address);
#endif
  };

} // end-of-namespace fail

#endif //__ELFREADER_HPP__

