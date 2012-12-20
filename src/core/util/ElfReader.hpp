#ifndef __ELFREADER_HPP__
#define __ELFREADER_HPP__

#include <string>
#ifndef __puma
#include <boost/bimap.hpp>
#endif

#include "sal/SALConfig.hpp"
#include "Logger.hpp"

template< class MapType >
void print_map(const MapType & map)
{
  typedef typename MapType::const_iterator const_iterator;

  for( const_iterator i = map.begin(), iend = map.end(); i != iend; ++i )
  {
    std::cout << i->first << " -- " << i->second << std::endl;
  }
}

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
       * Get guest address by symbol name
       * @param name The symbol name as string
       * @return The according addres if found, else -1
       */	
      guest_address_t getAddressByName(const std::string& name) ;


      /**
       * Get symbol name associated to an address
       * This is interesting when checking instruction pointers.
       * @param name The address of a symbol (or around a symbol -> instruction pointer)
       * @return The according address if found, else -1
       * 
       * \todo multimap sorted by addresses
       * Name is at first key <= address
       */
      std::string getNameByAddress(guest_address_t address) ; 


    private:
      Logger m_log;

#ifndef __puma
      typedef boost::bimap< std::string, guest_address_t > bimap_t;
      typedef bimap_t::value_type entry;
      bimap_t m_bimap;
#endif
      int process_symboltable(int sect_num, FILE* fp);
  };

} // end-of-namespace fail

#endif //__ELFREADER_HPP__

