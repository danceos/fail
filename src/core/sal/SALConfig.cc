#include "SALConfig.hpp"

#if defined(BUILD_RISCV_CHERI) && defined(BUILD_64BIT)
#include "gmpxx.h"
#endif


namespace fail {

const char* memtype_descriptions[MEMTYPE_LAST] = {
	"unknown",
	"ram",
	"flash",
	"tags",
	"eeprom",
};

// Flag initialization depends on the current selected simulator
// (For now, the initialization values are all the same):
#if defined BUILD_BOCHS || defined BUILD_GEM5 || \
    defined BUILD_T32   || defined BUILD_QEMU || \
    defined BUILD_PANDA || defined BUILD_SAIL
const address_t       ADDR_INV = static_cast<address_t>  (0);
const address_t       ANY_ADDR = static_cast<address_t> (-1);
const memory_type_t   ANY_MEMORY = static_cast<memory_type_t> (-1);
const unsigned       ANY_INSTR = static_cast<unsigned>  (-1);
const unsigned        ANY_TRAP = static_cast<unsigned>  (-1);
const unsigned   ANY_INTERRUPT = static_cast<unsigned>  (-1);
const timer_id_t INVALID_TIMER = static_cast<timer_id_t> (0);
#else
	#error SAL Config Target not defined
#endif

#if defined(BUILD_RISCV_CHERI) && defined(BUILD_64BIT)

std::ostream& operator<<(std::ostream& os, unsigned __int128 v) {
    mpz_class result;
    result = static_cast<uint64_t>(v >> 64 & UINT64_MAX);
    result <<= 64;
    result += static_cast<uint64_t>(v & UINT64_MAX);
    return (os << result);
}

std::istream& operator>>(std::istream& is, unsigned __int128& v) {
    mpz_class input;
    unsigned __int128 hi, lo;
    is >> input;
    lo =  input.get_ui();
    input >>= 64;
    hi = input.get_ui();
    v = hi << 64 | lo;
    return is;
}

#endif


} // end-of-namespace: fail
