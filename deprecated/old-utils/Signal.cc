
// Author: Adrian Böckenkamp
// Date:   15.06.2011

// FIXME: This is deprecated stuff. Delete it.

#include "Signal.hpp"

namespace fi
{

std::auto_ptr<Signal> Signal::m_This;
Mutex Signal::m_InstanceMutex;
	

} // end-of-namespace: fi
