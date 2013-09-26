#include <sstream>
#include <iostream>
#include "util/Logger.hpp"
#include "ElfImporter.hpp"
#include "util/pstream.h"
#ifndef __puma
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#endif


using namespace llvm;
using namespace llvm::object;
using namespace fail;
using namespace std;

static Logger LOG("ElfImporter");

/**
 * Callback function that can be used to add command line options
 * to the campaign
 */
bool ElfImporter::cb_commandline_init() {
	CommandLine &cmd = CommandLine::Inst();

	OBJDUMP = cmd.addOption("", "objdump", Arg::Required,
							"--objdump \tObjdump: location of objdump binary, otherwise LLVM Disassembler is used");
	return true;
}

bool ElfImporter::create_database() {
	std::stringstream create_statement;
	create_statement << "CREATE TABLE IF NOT EXISTS objdump ("
		"	variant_id int(11) NOT NULL,"
		"	instr_address int(11) NOT NULL,"
		"	opcode varchar(32) NOT NULL,"
		"	disassemble VARCHAR(64),"
		"	comment VARCHAR(128),"
		"	PRIMARY KEY (variant_id,instr_address)"
		") engine=MyISAM ";
	return db->query(create_statement.str().c_str());
}

bool ElfImporter::copy_to_database(ProtoIStream &ps) {
	if (!m_elf) {
		LOG << "please give an elf binary as parameter (-e/--elf)" << std::endl;
		return false;
	}

	CommandLine &cmd = CommandLine::Inst();
	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << std::endl;
		return false;
	}

	if (cmd[OBJDUMP]) { // using objdump
		return import_with_objdump(std::string(cmd[OBJDUMP].first()->arg));
	} else {
		LOG << "importing an objdump with internal llvm dissassembler is not yet implemented" << std::endl;
		return false;
	}


	return true;
}

bool ElfImporter::import_with_objdump(const std::string &binary) {
#ifndef __puma

	LOG << "importing with " << binary << std::endl;

	// -d: disassemble
	// -C: demangle C++
	std::string command = binary + " -C -d " + m_elf->getFilename();
	LOG << "Executing: " << command << std::endl;
	redi::ipstream objdump( command );
	std::string str;
	while(std::getline(objdump, str)){
		if (!evaluate_objdump_line(str)) {
			objdump.close();
			return false;
		}
	}
	objdump.close();
	if(objdump.rdbuf()->exited()){
		int ex = objdump.rdbuf()->status();
		if(ex != 0){
			clear_database();
			LOG << "Could not disassemble!" << std::endl;
			return false;
		}
	}

#endif
	return true;
}

bool ElfImporter::evaluate_objdump_line(const std::string& line){
#ifndef __puma
	// Only read in real code lines:
	// Code lines start with a leading whitespace! (hopefully in each objdump implementation!)
	if(line.size() > 0 && isspace(line[0])){
		// a line looks like: 800156c:\tdd14		  \tble.n	8001598 <_ZN2hw3hal7T32Term8PutBlockEPci+0x30>
		static boost::regex expr("\\s+([A-Fa-f0-9]+):((?:\\s+[A-Fa-f0-9]{2})+)\\s+(.+?)(;.*)?$");
		boost::smatch res;
		if(boost::regex_search(line, res, expr)){
			std::string address = res[1];
			std::stringstream ss;
			ss << std::hex << address;
			address_t addr = 0;
			ss >> addr;
			std::string opcode = res[2];
			boost::trim(opcode);
			std::string instruction = res[3];
			boost::trim(instruction);
			std::string comment = res[4];
			boost::trim(comment);

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
									 const std::string &instruction, const std::string &comment) {
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



bool ElfImporter::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM objdump WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	LOG << "deleted " << db->affected_rows() << " rows from objdump table" << std::endl;
	return ret;
}

