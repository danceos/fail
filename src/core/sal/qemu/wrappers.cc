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
	// TODO pass on command-line parameters
	fail::simulator.startup();
}

void fail_watchpoint_hit(struct CPUX86State *env, uint64_t addr, int width, int is_write)
{
	// FIXME: instruction pointer
	fail::simulator.onMemoryAccess(addr, width, is_write == 1, 0);
}

void fail_io(int port, int width, int32_t data, int is_write)
{
	// FIXME: width is discarded
	fail::simulator.onIOPort((unsigned char)data, port, is_write == 1);
}

void fail_timer_callback(void *opaque)
{
	fail::TimerListener *l = static_cast<fail::TimerListener *>(opaque);
	fail::simulator.onTimerTrigger(l);
}

}
