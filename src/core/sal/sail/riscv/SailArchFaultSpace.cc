#include <memory>
#include "SailArchFaultSpace.hpp"
#include "sal/faultspace/MemoryArea.hpp"
#include "sal/faultspace/RegisterArea.hpp"


fail::SailFaultSpace::SailFaultSpace() {
	// Order Matters!
	// 4GiB of RAM
	add_area(std::make_unique<MemoryArea>("ram", 0, 0xffffffff));
	// The Registers
	set_point(1L<<32);
	add_area(std::make_unique<RegisterArea>());
}

