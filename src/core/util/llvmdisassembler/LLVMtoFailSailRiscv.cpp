#include "LLVMtoFailSailRiscv.hpp"
#include "LLVMDisassembler.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

using namespace fail;

LLVMtoFailSailRiscv::LLVMtoFailSailRiscv(LLVMDisassembler *disas) {
    std::map<std::string, struct reginfo_t> m;

    std::string features = disas->GetSubtargetFeatures();

    std::cout << "features: " << features << std::endl;
    bool is_cheri = features.find("+xcheri") != std::string::npos;
    bool is_cheri_purecap = features.find("+cap-mode") != std::string::npos;

    std::cout << std::boolalpha << " is_cheri: " << is_cheri << " is_purecap: " << is_cheri_purecap << std::endl;

    namespace ids = sail_arch::registers;
    // generated from gp_regs.h with vim macro
    // s/GP(\([^,]\+\),[^,]\+,\s\+z\([^\)]\+\));/m["\1"] = reginfo_t(ids::\1);\rm["\2"] = reginfo_t(ids::\1);/gc
    m["pc"] = reginfo_t(ids::pc);

    m["ra"] = reginfo_t(ids::ra);
    m["x1"] = reginfo_t(ids::ra);
    m["sp"] = reginfo_t(ids::sp);
    m["x2"] = reginfo_t(ids::sp);
    m["gp"] = reginfo_t(ids::gp);
    m["x3"] = reginfo_t(ids::gp );
    m["tp"] = reginfo_t(ids::tp);
    m["x4"] = reginfo_t(ids::tp);
    m["t0"] = reginfo_t(ids::t0);
    m["x5"] = reginfo_t(ids::t0);
    m["t1"] = reginfo_t(ids::t1);
    m["x6"] = reginfo_t(ids::t1);
    m["t2"] = reginfo_t(ids::t2);
    m["x7"] = reginfo_t(ids::t2);
    m["s0"] = reginfo_t(ids::s0);
    m["x8"] = reginfo_t(ids::s0);
    m["s1"] = reginfo_t(ids::s1);
    m["x9"] = reginfo_t(ids::s1);
    m["a0"] = reginfo_t(ids::a0);
    m["x10"] = reginfo_t(ids::a0);
    m["a1"] = reginfo_t(ids::a1);
    m["x11"] = reginfo_t(ids::a1);
    m["a2"] = reginfo_t(ids::a2);
    m["x12"] = reginfo_t(ids::a2);
    m["a3"] = reginfo_t(ids::a3);
    m["x13"] = reginfo_t(ids::a3);
    m["a4"] = reginfo_t(ids::a4);
    m["x14"] = reginfo_t(ids::a4);
    m["a5"] = reginfo_t(ids::a5);
    m["x15"] = reginfo_t(ids::a5);
    m["a6"] = reginfo_t(ids::a6);
    m["x16"] = reginfo_t(ids::a6);
    m["a7"] = reginfo_t(ids::a7);
    m["x17"] = reginfo_t(ids::a7);
    m["s2"] = reginfo_t(ids::s2);
    m["x18"] = reginfo_t(ids::s2);
    m["s3"] = reginfo_t(ids::s3);
    m["x19"] = reginfo_t(ids::s3);
    m["s4"] = reginfo_t(ids::s4);
    m["x20"] = reginfo_t(ids::s4);
    m["s5"] = reginfo_t(ids::s5);
    m["x21"] = reginfo_t(ids::s5);
    m["s6"] = reginfo_t(ids::s6);
    m["x22"] = reginfo_t(ids::s6);
    m["s7"] = reginfo_t(ids::s7);
    m["x23"] = reginfo_t(ids::s7);
    m["s8"] = reginfo_t(ids::s8);
    m["x24"] = reginfo_t(ids::s8);
    m["s9"] = reginfo_t(ids::s9);
    m["x25"] = reginfo_t(ids::s9);
    m["s10"] = reginfo_t(ids::s10);
    m["x26"] = reginfo_t(ids::s10);
    m["s11"] = reginfo_t(ids::s11);
    m["x27"] = reginfo_t(ids::s11);
    m["t3"] = reginfo_t(ids::t3);
    m["x28"] = reginfo_t(ids::t3);
    m["t4"] = reginfo_t(ids::t4);
    m["x29"] = reginfo_t(ids::t4);
    m["t5"] = reginfo_t(ids::t5);
    m["x30"] = reginfo_t(ids::t5);
    m["t6"] = reginfo_t(ids::t6);
    m["x31"] = reginfo_t(ids::t6);

    if(is_cheri) {
#ifdef BUILD_RISCV_CHERI
        m["ddc"] = reginfo_t(ids::ddc);
        if(is_cheri_purecap) {
            m["c1"] = reginfo_t(ids::ra);
            m["c2"] = reginfo_t(ids::sp);
            m["c3"] = reginfo_t(ids::gp);
            m["c4"] = reginfo_t(ids::tp);
            m["c5"] = reginfo_t(ids::t0);
            m["c6"] = reginfo_t(ids::t1);
            m["c7"] = reginfo_t(ids::t2);
            m["c8"] = reginfo_t(ids::s0);
            m["c9"] = reginfo_t(ids::s1);
            m["c10"] = reginfo_t(ids::a0);
            m["c11"] = reginfo_t(ids::a1);
            m["c12"] = reginfo_t(ids::a2);
            m["c13"] = reginfo_t(ids::a3);
            m["c14"] = reginfo_t(ids::a4);
            m["c15"] = reginfo_t(ids::a5);
            m["c16"] = reginfo_t(ids::a6);
            m["c17"] = reginfo_t(ids::a7);
            m["c18"] = reginfo_t(ids::s2);
            m["c19"] = reginfo_t(ids::s3);
            m["c20"] = reginfo_t(ids::s4);
            m["c21"] = reginfo_t(ids::s5);
            m["c22"] = reginfo_t(ids::s6);
            m["c23"] = reginfo_t(ids::s7);
            m["c24"] = reginfo_t(ids::s8);
            m["c25"] = reginfo_t(ids::s9);
            m["c26"] = reginfo_t(ids::s10);
            m["c27"] = reginfo_t(ids::s11);
            m["c28"] = reginfo_t(ids::t3);
            m["c29"] = reginfo_t(ids::t4);
            m["c30"] = reginfo_t(ids::t5);
            m["c31"] = reginfo_t(ids::t6);
        }
        else {
            // FIXME: what is actually needed here?
            // TODO: explore how hybrid binary look
            throw std::runtime_error("non purecap code is currently not supported.");
        }
#endif
    }

    const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
    std::vector<std::string> failed_translations;
    for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i) {
        std::string name = reg_info.getName(i);
        std::transform(name.begin(),name.end(),name.begin(),[] (auto c) { return std::tolower(c); });
        if (m.count(name) > 0) {
            llvm_to_fail_map[i] = m[name];
        } else {
            failed_translations.push_back(name);
        }
    }
    std::cout << "failed to find translations for: ";
    std::string out;
    for(const auto& n: failed_translations)
        out += "," + n;
    std::cout << out << std::endl;
}
