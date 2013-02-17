#include "BochsListener.hpp"
#include "../SALInst.hpp"

namespace fail {

void onTimerTrigger(void* thisPtr)
{
	simulator.onTimerTrigger(thisPtr);
}

} // end-of-namespace: fail
