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



ElfReader::ElfReader() : m_log("Fail*Elfinfo", false){
  // try to open elf file from environment variable
  char * elfpath = getenv("FAIL_ELF_PATH");
  if(elfpath == NULL){
    m_log << "FAIL_ELF_PATH not set :(" << std::endl;
  }else{
    setup(elfpath);
  }
}

ElfReader::ElfReader(const char* path) : m_log("Fail*Elfinfo", false){
  setup(path);
}

void ElfReader::setup(const char* path) {
  // Try to open the ELF file
  FILE * fp = fopen(path, "r");
  if (!fp) {
    m_log << "Error: Could not open " << path << std::endl;
    return;
  }

  // Evaluate headers
  Elf32_Ehdr ehdr;
  Elf32_Shdr sec_hdr;
  int num_hdrs,i;
  fseek(fp,(off_t)0,SEEK_SET);
  read_ELF_file_header(fp, &ehdr);
  num_hdrs=ehdr.e_shnum;
  m_log << "Evaluating ELF File: " << path << std::endl;
  // Parse symbol table and generate internal map
  for(i=0;i<num_hdrs;i++)
  {
    if(read_ELF_section_header(i,&sec_hdr,fp)==-1)
    {
      m_log << "Wrong Section to read" << std::endl;
    }
    else
    {
      if((sec_hdr.sh_type==SHT_SYMTAB)||(sec_hdr.sh_type==SHT_DYNSYM))
      {
        process_symboltable(i,fp);
      }
      else
      {
        continue;
      }
    }
  }
  // Parse section information
  if(read_ELF_section_header(ehdr.e_shstrndx,&sec_hdr,fp)==-1)
  {
    m_log << "Error: reading section string table sect_num = " << ehdr.e_shstrndx << std::endl;
  }

  char* buff=(char*)malloc(sec_hdr.sh_size);
  if (!buff)
  {
    m_log << "Malloc failed to allocate buffer for shstrtab" << std::endl;
    exit(0);
  }
  //seek to the offset in the file,
  fseek(fp,(off_t)sec_hdr.sh_offset,SEEK_SET);
  fread(buff,sec_hdr.sh_size,1,fp);
  m_log << "Total number of sections: " << num_hdrs << std::endl;

  for(i=0;i<num_hdrs;i++)
  {
    if(read_ELF_section_header(i,&sec_hdr,fp)==-1)
    {
      m_log << "Wrong Section to read\n" << std::endl;
    }
    else
    {
      process_section(&sec_hdr, buff);
    }
  }
  if(buff)
    free(buff);

  fclose(fp);

 // printDemangled();
 // printSections();
}


int ElfReader::process_section(Elf32_Shdr *sect_hdr, char* sect_name_buff){
  // Add section name, start address and size to list
  int idx=sect_hdr->sh_name;
//  m_sections_map.push_back( sect_hdr->sh_addr, sect_hdr->sh_size, sect_name_buff+idx );
  m_sectiontable.push_back( ElfSymbol(sect_name_buff+idx, sect_hdr->sh_addr, sect_hdr->sh_size, ElfSymbol::SECTION)  );

  return 0;
}

int ElfReader::process_symboltable(int sect_num, FILE* fp){

  Elf32_Shdr sect_hdr;
  Elf32_Sym mysym;
  char *name_buf=NULL;
  int num_sym,link,i,idx;
  off_t sym_data_offset;
  int sym_data_size;
  if(read_ELF_section_header(sect_num,&sect_hdr,fp)==-1)
  {
    return -1;
  }
  //we have to check to which strtab it is linked
  link=sect_hdr.sh_link;
  sym_data_offset=sect_hdr.sh_offset;
  sym_data_size=sect_hdr.sh_size;
  num_sym=sym_data_size/sizeof(Elf32_Sym);

  //read the coresponding strtab
  if(read_ELF_section_header(link,&sect_hdr,fp)==-1)
  {
    return -1;
  }
  //get the size of strtab in file and allocate a buffer
  name_buf=(char*)malloc(sect_hdr.sh_size);
  if(!name_buf)
    return -1;
  //get the offset of strtab in file and seek to it
  fseek(fp,sect_hdr.sh_offset,SEEK_SET);
  //read all data from the section to the buffer.
  fread(name_buf,sect_hdr.sh_size,1,fp);
  //so we have the namebuf now seek to symtab data
  fseek(fp,sym_data_offset,SEEK_SET);

  for(i=0;i<num_sym;i++)
  {

    fread(&mysym,sizeof(Elf32_Sym),1,fp);
    idx=mysym.st_name;

    int type = ELF32_ST_TYPE(mysym.st_info);
    if((type != STT_SECTION) && (type != STT_FILE)){
      m_symboltable.push_back( ElfSymbol(name_buf+idx, mysym.st_value, mysym.st_size, ElfSymbol::SYMBOL,
                                         type) );
    }
  }
  free (name_buf);
  return 0;
}

const ElfSymbol& ElfReader::getSymbol(guest_address_t address){
  for(container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it){
    if(it->contains(address)){
      return *it;
    }
  }

  return g_SymbolNotFound;
}

// Symbol search
const ElfSymbol& ElfReader::getSymbol( const std::string& name ){
  container_t::const_iterator it;
  // Fist, try to find as mangled symbol
  it = std::find(m_symboltable.begin(), m_symboltable.end(), name);
  if(it != m_symboltable.end()){
    return *it;
  }

  // Then, try to find as demangled symbol
  std::string dname = Demangler::demangle(name);
  if(dname == Demangler::DEMANGLE_FAILED){
    return g_SymbolNotFound;
  }

  it = std::find(m_symboltable.begin(), m_symboltable.end(), dname);
  if(it != m_symboltable.end()){
    return *it;
  }

  return g_SymbolNotFound;
}

// Section search
const ElfSymbol& ElfReader::getSection(guest_address_t address){
  for(container_t::const_iterator it = m_sectiontable.begin(); it != m_sectiontable.end(); ++it){
    if(it->contains(address)){
      return *it;
    }
  }
  return g_SymbolNotFound;
}

const ElfSymbol& ElfReader::getSection( const std::string& name ){
  for(container_t::const_iterator it = m_sectiontable.begin(); it !=m_sectiontable.end(); ++it){
    if(it->getName() == name){
      return *it;
    }
  }
  return g_SymbolNotFound;
}

// "Pretty" Print
void ElfReader::printDemangled(){
  m_log << "Demangled: " << std::endl;
  for(container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it){
    std::string str = Demangler::demangle(it->getName());
    if(str == Demangler::DEMANGLE_FAILED){
      str = it->getName();
    }
    m_log << "0x"  << std::hex << it->getAddress()  << "\t" << str.c_str() << "\t"  << it->getSize() << std::endl;
  }
}

void ElfReader::printMangled(){
  for(container_t::const_iterator it = m_symboltable.begin(); it !=m_symboltable.end(); ++it){
    m_log  << "0x" << std::hex << it->getAddress() << "\t" << it->getName().c_str() << "\t" << it->getSize() << std::endl;
  }
}

void ElfReader::printSections() {
  for(container_t::const_iterator it = m_sectiontable.begin(); it !=m_sectiontable.end(); ++it){
    m_log  << "0x"  << it->getAddress() << "\t" << it->getName().c_str() << "\t" << it->getSize() << std::endl;
  }
}


} // end-of-namespace fail

