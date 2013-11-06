#ifndef __L4SYS_INSTRUCTIONFILTER_HPP__
  #define __L4SYS_INSTRUCTIONFILTER_HPP__

#include "sal/SALConfig.hpp"
#include "cpu/instr.h"
#include <vector>

using namespace fail;

/**
 * \class InstructionFilter
 *
 * \brief Filters instructions
 *
 * This class is an interface that can be used to
 * implement instruction filter classes that can
 * be used to decide if an instruction should be
 * included in the fault injection experiment.
 */
class InstructionFilter {
public:
	/**
	 * Decides if the given instruction at the given program location
	 * is valid for fault injection
	 * @param ip the instruction pointer of the instruction
	 * @param instr the instruction in its coded binary representation
	 * @returns \c true if the instruction should be included, \c false otherwise
	 */
	virtual bool isValidInstr(address_t ip, char const *instr) const = 0;
	virtual ~InstructionFilter() {}
};

/**
 * \class RangeInstructionFilter
 *
 * Permits an instruction if its instruction pointer lies within a certain range
 */
class RangeInstructionFilter : public InstructionFilter {
public:
	RangeInstructionFilter(address_t begin_addr, address_t end_addr)
	: beginAddress(begin_addr), endAddress(end_addr)
	{}
	~RangeInstructionFilter() {}
	/**
	 * check if the instruction pointer of an instruction lies within a certain range
	 * @param ip the instruction pointer to check
	 * @param instr this parameter is ignored
	 * @returns \c true if the instruction lies within the predefined range,
	 *          \c false otherwise
	 */
	bool isValidInstr(address_t ip, char const *instr = 0) const
	{ return (beginAddress <= ip && ip <= endAddress); }
private:
	address_t beginAddress; //<! the beginning of the address range
	address_t endAddress; //<! the end of the address range
};


/**
 * \class RangeSetInstructionFilter
 * 
 * Collects a list of filter ranges from an input file.
 */
class RangeSetInstructionFilter : public InstructionFilter {
private:
	std::vector<RangeInstructionFilter> _filters;

public:
	RangeSetInstructionFilter(char const *filename);
	~RangeSetInstructionFilter() {}

	bool isValidInstr(address_t ip, char const *instr = 0) const
	{
		std::vector<RangeInstructionFilter>::const_iterator it;
		for (it = _filters.begin(); it != _filters.end(); ++it) {
			if (it->isValidInstr(ip))
				return true;
		}
		return false;
	}
	
	void addFilter(address_t startAddr, address_t endAddr)
	{
		_filters.push_back(RangeInstructionFilter(startAddr, endAddr));
	}
};

#endif /* __L4SYS_INSTRUCTIONFILTER_HPP__ */
