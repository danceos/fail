#ifndef __GEM5_PCEVENT_HPP__
  #define __GEM5_PCEVENT_HPP__

#include "cpu/pc_event.hh"

class Gem5PCEvent : public PCEvent
{
public:
	Gem5PCEvent(PCEventQueue* queue, Addr ip)
		: PCEvent(queue, "Fail Breakpoint", ip) {}
	virtual void process(ThreadContext *tc);
};

#endif // __GEM5_PCEVENT_HPP__
