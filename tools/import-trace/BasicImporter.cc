#include <iostream>
#include "util/Logger.hpp"
#include "BasicImporter.hpp"

extern fail::Logger log;

bool BasicImporter::create_database() {
	std::string create_statement = "CREATE TABLE IF NOT EXISTS trace ("
		"	variant_id int(11) NOT NULL,"
		"	instr1 int(10) unsigned NOT NULL,"
		"	instr2 int(10) unsigned NOT NULL,"
		"	instr2_absolute int(10) unsigned DEFAULT NULL,"
		"	data_address int(10) unsigned NOT NULL,"
		"	width tinyint(3) unsigned NOT NULL,"
		"	accesstype enum('R','W') NOT NULL,"
		"	PRIMARY KEY (variant_id,instr2,data_address)"
		") engine=MyISAM ";
	return db->query(create_statement.c_str());
}

bool BasicImporter::add_trace_event(instruction_count_t begin, instruction_count_t end,
									const Trace_Event &event, bool is_fake) {

	static MYSQL_STMT *stmt = 0;
	if (!stmt) {
		std::string sql("INSERT INTO trace (variant_id, instr1, instr2, instr2_absolute, data_address, width,"
		                "					accesstype)"
		                "VALUES (?,?,?,?, ?,?,?)");
		stmt = mysql_stmt_init(db->getHandle());
		if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length())) {
			log << "query '" << sql << "' failed: " << mysql_error(db->getHandle()) << std::endl;
			return false;
		}
	}

	MYSQL_BIND bind[7];
	my_bool is_null = is_fake;
	unsigned long accesstype_len = 1;
	unsigned ip = event.ip();
	unsigned data_address = event.memaddr();
	unsigned width = event.width();
	char accesstype = event.accesstype() == event.READ ? 'R' : 'W';

	memset(bind, 0, sizeof(bind));
	for (int i = 0; i < 7; ++i) {
		bind[i].buffer_type = MYSQL_TYPE_LONG;
		bind[i].is_unsigned = 1;
		switch (i) {
		case 0: bind[i].buffer = &m_variant_id; break;
		case 1: bind[i].buffer = &begin; break;
		case 2: bind[i].buffer = &end; break;
		case 3: bind[i].buffer = &ip;
			bind[i].is_null = &is_null; break;
		case 4: bind[i].buffer = &data_address; break;
		case 5: bind[i].buffer = &width; break;
		case 6: bind[i].buffer = &accesstype;
			bind[i].buffer_type = MYSQL_TYPE_STRING;
			bind[i].buffer_length = accesstype_len;
			bind[i].length = &accesstype_len;
			break;
		}
	}
	if (mysql_stmt_bind_param(stmt, bind)) {
		log << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
		return false;
	}
	if (mysql_stmt_execute(stmt)) {
		log << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
		log << "IP: " << std::hex<< event.ip() << std::endl;
		return false;
	}
	return true;
}


