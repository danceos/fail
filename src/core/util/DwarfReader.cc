#include <fstream>
#include <fcntl.h>
#include <unistd.h>

#include "DwarfReader.hpp"
#include "libdwarf.h"
#include "libelf.h"
#include "Logger.hpp"

/**
	* This source code is based on bcov 0.2.
	* Sourcefile: src/coverage.cpp
	* http://bcov.sourceforge.net/
	* GNU GENERAL PUBLIC LICENSE
*/

using namespace fail;
using namespace std;

static Logger LOG("DwarfReader");

static void dwarfErrorHandler(Dwarf_Error error, Dwarf_Ptr /*userData*/)
{
	char* msg=dwarf_errmsg(error);
	cerr << "dwarf error: " << msg << endl;
}

static string normalize(const string& filePath)
{
	// A quick scan first...
	bool hadSep=false,needsFix=false;
	string::size_type len=filePath.length();
	if (!needsFix)
	for (string::size_type index=0;index<len;index++) {
		char c=filePath[index];
		if (c=='/') {
			if (hadSep) {
				needsFix=true;
			}
			hadSep=true;
		} else {
			if (c=='.') {
				if (hadSep||(index==0)) {
					needsFix=true;
				}
			}
			hadSep=false;
		}
	}
	if (!needsFix) {
		return filePath;
	}
	hadSep=false;
	// Construct the fixed result
	string result;
	for (string::size_type index=0;index<len;index++) {
		char c=filePath[index];
		if (c=='/') {
			if (hadSep) {
			} else result+=c;
			hadSep=true;
		} else {
			if ((c=='.')&&(hadSep||(index==0))) {
				if (index+1>=len) {
					if (hadSep)
						result.resize(result.length()-1);
				} else {
					result+=c;
				}
				continue;
			}
			char n=filePath[index+1];
			if (n=='/') {
				index++; continue;
			}
			if (n=='.') {
				if (index+2>=len) {
					index++;
					string::size_type split=result.rfind('/',result.length()-2);
					if (split!=string::npos) {
						if (result.substr(split)!="/../") {
							result.resize(split);
						}
					} else if (result.length()>0) {
						if ((result!="../")&&(result!="/")) {
							result.clear();
						}
					} else result="..";
					continue;
				} else {
					n=filePath[index+2];
					if (n=='/') {
						index+=2;
						string::size_type split=result.rfind('/',result.length()-2);
						if (split!=string::npos) {
							if (result.substr(split)!="/../") {
								result.resize(split+1);
							}
						} else if (result.length()>0) {
							if ((result!="../")&&(result!="/")) {
								result.clear();
							}
						} else result="../";
						continue;
					}
				}
			}
		}
		result+=c; hadSep=false;
	}
	return result;
}

