#include "Gem5Connector.hpp"

#include "base/trace.hh"
#include "debug/FailState.hh"
#include "sim/root.hh"

Gem5Connector connector;

void Gem5Connector::save(const std::string &path)
{
	DPRINTF(FailState, "Saving state to %s.\n", path);

	Root* root = Root::root();
	root->Serializable::serializeAll(path);
}

void Gem5Connector::restore(const std::string &path)
{
	DPRINTF(FailState, "Restoring state from %s.\n", path);

	Root* root = Root::root();
	Checkpoint cp(path);

	root->loadState(&cp);
}
