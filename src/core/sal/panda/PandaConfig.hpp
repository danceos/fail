/**
 * \file PandaConfig.hpp
 * \brief Type definitions and configuration settings for
 *        the Pandaboard.
 */

#ifndef __PANDA_CONFIG_HPP__
#define __PANDA_CONFIG_HPP__

namespace fail {

typedef uint32_t host_address_t;	//!< the host memory address type
typedef uint32_t guest_address_t;	//!< the guest memory address type
typedef uint32_t register_data_t;	//!< register data type (32 bit)
typedef int      timer_t;			//!< type of timer IDs

} // end-of-namespace: fail

#endif // __PANDA_CONFIG_HPP__
