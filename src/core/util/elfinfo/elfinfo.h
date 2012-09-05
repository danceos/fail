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

void read_ELF_file_header(FILE* fp,Elf32_Ehdr *filehdr);
int read_ELF_section_header(int sect_num,Elf32_Shdr *sect_hdr,FILE *fp);

#endif // ELFINFO_H
