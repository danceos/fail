#include "Gem5PCEvent.hh"
#include "../SALInst.hpp"

void Gem5PCEvent::process(ThreadContext *tc)
{
	fail::simulator.onBreakpoint(this->evpc, fail::ANY_ADDR);
}
