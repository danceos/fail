#ifndef __PRUNER_H__
#define __PRUNER_H__

#include "util/Database.hpp"

class Pruner {
protected:
	int m_variant_id;
    int m_method_id;
    fail::Database *db;

public:
	bool init(const std::string &variant, const std::string &benchmark,
              fail::Database *sql);

    virtual std::string method_name() = 0;

	virtual bool create_database();
	virtual bool clear_database();

    virtual bool prune_all() = 0;
};

#endif
