#ifndef __EVENT_HPP__
  #define __EVENT_HPP__

#include <ctime>
#include <string>
#include <cassert>
#include <vector>

#include "../SAL/SALConfig.hpp"

namespace fi
{
class ExperimentFlow;

typedef unsigned long EventId; //!< type of event ids

//! invalid event id (used as a return indicator)
const EventId    INVALID_EVENT = (EventId)-1;
//! address wildcard (e.g. for BPEvent)
const sal::address_t  ANY_ADDR = static_cast<sal::address_t>(-1);
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
class BaseEvent
{
	private:
		//! current class-scoped id counter to provide \a unique id's
		static EventId m_Counter;
	protected:
		EventId m_Id; //!< unique id of this event
		time_t m_tStamp; //!< time stamp of event
		unsigned int m_OccCounter; //!< event fires when 0 is reached
		unsigned int m_OccCounterInit; //!< initial value for m_OccCounter
		ExperimentFlow* m_Parent; //!< this event belongs to experiment m_Parent
	public:
		BaseEvent() : m_Id(++m_Counter), m_OccCounter(1), m_OccCounterInit(1), m_Parent(NULL)
		{ updateTime(); }
		virtual ~BaseEvent() { }
		/**
		 * Retrieves the unique event id for this event.
		 * @return the unique id
		 */
		EventId getId() const { return (m_Id); }
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
// FIXME: Dynamische Casts zur Laufzeit evtl. zu uneffizient?
//        (vgl. auf NULL evtl. akzeptabel?) Bessere Lösungen?

// ----------------------------------------------------------------------------
// Specialized events:
//

/**
 * \class BPEvent
 * A Breakpoint event to observe specific instruction pointers.
 * FIXME consider renaming to BPSingleEvent, introducing common BPEvent base class
 */
class BPEvent : virtual public BaseEvent
{
	private:
		sal::address_t m_WatchInstrPtr;
		sal::address_t m_TriggerInstrPtr;
	public:
		/**
		 * Creates a new breakpoint event.
		 * @param ip the instruction pointer of the breakpoint. If the control
		 *        flow reaches this address and its counter value is zero, the
		 *        event will be triggered. \a eip can be set to the ANY_ADDR
		 *        wildcard to allow arbitrary addresses.
		 */
		BPEvent(sal::address_t ip = 0)
			: m_WatchInstrPtr(ip), m_TriggerInstrPtr(ANY_ADDR) { }
		/**
		 * Returns the instruction pointer this event waits for.
		 */
		sal::address_t getWatchInstructionPointer() const
		{ return m_WatchInstrPtr; }
		/**
		 * Sets the instruction pointer this event waits for.
		 */
		void setWatchInstructionPointer(sal::address_t iptr)
		{ m_WatchInstrPtr = iptr; }
		/**
		 * Checks whether a given address is matching.
		 */
		bool isMatching(sal::address_t addr) const;
		/**
		 * Returns the instruction pointer that triggered this event.
		 */
		sal::address_t getTriggerInstructionPointer() const
		{ return m_TriggerInstrPtr; }
		/**
		 * Sets the instruction pointer that triggered this event.  Should not
		 * be used by experiment code.
		 */
		void setTriggerInstructionPointer(sal::address_t iptr)
		{ m_TriggerInstrPtr = iptr; }
};

/**
 * \class BPRangeEvent
 * A event type to observe ranges of instruction pointers.
 */
class BPRangeEvent : virtual public BaseEvent
{
	private:
		sal::address_t m_WatchStartAddr;
		sal::address_t m_WatchEndAddr;
		sal::address_t m_TriggerInstrPtr;
	public:
		/**
		 * Creates a new breakpoint-range event.  The range's ends are both
		 * inclusive, i.e. an address matches if start <= addr <= end.
		 * ANY_ADDR denotes the lower respectively the upper end of the address
		 * space.
		 */
		BPRangeEvent(sal::address_t start = 0, sal::address_t end = 0)
			: m_WatchStartAddr(start), m_WatchEndAddr(end),
			  m_TriggerInstrPtr(ANY_ADDR) { }
		/**
		 * Sets the instruction pointer watch range.  Both ends of the range
		 * may be ANY_ADDR (cf. constructor).
		 */
		void setWatchInstructionPointerRange(sal::address_t start,
											 sal::address_t end);
		/**
		 * Checks whether a given address is within the range.
		 */
		bool isMatching(sal::address_t addr) const;
		/**
		 * Returns the instruction pointer that triggered this event.
		 */
		sal::address_t getTriggerInstructionPointer() const
		{ return m_TriggerInstrPtr; }
		/**
		 * Sets the instruction pointer that triggered this event.  Should not
		 * be used by experiment code.
		 */
		void setTriggerInstructionPointer(sal::address_t iptr)
		{ m_TriggerInstrPtr = iptr; }
};

/**
 * \class MemAccessEvent
 * Observes memory read/write accesses.
 * FIXME? currently >8-bit accesses only match if their lowest address is being watched
 *        (e.g., a 32-bit write to 0x4 also accesses 0x7, but this cannot be matched)
 */
class MemAccessEvent : virtual public BaseEvent
{
	public:
		enum accessType_t
		{
			MEM_UNKNOWN   = 0x0,
			MEM_READ      = 0x1,
			MEM_WRITE     = 0x2,
			MEM_READWRITE = 0x3
		};
	private:
		//! Specific guest system address to watch, or ANY_ADDR.
		sal::address_t m_WatchAddr;
		/**
		 * Memory access type we want to watch
		 * (MEM_READ || MEM_WRITE || MEM_READWRITE).
		 */
		accessType_t m_WatchType;
		//! Specific guest system address that actually triggered the event.
		sal::address_t m_TriggerAddr;
		//! Width of the memory access (# bytes).
		size_t m_TriggerWidth;
		//! Address of the instruction that caused the memory access.
		sal::address_t m_TriggerIP;
		//! Memory access type at m_TriggerAddr.
		accessType_t m_AccessType;
	public:
		MemAccessEvent(accessType_t watchtype = MEM_READWRITE)
			: m_WatchAddr(ANY_ADDR), m_WatchType(watchtype),
			  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
			  m_AccessType(MEM_UNKNOWN) { }
		MemAccessEvent(sal::address_t addr, 
					   accessType_t watchtype = MEM_READWRITE)
			: m_WatchAddr(addr), m_WatchType(watchtype),
			  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
			  m_AccessType(MEM_UNKNOWN) { }
		/**
		 * Returns the memory address to be observed.
		 */
		sal::address_t getWatchAddress() const { return m_WatchAddr; }
		/**
		 * Sets the memory address to be observed.  (Wildcard: ANY_ADDR)
		 */
		void setWatchAddress(sal::address_t addr) { m_WatchAddr = addr; }
		/**
		 * Checks whether a given address is matching.
		 */
		bool isMatching(sal::address_t addr, accessType_t accesstype) const;
		/**
		 * Returns the specific memory address that actually triggered the
		 * event.
		 */
		sal::address_t getTriggerAddress() const { return (m_TriggerAddr); }
		/**
		 * Sets the specific memory address that actually triggered the event.
		 * Should not be used by experiment code.
		 */
		void setTriggerAddress(sal::address_t addr) { m_TriggerAddr = addr; }
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
		sal::address_t getTriggerInstructionPointer() const
		{ return (m_TriggerIP); }
		/**
		 * Sets the address of the instruction causing this memory
		 * access.  Should not be used by experiment code.
		 */
		void setTriggerInstructionPointer(sal::address_t addr)
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
class MemReadEvent : virtual public MemAccessEvent
{
	public:
		MemReadEvent()
			: MemAccessEvent(MEM_READ) { }
		MemReadEvent(sal::address_t addr)
			: MemAccessEvent(addr, MEM_READ) { }
};

/**
 * \class MemWriteEvent
 * Observes memory write accesses.
 */
class MemWriteEvent : virtual public MemAccessEvent
{
	public:
		MemWriteEvent()
			: MemAccessEvent(MEM_READ) { }
		MemWriteEvent(sal::address_t addr)
			: MemAccessEvent(addr, MEM_WRITE) { }
};

/**
 * \class TroubleEvent
 * Observes interrupt/trap activties.
 */
class TroubleEvent : virtual public BaseEvent
{
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
class InterruptEvent : virtual public TroubleEvent
{
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
class TrapEvent : virtual public TroubleEvent
{
	public:
		TrapEvent() { }
		TrapEvent(unsigned trap) { addWatchNumber(trap); }
};

/**
 * \class GuestEvent
 * Used to receive data from the guest system.
 */
class GuestEvent : virtual public BaseEvent
{
	private:
		char  m_Data;
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
		unsigned getPort() const { return (m_Data); }
		/**
		 * Sets the data length which had been transmitted by the guest system.
		 */
		void setPort(unsigned port) { m_Port = port; }
};

/**
 * \class JumpEvent
 * JumpEvents are used to observe conditional jumps (if...else if...else).
 */
class JumpEvent : virtual public BaseEvent
{
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
		 * Returns \c true, of the event was triggered due to specific register
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
 * \class TimerEvent
 * This event type is used to create timeouts/timers within in an experiment.
 */
class TimerEvent : public BaseEvent
{
	private:
		unsigned m_Timeout; //!< timeout interval in milliseconds
		sal::timer_id_t m_Id; //!< internal timer id (sim-specific)
		bool m_Once; //!< true, if the timer should be triggered only once
	public:
		/**
		 * Creates a new timer event. This can be used to implement a timeout-
		 * mechanism in the experiment-flow. The timer starts automatically when
		 * added to the simulator backend (@see SimulatorController::addEvent)
		 * @param timeout the time intervall in milliseconds (ms)
		 * @param once \c true, if the TimerEvent should be triggered once,
		 *        \c false if it should occur regularly
		 */
		TimerEvent(unsigned timeout, bool once)
			: m_Timeout(timeout), m_Id(-1), m_Once(once) { }
		~TimerEvent() { }
		/**
		 * Retrieves the internal timer id. Maybe useful for debug output.
		 * @return the timer id
		 */
		sal::timer_id_t getId() const { return m_Id; }
		/**
		 * Sets the internal timer id. This should not be used by the experiment.
		 * @param id the new timer id, given by the underlying simulator-backend
		 */
		void setId(sal::timer_id_t id) { m_Id = id; }
		/**
		 * Retrieves the timer's timeout value.
		 * @return the timout in milliseconds
		 */
		unsigned getTimeout() const { return m_Timeout; }
		/**
		 * Checks whether the timer occurs once or repetitive.
		 * @return \c true if timer triggers once, \c false if repetitive
		 */
		bool getOnceFlag() const { return m_Once; }
};

} // end-of-namespace: fi

#endif /* __EVENT_HPP__ */
