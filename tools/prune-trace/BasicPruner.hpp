#ifndef __BASIC_PRUNER_H__
#define __BASIC_PRUNER_H__

#include "Pruner.hpp"

class BasicPruner : public Pruner {
	virtual std::string method_name() { return "basic"; }
	virtual bool prune_all();
};

#endif
