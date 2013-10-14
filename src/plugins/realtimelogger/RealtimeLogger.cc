#include "RealtimeLogger.hpp"
#include "sal/Listener.hpp"
#include "sal/Memory.hpp"

using namespace std;
using namespace fail;

bool RealtimeLogger::run()
{
	MemoryManager& mm = simulator.getMemoryManager();

	uint32_t value; //! @todo more generic datatype

	m_log << "Realtime Logger started. Listening to: " << m_symbol << std::endl;
	MemAccessListener ev_mem(m_symbol.getAddress());

	if (m_ostream.is_open()) {
		m_log << "Writing output to: " << m_outputfile << std::endl;
		while (true) {
			simulator.addListenerAndResume(&ev_mem);
			/* A Memory Accesses happend: Get simulation time and log it */
			fail::simtime_t simtime = simulator.getTimerTicks();
			mm.getBytes(m_symbol.getAddress(), m_symbol.getSize(), &value);
			handleEvent(simtime, value);
		}
	} else {
		m_log << "No output file."  << std::endl;
	}
	return true;
}

void RealtimeLogger::handleEvent(fail::simtime_t simtime, uint32_t value)
{
	simtime_t per_second = simulator.getTimerTicksPerSecond();
	double sec = (double)simtime / per_second;
	double msec = sec*1000;
	if (m_ostream.is_open()) {
		m_ostream << std::dec << msec << ";" << value << std::endl;
	}
}

