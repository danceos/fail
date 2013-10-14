#include "Demangler.hpp"

#define HAVE_DECL_BASENAME 1
#include <demangle.h>
#include <stdio.h>

namespace fail {

const std::string Demangler::DEMANGLE_FAILED = "[Demangler] Demangle failed.";

std::string Demangler::demangle(const std::string& name){
	const char* res = cplus_demangle(name.c_str(), 0);
	if (res != NULL) {
		return std::string(res);
	} else {
		return Demangler::DEMANGLE_FAILED;
	}
}

} // end of namespace
