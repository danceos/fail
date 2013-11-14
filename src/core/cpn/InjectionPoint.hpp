#ifndef INJECTIONPOINT_HPP_
#define INJECTIONPOINT_HPP_

#include "comm/DatabaseCampaignMessage.pb.h"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"

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
	virtual void parseFromInjectionInstr(unsigned inj_instr) = 0;

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
	uint32_t m_curr_inst;	// !< Instruction for which currently a hop chain is available
public:
	InjectionPointHops();
	virtual ~InjectionPointHops();
	
	/**
	 * Parses a hop chain from a injection instruction (trace offset).
	 * @param inj_instr trace instruction offset to be parsed
	 */
	virtual void parseFromInjectionInstr(unsigned inj_instr);
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
	 * @param inj_instr trace instruction offset to be parsed
	 */
	virtual void parseFromInjectionInstr(unsigned inj_instr);
};

typedef InjectionPointSteps ConcreteInjectionPoint;
#endif

} /* namespace fail */

#endif /* INJECTIONPOINT_HPP_ */
