#ifndef __PRUNER_H__
#define __PRUNER_H__

#include <vector>
#include <string>
#include "util/Database.hpp"
#include "util/AliasedRegisterable.hpp"

class Pruner : public fail::AliasedRegisterable {
protected:
	int m_method_id;
	fail::Database *db;
	std::vector<fail::Database::Variant> m_variants;
	std::string m_variants_sql;

public:
	void set_db(fail::Database *db) { this->db = db; }

	bool init(
		const std::vector<std::string>& variants,
		const std::vector<std::string>& variants_exclude,
		const std::vector<std::string>& benchmarks,
		const std::vector<std::string>& benchmarks_exclude,
		bool overwrite);

	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool commandline_init() { return true; }

	virtual std::string method_name() = 0;

	virtual bool create_database();
	virtual bool clear_database();

	virtual bool prune_all() = 0;
};

#endif
