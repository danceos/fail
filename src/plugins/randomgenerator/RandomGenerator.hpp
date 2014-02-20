#ifndef __RandomGenerator_HPP__
#define __RandomGenerator_HPP__

#include <unistd.h>
#include "efw/ExperimentFlow.hpp"
#include "config/FailConfig.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include <math.h>

/**
 * @file
 * @brief A deterministic pseudo-random number generator
 */

/**
 * @class RandomGenerator
 * @brief Plugin to provide deterministic pseudo-random numbers on a specific memory location
 */
class RandomGenerator : public fail::ExperimentFlow
{
private:
	const fail::ElfSymbol m_symbol; //!< the target's memory symbol the plugin is listening on to generate value
	fail::Logger m_log; //!< debug output
	const unsigned m_seed; //!< PRNG seed value
public:
	/**
	 * Constructor
	 *
	 * @param symbol The resulting random is placed in the SUT symbol
	 * @param seed seed value for PRNG
	 */
	RandomGenerator( const fail::ElfSymbol & symbol, unsigned seed ) :   m_symbol(symbol), m_log("RandGen", false), m_seed(seed){}

	bool run();
};

#endif // __RandomGenerator_HPP__
