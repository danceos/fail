#include <iostream>
#include <deque>
#include <string>

#include "util/AliasedRegistry.hpp"
#include "util/AliasedRegisterable.hpp"

namespace fail {

bool AliasedRegistry::add(AliasedRegisterable *obj) {
	std::deque<std::string> aliases;
	obj->getAliases(&aliases);

	bool inserted = false;
	for (std::deque<std::string>::iterator it = aliases.begin(); it != aliases.end(); ++it) {
		if (m_registry.find(*it) == m_registry.end()) {
			if (!inserted) {
				m_primes[obj] = *it;
			}
			m_registry[*it] = obj;
			inserted = true;
		} else {
#ifndef __puma
			std::cerr << "AliasedRegistry: alias '" << *it << "' already exists!" << std::endl;
#endif
		}
	}
	return inserted;
}

AliasedRegisterable *AliasedRegistry::get(std::string key) {
	std::map<std::string, AliasedRegisterable*>::iterator it = m_registry.find(key);
	if (it != m_registry.end()) {
		return it->second;
	} else {
		return 0;
	}
}

bool AliasedRegistry::getPrimeAlias(AliasedRegisterable *obj, std::string& alias) {
	std::map<AliasedRegisterable*, std::string>::iterator it = m_primes.find(obj);
	if (it != m_primes.end()) {
		alias = it->second;
		return true;
	} else {
		return false;
	}
}

void AliasedRegistry::getPrimeAliases(std::deque<std::string>& aliases) {
	std::map<AliasedRegisterable*, std::string>::iterator it = m_primes.begin();
	for (;it != m_primes.end(); ++it) {
		aliases.push_back(it->second);
	}
}

std::string AliasedRegistry::getPrimeAliasesCSV(){
	std::string csv = "";
	std::deque<std::string> primes;
	getPrimeAliases(primes);
	for (std::deque<std::string>::iterator it = primes.begin(); it != primes.end(); ++it) {
		csv += *it + ", ";
	}
	csv.resize(csv.size()-2);
	return csv;
}

}; // end of namespace fail
