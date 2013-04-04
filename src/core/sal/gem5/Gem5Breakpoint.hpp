#ifndef __GEM5_BREAKPOINT_HPP__
  #define __GEM5_BREAKPOINT_HPP__

#include "../SALConfig.hpp"
#include "cpu/pc_event.hh"

namespace fail {

class Gem5Breakpoint : public PCEvent {
public:
	Gem5Breakpoint(PCEventQueue* queue, Addr ip)
		: PCEvent(queue, "Fail* experiment breakpoint", ip) { }
	virtual void process(ThreadContext *tc);
};

} // end-of-namespace: fail

#endif // __GEM5_BREAKPOINT_HPP__
