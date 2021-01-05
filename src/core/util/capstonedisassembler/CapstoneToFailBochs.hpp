#ifndef __CAPSTONETOFAILBOCHS_HPP_
#define __CAPSTONETOFAILBOCHS_HPP_

#include "CapstoneToFailTranslator.hpp"

namespace fail {

class CapstoneToFailBochs : public CapstoneToFailTranslator {
public:
	CapstoneToFailBochs(CapstoneDisassembler *disas);
};
} // end of namespace

#endif
