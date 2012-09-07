#include "Gem5Listener.hpp"
#include "../SALInst.hpp"

#include "sim/system.hh"

namespace fail {

Gem5BPSingleListener::Gem5BPSingleListener(address_t ip)
	: GenericBPSingleListener(ip)
{

}

bool Gem5BPSingleListener::onAddition()
{
	if(!m_Breakpoint)
	{
		System* sys = *System::systemList.begin();
		m_Breakpoint = new Gem5PCEvent(&sys->pcEventQueue, this->m_WatchInstrPtr);
		return true;
	}
	return false;
}

Gem5BPSingleListener::~Gem5BPSingleListener()
{
	if(m_Breakpoint)
	{
		delete m_Breakpoint;
	}
}

} // end-of-namespace: fail
