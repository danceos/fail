#include <iostream>

#include "../SALInst.hpp"
#include "../SALConfig.hpp"
#include "config/FailConfig.hpp"

struct CPUX86State; // fwd

extern "C" {

#include "qemu/failqemu.h"

void fail_init(struct CPUX86State *env)
{
	std::cout << "FailQEMU v" FAIL_VERSION << std::endl;
	fail::simulator.setCPUEnv(env);
	fail::simulator.startup();
}

void fail_watchpoint_hit(struct CPUX86State *env, uint64_t addr, int width, int is_write)
{
	std::cout << "fail_breakpoint_hit" << std::endl;
	// FIXME: instruction pointer
	fail::simulator.onMemoryAccess(addr, width, is_write == 1, 0);
}

}
