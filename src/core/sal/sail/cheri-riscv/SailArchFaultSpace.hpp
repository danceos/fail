#pragma once

#include <memory>
#include "sal/faultspace/BaseFaultSpace.hpp"
#include "sal/faultspace/RegisterArea.hpp"


namespace fail {

class SailCheriRegisterArea: public RegisterArea {
public:
	// We override the translate function to also touch the tag bit for capability registers
	virtual std::vector<std::pair<RegisterElement, unsigned>> translate(const RegisterView &) override;
};

class SailFaultSpace: public BaseFaultSpace {
public:
	SailFaultSpace();
};

} // end-of-namespace: fail

