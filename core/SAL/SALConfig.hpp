#ifndef __SAL_CONFIG_HPP__
  #define __SAL_CONFIG_HPP__



#include <stdint.h>
#include "../variant_config.h"

#if defined BUILD_BOCHS

#include "bochs/BochsConfig.hpp" // current simulator config

namespace sal
{

typedef guest_address_t  address_t;   //!< common address type to be used in experiment flows
typedef uint8_t          byte_t;      //!< 8 bit type for memory access (read or write)
typedef uint32_t         regwidth_t;  //!< type of register width [bits]
typedef register_data_t  regdata_t;   //!< type of register data

extern const address_t ADDR_INV; //!< invalid address flag (defined in Memory.cc)

}

#elif defined BUILD_OVP

#include "ovp/OVPConfig.hpp" // current simulator config

namespace sal
{

typedef guest_address_t  address_t;   //!< common address type to be used in experiment flows
typedef uint8_t          byte_t;      //!< 8 bit type for memory access (read or write)
typedef uint32_t         regwidth_t;  //!< type of register width [bits]
typedef register_data_t  regdata_t;   //!< type of register data

extern const address_t ADDR_INV; //!< invalid address flag (defined in Memory.cc)

}
#else
#error SAL Config Target not defined
#endif


#endif // __SAL_CONFIG_HPP__
