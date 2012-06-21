#ifndef __EVENT_HPP__
  #define __EVENT_HPP__

#include <ctime>
#include <string>
#include <cassert>
#include <vector>
#include <utility>
#include <iostream>

#include "SALConfig.hpp"

namespace fail {

class ExperimentFlow;

typedef unsigned long event_id_t; //!< type of event ids

//! invalid event id (used as a return indicator)
const event_id_t    INVALID_EVENT = static_cast<event_id_t>(-1);
//! address wildcard (e.g. for BPEvent's)
const address_t       ANY_ADDR = static_cast<address_t>(-1);
//! instruction wildcard
const unsigned       ANY_INSTR = static_cast<unsigned>(-1);
//! trap wildcard
const unsigned        ANY_TRAP = static_cast<unsigned>(-1);
//! interrupt wildcard
const unsigned   ANY_INTERRUPT = static_cast<unsigned>(-1);

/**
 * \class BaseEvent
 * This is the base class for all event types.
 */
class BaseEvent {
private:
	//! current class-scoped id counter to provide \a unique id's
	static event_id_t m_Counter;
protected:
	event_id_t m_Id; //!< unique id of this event
	time_t m_tStamp; //!< time stamp of event
	unsigned int m_OccCounter; //!< event fires when 0 is reached
	unsigned int m_OccCounterInit; //!< initial value for m_OccCounter
	ExperimentFlow* m_Parent; //!< this event belongs to experiment m_Parent
public:
	BaseEvent() : m_Id(++m_Counter), m_OccCounter(1), m_OccCounterInit(1), m_Parent(NULL)
	{ updateTime(); }
	virtual ~BaseEvent() { }
	/**
	 * This method is called when an experiment flow adds a new event by
	 * calling \c simulator.addEvent(pev) or \c simulator.addEventAndWait(pev).
	 * More specifically, the event will be added to the event-list first
	 * (buffer-list, to be precise) and then this event handler is called.
	 * @return You should return \c true to continue and \c false to prevent
	 *         the addition of the event \a pev, yielding an error in the
	 *         experiment flow (i.e. \c INVALID_EVENT is returned).
	 */
	virtual bool onEventAddition() { return true; }
	/**
	 * This method is called when an experiment flow removes an event from
	 * the event-management by calling \c removeEvent(prev), \c clearEvents()
	 * or by deleting a complete flow (\c removeFlow). More specifically, this
	 * event handler will be called *before* the event is actually deleted.
	 */
	virtual void onEventDeletion() { }
	/**
	 * This method is called when an previously added event is about to be
	 * triggered by the simulator-backend. More specifically, this event handler
	 * will be called *before* the event is actually triggered, i.e. before the
	 * corresponding coroutine is toggled.
	 */
	virtual void onEventTrigger() { }
	/**
	 * Retrieves the unique event id for this event.
	 * @return the unique id
	 */
	event_id_t getId() const { return (m_Id); }
	/**
	 * Retrieves the time stamp of this event. The time stamp is set when
	 * the event gets created, id est the constructor is called. The meaning
	 * of this value depends on the actual event type.
	 * @return the time stamp
	 */
	std::time_t getTime() const { return (m_tStamp); }
	/**
	 * Decreases the event counter by one. When this counter reaches zero, the
	 * event will be triggered.
	 */
	void decreaseCounter() { --m_OccCounter; }
	/**
	 * Sets the event counter to the specified value (default is one).
	 * @param val the new counter value
	 */
	void setCounter(unsigned int val = 1) { m_OccCounter = m_OccCounterInit = val; }
	/**
	 * Returns the current counter value.
	 * @return the counter value
	 */
	unsigned int getCounter() const { return (m_OccCounter); }
	/**
	 * Resets the event counter to its default value, or the last
	 * value that was set through setCounter().
	 */
	void resetCounter() { m_OccCounter = m_OccCounterInit; }
	/**
	 * Sets the time stamp for this event to the current system time.
	 */
	void updateTime() { time(&m_tStamp); }
	/**
	 * Returns the parent experiment of this event (context).
	 * If the event was created temporarily or wasn't linked to a context,
	 * the return value is \c NULL (unknown identity).
	 */
	ExperimentFlow* getParent() const { return (m_Parent); }
	/**
	 * Sets the parent (context) of this event. The default context
	 * is \c NULL (unknown identity).
	 */
	void setParent(ExperimentFlow* pFlow) { m_Parent = pFlow; }
};
// ----------------------------------------------------------------------------
// Specialized events:
//

/**
 * \class BEvent
 * A Breakpoint event to observe instruction changes within a given address space.
 */
class BPEvent : virtual public BaseEvent {
private:
	address_t m_CR3;
	address_t m_TriggerInstrPtr;
public:
	/**
	 * Creates a new breakpoint event. The range information is specific to
	 * the subclasses.
	 * @param address_space the address space to be oberserved, given as the
	 *        content of a CR3 register. The event will not be triggered unless
	 *        \a ip is part of the given address space.
	 *        ANY_ADDR can be used as a placeholder to allow debugging
	 *        in a random address space.
	 */
	BPEvent(address_t address_space = ANY_ADDR)
		: m_CR3(address_space), m_TriggerInstrPtr(ANY_ADDR) { }
	/**
	 * Returns the address space register of this event.
	 */
	address_t getCR3() const
	{ return m_CR3; }
	/**
	 * Sets the address space register for this event.
	 */
	void setCR3(address_t iptr)
	{ m_CR3 = iptr; }
	/**
	 * Checks whether a given address space is matching.
	 */
	bool aspaceIsMatching(address_t address_space = ANY_ADDR) const;
	/**
	 * Checks whether a given address is matching.
	 */
	virtual bool isMatching(address_t addr = 0, address_t address_space = ANY_ADDR) const = 0;
	/**
	 * Returns the instruction pointer that triggered this event.
	 */
	address_t getTriggerInstructionPointer() const
	{ return m_TriggerInstrPtr; }
	/**
	 * Sets the instruction pointer that triggered this event.  Should not
	 * be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t iptr)
	{ m_TriggerInstrPtr = iptr; }
};

/**
 * \class BPSingleEvent
 * A Breakpoint event to observe specific instruction pointers.
 */
class BPSingleEvent : virtual public BPEvent {
private:
	address_t m_WatchInstrPtr;
public:
	/**
	 * Creates a new breakpoint event.
	 * @param ip the instruction pointer of the breakpoint. If the control
	 *        flow reaches this address and its counter value is zero, the
	 *        event will be triggered. \a eip can be set to the ANY_ADDR
	 *        wildcard to allow arbitrary addresses.
	 * @param address_space the address space to be oberserved, given as the
	 *        content of a CR3 register. The event will not be triggered unless
	 *        \a ip is part of the given address space.
	 *        Here, too, ANY_ADDR is a placeholder to allow debugging
	 *        in a random address space.
	 */
	BPSingleEvent(address_t ip = 0, address_t address_space = ANY_ADDR)
		: BPEvent(address_space), m_WatchInstrPtr(ip) { }
	/**
	 * Returns the instruction pointer this event waits for.
	 */
	address_t getWatchInstructionPointer() const
	{ return m_WatchInstrPtr; }
	/**
	 * Sets the instruction pointer this event waits for.
	 */
	void setWatchInstructionPointer(address_t iptr)
	{ m_WatchInstrPtr = iptr; }
	/**
	 * Checks whether a given address is matching.
	 */
	bool isMatching(address_t addr, address_t address_space) const;
};

/**
 * \class BPRangeEvent
 * A event type to observe ranges of instruction pointers.
 */
class BPRangeEvent : virtual public BPEvent {
private:
	address_t m_WatchStartAddr;
	address_t m_WatchEndAddr;
public:
	/**
	 * Creates a new breakpoint-range event.  The range's ends are both
	 * inclusive, i.e. an address matches if start <= addr <= end.
	 * ANY_ADDR denotes the lower respectively the upper end of the address
	 * space.
	 */
	BPRangeEvent(address_t start = 0, address_t end = 0, address_t address_space = ANY_ADDR)
		: BPEvent(address_space), m_WatchStartAddr(start), m_WatchEndAddr(end)
		  { }
	/**
	 * Returns the instruction pointer watch range of this event.
	 */
	std::pair<address_t, address_t> getWatchInstructionPointerRange() const
	{ return std::make_pair(m_WatchStartAddr, m_WatchEndAddr); }
	/**
	 * Sets the instruction pointer watch range.  Both ends of the range
	 * may be ANY_ADDR (cf. constructor).
	 */
	void setWatchInstructionPointerRange(address_t start,
										 address_t end);
	/**
	 * Checks whether a given address is within the range.
	 */
	bool isMatching(address_t addr, address_t address_space) const;
};

/**
 * \class MemAccessEvent
 * Observes memory read/write accesses.
 * FIXME? currently >8-bit accesses only match if their lowest address is being watched
 *        (e.g., a 32-bit write to 0x4 also accesses 0x7, but this cannot be matched)
 */
class MemAccessEvent : virtual public BaseEvent {
public:
	enum accessType_t {
		MEM_UNKNOWN   = 0x0,
		MEM_READ      = 0x1,
		MEM_WRITE     = 0x2,
		MEM_READWRITE = 0x3
	};
private:
	//! Specific guest system address to watch, or ANY_ADDR.
	address_t m_WatchAddr;
	/**
	 * Memory access type we want to watch
	 * (MEM_READ || MEM_WRITE || MEM_READWRITE).
	 */
	accessType_t m_WatchType;
	//! Specific guest system address that actually triggered the event.
	address_t m_TriggerAddr;
	//! Width of the memory access (# bytes).
	size_t m_TriggerWidth;
	//! Address of the instruction that caused the memory access.
	address_t m_TriggerIP;
	//! Memory access type at m_TriggerAddr.
	accessType_t m_AccessType;
public:
	MemAccessEvent(accessType_t watchtype = MEM_READWRITE)
		: m_WatchAddr(ANY_ADDR), m_WatchType(watchtype),
		  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
		  m_AccessType(MEM_UNKNOWN) { }
	MemAccessEvent(address_t addr, 
				   accessType_t watchtype = MEM_READWRITE)
		: m_WatchAddr(addr), m_WatchType(watchtype),
		  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
		  m_AccessType(MEM_UNKNOWN) { }
	/**
	 * Returns the memory address to be observed.
	 */
	address_t getWatchAddress() const { return m_WatchAddr; }
	/**
	 * Sets the memory address to be observed.  (Wildcard: ANY_ADDR)
	 */
	void setWatchAddress(address_t addr) { m_WatchAddr = addr; }
	/**
	 * Checks whether a given address is matching.
	 */
	bool isMatching(address_t addr, accessType_t accesstype) const;
	/**
	 * Returns the specific memory address that actually triggered the
	 * event.
	 */
	address_t getTriggerAddress() const { return (m_TriggerAddr); }
	/**
	 * Sets the specific memory address that actually triggered the event.
	 * Should not be used by experiment code.
	 */
	void setTriggerAddress(address_t addr) { m_TriggerAddr = addr; }
	/**
	 * Returns the specific memory address that actually triggered the
	 * event.
	 */
	size_t getTriggerWidth() const { return (m_TriggerWidth); }
	/**
	 * Sets the specific memory address that actually triggered the event.
	 * Should not be used by experiment code.
	 */
	void setTriggerWidth(size_t width) { m_TriggerWidth = width; }
	/**
	 * Returns the address of the instruction causing this memory
	 * access.
	 */
	address_t getTriggerInstructionPointer() const
	{ return (m_TriggerIP); }
	/**
	 * Sets the address of the instruction causing this memory
	 * access.  Should not be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t addr)
	{ m_TriggerIP = addr; }
	/**
	 * Returns type (MEM_READ || MEM_WRITE) of the memory access that
	 * triggered this event.
	 */
	accessType_t getTriggerAccessType() const { return (m_AccessType); }
	/**
	 * Sets type of the memory access that triggered this event.  Should
	 * not be used by experiment code.
	 */
	void setTriggerAccessType(accessType_t type) { m_AccessType = type; }
	/**
	 * Returns memory access types (MEM_READ || MEM_WRITE || MEM_READWRITE)
	 * this event watches.  Should not be used by experiment code.
	 */
	accessType_t getWatchAccessType() const { return (m_WatchType); }
};

/**
 * \class MemReadEvent
 * Observes memory read accesses.
 */
class MemReadEvent : virtual public MemAccessEvent {
public:
	MemReadEvent()
		: MemAccessEvent(MEM_READ) { }
	MemReadEvent(address_t addr)
		: MemAccessEvent(addr, MEM_READ) { }
};

/**
 * \class MemWriteEvent
 * Observes memory write accesses.
 */
class MemWriteEvent : virtual public MemAccessEvent {
public:
	MemWriteEvent()
		: MemAccessEvent(MEM_READ) { }
	MemWriteEvent(address_t addr)
		: MemAccessEvent(addr, MEM_WRITE) { }
};

/**
 * \class TroubleEvent
 * Observes interrupt/trap activties.
 */
class TroubleEvent : virtual public BaseEvent {
private:
	/**
	 * Specific guest system interrupt/trap number that actually
	 * trigger the event.
	 */
	int m_TriggerNumber;
	/**
	 * Specific guest system interrupt/trap numbers to watch,
	 * or ANY_INTERRUPT/ANY_TRAP.
	 */
	std::vector<unsigned> m_WatchNumbers;
public:
	TroubleEvent() : m_TriggerNumber (-1) { }
	TroubleEvent(unsigned troubleNumber)
		: m_TriggerNumber(-1) 
	{ addWatchNumber(troubleNumber); }
	/**
	 * Add an interrupt/trap-number which should be observed
	 * @param troubleNumber number of an interrupt or trap
	 * @return \c true if number is added to the list \c false if the number
	 *         was already in the list
	 */
	bool addWatchNumber(unsigned troubleNumber);
	/**
	 * Remove an interrupt/trap-number which is in the list of observed
	 * numbers
	 * @param troubleNumber number of an interrupt or trap
	 * @return \c true if the number was found and removed \c false if the
	 *         number was not in the list
	 */
	bool removeWatchNumber(unsigned troubleNum);
	/**
	 * Returns the list of observed numbers
	 * @return a copy of the list which contains all observed numbers 
	 */
	std::vector<unsigned> getWatchNumbers() { return (m_WatchNumbers); }
	/**
	* Checks whether a given interrupt-/trap-number is matching.
	*/
	bool isMatching(unsigned troubleNum) const;
	/**
	* Sets the specific interrupt-/trap-number that actually triggered
	* the event. Should not be used by experiment code.
	*/
	void setTriggerNumber(unsigned troubleNum)
	{ m_TriggerNumber = troubleNum; } 
	/**
	* Returns the specific interrupt-/trap-number that actually triggered
	* the event.
	*/
	unsigned getTriggerNumber() { return (m_TriggerNumber); }
};

/**
 * \class InterruptEvent
 * Observes interrupts of the guest system.
 */
class InterruptEvent : virtual public TroubleEvent {
private:
	bool m_IsNMI; //!< non maskable interrupt flag
public:
	InterruptEvent() : m_IsNMI(false) { }
	InterruptEvent(unsigned interrupt) : m_IsNMI(false)
	{  addWatchNumber(interrupt); }                                                      
	/**
	 * Returns \c true if the interrupt is non maskable, \c false otherwise.
	 */
	bool isNMI() { return (m_IsNMI); }
	/**
	 * Sets the interrupt type (non maskable or not).
	 */
	void setNMI(bool enabled) { m_IsNMI = enabled; }
};

/**
 * \class TrapEvent
 * Observes traps of the guest system.
 */
class TrapEvent : virtual public TroubleEvent {
public:
	TrapEvent() { }
	TrapEvent(unsigned trap) { addWatchNumber(trap); }
};

/**
 * \class GuestEvent
 * Used to receive data from the guest system.
 */
class GuestEvent : virtual public BaseEvent {
private:
	char m_Data;
	unsigned m_Port;
public:
	GuestEvent() : m_Data(0), m_Port(0) { }
	/**
	 * Returns the data, transmitted by the guest system.
	 */
	char getData() const { return (m_Data); }
	/**
	 * Sets the data which had been transmitted by the guest system.
	 */
	void setData(char data) { m_Data = data; }
	/**
	 * Returns the data length, transmitted by the guest system.
	 */
	unsigned getPort() const { return (m_Port); }
	/**
	 * Sets the data length which had been transmitted by the guest system.
	 */
	void setPort(unsigned port) { m_Port = port; }
};

/**
 * \class IOPortEvent
 * Observes I/O access on architectures with a separate I/O access mechanism (e.g. IA-32)
 */
class IOPortEvent : virtual public BaseEvent {
private:
	unsigned char  m_Data;
	unsigned m_Port;
	bool m_Out;
public:
	/**
	 * Initialises an IOPortEvent
	 *
	 * @param port the port the event ist listening on
	 * @param out Defines the direction of the event.
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	IOPortEvent(unsigned port, bool out) : m_Data(0), m_Port(port), m_Out(out) { }
	/**
	 * Returns the data sent to the specified port
	 */
	unsigned char getData() const { return (m_Data); }
	/**
	 * Sets the data which had been transmitted.
	 */
	void setData(unsigned char data) { m_Data = data; }
	/**
	 * Retrieves the port which this event is bound to.
	 */
	unsigned getPort() const { return (m_Port); }
	/**
	 * Sets the port which this event is bound to.
	 */
	void setPort(unsigned port) { m_Port = port; }
	/**
	* Checks whether a given port number is matching.
	* @param p The port number an I/O event occured on
	* @param out True if the communication was outbound, false otherwise
	*/
	bool isMatching(unsigned p, bool out) const { return ( out = isOutEvent() && p == getPort()); }
	/**
	 * Tells you if this event is capturing outbound communication (inbound if false)
	 */
	bool isOutEvent() const { return m_Out; }
	/**
	 * Change the event direction.
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	void setOut(bool out) { m_Out = out; }
};

/**
 * \class JumpEvent
 * JumpEvents are used to observe conditional jumps (if...else if...else).
 */
class JumpEvent : virtual public BaseEvent {
private:
	unsigned m_Opcode;
	bool m_FlagTriggered;
public:
	/**
	 * Constructs a new event object.
	 * @param parent the parent object
	 * @param opcode the opcode of the jump-instruction to be observed
	 *        or ANY_INSTR to match all jump-instructions
	 */
	JumpEvent(unsigned opcode = ANY_INSTR)
		: m_Opcode(opcode), m_FlagTriggered(false) { }
	/**
	 * Retrieves the opcode of the jump-instruction.
	 */
	unsigned getOpcode() const { return (m_Opcode); }
	/**
	 * Returns \c true, if the event was triggered due to specific register
	 * content, \c false otherwise.
	 */
	bool isRegisterTriggered() { return (!m_FlagTriggered); }
	/**
	 * Returns \c true, of the event was triggered due to specific FLAG
	 * state, \c false otherwise. This is the common case.
	 */
	bool isFlagTriggered() { return (m_FlagTriggered); }
	/**
	 * Sets the requestet jump-instruction opcode.
	 */
	void setOpcode(unsigned oc) { oc = m_Opcode; }
	/**
	 * Sets the trigger flag.
	 */
	void setFlagTriggered(bool flagTriggered)
	{ m_FlagTriggered = flagTriggered; }
};

/**
 * \class GenericTimerEvent
 * This event type is used to create timeouts within in an experiment.
 *
 * Depending on your simulator backend, a concrete class needs to be derived
 * from \c GenericTimerEvent. \c onEventAddition should be used to register and
 * \c onEventDeletion to unregister the timer. \c onEventTrigger can be used to
 * re-register the timer if a repetitive timer is requested and the back-
 * end doesn't support such timer types natively.
 */
class GenericTimerEvent : public BaseEvent {
protected:
	unsigned m_Timeout; //!< timeout interval in milliseconds
	timer_id_t m_Id; //!< internal timer id (sim-specific)
	bool m_Once; //!< \c true, if the timer should be triggered only once, \c false otherwise
public:
	/**
	 * Creates a new timer event. This can be used to implement a timeout-
	 * mechanism in the experiment-flow. The timer starts automatically when
	 * added to the simulator backend.
	 * @param timeout the time intervall in milliseconds (ms)
	 * @see SimulatorController::addEvent
	 */
	GenericTimerEvent(unsigned timeout)
		: m_Timeout(timeout), m_Id(-1) { }
	~GenericTimerEvent() { }
	/**
	 * Retrieves the internal timer id. Maybe useful for debug output.
	 * @return the timer id
	 */
	timer_id_t getId() const { return m_Id; }
	/**
	 * Sets the internal timer id. This should not be used by the experiment.
	 * @param id the new timer id, given by the underlying simulator-backend
	 */
	void setId(timer_id_t id) { m_Id = id; }
	/**
	 * Retrieves the timer's timeout value.
	 * @return the timout in milliseconds
	 */
	unsigned getTimeout() const { return m_Timeout; }
};

} // end-of-namespace: fail

#endif // __EVENT_HPP__
