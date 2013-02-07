#ifndef __ELFREADER_HPP__
#define __ELFREADER_HPP__

#include <string>

#ifndef __puma
#include <boost/bimap.hpp>
#endif

#include "sal/SALConfig.hpp" // for ADDR_INV
#include "Logger.hpp"

namespace fail {

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

    private:
      Logger m_log;


      void setup(const char*);
      int process_symboltable(int sect_num, FILE* fp);
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
            m_log << std::hex  <<  iter->first << " - "<< std::hex << iter->second << std::endl;
          }
        }

      guest_address_t getAddress(const bimap_t& map, const std::string& name);
      std::string getName(const bimap_t& map, guest_address_t address);
#endif
  };

} // end-of-namespace fail

#endif //__ELFREADER_HPP__

