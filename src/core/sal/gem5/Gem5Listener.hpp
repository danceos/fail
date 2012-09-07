#ifndef __GEM5_LISTENER_HPP__
  #define __GEM5_LISTENER_HPP__

#include "../Listener.hpp"
#include "Gem5PCEvent.hh"

namespace fail {

class Gem5BPSingleListener : public GenericBPSingleListener
{
public:
	Gem5BPSingleListener(address_t ip = 0);
	virtual bool onAddition();
	~Gem5BPSingleListener();
private:
	Gem5PCEvent* m_Breakpoint;
};

typedef Gem5BPSingleListener BPSingleListener;

} // end-of-namespace: fail

#endif // __GEM5_LISTENER_HPP__
