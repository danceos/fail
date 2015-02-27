#ifndef __FAIL_BOCHS_GLOBALS_HPP__
#define __FAIL_BOCHS_GLOBALS_HPP__

#include <string>

#include "config.h"

namespace fail {

#ifdef DANCEOS_RESTORE
extern bx_bool restore_bochs_request;
extern bx_bool restore_bochs_finished;
extern bx_bool save_bochs_request;
extern std::string sr_path;
#endif

extern bx_bool reboot_bochs_request;
extern bx_bool interrupt_injection_request;

} // end-of-namespace: fail

#endif // __FAIL_BOCHS_GLOBALS_HPP__
