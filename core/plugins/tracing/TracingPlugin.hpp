#ifndef __TRACING_PLUGIN_HPP__
  #define __TRACING_PLUGIN_HPP__

#include <ostream>
#include "controller/ExperimentFlow.hpp"
#include "util/MemoryMap.hpp"

#include "plugins/tracing/trace.pb.h"

/**
 * \class TracingPlugin
 *
 * \brief Plugin to record instruction traces and memory accesses.
 *
 * This plugin is supposed to be instantiated, configured and run by
 * experiments needing instruction or memory access traces.  Tracing can be
 * restricted to a memory or instruction address map; the restrictions are
 * applied together, i.e., a memory access is only logged if neither its
 * instruction address nor its memory address is restricted.
 *
 * TODO: document usage by example
 * FIXME: handle configuration changes after tracing start properly
 * FIXME: more explicit startup/shutdown; listener-based event interface needed?
 *        -> should simulator.removeFlow make sure all remaining active events
 *           are delivered?
 * FIXME: trace a sequence of pb messages, not a giant single one (pb weren't
 *        made for huge messages)
 * FIXME: destructor -> removeFlow?
 */
class TracingPlugin : public fi::ExperimentFlow
{
private:
	MemoryMap *m_memMap; //!< address restriction for memory accesses
	MemoryMap *m_ipMap; //!< instruction address restriction
	bool m_memonly; //!< log instructions only if they are memory accesses
	bool m_iponly; //!< log instruction addresses only

	Trace *m_trace; //!< protobuf message to store trace in
	std::ostream *m_os; //!< ostream to write human-readable trace into

public:
	TracingPlugin()
	 : m_memMap(0), m_ipMap(0), m_memonly(false), m_iponly(false),
	   m_trace(0), m_os(0) { }
	bool run();
	/**
	 * Restricts tracing to memory addresses listed in this MemoryMap.  An
	 * access wider than 8 bit *is* logged if *one* of the bytes it
	 * reads/writes is listed.
	 */
	void restrictMemoryAddresses(MemoryMap *mm) { m_memMap = mm; }
	/**
	 * Restricts tracing to instruction addresses listed in this MemoryMap.
	 * This restriction currently silently assumes instructions are only
	 * one byte wide; make sure your memory map covers this first byte of
	 * the instructions you want to trace.
	 */
	void restrictInstructionAddresses(MemoryMap *mm) { m_ipMap = mm; }
	/**
	 * If invoked with memonly=true, instructions are only logged if they
	 * conducted a memory access.  Defaults to false: All instructions are
	 * logged.
	 */
	void setLogMemOnly(bool memonly) { m_memonly = memonly; }
	/**
	 * If invoked with iponly=true, only instruction addresses are logged.
	 */
	void setLogIPOnly(bool iponly) { m_iponly = iponly; }
	/**
	 * ostream to trace into (human-readable)
	 */
	void setOstream(std::ostream *os) { m_os = os; }
	/**
	 * Protobuf message to trace into (trace.proto instance)
	 */
	void setTraceMessage(Trace *trace) { m_trace = trace; }
};

#endif /* __TRACING_PLUGIN_HPP__ */
