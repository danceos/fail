#ifndef __sailcpu_hpp__
#define __sailcpu_hpp__


#include "../CPUState.hpp"
#include "../CPU.hpp"

#include "tuple.hpp"
#include <cstdarg>
#include <cassert>

namespace fail {
#ifdef __acweaving
    template <address_t Reset_vector, simtime_t Freq>
#else
    template <address_t Reset_vector, simtime_t Freq, auto&...Regs>
#endif

    class SailCPU: public CPUArchitecture, public CPUState {
        protected:
#ifndef __acweaving
			static constexpr std::tuple <decltype(Regs)...> regs { Regs... };

            static constexpr auto pc_idx = tuple::index_of<registers::is_pc, decltype(regs) >::value;
            static_assert(pc_idx != -1, "No PC register defined");
            static constexpr auto sp_idx = tuple::index_of<registers::is_sp, decltype(regs) >::value;
            static_assert(sp_idx != -1, "No SP register defined");
            static constexpr auto instcount_idx = tuple::index_of<registers::is_instcount, decltype(regs) >::value;
            static_assert(instcount_idx != -1, "No SP register defined");
            static constexpr auto tag_idx = tuple::index_of<registers::is_tag, decltype(regs) >::value;


            static constexpr auto& sp_reg = std::get<sp_idx>(regs);
			static constexpr auto& pc_reg = std::get<pc_idx>(regs);
			static constexpr auto& instcount_reg = std::get<instcount_idx>(regs);

        public:
            auto get_tag_reg() {
                if constexpr(tag_idx != -1) {
                    return std::get<tag_idx>(regs);
                } else {
                    return dummy_tag_reg();
                }
            }
#endif


        private:
            unsigned int id;
            friend reg;


        public:
            // FIXME: used by import-trace, to get CPUArchitecutre
            //        this needs to be refactored out of this class
            constexpr SailCPU(): SailCPU(0) { }
            constexpr SailCPU(unsigned int id): id(id) {
#ifndef __acweaving
				std::apply(
                        [this] (auto&... rs) {
                            (this->m_addRegister(static_cast<Register*>(&rs),rs.getTypes()),...);
                        }, regs);
#endif
            }
            virtual ~SailCPU() {}


            // CPUState interface
            regdata_t getRegisterContent(const Register* r) const {
#ifndef __acweaving
                return static_cast<const reg*>(r)->read();
#else
                return 0;
#endif
            }

            void setRegisterContent(const Register* r, regdata_t value) {
#ifndef __acweaving
                static_cast<const reg*>(r)->write(value);
#endif
            }

            address_t getInstructionPointer() const { 
#ifndef __acweaving
                return pc_reg.read(); 
#else
                return 0;
#endif
            }
            address_t getStackPointer() const { 
#ifndef __acweaving
                return sp_reg.read();
#else
                return 0;
#endif
            }

            // XXX: find a way to make this optional for architectures that have a external device for simtime
            simtime_t getInstructionCounter() const {
#ifndef __acweaving
                return instcount_reg.read();
#else
                return 0;
#endif
            }
            constexpr simtime_t getFrequency() const { return Freq; }

            unsigned int getId() const { return id; }

            // Serialize interfaces
            constexpr void serialize(std::ostream& os) const {
#ifndef __acweaving
                return serialize::to(os, regs);
#endif
            }
            constexpr void unserialize(std::istream& is) {
#ifndef __acweaving
                return serialize::from(is, regs);
#endif
            }

            // lifecycle
            void reset() {
#ifndef __acweaving
                pc_reg.write(Reset_vector);
#endif
            }
        };
}

#ifndef __acweaving
#define SAIL_DECLARE_CPU(RESET_VEC,...) \
    typedef SailCPU<RESET_VEC, 100000,  __VA_ARGS__> ConcreteCPU; \
    typedef SailCPU<RESET_VEC, 100000, __VA_ARGS__> Architecture
#else
#define SAIL_DECLARE_CPU(RESET_VEC,...) \
    typedef SailCPU<RESET_VEC, 100000> ConcreteCPU; \
    typedef SailCPU<RESET_VEC, 100000> Architecture
#endif


#endif
