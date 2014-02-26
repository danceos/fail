#ifndef __LLVMTOFAILBOCHS_HPP_
#define __LLVMTOFAILBOCHS_HPP_

#include "LLVMtoFailTranslator.hpp"
#include <stdlib.h>
#include <iostream>

namespace fail {

class LLVMDisassembler;

class LLVMtoFailBochs : public LLVMtoFailTranslator {

public:

	LLVMtoFailBochs(LLVMDisassembler *disas);

};
} // end of namespace

#endif
