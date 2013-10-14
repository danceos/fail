/**
 * \file QEMUConfig.hpp
 * \brief Type definitions and configuration settings for the
 * qemu-system-x86_64 target backend.
 */

#ifndef __QEMU_CONFIG_HPP__
#define __QEMU_CONFIG_HPP__

// FIXME: qemu/targphys.h defines address types (but relies on a global preprocessor macro)

struct QEMUTimer;

namespace fail {

typedef uint64_t guest_address_t; //!< the guest memory address type
typedef unsigned char* host_address_t;  //!< the host memory address type
typedef uint64_t register_data_t; //!< register data type (64 bit)
typedef QEMUTimer* timer_t;         //!< type of timer IDs

} // end-of-namespace: fail

#endif // __QEMU_CONFIG_HPP__
