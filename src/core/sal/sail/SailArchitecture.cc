#include "../Register.hpp"
#include "SailArchitecture.hpp"
#include "SailArchRegisters.hpp"
#include "SailArchRegistersExtern.hpp"

using namespace fail;

SailArchitecture::SailArchitecture() {
	Register *reg;
#define SAIL_REG_PTR(id, width, classes, variable)			\
	reg = new SailRegisterPtr<decltype(variable)>(RID_ ## id, width, &(variable)); \
	reg->setName(#id);									\
	m_addRegister(reg, classes);

#define SAIL_REG_FN(id, width, classes, fn_name) \
	reg = new SailRegisterFn<fn_name, fn_name>(RID_ ## id, width); \
	reg->setName(#id);													\
	m_addRegister(reg, classes);

#define SAIL_REG_CLASS(id, width, classes, CLS,variable)				\
	reg = new CLS(RID_ ## id, width, &(variable));	\
	reg->setName(#id);												\
	m_addRegister(reg, classes);

#define SAIL_REG_TMPL(id, width, classes, TMPL,variable)	\
	reg = new TMPL<decltype(variable)>(RID_ ## id, width, &(variable));	\
	reg->setName(#id);										\
	m_addRegister(reg, classes);

#include "SailArchRegisters.inc"

#undef SAIL_REG_PTR
#undef SAIL_REG_FN
#undef SAIL_REG_CLASS
#undef SAIL_REG_TMPL

}

SailArchitecture::~SailArchitecture() {
	for (auto reg : m_Registers) {
		if (reg) delete reg;
	}
	m_Registers.clear();
}

