#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <cstring>
#include <vector>

#include "Register.hpp"

namespace fail {

// TODO: Interrupt information?

/**
 * \class CPUArchitecture
 * This is the base class for CPU architectures that can be used to merge information and
 * functionality that every backend with the same target architecture will share.  The classes
 * directly derived from this are especially meant to be usable in campaigns, so they shouldn't
 * contain any backend specific code.
 */
class CPUArchitecture {
public:
	/**
	 * Iterator type to be used when iterating over the total register set.
	 */
	typedef std::vector<Register*>::iterator iterator;
	/**
	 * Returns an iterator pointing to the first register.
	 * @return the first register
	 */
	iterator begin() { return m_Registers.begin(); }
	/**
	 * Returns an iterator pointing behind the last register.
	 * @return an iterator past the last register
	 */
	iterator end() { return m_Registers.end(); }
	/**
	 * Retrieves the total number of registers over all homogeneous sets.
	 * @return the total register count
	 */
	size_t registerCount() const { return m_Registers.size(); }
	/**
	 * Retrieves the number of managed homogeneous register sets.
	 * @return the number of sets
	 */
	size_t registerSubsetCount() const { return m_RegisterSubsets.size(); }
	/**
	 * Retrieves the \a i-th register.
	 * @return a pointer to the \a i-th register; if \a i is invalid, an
	 *         assertion is thrown
	 */
	Register* getRegister(size_t i) const;
	/**
	 * Gets the \a i-th register set.
	 * @param i the index of the set to be returned
	 * @return a reference to the uniform register set
	 * @see registerSubsetCount()
	 */
	UniformRegisterSet& getRegisterSet(size_t i) const;
	/**
	 * Returns the set with register type \a t. The set can be used to
	 * loop over all registers of type \a t.
	 * @param t the type to check for
	 * @return a pointer to the retrieved register set (if found), \c NULL otherwise
	 */
	UniformRegisterSet* getRegisterSetOfType(RegisterType t) const;
protected:
	std::vector<Register*> m_Registers; //!< the total (!) register set
	/// a set of register subsets (each set has its own type)
	std::vector<UniformRegisterSet*> m_RegisterSubsets;
	/**
	 * Adds a new register to this set. The register object needs to be
	 * typed (see Register::getType).
	 * @param reg a pointer to the register object to be added
	 * @see getType()
	 */
	void m_addRegister(Register* reg, RegisterType type = RT_NONE);
};

} // end-of-namespace: fail

#endif // __CPU_HPP__
