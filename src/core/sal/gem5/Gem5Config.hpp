/**
 * \brief Type definitions and configuration settings for
 *        the gem5 simulator.
 */

#ifndef __GEM5_CONFIG_HPP__
  #define __GEM5_CONFIG_HPP__

#include "base/types.hh"

namespace fail {

typedef Addr guest_address_t; //!< the guest memory address type
// TODO: Set Host Address Type
typedef void*     host_address_t;  //!< the host memory address type
typedef uint64_t   register_data_t; //!< register data type (gem5 always uses 64 bit for registers)
typedef int        timer_t;         //!< type of timer IDs

} // end-of-namespace: fail

#endif // __GEM5_CONFIG_HPP__
