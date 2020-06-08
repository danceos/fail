#ifndef __QEMU_X86_CPU_HPP__
#define __QEMU_X86_CPU_HPP__

#include "../x86/X86Architecture.hpp"
#include "../x86/X86CPUState.hpp"

namespace fail {

/**
 * \class QEMUX86CPU
 *
 * \c QEMUX86CPU is the concrete x86 CPU implementation for the QEMU processor
 * emulator. It implements the CPU interfaces \c X86Architecture and
 * \c X86CPUState.
 */
class QEMUX86CPU : public X86Architecture, public X86CPUState {
private:
	unsigned int m_Id; //!< the numeric CPU identifier (ID)
public:
	/**
	 * Initializes the CPU with the provided \c id.
	 * @param id the CPU identifier (the 1st CPU is CPU0 -> id = 0, and so forth)
	 */
	QEMUX86CPU(unsigned int id) : m_Id(id) { }
	/**
	 * Virtual Destructor is required.
	 */
	virtual ~QEMUX86CPU() { }
	/**
	 * Retrieves the content of the register \c reg.
	 * @param reg the register pointer of interest (cannot be \c NULL)
	 * @return the content of the register \c reg
	 */
	regdata_t getRegisterContent(const Register* reg) const;
	/**
	 * Sets the content of the register \c reg to \c value.
	 * @param reg the destination register object pointer (cannot be \c NULL)
	 * @param value the new value to assign
	 */
	void setRegisterContent(const Register* reg, regdata_t value);
	/**
	 * Returns the current instruction pointer (aka program counter).
	 * @return the current (e)ip register content
	 */
	address_t getInstructionPointer() const { return getRegisterContent(getRegister(RID_PC)); }
	/**
	 * Returns the current stack pointer.
	 * @return the current (e)sp register content
	 */
	address_t getStackPointer() const { return getRegisterContent(getRegister(RID_CSP)); }
	/**
	 * Returns the current base pointer.
	 * @return the current (e)bp register content
	 */
	address_t getBasePointer() const { return getRegisterContent(getRegister(RID_CBP)); }
	/**
	 * Returns the current (E)FLAGS.
	 * @return the current (E)FLAGS processor register content
	 */
	regdata_t getFlagsRegister() const { return getRegisterContent(getRegister(RID_FLAGS)); }
	/**
	 * Returns \c true if the corresponding flag is set, or \c false
	 * otherwise.
	 * TODO not implemented
	 */
	bool getCarryFlag() const             { return false; }
	bool getParityFlag() const            { return false; }
	bool getZeroFlag() const              { return false; }
	bool getSignFlag() const              { return false; }
	bool getOverflowFlag() const          { return false; }
	bool getTrapFlag() const              { return false; }
	bool getInterruptFlag() const         { return false; }
	bool getDirectionFlag() const         { return false; }
	unsigned getIOPrivilegeLevel() const  { return false; }
	bool getNestedTaskFlag() const        { return false; }
	bool getResumeFlag() const            { return false; }
	bool getVMFlag() const                { return false; }
	bool getAlignmentCheckFlag() const    { return false; }
	bool getVInterruptFlag() const        { return false; }
	bool getVInterruptPendingFlag() const { return false; }
	bool getIdentificationFlag() const    { return false; }
	/**
	 * Sets/resets various status FLAGS.
	 * TODO not implemented
	 */
	void setCarryFlag(bool bit)             { }
	void setParityFlag(bool bit)            { }
	void setZeroFlag(bool bit)              { }
	void setSignFlag(bool bit)              { }
	void setOverflowFlag(bool bit)          { }
	void setTrapFlag(bool bit)              { }
	void setInterruptFlag(bool bit)         { }
	void setDirectionFlag(bool bit)         { }
	void setIOPrivilegeLevel(unsigned lvl)  { }
	void setNestedTaskFlag(bool bit)        { }
	void setResumeFlag(bool bit)            { }
	void setVMFlag(bool bit)                { }
	void setAlignmentCheckFlag(bool bit)    { }
	void setVInterruptFlag(bool bit)        { }
	void setVInterruptPendingFlag(bool bit) { }
	void setIdentificationFlag(bool bit)    { }
	/**
	 * Returns the current id of this CPU.
	 * @return the current id
	 */
	unsigned int getId() const { return m_Id; }
};

typedef QEMUX86CPU ConcreteCPU; //!< the concrete QEMUX86CPU type

} // end-of-namespace: fail

#endif // __QEMU_X86_CPU_HPP__
