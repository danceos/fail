#ifndef __LISTENER_HPP__
  #define __LISTENER_HPP__

#include <ctime>
#include <string>
#include <cassert>
#include <vector>
#include <utility>
#include <iostream>

#include "SALConfig.hpp"

namespace fail {

class ExperimentFlow;

//! address wildcard (e.g. for BPListeners)
const address_t       ANY_ADDR = static_cast<address_t>(-1);
//! instruction wildcard
const unsigned       ANY_INSTR = static_cast<unsigned>(-1);
//! trap wildcard
const unsigned        ANY_TRAP = static_cast<unsigned>(-1);
//! interrupt wildcard
const unsigned   ANY_INTERRUPT = static_cast<unsigned>(-1);

/**
 * \class BaseListener
 * This is the base class for all listener types.
 */
class BaseListener {
protected:
	time_t m_tStamp; //!< time stamp of listener
	unsigned int m_OccCounter; //!< listener fires when 0 is reached
	unsigned int m_OccCounterInit; //!< initial value for m_OccCounter
	ExperimentFlow* m_Parent; //!< this listener belongs to experiment m_Parent
public:
	BaseListener() : m_OccCounter(1), m_OccCounterInit(1), m_Parent(NULL)
	{ updateTime(); }
	virtual ~BaseListener() { }
	/**
	 * This method is called when an experiment flow adds a new listener by
	 * calling \c simulator.addListener() or \c simulator.addListenerAndResume().
	 * More specifically, the listener will be added to the listener manager first
	 * (buffer-list, to be precise) and then this listener handler is called.
	 * @return You should return \c true to continue and \c false to prevent
	 *         the addition of the listener \c this, yielding an error in the
	 *         experiment flow (i.e., \c false is returned).
	 */
	virtual bool onAddition() { return true; }
	/**
	 * This method is called when an experiment flow removes an listener from
	 * the listener-management by calling \c removeListener(), \c clearListeners()
	 * or by deleting a complete flow (\c removeFlow()). More specifically, this
	 * listener handler will be called *before* the listener is actually deleted.
	 */
	virtual void onDeletion() { }
	/**
	 * This method is called when an previously added listener is about to be
	 * triggered by the simulator-backend. More specifically, this listener handler
	 * will be called *before* the listener is actually triggered, i.e. before the
	 * corresponding coroutine is toggled.
	 */
	virtual void onTrigger() { }
	// TODO: Hier noch ne neue Methode oder reicht es, die Semantik von onTrigger umzudef.?
	/**
	 * Retrieves the time stamp of this listener. The time stamp is set when
	 * the listener gets created, i.e. the constructor is called. The meaning
	 * of this value depends on the actual listener type.
	 * @return the time stamp
	 */
	std::time_t getTime() const { return (m_tStamp); }
	/**
	 * Decreases the listener counter by one. When this counter reaches zero, the
	 * listener will be triggered.
	 */
	void decreaseCounter() { --m_OccCounter; }
	/**
	 * Sets the listener counter to the specified value.
	 * @param val the new counter value (default is 1)
	 */
	void setCounter(unsigned int val = 1) { m_OccCounter = m_OccCounterInit = val; }
	/**
	 * Returns the current counter value.
	 * @return the counter value
	 */
	unsigned int getCounter() const { return (m_OccCounter); }
	/**
	 * Resets the listener counter to its default value, or the last
	 * value that was set through \c setCounter().
	 */
	void resetCounter() { m_OccCounter = m_OccCounterInit; }
	/**
	 * Sets the time stamp for this listener to the current system time.
	 */
	void updateTime() { time(&m_tStamp); }
	/**
	 * Returns the parent experiment of this listener (context).
	 * If the listener was created temporarily or wasn't linked to a context,
	 * the return value is \c NULL (unknown identity).
	 */
	ExperimentFlow* getParent() const { return (m_Parent); }
	/**
	 * Sets the parent (context) of this listener. The default context
	 * is \c NULL (unknown identity).
	 */
	void setParent(ExperimentFlow* pFlow) { m_Parent = pFlow; }
};
// ----------------------------------------------------------------------------
// Specialized listeners:
//

/**
 * \class BListener
 * A Breakpoint listener to observe instruction changes within a given address space.
 */
class BPListener : virtual public BaseListener {
private:
	address_t m_CR3;
	address_t m_TriggerInstrPtr;
public:
	/**
	 * Creates a new breakpoint listener. The range information is specific to
	 * the subclasses.
	 * @param address_space the address space to be oberserved, given as the
	 *        content of a CR3 register. The listener will not be triggered unless
	 *        \a ip is part of the given address space.
	 *        ANY_ADDR can be used as a placeholder to allow debugging
	 *        in a random address space.
	 */
	BPListener(address_t address_space = ANY_ADDR)
		: m_CR3(address_space), m_TriggerInstrPtr(ANY_ADDR) { }
	/**
	 * Returns the address space register of this listener.
	 */
	address_t getCR3() const { return m_CR3; }
	/**
	 * Sets the address space register for this listener.
	 */
	void setCR3(address_t iptr) { m_CR3 = iptr; }
	/**
	 * Checks whether a given address space is matching.
	 */
	bool aspaceIsMatching(address_t address_space = ANY_ADDR) const;
	/**
	 * Checks whether a given address is matching.
	 */
	virtual bool isMatching(address_t addr = 0, address_t address_space = ANY_ADDR) const = 0;
	/**
	 * Returns the instruction pointer that triggered this listener.
	 */
	address_t getTriggerInstructionPointer() const { return m_TriggerInstrPtr; }
	/**
	 * Sets the instruction pointer that triggered this listener.  Should not
	 * be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t iptr) { m_TriggerInstrPtr = iptr; }
};

/**
 * \class BPSingleListener
 * A Breakpoint listener to observe specific instruction pointers.
 */
class BPSingleListener : virtual public BPListener {
private:
	address_t m_WatchInstrPtr;
public:
	/** 
	 * Creates a new breakpoint listener.
	 * @param ip the instruction pointer of the breakpoint. If the control
	 *        flow reaches this address and its counter value is zero, the
	 *        listener will be triggered. \a eip can be set to the ANY_ADDR
	 *        wildcard to allow arbitrary addresses. Defaults to 0.
	 * @param address_space the address space to be oberserved, given as the
	 *        content of a CR3 register. The listener will not be triggered unless
	 *        \a ip is part of the given address space. Defaults to \c ANY_ADDR.
	 *        Here, too, ANY_ADDR is a placeholder to allow debugging
	 *        in a random address space.
	 */
	BPSingleListener(address_t ip = 0, address_t address_space = ANY_ADDR)
		: BPListener(address_space), m_WatchInstrPtr(ip) { }
	/**
	 * Returns the instruction pointer this listener waits for.
	 * @return the instruction pointer specified in the constructor or by
	 *         calling \c setWatchInstructionPointer()
	 */
	address_t getWatchInstructionPointer() const
	{ return m_WatchInstrPtr; }
	/**
	 * Sets the instruction pointer this listener waits for.
	 * @param iptr the new instruction ptr to wait for
	 */
	void setWatchInstructionPointer(address_t iptr)
	{ m_WatchInstrPtr = iptr; }
	/**
	 * Checks whether a given address is matching.
	 * @param addr address to check
	 * @param address_space address space information
	 * @return \c true if address is within the range, \c false otherwise
	 */
	bool isMatching(address_t addr, address_t address_space) const;
};

/**
 * \class BPRangeListener
 * A listener type to observe ranges of instruction pointers.
 */
class BPRangeListener : virtual public BPListener {
private:
	address_t m_WatchStartAddr;
	address_t m_WatchEndAddr;
public:
	/**
	 * Creates a new breakpoint-range listener.  The range's ends are both
	 * inclusive, i.e. an address matches if start <= addr <= end.
	 * ANY_ADDR denotes the lower respectively the upper end of the address
	 * space.
	 */
	BPRangeListener(address_t start = 0, address_t end = 0, address_t address_space = ANY_ADDR)
		: BPListener(address_space), m_WatchStartAddr(start), m_WatchEndAddr(end)
		  { }
	/**
	 * Returns the instruction pointer watch range of this listener.
	 * @return the listener's range
	 */
	std::pair<address_t, address_t> getWatchInstructionPointerRange() const
	{ return std::make_pair(m_WatchStartAddr, m_WatchEndAddr); }
	/**
	 * Sets the instruction pointer watch range.  Both ends of the range
	 * may be ANY_ADDR (cf. constructor).
	 * @param start the (inclusive) lower bound, or \c ANY_ADDR
	 * @param end the (inclusive) upper bound, or \c ANY_ADDR
	 */
	void setWatchInstructionPointerRange(address_t start, address_t end);
	/**
	 * Checks whether a given address is within the range.
	 * @param addr address to check
	 * @param address_space address space information
	 * @return \c true if address is within the range (represented by
	 *         \c this), \c false otherwise
	 */
	bool isMatching(address_t addr, address_t address_space) const;
};

/**
 * \class MemAccessListener
 * Observes memory read/write accesses.
 */
class MemAccessListener : virtual public BaseListener {
public:
	enum accessType_t {
		MEM_UNKNOWN   = 0x0,
		MEM_READ      = 0x1,
		MEM_WRITE     = 0x2,
		MEM_READWRITE = 0x3
	};
private:
	//! Specific physical guest system address to watch, or ANY_ADDR.
	address_t m_WatchAddr;
	//! Width of the memory area being watched (# bytes).
	size_t m_WatchWidth;
	/**
	 * Memory access type we want to watch
	 * (MEM_READ || MEM_WRITE || MEM_READWRITE).
	 */
	accessType_t m_WatchType;
	//! Specific physical guest system address that actually triggered the listener.
	address_t m_TriggerAddr;
	//! Width of the memory access (# bytes).
	size_t m_TriggerWidth;
	//! Address of the instruction that caused the memory access.
	address_t m_TriggerIP;
	//! Memory access type at m_TriggerAddr.
	accessType_t m_AccessType;
public:
	MemAccessListener(accessType_t watchtype = MEM_READWRITE)
		: m_WatchAddr(ANY_ADDR), m_WatchWidth(1), m_WatchType(watchtype),
		  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
		  m_AccessType(MEM_UNKNOWN) { }
	MemAccessListener(address_t addr, 
				   accessType_t watchtype = MEM_READWRITE)
		: m_WatchAddr(addr), m_WatchWidth(1), m_WatchType(watchtype),
		  m_TriggerAddr(ANY_ADDR), m_TriggerIP(ANY_ADDR),
		  m_AccessType(MEM_UNKNOWN) { }
	/**
	 * Returns the physical memory address to be observed.
	 */
	address_t getWatchAddress() const { return m_WatchAddr; }
	/**
	 * Sets the physical memory address to be observed.  (Wildcard: ANY_ADDR)
	 */
	void setWatchAddress(address_t addr) { m_WatchAddr = addr; }
	/**
	 * Returns the width of the memory area being watched.
	 */
	size_t getWatchWidth() const { return m_WatchWidth; }
	/**
	 * Sets the width of the memory area being watched (defaults to 1).
	 */
	void setWatchWidth(size_t width) { m_WatchWidth = width; }
	/**
	 * Checks whether a given physical memory access is matching.
	 */
	bool isMatching(address_t addr, size_t width, accessType_t accesstype) const;
	/**
	 * Returns the specific physical memory address that actually triggered the
	 * listener.
	 */
	address_t getTriggerAddress() const { return m_TriggerAddr; }
	/**
	 * Sets the specific physical memory address that actually triggered the
	 * listener.  Should not be used by experiment code.
	 */
	void setTriggerAddress(address_t addr) { m_TriggerAddr = addr; }
	/**
	 * Returns the width (in bytes) of the memory access that triggered this
	 * listener.
	 */
	size_t getTriggerWidth() const { return m_TriggerWidth; }
	/**
	 * Sets the width (in bytes) of the memory access that triggered this
	 * listener.  Should not be used by experiment code.
	 */
	void setTriggerWidth(size_t width) { m_TriggerWidth = width; }
	/**
	 * Returns the address of the instruction causing this memory access.
	 */
	address_t getTriggerInstructionPointer() const { return m_TriggerIP; }
	/**
	 * Sets the address of the instruction causing this memory access.  Should
	 * not be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t addr) { m_TriggerIP = addr; }
	/**
	 * Returns type (MEM_READ || MEM_WRITE) of the memory access that triggered
	 * this listener.
	 */
	accessType_t getTriggerAccessType() const { return m_AccessType; }
	/**
	 * Sets type of the memory access that triggered this listener.  Should not
	 * be used by experiment code.
	 */
	void setTriggerAccessType(accessType_t type) { m_AccessType = type; }
	/**
	 * Returns memory access types (MEM_READ || MEM_WRITE || MEM_READWRITE)
	 * this listener watches.  Should not be used by experiment code.
	 */
	accessType_t getWatchAccessType() const { return m_WatchType; }
};

/**
 * \class MemReadListener
 * Observes memory read accesses.
 */
class MemReadListener : virtual public MemAccessListener {
public:
	MemReadListener()
		: MemAccessListener(MEM_READ) { }
	MemReadListener(address_t addr)
		: MemAccessListener(addr, MEM_READ) { }
};

/**
 * \class MemWriteListener
 * Observes memory write accesses.
 */
class MemWriteListener : virtual public MemAccessListener {
public:
	MemWriteListener()
		: MemAccessListener(MEM_READ) { }
	MemWriteListener(address_t addr)
		: MemAccessListener(addr, MEM_WRITE) { }
};

/**
 * \class TroubleListener
 * Observes interrupt/trap activties.
 */
class TroubleListener : virtual public BaseListener {
private:
	/**
	 * Specific guest system interrupt/trap number that actually
	 * trigger the listener.
	 */
	int m_TriggerNumber;
	/**
	 * Specific guest system interrupt/trap numbers to watch,
	 * or ANY_INTERRUPT/ANY_TRAP.
	 */
	std::vector<unsigned> m_WatchNumbers;
public:
	TroubleListener() : m_TriggerNumber (-1) { }
	TroubleListener(unsigned troubleNumber)
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
	std::vector<unsigned> getWatchNumbers() { return m_WatchNumbers; }
	/**
	* Checks whether a given interrupt-/trap-number is matching.
	*/
	bool isMatching(unsigned troubleNum) const;
	/**
	* Sets the specific interrupt-/trap-number that actually triggered
	* the listener. Should not be used by experiment code.
	*/
	void setTriggerNumber(unsigned troubleNum)
	{ m_TriggerNumber = troubleNum; } 
	/**
	* Returns the specific interrupt-/trap-number that actually triggered
	* the listener.
	*/
	unsigned getTriggerNumber() { return m_TriggerNumber; }
};

/**
 * \class InterruptListener
 * Observes interrupts of the guest system.
 */
class InterruptListener : virtual public TroubleListener {
private:
	bool m_IsNMI; //!< non maskable interrupt flag
public:
	InterruptListener() : m_IsNMI(false) { }
	InterruptListener(unsigned interrupt) : m_IsNMI(false)
	{  addWatchNumber(interrupt); }                                                      
	/**
	 * Returns \c true if the interrupt is non maskable, \c false otherwise.
	 */
	bool isNMI() { return m_IsNMI; }
	/**
	 * Sets the interrupt type (non maskable or not).
	 */
	void setNMI(bool enabled) { m_IsNMI = enabled; }
};

/**
 * \class TrapListener
 * Observes traps of the guest system.
 */
class TrapListener : virtual public TroubleListener {
public:
	TrapListener() { }
	TrapListener(unsigned trap) { addWatchNumber(trap); }
};

/**
 * \class GuestListener
 * Used to receive data from the guest system.
 */
class GuestListener : virtual public BaseListener {
private:
	char m_Data;
	unsigned m_Port;
public:
	GuestListener() : m_Data(0), m_Port(0) { }
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
 * \class IOPortListener
 * Observes I/O access on architectures with a separate I/O access mechanism (e.g. IA-32)
 */
class IOPortListener : virtual public BaseListener {
private:
	unsigned char  m_Data;
	unsigned m_Port;
	bool m_Out;
public:
	/**
	 * Initialises an IOPortListener
	 *
	 * @param port the port the listener ist listening on
	 * @param out Defines the direction of the listener.
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	IOPortListener(unsigned port, bool out) : m_Data(0), m_Port(port), m_Out(out) { }
	/**
	 * Returns the data sent to the specified port
	 */
	unsigned char getData() const { return m_Data; }
	/**
	 * Sets the data which had been transmitted.
	 */
	void setData(unsigned char data) { m_Data = data; }
	/**
	 * Retrieves the port which this listener is bound to.
	 */
	unsigned getPort() const { return m_Port; }
	/**
	 * Sets the port which this listener is bound to.
	 */
	void setPort(unsigned port) { m_Port = port; }
	/**
	* Checks whether a given port number is matching.
	* @param p The port number an I/O listener occured on
	* @param out True if the communication was outbound, false otherwise
	*/
	bool isMatching(unsigned p, bool out) const { return ( out = isOutListener() && p == getPort()); }
	/**
	 * Tells you if this listener is capturing outbound communication (inbound if false)
	 */
	bool isOutListener() const { return m_Out; }
	/**
	 * Change the listener direction.
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	void setOut(bool out) { m_Out = out; }
};

/**
 * \class JumpListener
 * JumpListeners are used to observe conditional jumps (if...else if...else).
 */
class JumpListener : virtual public BaseListener {
private:
	unsigned m_Opcode;
	bool m_FlagTriggered;
public:
	/**
	 * Constructs a new listener object.
	 * @param parent the parent object
	 * @param opcode the opcode of the jump-instruction to be observed
	 *        or ANY_INSTR to match all jump-instructions
	 */
	JumpListener(unsigned opcode = ANY_INSTR)
		: m_Opcode(opcode), m_FlagTriggered(false) { }
	/**
	 * Retrieves the opcode of the jump-instruction.
	 */
	unsigned getOpcode() const { return m_Opcode; }
	/**
	 * Returns \c true, if the listener was triggered due to specific register
	 * content, \c false otherwise.
	 */
	bool isRegisterTriggered() { return !m_FlagTriggered; }
	/**
	 * Returns \c true, of the listener was triggered due to specific FLAG
	 * state, \c false otherwise. This is the common case.
	 */
	bool isFlagTriggered() { return m_FlagTriggered; }
	/**
	 * Sets the requestet jump-instruction opcode.
	 */
	void setOpcode(unsigned oc) { oc = m_Opcode; }
	/**
	 * Sets the trigger flag.
	 */
	void setFlagTriggered(bool flagTriggered) { m_FlagTriggered = flagTriggered; }
};

/**
 * \class GenericTimerListener
 * This listener type is used to create timeouts within in an experiment.
 *
 * Depending on your simulator backend, a concrete class needs to be derived
 * from \c GenericTimerListener. \c onListenerAddition should be used to register and
 * \c onListenerDeletion to unregister the timer. \c onListenerTrigger can be used to
 * re-register the timer if a repetitive timer is requested and the back-
 * end doesn't support such timer types natively.
 */
class GenericTimerListener : public BaseListener {
protected:
	unsigned m_Timeout; //!< timeout interval in milliseconds
	timer_id_t m_Id; //!< internal timer id (sim-specific)
	bool m_Once; //!< \c true, if the timer should be triggered only once, \c false otherwise
public:
	/**
	 * Creates a new timer listener. This can be used to implement a timeout-
	 * mechanism in the experiment-flow. The timer starts automatically when
	 * added to the simulator backend.
	 * @param timeout the time intervall in milliseconds (ms)
	 * @see SimulatorController::addListener
	 */
	GenericTimerListener(unsigned timeout)
		: m_Timeout(timeout), m_Id(-1) { }
	~GenericTimerListener() { }
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

#endif // __LISTENER_HPP__
