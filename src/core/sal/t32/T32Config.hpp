/**
 * \file T32Config.hpp
 * \brief Type definitions and configuration settings for the
 *        T32 target backend.
 */

#ifndef __T32_CONFIG_HPP__
#define __T32_CONFIG_HPP__

namespace fail {

typedef uint32_t guest_address_t; //!< the guest memory address type
typedef unsigned char* host_address_t;  //!< the host memory address type
typedef uint32_t register_data_t; //!< register data type (64 bit)
typedef int        timer_t;         //!< type of timer IDs

} // end-of-namespace: fail

#endif // __T32_CONFIG_HPP__
