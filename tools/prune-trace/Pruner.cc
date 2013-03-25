#include <sstream>
#include <iostream>
#include "util/Logger.hpp"

using namespace fail;
static Logger LOG ("Pruner");

#include "Pruner.hpp"


bool Pruner::init(const std::string &variant, const std::string &benchmark, Database *db) {
	this->db = db;
	if (!(m_variant_id = db->get_variant_id(variant, benchmark))) {
		return false;
	}
	if (!(m_method_id = db->get_fspmethod_id(method_name()))) {
		return false;
	}
	LOG << "Pruning variant "
	    << variant << "/" << benchmark      << " (ID: " << m_variant_id << ")"
	    << " with method " << method_name() << " (ID: " << m_method_id << ")"
	    << std::endl;
	return true;
}

bool Pruner::create_database() {
	std::string create_statement = "CREATE TABLE IF NOT EXISTS fsppilot ("
	    "  id int(11) NOT NULL AUTO_INCREMENT,"
	    "  known_outcome tinyint(4) NOT NULL,"
	    "  variant_id int(11) NOT NULL,"
	    "  instr2 int(10) unsigned NOT NULL,"
	    "  data_address int(10) unsigned NOT NULL,"
	    "  fspmethod_id int(11) NOT NULL,"
	    "  PRIMARY KEY (id),"
	    "  KEY fspmethod_id (fspmethod_id,variant_id,instr2,data_address)"
		") engine=MyISAM ";
	bool success = (bool) db->query(create_statement.c_str());
	if (!success) return false;

	create_statement = "CREATE TABLE IF NOT EXISTS fspgroup ("
	    "  variant_id int(11) NOT NULL,"
	    "  instr2 int(10) unsigned NOT NULL,"
	    "  data_address int(10) unsigned NOT NULL,"
	    "  fspmethod_id int(11) NOT NULL,"
	    "  pilot_id int(11) NOT NULL,"
	    "  PRIMARY KEY (variant_id, instr2, data_address, fspmethod_id, pilot_id),"
	    "  KEY joinresults (pilot_id,fspmethod_id))";

	return db->query(create_statement.c_str());
}

bool Pruner::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM fsppilot WHERE variant_id = " << m_variant_id << " AND fspmethod_id = " << m_method_id;
	bool ret = (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fsppilot table" << std::endl;
	ss.str("");

	ss << "DELETE FROM fspgroup WHERE variant_id = " << m_variant_id << " AND fspmethod_id = " << m_method_id;
	ret = ret && (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fspgroup table" << std::endl;

	return ret;
}
