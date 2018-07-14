/* DISCLAIMER :This is a FREE SOFTWARE You can Use it at your own RISK
   that is if you are screwed by using this software, then
   accept the fact that you screwed yourself.
## THIS HAS BEEN DEVELOPED AND TESTED UNDER FEDORA-9 HOPEFULLY IT WILL
## WORK ON OTHER SYSTEMS
======================================================================
elfinfo.c - Copyright (C)2008 Ashok Shankar Das ashok.s.das@gmail.com

This code is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <elf.h>

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

bool read_ELF_file_header(FILE *fp, Elf64_Ehdr *filehdr, int *elfclass)
{
	Elf32_Ehdr filehdr32;
	size_t ret;

	rewind(fp);
	ret = fread(&filehdr32, sizeof(filehdr32), 1, fp);

	if (ret != 1 ||
		strncmp((const char *)filehdr32.e_ident, "\177ELF", 4) != 0) {
		return false;
	}

	*elfclass = filehdr32.e_ident[EI_CLASS];
	if (*elfclass == ELFCLASS32) {
		Elf32to64_Ehdr(&filehdr32, filehdr);
	} else if (*elfclass == ELFCLASS64) {
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

bool read_ELF_section_header(FILE *fp, Elf64_Ehdr const *filehdr, int elfclass, int section, Elf64_Shdr *sect_hdr)
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
	if (elfclass == ELFCLASS32) {
		int ret = fread(&sect_hdr32, sizeof(sect_hdr32), 1, fp);
		if (ret != 1) {
			return false;
		}

		Elf32to64_Shdr(&sect_hdr32, sect_hdr);
	} else if (elfclass == ELFCLASS64) {
		int ret = fread(sect_hdr, sizeof(*sect_hdr), 1, fp);
		if (ret != 1) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

#ifdef TESTPROGRAM
void process_sect_hdr(Elf64_Shdr const *sect_hdr, char const *sect_name_buff)
{
	int idx;
	idx = sect_hdr->sh_name;
	printf(" %-20s (0x%lx++0x%lx) ", sect_name_buff + idx, sect_hdr->sh_addr, sect_hdr->sh_size);
	if (sect_hdr->sh_entsize) {
		printf(" %c ",'*');
	} else {
		printf(" %c ",' ');
	}
	switch (sect_hdr->sh_type) {
	case 0: printf("NULL           \t");
			break;
	case 1: printf("PROGBITS       \t");
			break;
	case 2: printf("SYMTAB         \t");
			break;
	case 3: printf("STRTAB         \t");
			break;
	case 4: printf("RELOC ADN      \t");
			break;
	case 5: printf("HASH           \t");
			break;
	case 6: printf("SYMTAB         \t");
			break;
	case 7: printf("NOTE           \t");
			break;
	case 8: printf("NOBIT          \t");
			break;
	case 9: printf("RELOC          \t");
			break;
	case 10: printf("*reserved*     \t");
			 break;
	case 11: printf("DYNSYM         \t");
			 break;
	case 14: printf("INITARR        \t");
			 break;
	case 15: printf("FINIARR        \t");
			 break;
	case 16: printf("PREINIT        \t");
			 break;
	case 17: printf("SECTGRP        \t");
			 break;
	case 18: printf("EXT-SHNDX      \t");
			 break;
	case 19: printf("NUM-DEF-TYP    \t");
			 break;
	case 0x60000000: printf("OS SPECIFIC    \t");
					 break;
	case 0x6ffffff5: printf("GNU_ATTR       \t");
					 break;
	case 0x6ffffff6: printf("GNU_HASH       \t");
					 break;
	case 0x6ffffff7: printf("PRELNKLIBLST   \t");
					 break;
	case 0x6ffffff8: printf("CKSUM          \t");
					 break;
	case 0x6ffffffa: printf("SUN SPECIFIC   \t");
					 break;
	case 0x6ffffffb: printf("SUNCOMDAT      \t");
					 break;
	case 0x6ffffffc: printf("SUNSYMINFO     \t");
					 break;
	case 0x6ffffffd: printf("GNUVERDEF      \t");
					 break;
	case 0x6ffffffe: printf("GNUVERNEED     \t");
					 break;
	case 0x6fffffff: printf("GNUVERSYM      \t");
					 break;
	case 0x70000000: printf("LOPROC         \t");
					 break;
	case 0x7fffffff: printf("HIPROC         \t");
					 break;
	case 0x80000000: printf("APP beg        \t");
					 break;
	case 0x8fffffff: printf("APP end        \t");
					 break;
	}
	printf("linked to %3d sect\n", sect_hdr->sh_link);
}

void display_sections(FILE *fp)
{
	Elf64_Ehdr ehdr;
	Elf64_Shdr sec_hdr;
	int num_hdrs, elfclass;
	char *buff = NULL;

	if (!read_ELF_file_header(fp, &ehdr, &elfclass)) {
		printf("Not an ELF file\n");
		exit(0);
	}
	num_hdrs = ehdr.e_shnum;
	if (!read_ELF_section_header(fp, &ehdr, elfclass, ehdr.e_shstrndx, &sec_hdr)) {
		printf("Error: reading section string table sect_num= %d\n", ehdr.e_shstrndx);
		exit(0);
	}
	//we read the shstrtab
	// read content from the file
	buff = (char *)malloc(sec_hdr.sh_size);
	if (!buff) {
		printf("Malloc failed to allocate buffer for shstrtab\n");
		exit(0);
	}
	//seek to the offset in the file,
	fseek(fp, (off_t)sec_hdr.sh_offset, SEEK_SET);
	if (fread(buff, sec_hdr.sh_size, 1, fp) != 1) {
		printf("Couldn't read complete shstrtab\n");
		exit(0);
	}

	printf("There are [%d] sections\n",num_hdrs);
	for (int i = 0; i < num_hdrs; ++i) {
		if (!read_ELF_section_header(fp, &ehdr, elfclass, i, &sec_hdr)) {
			printf("Wrong Section to read\n");
		} else {
			printf("[section %3d] ", i);
			process_sect_hdr(&sec_hdr, buff);
        }
	}

	free(buff);
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

int process_symtab(FILE *fp, Elf64_Ehdr *ehdr, int elfclass, int sect_num)
{
	Elf64_Shdr sect_hdr;
	Elf32_Sym mysym32;
	Elf64_Sym mysym;
	char *name_buf = NULL;
	int num_sym, link, idx;
	off_t sym_data_offset;
	int sym_data_size;

	if (!read_ELF_section_header(fp, ehdr, elfclass, sect_num, &sect_hdr)) {
		return -1;
	}
	//we have to check to which strtab it is linked
	link = sect_hdr.sh_link;
	sym_data_offset = sect_hdr.sh_offset;
	sym_data_size = sect_hdr.sh_size;
	num_sym = sym_data_size / 
		(elfclass == ELFCLASS32 ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym));

	//read the coresponding strtab
	if (!read_ELF_section_header(fp, ehdr, elfclass, link, &sect_hdr)) {
		return -1;
	}
	//get the size of strtab in file and allocate a buffer
	name_buf = (char *) malloc(sect_hdr.sh_size);
	if (!name_buf) {
		return -1;
	}
	//get the offset of strtab in file and seek to it
	fseek(fp, sect_hdr.sh_offset, SEEK_SET);
	//read all data from the section to the buffer.
	if (fread(name_buf, sect_hdr.sh_size, 1, fp) != 1) {
		return -1;
	}
	//so we have the namebuf now seek to symtab data
	fseek(fp, sym_data_offset, SEEK_SET);
	printf("[%d] symbols\n", num_sym);
	for (int i = 0; i < num_sym; ++i) {
		if (elfclass == ELFCLASS32) {
			if (fread(&mysym32, sizeof(mysym32), 1, fp) != 1) {
				return -1;
			}
			Elf32to64_Sym(&mysym32, &mysym);
		} else if (elfclass == ELFCLASS64) {
			if (fread(&mysym, sizeof(mysym), 1, fp) != 1) {
				return -1;
			}
		}
		idx = mysym.st_name;
		//printf("symbol %d index in strtab %d\n",i,idx);
		printf("[%3d] %s ", i, name_buf + idx);
		int binding = elfclass == ELFCLASS32 ?
			ELF32_ST_BIND(mysym.st_info) :
			ELF64_ST_BIND(mysym.st_info);
		switch (binding) {
		case 0: printf(" LOCAL ");
				break;
		case 1: printf(" GLOBAL ");
				break;
		case 2: printf(" WEAK ");
				break;
		case 3: printf(" NUM ");
				break;
		case 10: printf(" OS-specific-start ");
				 break;
		case 12: printf(" OS-specific-end ");
				 break;
		case 13: printf(" CPU-specific-start ");
				 break;
		case 15: printf("CPU-specific-end ");
				 break;
		}

		int type = elfclass == ELFCLASS32 ?
			ELF32_ST_TYPE(mysym.st_info) :
			ELF64_ST_TYPE(mysym.st_info);
		switch (type) {
		case 0: printf(" NOTYPE ");
				break;
		case 1: printf(" OBJECT ");
				break;
		case 2: printf(" FUNC ");
				break;
		case 3: printf(" SECTION ");
				break;
		case 4: printf(" FILE ");
				break;
		case 5: printf(" COMMON ");
				break;
		case 6: printf(" TLS ");
				break;
		case 7: printf(" NUM ");
				break;
		case 10: printf(" OS-specific-start ");
				 break;
				 break;
		case 12: printf(" OS-specific-end ");
				 break;
		case 13: printf(" CPU-specific-start ");
				 break;
				 break;
		case 15: printf(" CPU-specific-end ");
				 break;
		}
		switch (mysym.st_shndx) {
		case SHN_UNDEF: printf(" UNDEF ");
						break;
		case SHN_LOPROC: printf(" CPU-specific-start ");
						 break;
		case SHN_HIPROC: printf(" CPU-specific-end ");
						 break;
		case SHN_LOOS: printf(" OS-specific-start ");
					   break;
		case SHN_HIOS: printf(" OS-specific-end ");
					   break;
		case SHN_ABS: printf(" ABSOLUTE ");
					  break;
		case SHN_COMMON: printf(" COMMON ");
						 break;
		case SHN_XINDEX: printf(" XINDEX ");
						 break;
		}
		printf("\n");

	}
	free(name_buf);
	return 0;
}

void dump_symbols(FILE *fp)
{
	Elf64_Ehdr ehdr;
	Elf64_Shdr sec_hdr;
	int num_hdrs, elfclass;

	if (!read_ELF_file_header(fp, &ehdr, &elfclass)) {
		printf("Not an ELF file\n");
		exit(0);
	}

	num_hdrs = ehdr.e_shnum;
	for (int i = 0; i < num_hdrs; ++i) {
		if (!read_ELF_section_header(fp, &ehdr, elfclass, i, &sec_hdr)) {
			printf("Wrong Section to read\n");
		} else if (sec_hdr.sh_type != SHT_SYMTAB && sec_hdr.sh_type != SHT_DYNSYM) {
			continue;
		}
		printf("\n[section %3d] contains ", i);
		process_symtab(fp, &ehdr, elfclass, i);
	}
}

int main(int argc, char const * const *argv)
{
	FILE *fp;
	if (--argc != 1) {
		printf("usage: %s elf-binary\n", argv[0]);
		return 1;
	}
	fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("cannot open file");
		return 1;
	}
	dump_symbols(fp);
	return 0;
}

#endif
