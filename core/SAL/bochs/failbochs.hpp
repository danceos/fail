#ifndef FAILBOCHS_H
#define FAILBOCHS_H

#include <string>
#include <string.h>

#include "config.h" 

namespace sal{

  //DanceOS Richard Hellwig
#ifdef DANCEOS_RESTORE
  extern bx_bool restore_bochs_request;
  extern bx_bool save_bochs_request;
  extern bx_bool reboot_bochs_request;
  extern std::string sr_path;
#endif

}

#endif //FAILBOCHS_H
