/**
 * ELF Helper
 * \author Martin Hoffmann <hoffmann@cs.fau.de>
 */

#ifndef ELFINFO_H
#define ELFINFO_H

#if !defined(__APPLE__)
#include <elf.h>
#else
#include "mac-elf.h" /* Mac OS X doesn't distribute elf.h... */
#endif

#include <stdio.h>
// ELFinfo

// Returns true if it finds a valid ELF header.  Stores ELFCLASS32 or 64 in elfclass.
bool read_ELF_file_header(FILE *fp, Elf64_Ehdr *filehdr, int *elfclass);
// Returns true if it finds a valid ELF section header.
bool read_ELF_section_header(FILE *fp, Elf64_Ehdr const *filehdr, int elfclass, int sect_num, Elf64_Shdr *sect_hdr);
void display_sections(FILE *fp);

#endif // ELFINFO_H
