#include <iostream>

#include "../SALInst.hpp"
#include "../SALConfig.hpp"
#include "config/FailConfig.hpp"

extern "C" {

void fail_init(struct CPUX86State *env)
{
	std::cout << "FailT32" FAIL_VERSION << std::endl;
	fail::simulator.startup();
}

void fail_watchpoint_hit(struct CPUX86State *env, uint64_t addr, int width, int is_write)
{
	// FIXME: instruction pointer
	fail::simulator.onMemoryAccess(addr, width, is_write == 1, 0);
}

}
