#include <sstream>
#include <iostream>
#include "util/Logger.hpp"

using namespace fail;
static Logger LOG ("Pruner");

#include "Pruner.hpp"


bool Pruner::init(
		const std::vector<std::string>& variants,
		const std::vector<std::string>& variants_exclude,
		const std::vector<std::string>& benchmarks,
		const std::vector<std::string>& benchmarks_exclude,
		bool overwrite, bool incremental)
{
	m_variants = db->get_variants(
		variants, variants_exclude,
		benchmarks, benchmarks_exclude);

	if (!(m_method_id = db->get_fspmethod_id(method_name()))) {
		return false;
	}
	LOG << "Pruning with method " << method_name() << " (ID: " << m_method_id << ")"
	    << std::endl;

	// make sure we only prune variants that haven't been pruned previously
	// (unless we run with --overwrite or --incremental)
	if (!overwrite && !incremental) {
		for (std::vector<fail::Database::Variant>::iterator it = m_variants.begin();
			it != m_variants.end(); ) {
			std::stringstream ss;
			MYSQL_RES *res;
			ss << "(SELECT variant_id FROM fsppilot WHERE "
			   << " variant_id = " << it->id << " AND "
			   << " fspmethod_id = " << m_method_id
			   << " LIMIT 1)"
			   << " UNION ALL "
			   << "(SELECT variant_id FROM fspgroup WHERE "
			   << " variant_id = " << it->id << " AND "
			   << " fspmethod_id = " << m_method_id
			   << " LIMIT 1)";
			if (!(res = db->query(ss.str().c_str(), true))) {
				return false;
			}
			if (mysql_num_rows(res) > 0) {
				// skip this variant
				LOG << "skipping " << it->variant << "/" << it->benchmark
				    << " due to existing pruning data (use --overwrite to skip this check)"
				    << std::endl;
				it = m_variants.erase(it);
			} else {
				++it;
			}
		}
	}

	// any variants left?
	if (m_variants.size() == 0) {
		LOG << "no variants found, nothing to do" << std::endl;
		return false;
	}

	// construct comma-separated list usable in SQL "IN (...)"
	std::stringstream commalist;
	for (std::vector<fail::Database::Variant>::const_iterator it = m_variants.begin();
		it != m_variants.end(); ++it) {

		if (it != m_variants.begin()) {
			commalist << ",";
		}
		commalist << it->id;
	}
	m_variants_sql = commalist.str();

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
	    "  data_width int(10) unsigned NOT NULL,"
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
	    "  weight int(11) UNSIGNED,"
	    "  PRIMARY KEY (variant_id, data_address, instr2, fspmethod_id),"
	    "  KEY joinresults (pilot_id,fspmethod_id)) engine=MyISAM";

	return db->query(create_statement.c_str());
}

bool Pruner::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM fsppilot WHERE variant_id IN (" << m_variants_sql
		<< ") AND fspmethod_id = " << m_method_id;
	bool ret = (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fsppilot table" << std::endl;
	ss.str("");

	ss << "DELETE FROM fspgroup WHERE variant_id IN (" << m_variants_sql
		<< ") AND fspmethod_id = " << m_method_id;
	ret = ret && (bool) db->query(ss.str().c_str());
	LOG << "deleted " << db->affected_rows() << " rows from fspgroup table" << std::endl;

	return ret;
}
