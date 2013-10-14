/*
 * Some constants for the T32 remote api calls.
 * see also doc/t32_remote_api.pdf
 */
#ifndef __T32_CONSTANTS_HPP__
#define __T32_CONSTANTS_HPP__
namespace fail {

namespace T32 {

//!< Breakpoint configuration
struct BP {
	enum {
		EXECUTION = 1<<0,
		HLL_STEP  = 1<<1,
		SPOT      = 1<<2,
		READ      = 1<<3,
		WRITE     = 1<<4,
		ALPHA     = 1<<5,
		BETA      = 1<<6,
		CHARLY    = 1<<7,
		CLEAR     = 1<<8,
	};
}; // struct BP


//!< Memory access variants
struct MEMACCESS {
	enum {
		DATA = 0,
		PROGRAM = 1,
		AD = 12,
		AP = 13,
		USR = 15,
	};
}; // struct MEMACCESS

}; // namespace T32
}; // namespace fail
#endif // __T32_CONSTANTS_HPP__

