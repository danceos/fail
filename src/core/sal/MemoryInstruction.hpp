#ifndef __MEMORYINSTRUCTION_HPP__
#define __MEMORYINSTRUCTION_HPP__

#include <string>
#include "SALConfig.hpp"

namespace fail {

class MemoryInstruction {
	regdata_t address;
	regdata_t value;
	uint8_t width;
	bool writeAccess;
public:
	MemoryInstruction(regdata_t address = ADDR_INV, regdata_t value = 0, uint8_t width = 0, bool writeAccess = false) :
		address(address), value(value), width(width), writeAccess(writeAccess) { };

	bool isWriteAccess(void) const { return writeAccess; }
	regdata_t getAddress() const { return address; }
	regdata_t getValue() const { return value; }
	uint8_t getWidth() const { return width; }

	bool isValid(void) const { return address != ADDR_INV; }

	void setAddress(regdata_t addr) { address = addr; }
	void setValue(regdata_t val) { value = val; }
	void setWidth(uint8_t w) { width = w; }
	void setWriteAccess(bool iswrite) { writeAccess = iswrite; }
};

class MemoryInstructionAnalyzer {
public:
	virtual bool eval(address_t opcode, MemoryInstruction& result) = 0;
};

extern MemoryInstructionAnalyzer& meminstruction;
} // end of namespace fail

#endif // __MEMORYINSTRUCTION_HPP__

