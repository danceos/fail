#include <sstream>
#include <stdlib.h>
#include "Database.hpp"
#include "util/CommandLine.hpp"
#include "util/Logger.hpp"

static fail::Logger LOG("Database", true);

using namespace fail;

#ifndef __puma
boost::mutex Database::m_global_lock;
#endif

static CommandLine::option_handle DATABASE, HOSTNAME, USERNAME, DBDEFAULTS;

Database::Database(const std::string &username, const std::string &host, const std::string &database) {
#ifndef __puma
	boost::lock_guard<boost::mutex> guard(m_global_lock);
#endif
	CommandLine &cmd = CommandLine::Inst();

	std::string db_conf_file = std::string(getenv("HOME")) + "/.my.cnf";
	if (cmd[DBDEFAULTS].count()) {
		db_conf_file = cmd[DBDEFAULTS].first()->arg;
	}
	handle = mysql_init(0);
	last_result = 0;
	mysql_options(handle, MYSQL_READ_DEFAULT_FILE, db_conf_file.c_str());
	if (!mysql_real_connect(handle, host.c_str(),
							username.c_str(),
							0, database.c_str(), 0, 0, 0)) {
		LOG << "cannot connect to MySQL server: " << mysql_error(handle) << std::endl;
		exit(-1);
	}
	LOG << "opened MYSQL connection" << std::endl;
}

Database::~Database()
{
	// flush cached INSERTs if available
	insert_multiple();

#ifndef __puma
	boost::lock_guard<boost::mutex> guard(m_global_lock);
#endif
	mysql_close(handle);
}

MYSQL_RES* Database::query(char const *query, bool get_result) {
#ifndef __puma
	boost::lock_guard<boost::mutex> guard(m_handle_lock);
#endif

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

MYSQL_RES* Database::query_stream(char const *query)
{
#ifndef __puma
	boost::lock_guard<boost::mutex> guard(m_handle_lock);
#endif

	if (mysql_query(handle, query)) {
		std::cerr << "query '" << query << "' failed: " << mysql_error(handle) << std::endl;
		return 0;
	}

	MYSQL_RES *res = mysql_use_result(handle);
	if (!res && mysql_errno(handle)) {
		std::cerr << "mysql_use_result for query '" << query << "' failed: " << mysql_error(handle) << std::endl;
		return 0;
	}

	return res;
}

bool Database::insert_multiple(char const *insertquery, char const *values)
{
	// different insertquery, but still cached values?
	if (insertquery && m_insertquery != insertquery && m_insertquery_values.size() > 0) {
		// flush cache
		insert_multiple();
	} else if (!insertquery && m_insertquery.size() == 0) {
		// don't know how to INSERT, no insertquery available!
		return false;
	}

	if (insertquery) {
		m_insertquery = insertquery;
	}

	if (values) {
		m_insertquery_values.push_back(values);
	}

	if ((!values && m_insertquery_values.size() > 0) || m_insertquery_values.size() >= 2048) {
		std::stringstream sql;
		sql << m_insertquery;
		bool first = true;
		for (std::vector<std::string>::const_iterator it = m_insertquery_values.begin();
		     it != m_insertquery_values.end(); ++it) {
			if (!first) {
				sql << ",";
			}
			first = false;
			sql << *it;
		}
		m_insertquery_values.clear();
		return query(sql.str().c_str());
	}

	return true;
}

my_ulonglong Database::affected_rows()
{
	return mysql_affected_rows(handle);
}

my_ulonglong Database::insert_id()
{
	return mysql_insert_id(handle);
}

bool Database::create_variants_table()
{
	if (!query("CREATE TABLE IF NOT EXISTS variant ("
		"      id int(11) NOT NULL AUTO_INCREMENT,"
		"      variant varchar(100) NOT NULL,"
		"      benchmark varchar(100) NOT NULL,"
		"      PRIMARY KEY (id),"
		"UNIQUE KEY variant (variant,benchmark)) ENGINE=MyISAM")) {
		return false;
	}
	return true;
}

std::vector<Database::Variant> Database::get_variants(const std::string &variant, const std::string &benchmark)
{
	std::vector<std::string> variants;
	variants.push_back(variant);
	std::vector<std::string> benchmarks;
	benchmarks.push_back(benchmark);
	std::vector<std::string> dummy;

	return get_variants(variants, dummy, benchmarks, dummy);
}

std::vector<Database::Variant> Database::get_variants(
	const std::vector<std::string>& variants,
	const std::vector<std::string>& variants_exclude,
	const std::vector<std::string>& benchmarks,
	const std::vector<std::string>& benchmarks_exclude)
{
	std::vector<Variant> result;
	std::stringstream ss;

	// make sure variant table exists
	if (!create_variants_table()) {
		return result;
	}

	// FIXME string escaping
	ss << "SELECT id, variant, benchmark FROM variant WHERE ";
	ss << "(";
	for (std::vector<std::string>::const_iterator it = variants.begin();
	     it != variants.end(); ++it) {
		ss << "variant LIKE '" << *it << "' OR ";
	}
	ss << "0) AND (";
	for (std::vector<std::string>::const_iterator it = benchmarks.begin();
	     it != benchmarks.end(); ++it) {
		ss << "benchmark LIKE '" << *it << "' OR ";
	}
	// dummy terminator to avoid special cases in query construction above
	ss << "0) AND NOT (";
	for (std::vector<std::string>::const_iterator it = variants_exclude.begin();
	     it != variants_exclude.end(); ++it) {
		ss << "variant LIKE '" << *it << "' OR ";
	}
	for (std::vector<std::string>::const_iterator it = benchmarks_exclude.begin();
	     it != benchmarks_exclude.end(); ++it) {
		ss << "benchmark LIKE '" << *it << "' OR ";
	}
	// dummy terminator to avoid special cases in query construction above
	ss << "0)";

	MYSQL_RES *variant_id_res = query(ss.str().c_str(), true);
	if (!variant_id_res) {
		return result;
	}

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(variant_id_res))) {
		Variant var;
		var.id = atoi(row[0]);
		var.variant = row[1];
		var.benchmark = row[2];
		result.push_back(var);
	}

	return result;
}

