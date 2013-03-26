#ifndef __BASIC_PRUNER_H__
#define __BASIC_PRUNER_H__

#include "Pruner.hpp"

class BasicPruner : public Pruner {
	bool use_instr1;
public:
	BasicPruner(bool use_instr1 = false) : Pruner(), use_instr1(use_instr1) {}
	virtual std::string method_name() { return std::string("basic") + (use_instr1 ? "-left" : ""); }
	virtual bool prune_all();
};

#endif
