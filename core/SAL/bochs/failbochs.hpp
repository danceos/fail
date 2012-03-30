#ifndef FAILBOCHS_H
#define FAILBOCHS_H

#include <string>
#include <string.h>

#include "config.h" 

namespace sal{

#ifdef DANCEOS_RESTORE
  extern bx_bool restore_bochs_request;
  extern bx_bool save_bochs_request;
  extern bx_bool reboot_bochs_request;
  extern bx_bool interrupt_injection_request;
  extern unsigned interrupt_to_fire;
  extern std::string sr_path;
#endif

}

#endif //FAILBOCHS_H
