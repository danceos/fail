#pragma once

#include <memory>
#include "sal/faultspace/BaseFaultSpace.hpp"

namespace fail {
class ArmFaultSpace: public BaseFaultSpace {
public:
	ArmFaultSpace();
};

} // end-of-namespace: fail

