#include "Gem5PCEvents.hpp"
#include "../SALInst.hpp"

#include "cpu/thread_context.hh"

void Gem5BreakpointEvent::process(ThreadContext *tc)
{
	fail::simulator.onBreakpoint(this->evpc, fail::ANY_ADDR);
}

void Gem5InstructionEvent::process(ThreadContext *tc)
{
	m_FailDevice->setNextBreakpoints(tc);
}
