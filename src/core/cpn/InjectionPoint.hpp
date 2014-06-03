#ifndef INJECTIONPOINT_HPP_
#define INJECTIONPOINT_HPP_

#include "comm/DatabaseCampaignMessage.pb.h"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

#include <vector>

namespace fail {

/**
 * \class InjectionPointBase
 *
 * Interface for a generic InjectionPoint (e.g. trace instruction offset or
 * hop chain) for use in the generic DatabaseCampaign. Every
 * DataBaseCampaignMessage contains a concrete InjectionPoint.
 */
class InjectionPointBase {
protected:
	InjectionPointMessage m_ip;	// !< Protobuf message of concrete injection point, this class wraps
	Logger m_log;
public:
	InjectionPointBase() : m_log("InjectionPoint") {}

	/**
	 * Parses generic representation from a injection instruction (trace offset).
	 * Used by server.
	 * @param inj_instr trace instruction offset to be parsed
	 */
	virtual void parseFromInjectionInstr(unsigned instr1, unsigned instr2) = 0;

	/**
	 * Parses (extracts) generic representation from a DatabaseCampaignMessage.
	 * Used by client.
	 * @param pilot DatabaseCampaignMessage which contains a InjectionPoint
	 */
	void parseFromCampaignMessage(const DatabaseCampaignMessage &pilot) {m_ip.CopyFrom(pilot.injection_point());}

	/**
	 * Adds generic representation to a DatabaseCampaignMessage
	 * @param pilot DatabaseCampaignMessage to which the generic protobuf representation will be added
	 */
	void addToCampaignMessage(DatabaseCampaignMessage &pilot) {pilot.mutable_injection_point()->CopyFrom(m_ip);}

	/**
	 * Get a copy of the contained InjectionPointMessage
	 * @param ipm Generic InjectionPointMessage to which the content will be copied
	 */
	void copyInjectionPointMessage(InjectionPointMessage &ipm) {ipm.CopyFrom(m_ip);}
};

#ifdef CONFIG_INJECTIONPOINT_HOPS
#include "comm/InjectionPointHopsMessage.pb.h"

class SmartHops;

/**
 * \class InjectionPointHops
 *
 * Concrete injection point which contains a hop chain to the target
 * trace instruction.
 */
class InjectionPointHops : public InjectionPointBase {
private:
	SmartHops *m_sa;	// !< Hop calculator which generates the hop chain

	// Boundaries must be signed to ensure, they can be initialized as outside of beginning
	// of the trace (instr is -1).
	long m_curr_instr1;		// !< Lower end of instructions for which currently a hop chain is available
	long m_curr_instr2;		// !< Upper end of instructions for which currently a hop chain is available

	bool m_initialized;
	std::vector<InjectionPointMessage> m_results;

	void init();
public:
	InjectionPointHops() : InjectionPointBase(), m_sa(NULL), m_curr_instr1(-1),
							m_curr_instr2(-1), m_initialized(false) {}

	virtual ~InjectionPointHops();

	/**
	 * Parses a hop chain from a injection instruction (trace offset).
	 * @param instr1 trace instruction offset of beginning of equivalence class
	 * @param instr2 trace instruction offset of ending of equivalence class
	 */
	virtual void parseFromInjectionInstr(unsigned instr1, unsigned instr2);
};

typedef InjectionPointHops ConcreteInjectionPoint;

#else
#include "comm/InjectionPointStepsMessage.pb.h"

/**
 * \class InjectionPointSteps
 *
 * Concrete injection point which contains a trace instruction offset.
 */
class InjectionPointSteps : public InjectionPointBase {
public:
	virtual ~InjectionPointSteps() {}

	/**
	 * Parses a trace offset from a injection instruction (trace offset),
	 * so it effectively just stores the value in the protobuf message.
	 * @param instr1 trace instruction offset of beginning of equivalence class
	 * @param instr2 trace instruction offset of ending of equivalence class
	 */
	virtual void parseFromInjectionInstr(unsigned instr1, unsigned instr2);
};

typedef InjectionPointSteps ConcreteInjectionPoint;
#endif

} /* namespace fail */

#endif /* INJECTIONPOINT_HPP_ */
