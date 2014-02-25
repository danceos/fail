#ifndef __UTIL_ALIASEDREGISTERABLE_H__
#define __UTIL_ALIASEDREGISTERABLE_H__

#include <deque>
#include <string>

namespace fail {

class AliasedRegisterable {
public:
	virtual void getAliases(std::deque<std::string> *aliases) = 0;
};

}; // end of namespace fail

#endif // __UTIL_ALIASEDREGISTERABLE_H__
