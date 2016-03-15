#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <ctime>
#include <string>
#include <cassert>
#include <vector>
#include <utility>
#include <iostream>

#include "SALConfig.hpp"
#include "ConcreteCPU.hpp"

namespace fail {

/**
 * \class BaseEvent
 * This is the base class for all event types.  It encapsulates the information
 * about an event reported by the simulator backend.
 */
class BaseEvent {
protected:
	ConcreteCPU* m_CPU; //!< the CPU object that triggered the event
public:
	BaseEvent(ConcreteCPU* cpu = NULL) : m_CPU(cpu) { }
	virtual ~BaseEvent() { }
	/**
	 * Returns a pointer to the CPU that triggered this event.
	 * @return triggering CPU
	 */
	ConcreteCPU* getTriggerCPU() const { return m_CPU; }
	/**
	 * Sets the pointer the CPU that triggered this event.
	 * @param cpu new CPU which caused this event
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_CPU = cpu; }
};
// ----------------------------------------------------------------------------
// Specialized events:
//

/**
 * \class BPEvent
 * A breakpoint, i.e. a specific instruction address, was reached.
 */
class BPEvent : public BaseEvent {
protected:
	address_t m_TriggerInstrPtr; //!< the address which triggered the event
	address_t m_AddressSpace; //!< the address space identifier
public:
	/**
	 * Creates a new breakpoint event. The range information is specific to
	 * the subclasses.
	 * @param trigger the triggering address of the breakpoint event
	 * @param address_space the address space identifier for this event
	 * @param cpu the FAIL* CPU object that triggered the breakpoint
	 */
	BPEvent(address_t trigger, address_t address_space, ConcreteCPU* cpu = NULL)
	: BaseEvent(cpu), m_TriggerInstrPtr(trigger), m_AddressSpace(address_space) { }
	/**
	 * Returns the instruction pointer that triggered this event.
	 * @return triggering IP
	 */
	address_t getTriggerInstructionPointer() const { return m_TriggerInstrPtr; }
	/**
	 * Sets the instruction pointer that triggered this event.
	 * @param iptr new IP which caused this event
	 */
	void setTriggerInstructionPointer(address_t iptr) { m_TriggerInstrPtr = iptr; }
	/**
	 * Returns the address space register of this event.
	 */
	address_t getAddressSpace() const { return m_AddressSpace; }
	/**
	 * Sets the address space register for this event.
	 */
	void setAddressSpace(address_t iptr) { m_AddressSpace = iptr; }
};

/**
 * \class MemAccessEvent
 * A read/write memory access to a physical address with a specific width was
 * observed.
 */
class MemAccessEvent : public BaseEvent {
public:
	enum access_type_t {
		MEM_UNKNOWN   = 0x0, //!< internal initialization flag, indicating an uninitialized state
		MEM_READ      = 0x1, //!< read access flag
		MEM_WRITE     = 0x2, //!< write access flag
		MEM_READWRITE = 0x3  //!< read and write access flag
	};
private:
	//! Specific physical guest system *memory* address that actually triggered the event.
	address_t m_TriggerAddr;
	//! Width of the memory access (# bytes).
	size_t m_TriggerWidth;
	//! Address of the \b instruction that caused the memory access.
	address_t m_TriggerIP;
	//! Memory access type at m_TriggerAddr.
	access_type_t m_AccessType;
public:
	/**
	 * Creates a new \c MemAccessEvent using default initialization values, i.e.
	 * \c setTriggerAddress(ANY_ADDR), \c setTriggerWidth(0), \c setTriggerAccessType(MEM_UNKNOWN),
	 * \c setTriggerInstructionPointer(ANY_ADDR) and setTriggerCPU(NULL).
	 */
	MemAccessEvent()
		: m_TriggerAddr(ANY_ADDR), m_TriggerWidth(0),
		  m_TriggerIP(ANY_ADDR), m_AccessType(MEM_UNKNOWN) { }
	/**
	 * Creates a new \c MemAccessEvent and initializes the provided values.
	 * @param triggerAddr actual address that triggered the event
	 * @param width width of memory access (= # Bytes)
	 * @param triggerIP the instruction pointer that actually triggered the memory access
	 * @param type the type of memory access (r, w, rw)
	 * @param cpu the CPU that triggered the event
	 */
	MemAccessEvent(address_t triggerAddr, size_t width, address_t triggerIP, access_type_t type,
				   ConcreteCPU* cpu = NULL)
		: BaseEvent(cpu), m_TriggerAddr(triggerAddr), m_TriggerWidth(width),
		  m_TriggerIP(triggerIP), m_AccessType(type) { }
	/**
	 * Returns the specific memory address that actually triggered the event.
	 * @return the triggering address
	 */
	address_t getTriggerAddress() const { return m_TriggerAddr; }
	/**
	 * Sets the specific memory address that actually triggered the event.
	 * Should not be used by experiment code.
	 * @param addr the new triggering address
	 */
	void setTriggerAddress(address_t addr) { m_TriggerAddr = addr; }
	/**
	 * Returns the specific number of bytes read or written at \c getTriggerAddress().
	 * @return the width of the memory access
	 */
	size_t getTriggerWidth() const { return m_TriggerWidth; }
	/**
	 * Sets the specific memory access width.
	 * @param new width (number of bytes)
	 */
	void setTriggerWidth(size_t width) { m_TriggerWidth = width; }
	/**
	 * Returns the address of the instruction causing this memory access.
	 * @return triggering IP
	 */
	address_t getTriggerInstructionPointer() const { return m_TriggerIP; }
	/**
	 * Sets the address of the instruction causing this memory access.
	 * @param addr new triggering IP
	 */
	void setTriggerInstructionPointer(address_t addr) { m_TriggerIP = addr; }
	/**
	 * Returns type (MEM_READ || MEM_WRITE) of the memory access that
	 * triggered this event.
	 * @return the type of memory access (r or w) of this event
	 */
	access_type_t getTriggerAccessType() const { return m_AccessType; }
	/**
	 * Sets type of the memory access that triggered this event.
	 * @param type type of memory access
	 */
	void setTriggerAccessType(access_type_t type) { m_AccessType = type; }
};

/**
 * \class TroubleEvent
 * An interrupt or trap was observed.
 * FIXME: Naming.  Interrupts are not exactly "trouble".
 */
class TroubleEvent : public BaseEvent {
private:
	/**
	 * Specific guest system interrupt/trap number that actually
	 * trigger the event.
	 */
	int m_TriggerNumber;
public:
	/**
	 * Constructs a default initialized \c TroubleEvent, setting the trigger-number
	 * to -1 and the trigger-CPU to NULL.
	 */
	TroubleEvent() : m_TriggerNumber(-1) { }
	/**
	 * Constructs a new \c TroubleEvent.
	 * @param triggerNum system and type specific number identifying the requestet
	 *        "trouble-type"
	 * @param cpu the CPU that triggered the event
	 */
	TroubleEvent(int triggerNum, ConcreteCPU* cpu = NULL)
		: BaseEvent(cpu), m_TriggerNumber(triggerNum) { }
	/**
	 * Sets the specific interrupt-/trap-number that actually triggered the event.
	 * @param triggerNum system and type specific number identifying the requested
	 *        "trouble-type"
	 */
	void setTriggerNumber(unsigned triggerNum) { m_TriggerNumber = triggerNum; }
	/**
	 * Returns the specific interrupt-/trap-number that actually triggered the event.
	 * @return the trigger number
	 */
	unsigned getTriggerNumber() const { return m_TriggerNumber; }
};

/**
 * \class InterruptEvent
 * An interrupt was observed.
 */
class InterruptEvent : public TroubleEvent {
private:
	bool m_IsNMI; //!< non maskable interrupt flag
public:
	/**
	 * Constructs a default initialized \c InterruptEvent, setting the non maskable
	 * interrupt flag to \c false and the CPU to NULL.
	 */
	InterruptEvent() : m_IsNMI(false) { }
	/**
	 * Creates a new \c InterruptEvent.
	 * @param nmi the new NMI (non maskable interrupt) flag state
	 * @param triggerNum system and type specific number identifying the requestet
	 *        "trouble-type"
	 * @param cpu the CPU that triggered the event
	 */
	InterruptEvent(bool nmi, int triggerNum, ConcreteCPU* cpu = NULL)
		: TroubleEvent(triggerNum, cpu), m_IsNMI(nmi) { }
	/**
	 * Returns \c true if the interrupt is non maskable, \c false otherwise.
	 * @return \c true if NMI flag is set, \c false otherwise
	 */
	bool isNMI() const { return m_IsNMI; }
	/**
	 * Sets the interrupt type (non maskable or not).
	 * @param nmi the new NMI (non maskable interrupt) flag state
	 */
	void setNMI(bool enabled) { m_IsNMI = enabled; }
};

/**
 * \class GuestEvent
 * The guest system emitted explicit guest->experiment communication.
 */
// FIXME: cf. GuestListener
class GuestEvent : public BaseEvent {
private:
	char m_Data; //!< guest event data
	unsigned m_Port; //!< communication port
public:
	GuestEvent() : m_Data(0), m_Port(0) { }
	/**
	 * Returns the data, transmitted by the guest system.
	 */
	char getData() const { return m_Data; }
	/**
	 * Sets the data which had been transmitted by the guest system.
	 */
	void setData(char data) { m_Data = data; }
	/**
	 * Returns the data length, transmitted by the guest system.
	 */
	unsigned getPort() const { return m_Port; }
	/**
	 * Sets the data length which had been transmitted by the guest system.
	 */
	void setPort(unsigned port) { m_Port = port; }
};

/**
 * \class IOPortEvent
 * Observes I/O access on architectures with a separate I/O access mechanism (e.g. IA-32)
 */
class IOPortEvent : public BaseEvent {
private:
	unsigned char m_Data;
public:
	/**
	 * Initialises an IOPortEvent
	 * @param data the data which has been communicated through the I/O port
	 * @param cpu the CPU that triggered the event
	 */
	IOPortEvent(unsigned char data = 0, ConcreteCPU* cpu = NULL) : BaseEvent(cpu), m_Data(data) { }
	/**
	 * Returns the data sent to the specified port
	 */
	unsigned char getData() const { return m_Data; }
	/**
	 * Sets the data which had been transmitted.
	 */
	void setData(unsigned char data) { m_Data = data; }
};

/**
 * \class JumpEvent
 * A conditional jump instruction is about to execute.
 */
class JumpEvent : public BaseEvent {
private:
	unsigned m_OpcodeTrigger;
	bool m_FlagTriggered;
public:
	/**
	 * Constructs a new event object.
	 * @param opcode the opcode of the jump-instruction to be observed
	 *        or ANY_INSTR to match all jump-instructions
	 * @param flagreg \c true if the event has been triggered due to a
	 *        specific FLAG register content, \c false otherwise
	 * @param cpu the CPU that triggered the event
	 */
	JumpEvent(unsigned opcode = ANY_INSTR, bool flagreg = false, ConcreteCPU* cpu = NULL)
		: BaseEvent(cpu), m_OpcodeTrigger(opcode), m_FlagTriggered(flagreg) { }
	/**
	 * Retrieves the opcode of the jump-instruction.
	 */
	unsigned getTriggerOpcode() const { return m_OpcodeTrigger; }
	/**
	 * Returns \c true, if the event was triggered due to specific register
	 * content, \c false otherwise.
	 */
	bool isRegisterTriggered() const { return !m_FlagTriggered; }
	/**
	 * Returns \c true, of the event was triggered due to specific FLAG
	 * state, \c false otherwise. This is the common case.
	 */
	bool isFlagTriggered() const { return m_FlagTriggered; }
	/**
	 * Sets the requestet jump-instruction opcode.
	 */
	void setTriggerOpcode(unsigned oc) { m_OpcodeTrigger = oc; }
	/**
	 * Sets the trigger flag.
	 */
	void setFlagTriggered(bool flagTriggered) { m_FlagTriggered = flagTriggered; }
};

/**
 * \class TimerEvent
 * This event type is used to encapsulate timeout-specific data.
 */
class TimerEvent : public BaseEvent {
protected:
	timer_id_t m_Id; //!< internal timer id (sim-specific)
public:
	/**
	 * Creates a new timer event.
	 */
	TimerEvent(timer_id_t id = INVALID_TIMER) : m_Id(id) { }
	~TimerEvent() { }
	/**
	 * Retrieves the internal timer id. Maybe useful for debug output.
	 * @return the timer id or \c INVALID_TIMER if the timer is invalid
	 */
	timer_id_t getId() const { return m_Id; }
	/**
	 * Sets the internal timer id. This should not be used by the experiment.
	 * @param id the new timer id, given by the underlying simulator-backend
	 */
	void setId(timer_id_t id) { m_Id = id; }
};

} // end-of-namespace: fail

#endif // __EVENT_HPP__
