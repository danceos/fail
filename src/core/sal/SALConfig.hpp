#ifndef __SAL_CONFIG_HPP__
  #define __SAL_CONFIG_HPP__

#include <stdint.h>

#include "config/VariantConfig.hpp"

// Type-config depends on the current selected simulator:
#if defined BUILD_BOCHS
  #include "bochs/BochsConfig.hpp"
#elif defined BUILD_OVP
  #include "ovp/OVPConfig.hpp"
#elif defined BUILD_GEM5
  #include "gem5/Gem5Config.hpp"
#else
  #error SAL Config Target not defined
#endif

namespace fail {

typedef guest_address_t  address_t;   //!< common address type to be used in experiment flows
typedef uint8_t          byte_t;      //!< 8 bit type for memory access (read or write)
typedef uint32_t         regwidth_t;  //!< type of register width [bits]
typedef register_data_t  regdata_t;   //!< type of register data
typedef timer_t          timer_id_t;  //!< type of timer IDs

extern const address_t ADDR_INV; //!< invalid address flag (defined in Memory.cc)

} // end-of-namespace: fail

#endif // __SAL_CONFIG_HPP__
