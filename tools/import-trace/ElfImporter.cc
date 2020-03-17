#include <sstream>
#include <iostream>
#include <fstream>
#include "util/Logger.hpp"
#include "ElfImporter.hpp"
#include "util/pstream.h"
#ifndef __puma
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#endif

#ifdef BUILD_LLVM_DISASSEMBLER
using namespace llvm;
using namespace llvm::object;
#endif
using namespace fail;
using namespace std;

static Logger LOG("ElfImporter");

/**
 * Callback function that can be used to add command line options
 * to the campaign
 */
bool ElfImporter::cb_commandline_init()
{
	CommandLine &cmd = CommandLine::Inst();

	OBJDUMP = cmd.addOption("", "objdump", Arg::Required,
		"--objdump \tObjdump: location of objdump binary, otherwise LLVM Disassembler is used");
	SOURCECODE = cmd.addOption("", "sources", Arg::None,
		"--sources \timport all source files and the mapping of code line<->static instruction into the database");
	return true;
}

bool ElfImporter::create_database()
{
	CommandLine &cmd = CommandLine::Inst();
	std::stringstream create_statement;

	create_statement.str("");
	create_statement << "CREATE TABLE IF NOT EXISTS objdump ("
		"	variant_id int(11) NOT NULL,"
		"	instr_address int(11) UNSIGNED NOT NULL,"
		"	opcode varchar(32) NOT NULL,"
		"	disassemble VARCHAR(64),"
		"	comment VARCHAR(128),"
		"	PRIMARY KEY (variant_id,instr_address)"
		") engine=MyISAM ";
	if (!db->query(create_statement.str().c_str())) {
		return false;
	}

	if (cmd[SOURCECODE]) {
		create_statement.str("");
		create_statement << "CREATE TABLE IF NOT EXISTS dbg_filename ("
			"	file_id int(11) NOT NULL AUTO_INCREMENT,"
			"	variant_id int(11) NOT NULL,"
			"	path VARCHAR(1024),"
			"	PRIMARY KEY (file_id)"
			") engine=MyISAM ";
		if (!db->query(create_statement.str().c_str())) {
			return false;
		}

		create_statement.str("");
		create_statement << "CREATE TABLE IF NOT EXISTS dbg_source ("
			"	variant_id int(11) NOT NULL,"
			"	linenumber int(11) NOT NULL,"
			"	file_id int(11) NOT NULL,"
			"	line VARCHAR(1024),"
			"	PRIMARY KEY (variant_id, linenumber, file_id)"
			") engine=MyISAM ";
		if (!db->query(create_statement.str().c_str())) {
			return false;
		}

		create_statement.str("");
		create_statement << "CREATE TABLE IF NOT EXISTS dbg_mapping ("
			"	variant_id int(11) NOT NULL,"
			"	instr_absolute int(11) UNSIGNED NOT NULL,"
			"	line_range_size int(11) UNSIGNED NOT NULL,"
			"	linenumber int(11) UNSIGNED NOT NULL,"
			"	file_id int(11) NOT NULL,"
			"	PRIMARY KEY (variant_id, instr_absolute ,linenumber)"
			") engine=MyISAM ";
		if (!db->query(create_statement.str().c_str())) {
			return false;
		}
	}

	return true;
}

bool ElfImporter::copy_to_database(ProtoIStream &ps)
{
	if (!m_elf) {
		LOG << "Please give an ELF binary as parameter (-e/--elf)." << std::endl;
		return false;
	}

	CommandLine &cmd = CommandLine::Inst();
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return false;
	}

	if (cmd[OBJDUMP]) { // using objdump
		if (!import_with_objdump(std::string(cmd[OBJDUMP].first()->arg))) {
			return false;
		}
	} else {
		LOG << "importing an objdump with internal llvm dissassembler is not yet implemented" << std::endl;
	}

	if (cmd[SOURCECODE]) { // import sources
		std::list<std::string> sourceFiles;

		if (!import_source_files(m_elf->getFilename(), sourceFiles)) {
			cerr << "unable to read dwarf2 debug info" << endl;
			return false;
		}

		for (std::list<std::string>::iterator it = sourceFiles.begin(); it != sourceFiles.end(); ++it) {
			if (!import_source_code(*it)) {
				return false;
			}
		}

		// import debug information
		if (!import_mapping(m_elf->getFilename())) {
			return false;
		}
	}

	return true;
}

