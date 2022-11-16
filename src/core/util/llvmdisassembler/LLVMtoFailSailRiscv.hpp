#ifndef LLVMTOFAILSAILRISCV_HPP
#define LLVMTOFAILSAILRISCV_HPP

#include "LLVMtoFailTranslator.hpp"

namespace fail {

class LLVMDisassembler;

class LLVMtoFailSailRiscv : public LLVMtoFailTranslator {
public:
	LLVMtoFailSailRiscv(LLVMDisassembler *disas);
};
} // end of namespace

#endif /* LLVMTOFAILSAILRISCV_HPP */
