/**
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
#if BX_SUPPORT_X86_64
  typedef Bit64u   register_data_t; //!< register data type (64 bit)
#else
  typedef Bit32u   register_data_t; //!< register data type (32 bit)
#endif
typedef int        timer_t;         //!< type of timer IDs

} // end-of-namespace: fail

#endif // __BOCHS_CONFIG_HPP__
