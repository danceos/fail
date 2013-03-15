#ifndef __UTIL_DATABASE_H__
#define __UTIL_DATABASE_H__

#include <mysql/mysql.h>
#include <iostream>
#include <string>

namespace fail {
class Database {
    MYSQL *handle;
    MYSQL_RES *last_result;

public:
    Database(const std::string &username, const std::string &host, const std::string &database);
    ~Database() { mysql_close(handle); }

	int get_variant_id(const std::string &variant, const std::string &benchmark);
	int get_fspmethod_id(const std::string &method);


    MYSQL * getHandle() const { return handle; }
	MYSQL_RES *query(char const *query, bool get_result = false);
	my_ulonglong affected_rows();

    // Interface to the command line parser
    static void cmdline_setup();
    static Database * cmdline_connect();
};

}

#endif