bool DwarfReader::read_source_files(const std::string& fileName,std::list<std::string>& lines) {

	// Open The file
	int fd=open(fileName.c_str(),O_RDONLY);
	if (fd<0) return false;

	// Initialize libdwarf
	Dwarf_Debug dbg;
	int status = dwarf_init(fd, DW_DLC_READ,dwarfErrorHandler,0,&dbg,0);
	if (status==DW_DLV_ERROR) { close(fd); return false; }
	if (status==DW_DLV_NO_ENTRY) { close(fd); return true; }

	// Iterator over the headers
	Dwarf_Unsigned header;
	while (dwarf_next_cu_header(dbg,0,0,0,0,&header,0)==DW_DLV_OK) {
		// Access the die
		Dwarf_Die die;
		if (dwarf_siblingof(dbg,0,&die,0)!=DW_DLV_OK)
			return false;

		// Get the source lines
		Dwarf_Line* lineBuffer;
		Dwarf_Signed lineCount;
		if (dwarf_srclines(die,&lineBuffer,&lineCount,0)!=DW_DLV_OK)
			continue; //return false;

		// Store them
		for (int index=0;index<lineCount;index++) {
			Dwarf_Unsigned lineNo;
			if (dwarf_lineno(lineBuffer[index],&lineNo,0)!=DW_DLV_OK)
				return false;
			char* lineSource;
			if (dwarf_linesrc(lineBuffer[index],&lineSource,0)!=DW_DLV_OK)
				return false;
			Dwarf_Bool isCode;
			if (dwarf_linebeginstatement(lineBuffer[index],&isCode,0)!=DW_DLV_OK)
				return false;
			Dwarf_Addr addr;
			if (dwarf_lineaddr(lineBuffer[index],&addr,0)!=DW_DLV_OK)
				return false;

			if (lineNo&&isCode) {
				//LOG << "lineNo: " << lineNo   << " addr: " << reinterpret_cast<void*>(addr) << " line source:" << normalize(lineSource) << endl;
				lines.push_back(normalize(lineSource));
			}

			dwarf_dealloc(dbg,lineSource,DW_DLA_STRING);
		}

		// Release the memory
		for (int index=0;index<lineCount;index++)
			dwarf_dealloc(dbg,lineBuffer[index],DW_DLA_LINE);
		dwarf_dealloc(dbg,lineBuffer,DW_DLA_LIST);
	}

	//Remove duplicated entrys
	lines.sort();
	lines.unique();

	// Shut down libdwarf
	if (dwarf_finish(dbg,0)!=DW_DLV_OK)
		return false;

	close(fd);
	return true;
}

bool DwarfReader::read_mapping(std::string fileName, std::list<addrToLine>& addrToLineList) {

	// Open The file
	int fd=open(fileName.c_str(),O_RDONLY);
	if (fd<0) {
		return false;
	}

	// Initialize libdwarf
	Dwarf_Debug dbg;
	int status = dwarf_init(fd, DW_DLC_READ,dwarfErrorHandler,0,&dbg,0);
	if (status==DW_DLV_ERROR) {
		close(fd);
		return false;
	}
	if (status==DW_DLV_NO_ENTRY) {
		close(fd);
		return true;
	}

	// Iterator over the headers
	Dwarf_Unsigned header;
	while (dwarf_next_cu_header(dbg,0,0,0,0,&header,0)==DW_DLV_OK) {
		// Access the die
		Dwarf_Die die;
		if (dwarf_siblingof(dbg,0,&die,0)!=DW_DLV_OK) {
			return false;
		}

		// Get the source lines
		Dwarf_Line* lineBuffer;
		Dwarf_Signed lineCount;
		if (dwarf_srclines(die,&lineBuffer,&lineCount,0)!=DW_DLV_OK) {
			continue; //return false;
		}

		// Store them
		for (int index=0;index<lineCount;index++) {
			Dwarf_Unsigned lineNo;
			if (dwarf_lineno(lineBuffer[index],&lineNo,0)!=DW_DLV_OK) {
				return false;
			}
			char* lineSource;
			if (dwarf_linesrc(lineBuffer[index],&lineSource,0)!=DW_DLV_OK) {
				return false;
			}
			Dwarf_Bool isCode;
			if (dwarf_linebeginstatement(lineBuffer[index],&isCode,0)!=DW_DLV_OK) {
				return false;
			}
			Dwarf_Addr addr;
			if (dwarf_lineaddr(lineBuffer[index],&addr,0)!=DW_DLV_OK) {
				return false;
			}

			if (lineNo&&isCode) {
				struct addrToLine newLine = { (int) addr, (int) lineNo, normalize(lineSource) };
				addrToLineList.push_back(newLine);
			}

			dwarf_dealloc(dbg,lineSource,DW_DLA_STRING);
		}

		// Release the memory
		for (int index=0;index<lineCount;index++) {
			dwarf_dealloc(dbg,lineBuffer[index],DW_DLA_LINE);
		}
		dwarf_dealloc(dbg,lineBuffer,DW_DLA_LIST);
	}

	// Shut down libdwarf
	if (dwarf_finish(dbg,0)!=DW_DLV_OK) {
		return false;
	}

	close(fd);
	return true;
}
