#ifndef __OVP_CONFIG_HPP__
  #define __OVP_CONFIG_HPP__

// Type definitions and configuration settings for
// the OVP simulator.
 
namespace fail {

typedef uint32_t guest_address_t;  //!< the guest memory address type
typedef uint8_t* host_address_t;   //!< the host memory address type
typedef uint32_t register_data_t;  //!< register data type (32 bit)
typedef int      timer_t;          //!< type of timer IDs

}

#endif // __OVP_CONFIG_HPP__
