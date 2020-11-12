#ifndef __serialize_hpp__
#define __serialize_hpp__

#include <type_traits>
#include <functional>
#include <iostream>

namespace fail {
    namespace serialize {
        template <class Tuple, std::size_t...Is>
            constexpr void to_impl(std::ostream& os, const Tuple& t, std::index_sequence<Is...>) {
#ifndef __acweaving
                ((os << std::get<Is>(t) << " "), ... );
#endif
            }
        template <class...Args>
            constexpr void to(std::ostream& os, const std::tuple<Args...>& t) {
                to_impl(os, t, std::index_sequence_for<Args...>{});
            }

        template <class Tuple, std::size_t...Is>
            constexpr void from_impl(std::istream& is, const Tuple& t, std::index_sequence<Is...>) {
#ifndef __acweaving
                ((is >> std::get<Is>(t)), ... );
#endif
            }

        template <class...Args>
            constexpr void from(std::istream& is, const std::tuple<Args...>& t) {
                from_impl(is, t, std::index_sequence_for<Args...>{});
            }
    }
}

#endif
