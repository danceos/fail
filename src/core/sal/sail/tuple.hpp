#include <tuple>
#include "types.hpp"

namespace tuple {
#ifndef __acweaving
	template< int I, template<typename> class Cond, typename Tuple_t>
		constexpr int index_of_impl(){
            if constexpr(I >= std::tuple_size<Tuple_t>::value) {
                return -1;
            }
			else if constexpr(Cond<typename std::decay<typename std::tuple_element<I,Tuple_t>::type>::type>::value){
				return I;
			}else{
				return index_of_impl<I+1,Cond,Tuple_t>();
			}
		}

	template<template<typename> class Cond, typename Tuple_t>
		struct index_of {
			static constexpr int value = index_of_impl<0,Cond,Tuple_t>();
        };
#endif
}
