#ifndef __FAILBOCHS_HPP__
  #define __FAILBOCHS_HPP__

#include <string>

#include "config.h" 

// FIXME: Maybe rename this file to "FailBochsGlobals.hpp"?

namespace fail {

#ifdef DANCEOS_RESTORE
  extern bx_bool restore_bochs_request;
  extern bx_bool save_bochs_request;
  extern std::string sr_path;
#endif

extern bx_bool reboot_bochs_request;
extern bx_bool interrupt_injection_request;
extern int interrupt_to_fire;

}

#endif // __FAILBOCHS_HPP__
