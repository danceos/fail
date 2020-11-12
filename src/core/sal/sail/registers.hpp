#pragma once

#include "../Register.hpp"
#include "types.hpp"
#include "serialize.hpp"
#include <functional>
#include <array>
#include <cassert>
#include <stdexcept>
#include "../SALConfig.hpp"

namespace fail {
    namespace registers {
        template <class T>
        struct is_pc: false_t {};
        template <class T>
        struct is_sp: false_t {};
        template <class T>
        struct is_instcount: false_t {} ;
        template <class T>
        struct is_tag: false_t {} ;

    }
    class reg: public Register {
        private:
#ifndef __acweaving
        inline static unsigned unique_id = sail_arch::registers::LAST_NAMED_REG_ID;
#else
        static unsigned unique_id;
#endif
        friend std::ostream& operator<<(std::ostream& os, const reg& r);
        friend std::istream& operator>>(std::istream& is, const reg& r);

        protected:
        const std::vector<RegisterType> getTypes() const { return std::vector<RegisterType>{0}; }

        public:
        // constructor for usage with llvm translatable registers
        reg(unsigned id):
            Register(id, sizeof(regdata_t)*8) {}

        // constructor for usage with non-llvm translatable registers
        reg(): reg(unique_id++) {}

        virtual regdata_t read() const = 0;
		// NOTE: this can be const too, since we don't technically change the this ptr
		virtual void write(regdata_t val) const = 0;
    };
    inline std::ostream& operator<<(std::ostream& os, const reg& r) {
        regdata_t value = r.read();
        os << value;
        return os;
    }
    inline std::istream& operator>>(std::istream& is, const reg& r) {
        regdata_t dummy;
        is >> dummy;
        r.write(dummy);
        return is;
    }


    template<regdata_t (*RFn)(void), void (*WFn)(regdata_t)>
    class fn_reg: public reg {
        public:
        fn_reg(unsigned id): reg(id) {}
        fn_reg(): reg() {}
        virtual regdata_t read() const { return RFn(); }
        virtual void write(regdata_t val) const { return WFn(val); }
    };

    class ptr_reg: public reg {
        uint64_t*  m_ref;

        public:
        ptr_reg(uint64_t* ref, unsigned id): reg(id), m_ref(ref) {}
        ptr_reg(uint64_t* ref): reg(), m_ref(ref) {}
        virtual regdata_t read() const { assert(m_ref); return *m_ref; }
		virtual void write(regdata_t val) const { assert(m_ref); *m_ref = val; }
    };

    /* abstraction for N=sizeof..(Regs) tag bits of registers,
     * where Types must be a homogenous typelist for a type T
     * which has the ztag member (designed for CHERIs capability format)
     * This could optionally be extended to support arbitrary members by
     * using a member accesor function instead of hardcoding ztag.
     * NOTE: sizeof...(Types) bits must fit into regdata_t for this
     * to work.
     */
#define MAKE_CONST(x) const_val<decltype(x), x>{}
#define STATIC_ASSERT_EQUAL(x, y) assert_equal(MAKE_CONST(x), MAKE_CONST(y));

    template<typename T, T val>
        struct const_val {
            constexpr const_val() = default;
            constexpr const_val(T v) {}
        };

    template<typename T, T A, typename U, U B>
        constexpr void assert_equal(const_val<T, A>, const_val<U, B>) {
            static_assert(A == B, "Values are not equal!");
        }

    template<typename T, T... Regs>
    class tag_reg: public reg {
        static_assert(std::is_pointer<T>::value, "Regs must be pointer type.");
        constexpr static size_t m_len = sizeof...(Regs);
        std::array<T,m_len> m_refs = { Regs... };
        // this assumes that sizeof...(IDs) == sizeof(Regs)
        // which is asserted in the constructor
        std::array<unsigned,m_len> m_ids;

    public:
        template<typename ... IDs>
        constexpr tag_reg(IDs ... ids): reg(), m_ids{ids...} {
            //static_assert(sizeof...(IDs) == sizeof...(Regs), "mismatched sizeof(IDs) and sizeof(Regs)");
            constexpr auto id_len = sizeof...(IDs);
            STATIC_ASSERT_EQUAL(id_len,m_len);
        }
        virtual regdata_t read() const {
            static_assert(m_len/8 <= sizeof(regdata_t), "can't fit all registers into regdata_t");
            regdata_t val = 0;
            for(size_t i = 0; i < m_len; ++i) {
                val |= (m_refs[i]->ztag << i);
            }
            return val;
        }

