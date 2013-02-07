#include "ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include <stdio.h>
#include <cstdlib>

#include "Demangler.hpp"

namespace fail {

const std::string ElfReader::NOTFOUND = "[ELFReader] Function not found.";


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
}


int ElfReader::process_section(Elf32_Shdr *sect_hdr, char* sect_name_buff){
  // Add section name, start address and size to list
  int idx=sect_hdr->sh_name;
  m_sections_map.push_back( sect_hdr->sh_addr, sect_hdr->sh_size, sect_name_buff+idx );

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
#ifndef __puma
      m_bimap_mangled.insert( entry(name_buf+idx, mysym.st_value)  );
      m_bimap_demangled.insert( entry ( Demangler::demangle(name_buf+idx), mysym.st_value) );

#endif
    }
  }
  free (name_buf);
  return 0;
}

guest_address_t ElfReader::getAddressByName(const std::string& name) {
#ifndef __puma
  guest_address_t res = getAddress(m_bimap_demangled, name);
  if(res == ADDR_INV){
    res = getAddress(m_bimap_mangled, name);
  }
  return res;
#endif
}

#ifndef __puma
guest_address_t ElfReader::getAddress(const bimap_t& map, const std::string& name){
  typedef bimap_t::left_map::const_iterator const_iterator_t;

  const_iterator_t iterator = map.left.find(name);
  if(iterator == map.left.end()){
    return ADDR_INV;
  }else{
    return iterator->second;
  }
}
#endif

#ifndef __puma
std::string ElfReader::getName(const bimap_t& map, guest_address_t address){
  // .right switches key/value
  typedef bimap_t::right_map::const_iterator const_iterator_t;

  const_iterator_t iterator = map.right.find(address);
  if(iterator != map.right.end()){
    return iterator->second;
  }
  return NOTFOUND;
}

std::string ElfReader::getNameByAddress(guest_address_t address) {
  std::string res = getName(m_bimap_demangled, address);
  if(res == NOTFOUND){
    return getName(m_bimap_mangled, address);
  }
  return res;
}

std::string ElfReader::getMangledNameByAddress(guest_address_t address) {
  return getName(m_bimap_mangled, address);
}

std::string ElfReader::getDemangledNameByAddress(guest_address_t address) {
  return getName(m_bimap_demangled, address);
}

void ElfReader::printDemangled(){
  print_map(m_bimap_demangled.right); // print Address as first element
}

void ElfReader::printMangled(){
  print_map(m_bimap_mangled.right); // print Address as first element
}
#endif

#ifndef __puma
std::string ElfReader::getSection(guest_address_t address) {
  return m_sections_map.find_name_by(address);
}

guest_address_t ElfReader::getSectionStart(const std::string& sectionname) {
  SectionsMap::address_pair_t pair;
  pair = m_sections_map.find_range_by(sectionname);

  return pair.first;
}

guest_address_t ElfReader::getSectionEnd(const std::string& sectionname) {
  SectionsMap::address_pair_t pair = m_sections_map.find_range_by(sectionname);
  if ( pair.first == ADDR_INV ) {
    return ADDR_INV;
  }
  return pair.first + pair.second;
}

size_t ElfReader::getSectionSize(const std::string& sectionname) {
  SectionsMap::address_pair_t pair = m_sections_map.find_range_by(sectionname);
  return pair.second;
}

#endif

} // end-of-namespace fail

