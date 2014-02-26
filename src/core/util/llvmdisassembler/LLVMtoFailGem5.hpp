#ifndef __LLVMTOFAILGEM5_HPP_
#define __LLVMTOFAILGEM5_HPP_

#include "LLVMtoFailTranslator.hpp"
#include <stdlib.h>
#include <iostream>

namespace fail {

class LLVMDisassembler;

class LLVMtoFailGem5 : public LLVMtoFailTranslator {

public:

	LLVMtoFailGem5(LLVMDisassembler *disas);

};
} // end of namespace

#endif
