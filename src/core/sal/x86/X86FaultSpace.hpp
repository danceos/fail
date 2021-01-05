#pragma once

#include <memory>
#include "sal/faultspace/BaseFaultSpace.hpp"

namespace fail {
class X86FaultSpace: public BaseFaultSpace {
public:
	X86FaultSpace();
};

} // end-of-namespace: fail

