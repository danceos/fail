#ifndef __BASIC_PRUNER_H__
#define __BASIC_PRUNER_H__

#include "Pruner.hpp"

class BasicPruner : public Pruner {
	bool use_instr1;
public:
	BasicPruner(bool use_instr1 = false) : use_instr1(use_instr1) {}
	virtual std::string method_name() { return std::string("basic") + (use_instr1 ? "-left" : ""); }
	virtual bool prune_all();

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("BasicPruner");
		aliases->push_back("basic");
	}
};

class BasicPrunerLeft : public BasicPruner {
public:
	BasicPrunerLeft() : BasicPruner(true) {}
	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("BasicPrunerLeft");
		aliases->push_back("basic-left");
	}
};

#endif
