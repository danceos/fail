#ifndef __BOCHS_CONFIG_HPP__
  #define __BOCHS_CONFIG_HPP__

#include "../../../bochs/bochs.h"
#include "../../../bochs/config.h"

// Type definitions and configuration settings for
// the Bochs simulator.
 
namespace sal
{

typedef bx_address guest_address_t; //!< the guest memory address type
typedef Bit8u*     host_address_t;  //!< the host memory address type
#if BX_SUPPORT_X86_64
  typedef Bit64u   register_data_t; //!< register data type (64 bit)
#else
  typedef Bit32u   register_data_t; //!< register data type (32 bit)
#endif
typedef int        timer_t;         //!< type of timer IDs

};

#endif /* __BOCHS_CONFIG_HPP__ */

