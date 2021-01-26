#include <iostream>
#include "../../SALConfig.hpp"
#include "../SailArchitecture.hpp"
#include "SailArchRegisters.hpp"
#include "SailArchRegistersExtern.hpp"

using namespace fail;

regdata_t fail::minstret_written() {
	return static_cast<regdata_t>(zminstret_written);
}

void fail::minstret_written(regdata_t value) {
	zminstret_written = static_cast<bool>(value);
}

regdata_t fail::parity_mismatch() {
	if (&zparity_mismatch)
		return static_cast<regdata_t>(zparity_mismatch);
	return 0;
}

void fail::parity_mismatch(regdata_t value) {
	if (&zparity_mismatch)
		zparity_mismatch = static_cast<bool>(value);
}


simtime_t SailArchitecture::getInstructionCounter() const {
	return  static_cast<regdata_t>(zminstret);
}
