#ifndef __types_hpp__
#define __types_hpp__

#include <type_traits>

using true_t = std::true_type;
using false_t = std::false_type;
struct void_t {
    using type = void;
};

#endif
