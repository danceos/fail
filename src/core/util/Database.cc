#include <sstream>
#include <stdlib.h>
#include "Database.hpp"
#include "util/CommandLine.hpp"
#include "util/Logger.hpp"
static fail::Logger log("Database", true);

using namespace fail;

Database::Database(const std::string &username, const std::string &host, const std::string &database) {
    handle = mysql_init(0);
    last_result = 0;
	mysql_options(handle, MYSQL_READ_DEFAULT_FILE, "~/.my.cnf");
	if (!mysql_real_connect(handle, host.c_str(),
							username.c_str(),
							0, database.c_str(), 0, 0, 0)) {
		log << "cannot connect to MySQL server: " << mysql_error(handle) << std::endl;
		exit(-1);
	}
	log << "opened MYSQL connection" << std::endl;
}

MYSQL_RES* Database::query(char const *query, bool get_result)
{
	if (mysql_query(handle, query)) {
		std::cerr << "query '" << query << "' failed: " << mysql_error(handle) << std::endl;
		return 0;
	}

	if (get_result) {
        if (last_result != 0) {
            mysql_free_result(last_result);
            last_result = 0;
        }
		MYSQL_RES *res = mysql_store_result(handle);
		if (!res && mysql_errno(handle)) {
			std::cerr << "mysql_store_result for query '" << query << "' failed: " << mysql_error(handle) << std::endl;
			return 0;
		}
        last_result = res;
		return res;
	}
	return (MYSQL_RES *) 1; // Invalid PTR!!!
}


my_ulonglong Database::affected_rows()
{
	return mysql_affected_rows(handle);
}


int Database::get_variant_id(const std::string &variant, const std::string &benchmark)
{
	if (!query("CREATE TABLE IF NOT EXISTS variant ("
		  "  id int(11) NOT NULL AUTO_INCREMENT,"
		  "  variant varchar(255) NOT NULL,"
		  "  benchmark varchar(255) NOT NULL,"
		  "  PRIMARY KEY (id),"
		  "UNIQUE KEY variant (variant,benchmark))")) {
		return 0;
	}

	int variant_id;
	std::stringstream ss;
	// FIXME SQL injection possible
	ss << "SELECT id FROM variant WHERE variant = '" << variant << "' AND benchmark = '" << benchmark << "'";
	MYSQL_RES *variant_id_res = query(ss.str().c_str(), true);
	if (!variant_id_res) {
		return 0;
	} else if (mysql_num_rows(variant_id_res)) {
		MYSQL_ROW row = mysql_fetch_row(variant_id_res);
		variant_id = atoi(row[0]);
	} else {
		ss.str("");
		ss << "INSERT INTO variant (variant, benchmark) VALUES ('" << variant << "', '" << benchmark << "')";
		if (!query(ss.str().c_str())) {
			return 0;
		}
		variant_id = mysql_insert_id(handle);
	}
	return variant_id;
}

int Database::get_fspmethod_id(const std::string &method)
{
	if (!query("CREATE TABLE IF NOT EXISTS fspmethod ("
          "  id int(11) NOT NULL AUTO_INCREMENT,"
          "  method varchar(255) NOT NULL,"
          "  PRIMARY KEY (id), UNIQUE KEY method (method))")) {
		return 0;
	}

    std::stringstream ss;
	ss << "SELECT id FROM fspmethod WHERE method = '" << method << "'";
	MYSQL_RES *res = query(ss.str().c_str(), true);

    int id;

	if (!res) {
		return 0;
	} else if (mysql_num_rows(res)) {
		MYSQL_ROW row = mysql_fetch_row(res);
		id = atoi(row[0]);
	} else {
		ss.str("");
		ss << "INSERT INTO fspmethod (method) VALUES ('" << method << "')";
		if (!query(ss.str().c_str())) {
			return 0;
		}
		id = mysql_insert_id(handle);
	}

	return id;
}

static CommandLine::option_handle DATABASE, HOSTNAME, USERNAME;

void Database::cmdline_setup() {
	CommandLine &cmd = CommandLine::Inst();

	DATABASE	  = cmd.addOption("d", "database", Arg::Required,
                                  "-d/--database\t MYSQL Database (default: taken from ~/.my.cnf)");
	HOSTNAME	  = cmd.addOption("H", "hostname", Arg::Required,
                                  "-h/--hostname\t MYSQL Hostname (default: taken from ~/.my.cnf)");
	USERNAME	  = cmd.addOption("u", "username", Arg::Required,
                                  "-u/--username\t MYSQL Username (default: taken from ~/.my.cnf, or your current user)");
}

Database * Database::cmdline_connect() {
    std::string username, hostname, database;

	CommandLine &cmd = CommandLine::Inst();

	if (cmd[USERNAME].count() > 0)
		username = std::string(cmd[USERNAME].first()->arg);
	else
		username = "";

	if (cmd[HOSTNAME].count() > 0)
		hostname = std::string(cmd[HOSTNAME].first()->arg);
	else
		hostname = "";

	if (cmd[DATABASE].count() > 0)
		database = std::string(cmd[DATABASE].first()->arg);
	else
		database = "";

	return new Database(username, hostname, database);
}



