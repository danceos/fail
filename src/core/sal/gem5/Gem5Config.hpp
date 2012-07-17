/**
 * \brief Type definitions and configuration settings for
 *        the gem5 simulator.
 */

#ifndef __GEM5_CONFIG_HPP__
  #define __GEM5_CONFIG_HPP__

#include "base/types.hh"
#include "arch/arm/registers.hh"

namespace fail {

typedef Addr guest_address_t; //!< the guest memory address type
// TODO: Set Host Address Type
typedef void*     host_address_t;  //!< the host memory address type
typedef ArmISA::AnyReg   register_data_t; //!< register data type (32 bit)
typedef int        timer_t;         //!< type of timer IDs

} // end-of-namespace: fail

#endif // __GEM5_CONFIG_HPP__
