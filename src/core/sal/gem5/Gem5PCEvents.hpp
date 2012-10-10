#ifndef __GEM5_PCEVENTS_HPP__
  #define __GEM5_PCEVENTS_HPP__

#include "cpu/pc_event.hh"

#include "FailGem5Device.hpp"

class Gem5BreakpointEvent : public PCEvent
{
public:
	Gem5BreakpointEvent(PCEventQueue* queue, Addr ip)
		: PCEvent(queue, "Fail breakpoint event", ip) {}
	virtual void process(ThreadContext *tc);
};

class Gem5InstructionEvent : public PCEvent
{
public:
	Gem5InstructionEvent(FailGem5Device* device, PCEventQueue* queue, Addr ip)
		: PCEvent(queue, "Fail instruction event", ip), m_FailDevice(device) {}
	virtual void process(ThreadContext *tc);

private:
	FailGem5Device* m_FailDevice;
};

#endif // __GEM5_PCEVENTS_HPP__
