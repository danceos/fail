#include "InjectionPoint.hpp"
#include "util/smarthops/SmartHops.hpp"

namespace fail {

InjectionPointHops::InjectionPointHops() : InjectionPointBase(), m_sa(NULL), m_curr_inst(0), m_initialized(false)  {
}


InjectionPointHops::~InjectionPointHops() {
	if (m_initialized)
		delete m_sa;
}

void InjectionPointHops::init()
{
	m_sa = new SmartHops();
	m_curr_inst = 0;
	char * elfpath = getenv("FAIL_TRACE_PATH");
	if(elfpath == NULL){
		m_log << "FAIL_TRACE_PATH not set :(" << std::endl;
		exit(-1);
	}else{
		m_sa->init((const char*)elfpath);
	}

	m_initialized = true;
}

void InjectionPointHops::parseFromInjectionInstr(unsigned inj_instr) {
	if (!m_initialized) {
		init();
	}

	// Already calculated result needed?
	if (m_curr_inst == inj_instr) {
		return;
	}
	
	// Non monotonic ascending injection instruction given?
	if (m_curr_inst > inj_instr) {
		m_log << "FATAL ERROR: Got not monotonic ascending values of injection instruction" << std::endl;
		exit(-1);
	}

	if (!m_sa->calculateFollowingHop(m_ip, inj_instr)) {
		m_log << "FATAL ERROR: Trace does not contain enough instructions (" << inj_instr << ")" << std::endl;
		exit(-1);
	}
	m_curr_inst = inj_instr;
}

} /* namespace fail */
