#include <sstream>
#include <iostream>
#include "util/Logger.hpp"

using namespace fail;
static Logger LOG ("Pruner");

#include "Pruner.hpp"


bool Pruner::init(fail::Database *db,
		const std::vector<std::string>& variants,
		const std::vector<std::string>& variants_exclude,
		const std::vector<std::string>& benchmarks,
		const std::vector<std::string>& benchmarks_exclude)
{
	this->db = db;
	std::stringstream ss;

	// FIXME string escaping
	ss << "SELECT id FROM variant WHERE ";
	for (std::vector<std::string>::const_iterator it = variants.begin();
	     it != variants.end(); ++it) {
		ss << "variant LIKE '" << *it << "' AND ";
	}
	for (std::vector<std::string>::const_iterator it = variants_exclude.begin();
	     it != variants_exclude.end(); ++it) {
		ss << "variant NOT LIKE '" << *it << "' AND ";
	}
	for (std::vector<std::string>::const_iterator it = benchmarks.begin();
	     it != benchmarks.end(); ++it) {
		ss << "benchmark LIKE '" << *it << "' AND ";
	}
	for (std::vector<std::string>::const_iterator it = benchmarks_exclude.begin();
	     it != benchmarks_exclude.end(); ++it) {
		ss << "benchmark NOT LIKE '" << *it << "' AND ";
	}
	// dummy terminator to avoid special cases in query construction above
	ss << "1";
	m_variant_id_query = ss.str();

	if (!(m_method_id = db->get_fspmethod_id(method_name()))) {
		return false;
	}

	LOG << "Pruning with method " << method_name() << " (ID: " << m_method_id << ")"
	    << std::endl;
	return true;
}

bool Pruner::create_database() {
	std::string create_statement = "CREATE TABLE IF NOT EXISTS fsppilot ("
	    "  id int(11) NOT NULL AUTO_INCREMENT,"
	    "  known_outcome tinyint(4) NOT NULL,"
	    "  variant_id int(11) NOT NULL,"
	    "  instr2 int(10) unsigned NOT NULL,"
	    "  injection_instr int(10) unsigned NOT NULL,"
	    "  injection_instr_absolute int(10) unsigned,"
	    "  data_address int(10) unsigned NOT NULL,"
	    "  fspmethod_id int(11) NOT NULL,"
	    "  PRIMARY KEY (id),"
	    "  KEY fspmethod_id (fspmethod_id,variant_id,data_address,instr2)"
	    ") engine=MyISAM ";
	bool success = (bool) db->query(create_statement.c_str());
	if (!success) return false;

	create_statement = "CREATE TABLE IF NOT EXISTS fspgroup ("
	    "  variant_id      int(11) NOT NULL,"
	    "  instr2          int(11) unsigned NOT NULL,"
	    "  data_address    int(10) unsigned NOT NULL,"
	    "  fspmethod_id    int(11) NOT NULL,"
	    "  pilot_id        int(11) NOT NULL,"
	    "  PRIMARY KEY (variant_id, data_address, instr2, fspmethod_id),"
	    "  KEY joinresults (pilot_id,fspmethod_id)) engine=MyISAM";

	return db->query(create_statement.c_str());
}

bool Pruner::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM fsppilot WHERE variant_id IN (" << m_variant_id_query << ")";
	bool ret = (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fsppilot table" << std::endl;
	ss.str("");

	ss << "DELETE FROM fspgroup WHERE variant_id IN (" << m_variant_id_query << ")";
	ret = ret && (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fspgroup table" << std::endl;

	return ret;
}
