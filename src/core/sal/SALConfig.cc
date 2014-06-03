#include "SALConfig.hpp"

namespace fail {

// Flag initialization depends on the current selected simulator
// (For now, the initialization values are all the same):
#if defined BUILD_BOCHS || defined BUILD_GEM5 || \
    defined BUILD_T32   || defined BUILD_QEMU || \
    defined BUILD_PANDA
const address_t       ADDR_INV = static_cast<address_t>  (0);
const address_t       ANY_ADDR = static_cast<address_t> (-1);
const unsigned       ANY_INSTR = static_cast<unsigned>  (-1);
const unsigned        ANY_TRAP = static_cast<unsigned>  (-1);
const unsigned   ANY_INTERRUPT = static_cast<unsigned>  (-1);
const timer_id_t INVALID_TIMER = static_cast<timer_id_t> (0);
#else
	#error SAL Config Target not defined
#endif

} // end-of-namespace: fail
