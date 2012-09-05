#ifndef __ELFREADER_HPP__
  #define __ELFREADER_HPP__

#include <string>
#include <map>

#include "sal/SALConfig.hpp"
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
	std::map<std::string, guest_address_t> m_map;	

	int process_symboltable(int sect_num, FILE* fp);
};

} // end-of-namespace fail

#endif //__ELFREADER_HPP__

