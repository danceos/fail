#ifndef __UTIL_ALIASEDREGISTRY_H__
#define __UTIL_ALIASEDREGISTRY_H__

#include "util/AliasedRegisterable.hpp"
#include <map>

namespace fail {

class AliasedRegistry {
private:
	std::map<std::string, AliasedRegisterable*> m_registry;
	std::map<AliasedRegisterable*, std::string> m_primes;

public:
	/**
	 * Register given AliasedRegisterable by it's aliases
	 */
	bool add(AliasedRegisterable* obj);

	/**
	 * Get an AliasedRegisterable by an arbitrary alias
	 */
	AliasedRegisterable *get(std::string alias);

	/**
	 * Get the prime (i.e. first to be specified) alias for given object
	 */
	bool getPrimeAlias(AliasedRegisterable* target, std::string& alias);

	/**
	 * Get all registered prime aliases
	 */
	void getPrimeAliases(std::deque<std::string>& prime_aliases);

	/**
	 * Get prime aliases' names as CSV
	 */
	std::string getPrimeAliasesCSV();
};

}; // end of namespace fail

#endif // __UTIL_ALIASEDREGISTRY_H__
