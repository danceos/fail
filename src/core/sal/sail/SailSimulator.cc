#include "SailSimulator.hpp"
#include "../Listener.hpp"
#include <fstream>
#include "SailCPU.hpp"

using namespace fail;

SailBaseSimulator::SailBaseSimulator()
	: SimulatorController() {
	setMemoryManager(&m_mem_manager);
	addCPU(new ConcreteCPU(0));
}

SailBaseSimulator::~SailBaseSimulator() {
	auto it = m_CPUs.begin();
	while (it != m_CPUs.end()) {
		delete *it;
		it = m_CPUs.erase(it);
	}
}

void SailBaseSimulator::do_save(const std::string& base) {
	// create base directory if it does not exist
	// XXX: c++17 use <fs>
	assert(base.size() > 0 && "FATAL ERROR: tried to save state without valid path");
	std::string mkdirState("mkdir -p ");
	mkdirState.append(base);
	int status = system(mkdirState.c_str());
	if (status == -1) {
		throw std::runtime_error("[FAIL] Error: do_save() directory creation failed");
	}

	// serialize registers
	std::string reg_base = base + "/registers";
	for(const auto c: m_CPUs) {
		std::ofstream f { reg_base + std::to_string(c->getId())};
		if (!f.is_open()) throw std::runtime_error("[FAIL] couldnt open register file for writing");
		c->serialize(f);
	}
	// serialize memory
	std::ofstream fm { base + "/memory", std::ios::binary };
	if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open memory file for writing");
	m_mem_manager.serialize(fm);

}
void SailBaseSimulator::do_restore(const std::string& base) {
	// unserialize registers
	std::string reg_base = base + "/registers";
	for( const auto c: m_CPUs ) {
		std::ifstream f { reg_base + std::to_string(c->getId()) };
		if (!f.is_open()) throw std::runtime_error("[FAIL] couldnt open register file for reading");
		c->unserialize(f);
	}

	// unserialize memory
	std::ifstream fm { base + "/memory", std::ios::binary };
	if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open memory file for reading");
	m_mem_manager.unserialize(fm);
}


bool SailBaseSimulator::save(const std::string& path) {
#if defined(CONFIG_SR_SAVE)
	queueRequest(req_type::save, path);
	return true;
#else
	return false;
#endif
}

void SailBaseSimulator::restore(const std::string& path) {
#if defined(CONFIG_SR_RESTORE)
	clearListeners();
	queueRequest(req_type::restore, path);
#endif
}

void SailBaseSimulator::reboot() {
	m_mem_manager.reset();
	for(const auto c: m_CPUs)
		c->reset();
}

void SailBaseSimulator::executeRequests() {
	while(!requests.empty()) {
		auto e = requests.front();
		requests.pop_front();
		if(e.t == req_type::save)
			do_save(e.path);
		else if(e.t == req_type::restore)
			do_restore(e.path);
		m_Flows.toggle(e.parent);
	}
}

void SailBaseSimulator::checkTimers() {
	const auto ticks = getTimerTicks();

	ListenerManager::iterator it = m_LstList.begin();
	while(it != m_LstList.end()) {
		BaseListener* pev = *it;
		if(auto t = dynamic_cast<TimerListener*>(pev)) {
			if(t->getTimeout() <= ticks) {
				std::cout << "xxx" << t->getTimeout() << " timer has gone off" << std::endl;
				it = m_LstList.makeActive(it);
				continue;
			}
		}
		it++;
	}
	m_LstList.triggerActiveListeners();
}

simtime_t SailBaseSimulator::getTimerTicks() {
	return getCPU(0).getInstructionCounter();
}

simtime_t SailBaseSimulator::getTimerTicksPerSecond() {
	return getCPU(0).getFrequency();
}