bool ElfImporter::import_with_objdump(const std::string &binary)
{
#ifndef __puma

	LOG << "importing with " << binary << std::endl;

	// -d: disassemble
	// -C: demangle C++
	std::string command = binary + " -C -d " + m_elf->getFilename();
	LOG << "Executing: " << command << std::endl;
	redi::ipstream objdump( command );
	std::string str;
	while (std::getline(objdump, str)) {
		if (!evaluate_objdump_line(str)) {
			objdump.close();
			return false;
		}
	}
	objdump.close();
	if (objdump.rdbuf()->exited()) {
		int ex = objdump.rdbuf()->status();
		if (ex != 0) {
			clear_database();
			LOG << "Could not disassemble!" << std::endl;
			return false;
		}
	}

#endif
	return true;
}

bool ElfImporter::evaluate_objdump_line(const std::string& line)
{
#ifndef __puma
	// Only read in real code lines:
	// Code lines start with a leading whitespace! (hopefully in each objdump implementation!)
	if (line.size() > 0 && isspace(line[0])) {
		// a line looks like: 800156c:\tdd14		  \tble.n	8001598 <_ZN2hw3hal7T32Term8PutBlockEPci+0x30>
		static boost::regex expr("\\s+([A-Fa-f0-9]+):((?:\\s+[A-Fa-f0-9]{2})+)\\s+(.+?)(;.*)?$");
		boost::smatch res;
		if (boost::regex_search(line, res, expr)) {
			std::string address = res[1];
			std::stringstream ss;
			ss << std::hex << address;
			address_t addr = 0;
			ss >> addr;
			std::string opcode = res[2];
			boost::trim(opcode);
			opcode = opcode.substr(0,32);
			std::string instruction = res[3];
			boost::trim(instruction);
			instruction = instruction.substr(0, 64);
			std::string comment = res[4];
			boost::trim(comment);
			comment = comment.substr(0, 128);

			// transform hex opcode to char array
			char opcode_read[33];
			std::stringstream opcode_stream(opcode);
			unsigned long opcode_length = 0;
			char c;
			while (opcode_length < 16 && opcode_stream) {
				opcode_stream << std::hex;
				opcode_stream >> c;
				if (!opcode_stream) break;
				opcode_read[opcode_length++] = c;
			}
			opcode_read[opcode_length] = 0;

			/* import instruction into database */
			if (!import_instruction(addr, opcode_read, opcode_length,
				instruction, comment))
				return false;
		}
	}
#endif
	return true;
}



bool ElfImporter::import_instruction(fail::address_t addr, char *opcode, int opcode_length,
	const std::string &instruction, const std::string &comment)
{
	/* Prepare a mysql statement if it was not done before */
	static MYSQL_STMT *stmt = 0;
	if (!stmt) {
		std::string sql("INSERT INTO objdump (variant_id, instr_address, opcode, disassemble, comment) VALUES (?,?,?,?,?)");
		stmt = mysql_stmt_init(db->getHandle());
		if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length())) {
			LOG << "query '" << sql << "' failed: " << mysql_error(db->getHandle()) << std::endl;
			return false;
		}
	}
	MYSQL_BIND bind[5];
	memset(bind, 0, sizeof(bind));
	for (unsigned i = 0; i < 5; ++i) {
		bind[i].buffer_type = MYSQL_TYPE_STRING;
		bind[i].is_unsigned = 1;
		switch (i) {
		case 0:
			bind[i].buffer = &m_variant_id;
			bind[i].buffer_type = MYSQL_TYPE_LONG;
			break;
		case 1:
			bind[i].buffer = &addr;
			bind[i].buffer_type = MYSQL_TYPE_LONG;
			break;
		case 2:
			bind[i].buffer_type = MYSQL_TYPE_BLOB;
			bind[i].buffer = opcode;
			bind[i].buffer_length = opcode_length;
			break;
		case 3:
			bind[i].buffer = const_cast<char *>(instruction.c_str());
			bind[i].buffer_length = instruction.length();
			break;
		case 4:
			bind[i].buffer = const_cast<char *>(comment.c_str());
			bind[i].buffer_length = comment.length();
			break;
		}
	}


	if (mysql_stmt_bind_param(stmt, bind)) {
		LOG << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
		return false;
	}
	if (mysql_stmt_execute(stmt)) {
		LOG << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
		return false;
	}

	return true;
}

static inline std::string rtrim(std::string str)
{
	std::string whitespaces(" \t\f\v\n\r");

	std::size_t found = str.find_last_not_of(whitespaces);
	if (found != std::string::npos) {
		str.erase(found+1);
	} else {
		str.clear();
	}
	return str;
}