int Database::get_variant_id(const std::string &variant, const std::string &benchmark)
{
	std::vector<Variant> variants = get_variants(variant, benchmark);
	if (variants.size() == 0) {
		// Insert a new variant
		std::stringstream ss;
		ss << "INSERT INTO variant (variant, benchmark) VALUES ('" << variant << "', '" << benchmark << "')";
		if (!query(ss.str().c_str())) {
			return 0;
		}
		return mysql_insert_id(handle);
	} else if (variants.size() == 1) {
		return variants[0].id;
	} else {
		LOG << "Variant identifier " << variant << "/" << benchmark << " is ambigious!" << std::endl;
		return 0;
	}
}

int Database::get_fspmethod_id(const std::string &method)
{
	if (!query("CREATE TABLE IF NOT EXISTS fspmethod ("
		  "	 id int(11) NOT NULL AUTO_INCREMENT,"
		  "	 method varchar(100) NOT NULL,"
		  "	 PRIMARY KEY (id), UNIQUE KEY method (method)) ENGINE=MyISAM")) {
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

std::string Database::escape_string(const std::string unescaped_string) {

	char *temp = new char[(unescaped_string.size() * 2) + 1];

	mysql_real_escape_string(handle, temp, unescaped_string.c_str(), unescaped_string.size());

	std::string result = temp;

	delete[] temp;

	return result;
}

void Database::cmdline_setup() {
	CommandLine &cmd = CommandLine::Inst();

	DATABASE	  = cmd.addOption("d", "database", Arg::Required,
								  "-d/--database \tMYSQL Database (default: taken from ~/.my.cnf)");
	HOSTNAME	  = cmd.addOption("H", "hostname", Arg::Required,
								  "-h/--hostname \tMYSQL Hostname (default: taken from ~/.my.cnf)");
	USERNAME	  = cmd.addOption("u", "username", Arg::Required,
								  "-u/--username \tMYSQL Username (default: taken from ~/.my.cnf, or your current user)");
	DBDEFAULTS	  = cmd.addOption("", "database-option--file", Arg::Required,
								  "--database-option-file \toverride MySQL ~/.my.cnf option file (prepend with './' for files in the CWD)\n");

	// should be called before any threads are spawned
	mysql_library_init(0, NULL, NULL);
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
