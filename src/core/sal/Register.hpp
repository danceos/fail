#ifndef __REGISTER_HPP__
  #define __REGISTER_HPP__

#include <vector>
#include <cstdlib>
#include <cassert>
#include <string>
#include <stdint.h>

#include "SALConfig.hpp"

namespace fail {

/**
 * \enum RegisterType
 *
 * Lists the different register types. You need to expand this enumeration
 * to provide more detailed types for your concrete derived register classes
 * specific to a simulator/architecture.
 */
enum RegisterType {
	RT_GP, //!< general purpose
	RT_PC, //!< program counter / instruction pointer
	RT_ST  //!< status register
};

/**
 * \class Register
 *
 * Represents the basic generic register class. A set of registers is composed
 * of classes which had been derived from this class.
 */
class Register {
protected:
	RegisterType m_Type; //!< the type of this register
	regwidth_t m_Width; //!< the register width
	unsigned int m_Id; //!< the unique id of this register
	//! \c true if register has already been assigned, \c false otherwise
	bool m_Assigned;
	//! The index in it's register set if assigned (\c -1 otherwise)
	size_t m_Index;
	std::string m_Name; //!< The (optional) name, maybe empty
	friend class UniformRegisterSet;
public:
	/**
	 * Creates a new register.
	 * @param id the unique id of this register (simulator specific)
	 * @param t the type of the register to be constructed
	 * @param w the width of the register in bits
	 */
	Register(unsigned int id, RegisterType t, regwidth_t w)
		: m_Type(t), m_Width(w), m_Id(id), m_Assigned(false),
		  m_Index(static_cast<size_t>(-1)) { }
	/**
	 * Returns the (fixed) type of this register.
	 * @return the type of this register
	 */
	RegisterType getType() const { return (m_Type); }
	/**
	 * Returns the (fixed) width of this register.
	 * @return the width in bits
	 */
	regwidth_t getWidth() const { return (m_Width); }
	/**
	 * Returns the data referenced by this register. In a concrete
	 * derived class this method has to be defined appropriately.
	 * @return the current data
	 */
	virtual regdata_t getData() /*!const*/ = 0;
	/**
	 * Sets new data to be stored in this register.
	 * @param data the data to be written to the register
	 */
	virtual void setData(regdata_t data) = 0;
	/**
	 * Sets the (optional) name of this register.
	 * @param name the textual register name, e.g. "EAX"
	 */
	void setName(const std::string& name) { m_Name = name; }
	/**
	 * Retrieves the register name.
	 * @return the textual register description
	 */
	const std::string& getName() const { return (m_Name); }
	/**
	 * Retrieves the unique index within it's assigned register set.
	 * If the register has not been assigned, \c (size_t)-1 will be
	 * returned.
	 * @return the register index or -1 if not assigned
	 * @see isAssigned()
	 */
	size_t getIndex() const { return (m_Index); }
	/**
	 * Checks whether this register has already been assigned. On
	 * creation the register isn't initially assigned.
	 * @return \c true if assigned, \c false otherwise
	 */
	bool isAssigned() const { return (m_Assigned); }
	/**
	 * Returns the unique id of this register.
	 * @return the unique id
	 */
	unsigned int getId() const { return (m_Id); }
};

/**
 * \class UniformRegisterSet
 *
 * Represents a (type-uniform) set of registers, e.g. all general purpose
 * registers. The granularity of the register type is determined by the
 * enumeration \c RegisterType. (All registers within this set must be of the
 * same register type.) The capacity of the set is managed automatically.
 */
class UniformRegisterSet {
private:
	std::vector< Register* > m_Regs; //!< the unique set of registers
	RegisterType m_Type; //!< the overall type of this container (set)
	void m_add(Register* preg);
	friend class RegisterManager;
public:
	/**
	 * The iterator of this class used to loop through the list of
	 * added registers. To retrieve an iterator to the first element, call
	 * begin(). end() returns the iterator, pointing after the last element.
	 * (This behaviour equals the STL iterator in C++.)
	 */
	typedef std::vector< Register* >::iterator iterator;
	/**
	 * Returns an iterator to the beginning of the internal data structure.
	 * \code
	 * [1|2| ... |n]
	 *  ^
	 * \endcode
	 */
	iterator begin() { return (m_Regs.begin()); }
	/**
	 * Returns an iterator to the end of the interal data structure.
	 * \code
	 * [1|2| ... |n]X
	 *              ^
	 * \endcode
	 */
	iterator end() { return (m_Regs.end()); }
	/**
	 * Constructs a new register set with type \a containerType.
	 * @param containerType the type of registers which should be stored
	 *        in this set
	 */
	UniformRegisterSet(RegisterType containerType)
		: m_Type(containerType) { }
	/**
	 * Returns the type of this set.
	 * @return the type
	 */
	RegisterType getType() const { return (m_Type); }
	/**
	 * Gets the number of registers of this set.
	 * @return the number of registers
	 */
	size_t count() const { return (m_Regs.size()); }
	/**
	 * Retrieves the \a i-th register within this set.
	 * @return a pointer to the \a i-th register; if \a i is invalid, an
	 *         assertion is thrown
	 */
	Register* getRegister(size_t i);
	/**
	 * Retrieves the first register within this set (syntactical sugar).
	 * @return a pointer to the first register (if existing -- otherwise an
	 *         assertion is thrown)
	 */
	virtual Register* first() { return (getRegister(0)); }
};

/**
 * \class RegisterManager
 * Represents a complete set of (inhomogeneous) registers specific to a concrete
 * architecture, e.g. x86 or ARM.
 */
class RegisterManager {
protected:
	std::vector< Register* > m_Registers;
	//!< the managed set of homogeneous sets
	std::vector< UniformRegisterSet* > m_Subsets;
public:
	/**
	 * The iterator of this class used to loop through the list of
	 * added registers. To retrieve an iterator to the first element, call
	 * begin(). end() returns the iterator, pointing after the last element.
	 * (This behaviour equals the STL iterator in C++.)
	 */
	typedef std::vector< Register* >::iterator iterator;
	/**
	 * Returns an iterator to the beginning of the internal data structure.
	 * \code
	 * [1|2| ... |n]
	 *  ^
	 * \endcode
	 */
	iterator begin() { return (m_Registers.begin()); }
	/**
	 * Returns an iterator to the end of the interal data structure.
	 * \code
	 * [1|2| ... |n]X
	 *              ^
	 * \endcode
	 */
	iterator end() { return (m_Registers.end()); }