bool ElfImporter::import_source_code(std::string fileName)
{
	LOG << "Importing Sourcefile: " << fileName << endl;

	ifstream in(fileName.c_str());
	if (!in.is_open()) {
		LOG << "Sourcefile not found: " << fileName << endl;
		return true; // we can live with this
	}

	// retrieve file_id
	std::stringstream ss;
	ss << "SELECT file_id FROM dbg_filename "
		<< "WHERE path = '" << fileName.c_str() << "' "
		<< "AND variant_id = " << m_variant_id;
	MYSQL_RES *res = db->query(ss.str().c_str(), true);
	if (!res) {
		return false;
	}
	MYSQL_ROW row;
	if (!(row = mysql_fetch_row(res))) {
		// this should not happen
		LOG << "error: no entry for '" << fileName.c_str() << "' in dbg_filename, aborting" << std::endl;
		return false;
	}
	std::string file_id = row[0];

	// import lines from this file
	for (unsigned lineNo = 1; !in.eof(); ++lineNo) {
		// read and strip a line
		string currentLine;
		getline(in, currentLine);
		if (!currentLine.length() && in.eof()) {
			break;
		}
		currentLine = rtrim(currentLine);

		// escape special characters for use in SQL
		currentLine = db->escape_string(currentLine);

		// insert line into dbg_source
		std::stringstream sql;
		sql << "(" << m_variant_id << "," << lineNo
			<< "," << file_id << ",'" << currentLine << "')";
		if (!db->insert_multiple("INSERT INTO dbg_source (variant_id, linenumber, file_id, line) VALUES ",
			sql.str().c_str())) {
			return false;
		}
	}
	db->insert_multiple();
	return true;
}

bool ElfImporter::import_source_files(const std::string& elf_filename, std::list<std::string>& filenames)
{
	if (!dwReader.read_source_files(elf_filename, filenames)) {
		return false;
	}
	for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it) {
		// INSERT group entry
		std::stringstream sql;
		sql << "(" << m_variant_id << ",'" << it->c_str() << "')";

		if (!db->insert_multiple("INSERT INTO dbg_filename (variant_id, path) VALUES ",
			sql.str().c_str())) {
			return false;
		}
	}
	db->insert_multiple();
	return true;
}

bool ElfImporter::import_mapping(std::string fileName)
{
	std::list<DwarfLineMapping> mapping;
	if (!dwReader.read_mapping(fileName, mapping)) {
		return false;
	}

	while (!mapping.empty()) {
		DwarfLineMapping tmp_mapping = mapping.front();

		// FIXME reuse file_id from previous iteration if file name stays constant
		std::stringstream ss;
		ss << "SELECT file_id FROM dbg_filename "
			<< "WHERE path = '" << tmp_mapping.line_source << "' "
			<< "AND variant_id = " << m_variant_id;

		MYSQL_RES *res = db->query(ss.str().c_str(), true);
		if (!res) {
			return false;
		}
		MYSQL_ROW row;
		row = mysql_fetch_row(res);

		// INSERT group entry
		std::stringstream sql;
		sql << "(" << m_variant_id << "," << tmp_mapping.absolute_addr << ","
			<< tmp_mapping.line_range_size << "," << tmp_mapping.line_number << ",";
		if (row != NULL) {
			sql << row[0] << ")";
		} else {
			// this should not happen
			LOG << "error: no entry for '" << tmp_mapping.line_source << "' in dbg_filename, aborting" << std::endl;
			return false;
		}

		// TODO: Skip duplicated entries with ON DUPLICATE KEY UPDATE (?)
		if (!db->insert_multiple("INSERT IGNORE INTO dbg_mapping (variant_id, instr_absolute, line_range_size, linenumber, file_id) VALUES ",
			sql.str().c_str())) {
			return false;
		}

		mapping.pop_front();
	}

	db->insert_multiple();

	return true;
}

bool ElfImporter::clear_database()
{
	CommandLine &cmd = CommandLine::Inst();
	std::stringstream ss;
	bool ret = true, result;

	ss.str("");
	ss << "DELETE FROM objdump WHERE variant_id = " << m_variant_id;
	result = db->query(ss.str().c_str());
	ret = ret && result;
	LOG << "deleted " << db->affected_rows() << " rows from objdump table" << std::endl;

	if (cmd[SOURCECODE]) {
		ss.str("");
		ss << "DELETE FROM dbg_source WHERE variant_id = " << m_variant_id;
		result = db->query(ss.str().c_str());
		ret = ret && result;
		LOG << "deleted " << db->affected_rows() << " rows from dbg_source table" << std::endl;

		ss.str("");
		ss << "DELETE FROM dbg_filename WHERE variant_id = " << m_variant_id;
		result = db->query(ss.str().c_str());
		ret = ret && result;
		LOG << "deleted " << db->affected_rows() << " rows from dbg_filename table" << std::endl;

		//ToDo: Reset auto increment value to 1
		ss.str("");
		ss << "DELETE FROM dbg_mapping WHERE variant_id = " << m_variant_id;
		result = db->query(ss.str().c_str());
		ret = ret && result;
		LOG << "deleted " << db->affected_rows() << " rows from source table" << std::endl;
	}

	return ret;
}
