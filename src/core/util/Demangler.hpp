#ifndef __DEMANGLER_HPP
#define __DEMANGLER_HPP

#include <string>

namespace fail {

class Demangler {
public:

	/**
	 * Get the demangled symbol name of a mangled string.
	 * @param name The mangled symbol
	 * @return The according demangled name if found, else Demangler::DEMANGLE_FAILED
	 */
	static std::string demangle(const std::string & name);

	//! Inform about failed demangling.
	static const std::string DEMANGLE_FAILED;
};

} // end of namespace

#endif // DEMANGLER_HPP
