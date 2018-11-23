/**
 * \file BochsConfig.hpp
 * \brief Type definitions and configuration settings for
 *        the Bochs simulator.
 */

#ifndef __BOCHS_CONFIG_HPP__
#define __BOCHS_CONFIG_HPP__

#include "bochs.h"
#include "config.h"

namespace fail {

typedef bx_address guest_address_t; //!< the guest memory address type
typedef Bit8u*     host_address_t;  //!< the host memory address type
//! register data type (64 bit, regardless of BX_SUPPORT_X86_64: FPU and vector
//! registers are that size or larger also for 32-bit machines)
typedef Bit64u     register_data_t;
typedef int        timer_t;         //!< type of timer IDs

// 'Publish' 64 bit ability (if enabled in Bochs):
#if BX_SUPPORT_X86_64
	#define SIM_SUPPORT_64
#endif

} // end-of-namespace: fail

#endif // __BOCHS_CONFIG_HPP__
