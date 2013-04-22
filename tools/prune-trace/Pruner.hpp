#ifndef __PRUNER_H__
#define __PRUNER_H__

#include <vector>
#include <string>
#include "util/Database.hpp"

class Pruner {
protected:
	int m_method_id;
	std::string m_variant_id_query;
	fail::Database *db;

public:
	bool init(fail::Database *db,
		const std::vector<std::string>& variants,
		const std::vector<std::string>& variants_exclude,
		const std::vector<std::string>& benchmarks,
		const std::vector<std::string>& benchmarks_exclude);

	virtual std::string method_name() = 0;

	virtual bool create_database();
	virtual bool clear_database();

	virtual bool prune_all() = 0;
};

#endif
