#include "../SALInst.hpp"
#include "config/FailConfig.hpp"

extern "C" {

#include <stdio.h>
//#include "qemu/failqemu.h"

struct CPUX86State;
void failqemu_init(struct CPUX86State *env)
{
	printf("FailQEMU v%s\n", FAIL_VERSION);
	fail::simulator.startup();
}

}