        void write(regdata_t val) const {
            static_assert(m_len/8 <= sizeof(regdata_t), "can't fit all registers into regdata_t");
            for(size_t i = 0; i < m_len; ++i) {
                m_refs[i]->ztag = val & (1 << i);
            }
        }

        size_t get_index(unsigned id) const {
            for(size_t i = 0; i < m_len; ++i) {
                if(m_ids[i] == id) { return i; }
            }
            throw std::out_of_range("register id " + std::to_string(id) + " not found in tag register");
        }
        unsigned get_id(size_t index) const {
            if(index >= m_len) {
                throw std::out_of_range("tag bit index " + std::to_string(index) + " is out-of-range");
            }
            return m_ids[index];
        }
    };

    struct dummy_tag_struct { bool ztag; };
    class dummy_tag_reg: public tag_reg<dummy_tag_struct*> { };
}

// TODO: move the ref back to SAIL_DECLARE_REG and
//       statically pass the macro parameter into the reg(..)
//       constructor.
#define SAIL_DECLARE_REG(CLS, T, ...) \
    SAIL_REG_BODY(CLS,ptr_reg,(uint64_t* r),(r),T,__VA_ARGS__)

#define SAIL_DECLARE_REG_ID(CLS, ID, T, ...) \
    SAIL_REG_BODY(CLS,ptr_reg,(uint64_t* r),(r,ID),T,__VA_ARGS__)

#define SAIL_DECLARE_FN_REG(CLS, RFUN, WFUN, T, ...) \
    SAIL_REG_BODY(CLS,QUOTE_ARG(fn_reg<RFUN,WFUN>),(),(),T,__VA_ARGS__)

#define SAIL_DECLARE_FN_REG_ID(CLS, ID, RFUN, WFUN, T, ...) \
    SAIL_REG_BODY(CLS,QUOTE_ARG(fn_reg<RFUN,WFUN>),(),(ID),T,__VA_ARGS__)

#define SAIL_DECLARE_TAG_REG(CLS, TYPE, ...) \
    class CLS##_t: public tag_reg<TYPE,__VA_ARGS__> { \
        public: \
        template<typename ... Args> \
        CLS##_t (Args... args): tag_reg<TYPE,__VA_ARGS__> (args...) { setName(#CLS); } \
        const std::vector<RegisterType> getTypes() { return { RT_GP }; } \
    }; \
    extern CLS##_t CLS;


#define QUOTE_ARG(...) __VA_ARGS__

#define SAIL_REG_BODY(CLS,PARENT,CTOR, PCTOR,...)\
    class CLS##_t: public PARENT { \
        public: \
        CLS##_t CTOR: PARENT PCTOR { setName(#CLS); } \
        const std::vector<RegisterType> getTypes() { return { __VA_ARGS__ }; } \
    }; \
    extern CLS##_t CLS


#define SAIL_REG(CLS, SAILVAR)  \
	CLS##_t CLS(&SAILVAR)

#define SAIL_FN_REG(CLS) \
    CLS##_t CLS

#define SAIL_TAG_REG(CLS, ...)  \
    CLS##_t CLS(__VA_ARGS__)

#define SAIL_DECLARE_IMPL_REG(CLS) SAIL_DECLARE_REG(CLS, RT_ARCH_SPECIFIC)

#ifndef __acweaving
#define SAIL_SET_SP(CLS) \
    template <> struct fail::registers::is_sp <CLS##_t>: true_t {}
#else
#define SAIL_SET_SP(CLS)
#endif

#ifndef __acweaving
#define SAIL_SET_PC(CLS) \
    template <> struct fail::registers::is_pc <CLS##_t>: true_t {}
#else
#define SAIL_SET_PC(CLS)
#endif

#ifndef __acweaving
#define SAIL_SET_INSTCOUNT(CLS)\
    template <> struct fail::registers::is_instcount <CLS##_t>: true_t {}
#else
#define SAIL_SET_INSTCOUNT(CLS)
#endif

#ifndef __acweaving
#define SAIL_SET_TAG(CLS) \
    template <> struct fail::registers::is_tag <CLS##_t>: true_t {}
#else
#define SAIL_SET_TAG(CLS)
#endif