	RegisterManager() { }
	~RegisterManager() { clear(); }
	/**
	 * Retrieves the total number of registers over all homogeneous sets.
	 * @return the total register count
	 */
	virtual size_t count() const;
	/**
	 * Retrieves the number of managed homogeneous register sets.
	 * @return the number of sets
	 */
	virtual size_t subsetCount() const { return (m_Subsets.size()); }
	/**
	 * Gets the \a i-th register set.
	 * @param i the index of the set to be returned
	 * @return a reference to the uniform register set
	 * @see subsetCount()
	 */
	virtual UniformRegisterSet& getSet(size_t i);
	/**
	 * Returns the set with register type \a t. The set can be used to
	 * loop over all registers of type \a t.
	 * @param t the type to check for
	 * @return a pointer to the retrieved register set (if found), NULL
	 *         otherwise
	 */
	virtual UniformRegisterSet* getSetOfType(RegisterType t);
	/**
	 * Adds a new register to this set. The register object needs to be
	 * typed (see Register::getType).
	 * @param reg a pointer to the register object to be added
	 * @see getType()
	 */
	void add(Register* reg);
	/**
	 * Retrieves the \a i-th register.
	 * @return a pointer to the \a i-th register; if \a i is invalid, an
	 *         assertion is thrown
	 */
	Register* getRegister(size_t i);
	/**
	 * Removes all registers and sets from the RegisterManager.
	 */
	virtual void clear();
	/**
	 * Returns the current instruction pointer.
	 * @return the current eip
	 */
	virtual address_t getInstructionPointer() = 0;
	/**
	 * Retruns the top address of the stack.
	 * @return the starting address of the stack
	 */
	virtual address_t getStackPointer() = 0;
	/**
	 * Retrieves the base ptr (holding the address of the
	 * current stack frame).
	 * @return the base pointer
	 */
	virtual address_t getBasePointer() = 0;
};

} // end-of-namespace: fail

#endif // __REGISTER_HPP__
