#ifndef __LISTENER_HPP__
#define __LISTENER_HPP__

#include <string>
#include <cassert>
#include <vector>
#include <utility>
#include <iostream>

#include "SALConfig.hpp"
#include "Event.hpp"
#include "ConcreteCPU.hpp"
#include "perf/BufferInterface.hpp"
#include "util/ElfReader.hpp"

#include "config/FailConfig.hpp"

namespace fail {

class ExperimentFlow;

// Warning: Inheriting from these listeners breaks the (static) dependency check for
// derived classes. One should therefore deploy a check for derived classes in
// the same way.

/**
 * \class BaseListener
 * This is the base class for all listener types.
 */
class BaseListener {
protected:
	unsigned int m_OccCounter; //!< listener fires when 0 is reached
	unsigned int m_OccCounterInit; //!< initial value for m_OccCounter
	ExperimentFlow* m_Parent; //!< this listener belongs to experiment m_Parent
	index_t m_Loc; //!< location of this listener object within the buffer-list
	PerfBufferBase* m_Home; //!< ptr to performance buffer-list impl. or NULL of not existing
	ConcreteCPU* m_CPU; //!< this listener should only fire for events from this cpu (all cpus if NULL)
public:
	BaseListener(ConcreteCPU* cpu = NULL)
		: m_OccCounter(1), m_OccCounterInit(1), m_Parent(NULL), m_Loc(INVALID_INDEX), m_Home(NULL),
		  m_CPU(cpu)
	{ }
	virtual ~BaseListener();
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
	 * This method is called when an experiment flow removes a listener from
	 * the listener-management by calling \c removeListener(), \c clearListeners()
	 * or by deleting a complete flow (\c removeFlow()). More specifically, this
	 * listener handler will be called *before* the listener is actually deleted.
	 */
	virtual void onDeletion() { }
	/**
	 * This method is called when an previously added listener is about to be
	 * triggered by the simulator-backend. \c onTrigger() toggles the experiment
	 * flow specified by \c BaseListener::getParent(). You can use this handler
	 * as a callback function by overwriting this method appropriately.
	 * @note Re-adding the current listener (= \c this) within onTrigger can be
	 * done using \c addListener(this). When adding *new* listeners (allocated
	 * within onTrigger), you have two options:
	 *  a) overwrite the onTrigger method as well (recommended) or
	 *  b) set a valid experiment flow pointer (using \c setParent()) *before*
	 *     calling \c addListener(). (This pointer can be passed to a new method
	 *     of your derived listener class.)
	 */
	virtual void onTrigger();
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
	unsigned int getCounter() const { return m_OccCounter; }
	/**
	 * Resets the listener counter to its default value, or the last
	 * value that was set through \c setCounter().
	 */
	void resetCounter() { m_OccCounter = m_OccCounterInit; }
	/**
	 * Returns the parent experiment of this listener (context).
	 * If the listener was created temporarily or wasn't linked to a context,
	 * the return value is \c NULL (unknown identity).
	 * @return the listener's parent ptr
	 */
	ExperimentFlow* getParent() const { return m_Parent; }
	/**
	 * Sets the parent (context) of this listener. The default context
	 * is \c NULL (unknown identity).
	 * @param pFlow the new parent ptr or \c NULL (= unknown identity)
	 */
	void setParent(ExperimentFlow* pFlow) { m_Parent = pFlow; }
	/**
	 * Returns the CPU for which the listener should fire it's events. Can be \c NULL for any cpu.
	 */
	ConcreteCPU* getCPU() const { return m_CPU; }
	/**
	 * Sets the CPU for which the listener should fire it's events. \c Null can be used as a
	 * wildcard.
	 */
	void setCPU(ConcreteCPU* cpu) { m_CPU = cpu; }
	/**
	 * Sets the location of this listener within the buffer-list (which itself
	 * part of the buffer-list).
	 * @param idx the new index or \c INVALID_INDEX if not managed by (= added to)
	 *        the ListenerManager.
	 */
	void setLocation(index_t idx) { m_Loc = idx; }
	/**
	 * Returns the current location of this listener in the buffer-list. See
	 * \c setLocation for further details.
	 * @return the location (index) within the buffer-list
	 */
	index_t getLocation() { return m_Loc; }
	/**
	 * Sets the performance buffer(-list), this listener is stored in. Upon calling
	 * this method, the listener should already be stored in the buffer \c pHome.
	 * @param pHome a pointer to the (fast) performance buffer-list, specific to this
	 *        event type
	 */
	void setPerformanceBuffer(PerfBufferBase* pHome) { m_Home = pHome; }
	/**
	 * Retrieves the pointer to the performance buffer-list implementation for
	 * this listener. The returned value may be \c NULL in case of a missing
	 * performance implementation.
	 * @return the ptr to the performance buffer-list, or \c NULL if not existing
	 */
	PerfBufferBase* getPerformanceBuffer() { return m_Home; }
};
// ----------------------------------------------------------------------------
// Specialized listeners:
//

/**
 * \class BListener
 * A Breakpoint listener to observe instruction changes within a given address space.
 */
class BPListener : public BaseListener {
protected:
	BPEvent m_Data;
public:
	/**
	 * Creates a new breakpoint listener. The range information is specific to
	 * the subclasses.
	 * @param address_space the address space to be oberserved.
	 *        The nature if its identifier is implementation-specific.
	 *        For IA-32, it is given as the
	 *        content of the CR3 register. The listener will not be triggered unless
	 *        \a ip is part of the given address space.
	 *        ANY_ADDR can be used as a placeholder to allow debugging
	 *        in a random address space.
	 */
	BPListener(address_t address_space = ANY_ADDR, ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_Data(ANY_ADDR, address_space) { }
	/**
	 * Returns the address space register of this listener.
	 */
	address_t getAddressSpace() const { return m_Data.getAddressSpace(); }
	/**
	 * Sets the address space register for this listener.
	 */
	void setAddressSpace(address_t iptr) { m_Data.setAddressSpace(iptr); }
	/**
	 * Checks whether a given address space is matching.
	 */
	bool aspaceIsMatching(address_t address_space = ANY_ADDR) const;
	/**
	 * Sets the CPU that triggered this listener. Should not be used by experiment code.
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_Data.setTriggerCPU(cpu); }
	/**
	 * Returns the CPU that triggered this listener.
	 */
	ConcreteCPU* getTriggerCPU() { return m_Data.getTriggerCPU(); }
	/**
	 * Returns the instruction pointer that triggered this listener.
	 */
	address_t getTriggerInstructionPointer() const { return m_Data.getTriggerInstructionPointer(); }
	/**
	 * Sets the instruction pointer that triggered this listener.  Should not
	 * be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t iptr) { m_Data.setTriggerInstructionPointer(iptr); }
	/**
	 * Checks whether a given address is matching.
	 * @param pEv Breakpoint event data, retrieved by the simulator
	 * @return \c true if matching, \c false otherwise
	 */
	virtual bool isMatching(const BPEvent* pEv) const = 0;
};

#if defined CONFIG_EVENT_BREAKPOINTS
	#define BP_CTOR_SCOPE public
#else
	#define BP_CTOR_SCOPE protected
	// This prevents an experiment from instantiating an object of BPSingleListener
	// without having enabled the appropriate configuration flag, i.e.,
	// CONFIG_EVENT_BREAKPOINTS = ON.
#endif
/**
 * \class BPSingleListener
 * A Breakpoint listener to observe specific instruction pointers.
 */
class BPSingleListener : public BPListener {
protected:
	address_t m_WatchInstrPtr;
BP_CTOR_SCOPE:
	/**
	 * Creates a new breakpoint listener.
	 * @param ip the instruction pointer of the breakpoint. If the control
	 *        flow reaches this address and its counter value is zero, the
	 *        listener will be triggered. \a ip can be set to the ANY_ADDR
	 *        wildcard to allow arbitrary addresses. Defaults to 0.
	 * @param address_space the address space to be oberserved.
	 *        \see BPListener
	 */
	BPSingleListener(address_t ip = 0, address_t address_space = ANY_ADDR, ConcreteCPU* cpu = NULL)
		: BPListener(address_space, cpu), m_WatchInstrPtr(ip) { }
public: // reset scope in order to allow compiling the various other FAIL* sources
	/**
	 * Returns the instruction pointer this listener waits for.
	 * @return the instruction pointer specified in the constructor or by
	 *         calling \c setWatchInstructionPointer()
	 */
	address_t getWatchInstructionPointer() const { return m_WatchInstrPtr; }
	/**
	 * Sets the instruction pointer this listener waits for.
	 * @param iptr the new instruction ptr to wait for
	 */
	void setWatchInstructionPointer(address_t iptr) { m_WatchInstrPtr = iptr; }
	/**
	 * Checks whether a given address (encapsulated in \c pEv) is matching.
	 * @param pEv address to check, including address space information
	 * @return \c true if address is within the range, \c false otherwise
	 */
	bool isMatching(const BPEvent* pEv) const;
};

#if defined CONFIG_EVENT_BREAKPOINTS_RANGE
	#define BPRANGE_CTOR_SCOPE public
#else
	#define BPRANGE_CTOR_SCOPE protected
#endif
/**
 * \class BPRangeListener
 * A listener type to observe ranges of instruction pointers.
 */
class BPRangeListener : public BPListener {
protected:
	address_t m_WatchStartAddr;
	address_t m_WatchEndAddr;
BPRANGE_CTOR_SCOPE:
	/**
	 * Creates a new breakpoint-range listener.  The range's ends are both
	 * inclusive, i.e. an address matches if start <= addr <= end.
	 * ANY_ADDR denotes the lower respectively the upper end of the address
	 * space.
	 */
	BPRangeListener(address_t start = 0, address_t end = 0, address_t address_space = ANY_ADDR,
					ConcreteCPU* cpu = NULL)
		: BPListener(address_space, cpu), m_WatchStartAddr(start), m_WatchEndAddr(end)
	{ }
public:
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
	bool isMatching(const BPEvent* pEv) const;
};

#if defined CONFIG_EVENT_MEMREAD || defined CONFIG_EVENT_MEMWRITE
	#define WP_CTOR_SCOPE public
#else
	#define WP_CTOR_SCOPE protected
	// Note: "private" works only in case of a "final class" (a leaf class) because when using
	// "private", the derived classes wouldn't compile anymore (even if they are not used anyway).
	// Clearly, MemAccessListener is *not* a leaf class.
#endif
/**
 * \class MemAccessListener
 * Observes memory read/write accesses.
 */
class MemAccessListener : public BaseListener {
protected:
	//! Specific physical guest system address to watch, or ANY_ADDR.
	address_t m_WatchAddr;
	//! Width of the memory area being watched (# bytes).
	size_t m_WatchWidth;
	/**
	 * Memory access type we want to watch
	 * (MEM_READ || MEM_WRITE || MEM_READWRITE).
	 */
	MemAccessEvent::access_type_t m_WatchType;
	MemAccessEvent m_Data;
WP_CTOR_SCOPE:
	MemAccessListener(MemAccessEvent::access_type_t type = MemAccessEvent::MEM_READWRITE,
					  ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_WatchAddr(ANY_ADDR), m_WatchWidth(1), m_WatchType(type) { }
	MemAccessListener(address_t addr,
	                  MemAccessEvent::access_type_t type = MemAccessEvent::MEM_READWRITE,
					  ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_WatchAddr(addr), m_WatchWidth(1), m_WatchType(type) { }
	MemAccessListener(const ElfSymbol &symbol,
	                  MemAccessEvent::access_type_t type = MemAccessEvent::MEM_READWRITE,
					  ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_WatchAddr(symbol.getAddress()), m_WatchWidth(symbol.getSize()), m_WatchType(type) { }
public:
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
	 * Sets the CPU that triggered this listener. Should not be used by experiment code.
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_Data.setTriggerCPU(cpu); }
	/**
	 * Returns the CPU that triggered this listener.
	 */
	ConcreteCPU* getTriggerCPU() { return m_Data.getTriggerCPU(); }
	/**
	 * Returns the specific physical memory address that actually triggered the
	 * listener.
	 */
	address_t getTriggerAddress() const { return m_Data.getTriggerAddress(); }
	/**
	 * Sets the specific physical memory address that actually triggered the
	 * listener.  Should not be used by experiment code.
	 */
	void setTriggerAddress(address_t addr) { m_Data.setTriggerAddress(addr); }
	/**
	 * Returns the width (in bytes) of the memory access that triggered this
	 * listener.
	 */
	size_t getTriggerWidth() const { return m_Data.getTriggerWidth(); }
	/**
	 * Sets the width (in bytes) of the memory access that triggered this
	 * listener.  Should not be used by experiment code.
	 */
	void setTriggerWidth(size_t width) { m_Data.setTriggerWidth(width); }
	/**
	 * Returns the address of the instruction causing this memory access.
	 */
	address_t getTriggerInstructionPointer() const { return m_Data.getTriggerInstructionPointer(); }
	/**
	 * Sets the address of the instruction causing this memory access.  Should
	 * not be used by experiment code.
	 */
	void setTriggerInstructionPointer(address_t addr) { m_Data.setTriggerInstructionPointer(addr); }
	/**
	 * Returns type (MEM_READ || MEM_WRITE) of the memory access that triggered
	 * this listener.
	 */
	MemAccessEvent::access_type_t getTriggerAccessType() const
	{ return m_Data.getTriggerAccessType(); }
	/**
	 * Sets the type of the memory access that triggered this listener.
	 * Should not be used by experiment code.
	 */
	void setTriggerAccessType(MemAccessEvent::access_type_t type)
	{ m_Data.setTriggerAccessType(type); }
	/**
	 * Returns memory access types (MEM_READ || MEM_WRITE || MEM_READWRITE)
	 * this listener watches.  Should not be used by experiment code.
	 */
	MemAccessEvent::access_type_t getWatchAccessType() const { return m_WatchType; }
	/**
	 * Checks whether a given physical memory access is matching.
	 */
	bool isMatching(const MemAccessEvent* pEv) const;
};

#ifdef CONFIG_EVENT_MEMREAD
	#define WPREAD_CTOR_SCOPE public
#else
	#define WPREAD_CTOR_SCOPE protected
#endif
/**
 * \class MemReadListener
 * Observes memory read accesses.
 */
class MemReadListener : public MemAccessListener {
WPREAD_CTOR_SCOPE:
	MemReadListener(ConcreteCPU* cpu = NULL)
		: MemAccessListener(MemAccessEvent::MEM_READ, cpu) { }
	MemReadListener(address_t addr, ConcreteCPU* cpu = NULL)
		: MemAccessListener(addr, MemAccessEvent::MEM_READ, cpu) { }
};

#ifdef CONFIG_EVENT_MEMWRITE
	#define WPWRITE_CTOR_SCOPE public
#else
	#define WPWRITE_CTOR_SCOPE protected
#endif
/**
 * \class MemWriteListener
 * Observes memory write accesses.
 */
class MemWriteListener : public MemAccessListener {
WPWRITE_CTOR_SCOPE:
	MemWriteListener(ConcreteCPU* cpu = NULL)
		: MemAccessListener(MemAccessEvent::MEM_WRITE, cpu) { }
	MemWriteListener(address_t addr, ConcreteCPU* cpu = NULL)
		: MemAccessListener(addr, MemAccessEvent::MEM_WRITE, cpu) { }
};

#if defined CONFIG_EVENT_INTERRUPT || defined CONFIG_EVENT_TRAP
	#define TROUBLE_CTOR_SCOPE public
#else
	#define TROUBLE_CTOR_SCOPE protected
#endif
/**
 * \class TroubleListener
 * Observes interrupt/trap activties.
 */
class TroubleListener : public BaseListener {
protected:
	TroubleEvent m_Data; //!< event related data, e.g. trap number
	/**
	 * Specific guest system interrupt/trap numbers to watch,
	 * or ANY_INTERRUPT/ANY_TRAP.
	 */
	std::vector<unsigned> m_WatchNumbers;
TROUBLE_CTOR_SCOPE:
	TroubleListener(ConcreteCPU* cpu = NULL) : BaseListener(cpu) { }
	TroubleListener(unsigned troubleNumber, ConcreteCPU* cpu = NULL) : BaseListener(cpu)
	{ addWatchNumber(troubleNumber); }
public:
	/**
	 * Add an interrupt/trap-number which should be observed.
	 * @param troubleNumber number of an interrupt or trap
	 * @return \c true if number is added to the list \c false if the number
	 *         was already in the list
	 */
	bool addWatchNumber(unsigned troubleNumber);
	/**
	 * Remove an interrupt/trap-number which is in the list of observed
	 * numbers.
	 * @param troubleNumber number of an interrupt or trap
	 * @return \c true if the number was found and removed \c false if the
	 *         number was not in the list
	 */
	bool removeWatchNumber(unsigned troubleNum);
	/**
	 * Returns the list of observed numbers.
	 * @return a const reference to the list which contains all observed numbers
	 */
	const std::vector<unsigned>& getWatchNumbers() const { return m_WatchNumbers; }
	/**
	* Checks whether a given interrupt-/trap-number is matching.
	*/
	bool isMatching(const TroubleEvent* pEv) const;
	/**
	 * Sets the CPU that triggered this listener. Should not be used by experiment code.
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_Data.setTriggerCPU(cpu); }
	/**
	 * Returns the CPU that triggered this listener.
	 */
	ConcreteCPU* getTriggerCPU() { return m_Data.getTriggerCPU(); }
	/**
	* Sets the specific interrupt-/trap-number that actually triggered
	* the listener. Should not be used by experiment code.
	*/
	void setTriggerNumber(unsigned troubleNum) { m_Data.setTriggerNumber(troubleNum); }
	/**
	* Returns the specific interrupt-/trap-number that actually triggered
	* the listener.
	*/
	unsigned getTriggerNumber() const { return m_Data.getTriggerNumber(); }
};

#ifdef CONFIG_EVENT_INTERRUPT
	#define INT_CTOR_SCOPE public
#else
	#define INT_CTOR_SCOPE protected
#endif
/**
 * \class InterruptListener
 * Observes interrupts of the guest system.
 */
class InterruptListener : public TroubleListener {
protected:
	InterruptEvent m_Data; //!< event related data, e.g. NMI flag
INT_CTOR_SCOPE:
	InterruptListener(ConcreteCPU* cpu = NULL) : TroubleListener(cpu) { }
	InterruptListener(unsigned interrupt, ConcreteCPU* cpu = NULL) : TroubleListener(cpu)
	{ addWatchNumber(interrupt); }
public:
	/**
	 * Returns \c true if the interrupt is non maskable, \c false otherwise.
	 */
	bool isNMI() const { return m_Data.isNMI(); }
	/**
	 * Sets the interrupt type (non maskable or not).
	 */
	void setNMI(bool enabled) { m_Data.setNMI(enabled); }
};

#ifdef CONFIG_EVENT_TRAP
	#define TRAP_CTOR_SCOPE public
#else
	#define TRAP_CTOR_SCOPE protected
#endif
/**
 * \class TrapListener
 * Observes traps of the guest system.
 */
class TrapListener : public TroubleListener {
TRAP_CTOR_SCOPE:
	TrapListener(ConcreteCPU* cpu = NULL) : TroubleListener(cpu) { }
	TrapListener(unsigned trap, ConcreteCPU* cpu = NULL) : TroubleListener(cpu)
	{ addWatchNumber(trap); }
};

#ifdef CONFIG_EVENT_GUESTSYS
	#define GUESTSYS_CTOR_SCOPE public
#else
	#define GUESTSYS_CTOR_SCOPE protected
#endif
/**
 * \class GuestListener
 * Used to receive data from the guest system.
 */
// FIXME: This is not a "clean design" ... IOPortListener looks much like a copy of this class.
//        Additionaly, the port is fixed (at least in Bochs) but can be modified using setPort
//        (effectless for now).
class GuestListener : public BaseListener {
protected:
	GuestEvent m_Data;
GUESTSYS_CTOR_SCOPE:
	GuestListener() { }
public:
	/**
	 * Returns the data, transmitted by the guest system.
	 */
	char getData() const { return m_Data.getData(); }
	/**
	 * Sets the data which had been transmitted by the guest system.
	 */
	void setData(char data) { m_Data.setData(data); }
	/**
	 * Returns the data length, transmitted by the guest system.
	 */
	unsigned getPort() const { return m_Data.getPort(); }
	/**
	 * Sets the data length which had been transmitted by the guest system.
	 */
	void setPort(unsigned port) { m_Data.setPort(port); }
};

#ifdef CONFIG_EVENT_IOPORT
	#define IOPORT_CTOR_SCOPE public
#else
	#define IOPORT_CTOR_SCOPE protected
#endif
/**
 * \class IOPortListener
 * Observes I/O access on architectures with a separate I/O access mechanism (e.g. IA-32)
 */
class IOPortListener : public BaseListener {
protected:
	IOPortEvent m_Data;
	unsigned m_Port;
	bool m_Out;
IOPORT_CTOR_SCOPE:
	/**
	 * Initialises an IOPortListener
	 *
	 * @param port the port the listener is listening on
	 * @param out Defines the direction of the listener.
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	IOPortListener(unsigned port, bool out, ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_Port(port), m_Out(out) { }
public:
	/**
	 * Returns the data sent to the specified port
	 */
	unsigned char getData() const { return m_Data.getData(); }
	/**
	 * Sets the data which had been transmitted.
	 */
	void setData(unsigned char data) { m_Data.setData(data); }
	/**
	 * Retrieves the port which this listener is bound to.
	 */
	unsigned getPort() const { return m_Port; }
	/**
	 * Sets the port which this listener is bound to.
	 */
	void setPort(unsigned port) { m_Port = port; }
	/**
	 * Sets the CPU that triggered this listener. Should not be used by experiment code.
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_Data.setTriggerCPU(cpu); }
	/**
	 * Returns the CPU that triggered this listener.
	 */
	ConcreteCPU* getTriggerCPU() { return m_Data.getTriggerCPU(); }
	/**
	* Checks whether a given port number is matching.
	* @param p The port number an I/O listener occured on
	* @param out True if the communication was outbound, false otherwise
	*/
	bool isMatching(unsigned p, bool out) const { return out == isOutListener() && p == getPort(); }
	/**
	 * Tells you if this listener is capturing outbound communication (inbound if false)
	 */
	bool isOutListener() const { return m_Out; }
	/**
	 * Change the listener direction.
	 * @param out direction flag
	 * \arg \c true Output on the given port is captured.
	 * \arg \c false Input on the given port is captured.
	 */
	void setOut(bool out) { m_Out = out; }
};

#ifdef CONFIG_EVENT_JUMP
	#define JUMP_CTOR_SCOPE public
#else
	#define JUMP_CTOR_SCOPE protected
#endif
/**
 * \class JumpListener
 * JumpListeners are used to observe conditional jumps (if...else if...else).
 */
class JumpListener : public BaseListener {
protected:
	JumpEvent m_Data;
JUMP_CTOR_SCOPE:
	/**
	 * Constructs a new listener object.
	 * @param opcode the opcode of the jump-instruction to be observed
	 *        or \c ANY_INSTR to match all jump-instructions
	 */
	JumpListener(unsigned opcode = ANY_INSTR, ConcreteCPU* cpu = NULL)
		: BaseListener(cpu), m_Data(opcode) { }
public:
	/**
	 * Retrieves the opcode of the jump-instruction.
	 */
	unsigned getOpcode() const { return m_Data.getTriggerOpcode(); }
	/**
	 * Sets the CPU that triggered this listener. Should not be used by experiment code.
	 */
	void setTriggerCPU(ConcreteCPU* cpu) { m_Data.setTriggerCPU(cpu); }
	/**
	 * Returns the CPU that triggered this listener.
	 */
	ConcreteCPU* getTriggerCPU() { return m_Data.getTriggerCPU(); }
	/**
	 * Returns \c true, if the listener was triggered due to specific register
	 * content, \c false otherwise.
	 */
	bool isRegisterTriggered() const { return m_Data.isRegisterTriggered(); }
	/**
	 * Returns \c true, of the listener was triggered due to specific FLAG
	 * state, \c false otherwise. This is the common case.
	 */
	bool isFlagTriggered() const { return m_Data.isFlagTriggered(); }
	/**
	 * Sets the requested jump-instruction opcode.
	 */
	void setOpcode(unsigned oc) { m_Data.setTriggerOpcode(oc); }
	/**
	 * Sets the trigger flag.
	 */
	void setFlagTriggered(bool flagTriggered) { m_Data.setFlagTriggered(flagTriggered); }
};

/**
 * \class TimerListener
 * This listener type is used to create timeouts within in an experiment.
 *
 * Depending on your simulator backend, a concrete class needs to be derived from
 * \c TimerListener. \c onAddition should be used to register and \c onDeletion
 * to unregister the timer. \c onTrigger can be used to re-register the timer if a
 * repetitive timer is requested and the back-end doesn't support such timer types
 * natively.
 */
class TimerListener : public BaseListener {
protected:
	unsigned m_Timeout; //!< timeout interval in microseconds
	TimerEvent m_Data;
public:
	/**
	 * Creates a new timer listener. This can be used to implement a timeout-
	 * mechanism in the experiment-flow. The timer starts automatically when
	 * added to the simulator backend.
	 * @param timeout the (simulated) time interval in microseconds
	 * @see SimulatorController::addListener
	 */
	TimerListener(unsigned timeout) : m_Timeout(timeout) { }
	~TimerListener() { }
	/**
	 * Retrieves the internal timer id. Maybe useful for debug output.
	 * @return the timer id
	 */
	timer_id_t getId() const { return m_Data.getId(); }
	/**
	 * Sets the internal timer id. This should not be used by the experiment.
	 * @param id the new timer id, given by the underlying simulator-backend
	 */
	void setId(timer_id_t id) { m_Data.setId(id); }
	/**
	 * Retrieves the timer's timeout value.
	 * @return the timeout in microseconds
	 */
	unsigned getTimeout() const { return m_Timeout; }
	/**
	 * Set the timeout value. Returns the old timeout value. Be aware,
	 * that this method might have no effects, after the listener was added.
	 * @return the old timeout in microseconds
	 */
	unsigned setTimeout(unsigned timeout) {
		unsigned tmp = m_Timeout;
		m_Timeout = timeout;
		return tmp;
	}

};

} // end-of-namespace: fail

#endif // __LISTENER_HPP__
