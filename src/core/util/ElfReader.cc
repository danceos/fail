#include "ElfReader.hpp"
#include "elfinfo/elfinfo.h"
#include <stdio.h>
#include <cstdlib>

namespace fail {

ElfReader::ElfReader(const char* path) : m_log("Fail*Elfinfo", false){
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

	fclose(fp);
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

	m_log << "[section " << sect_num << "] contains " << num_sym << " symbols." << std::endl;
  for(i=0;i<num_sym;i++)
  {
                          
          fread(&mysym,sizeof(Elf32_Sym),1,fp);
          idx=mysym.st_name;

		   		int type = ELF32_ST_TYPE(mysym.st_info);
					if((type != STT_SECTION) && (type != STT_FILE)){
						m_log << " " <<  (i) << " " << name_buf+idx  << " @ "  << mysym.st_value << std::endl;
						m_map[name_buf+idx] = mysym.st_value;
					}

	}

  free (name_buf);
	return 0;
}


guest_address_t ElfReader::getAddressByName(const std::string& name) {
				if( m_map.find(name) == m_map.end() ) {
					return -1;
				}else{
					return m_map[name];
				}
}

std::string getNameByAddress(guest_address_t address) {
				return "Test";
}

} // end-of-namespace fail

