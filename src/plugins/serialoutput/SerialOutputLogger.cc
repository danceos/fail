#include "SerialOutputLogger.hpp"
#include "sal/Listener.hpp"

using namespace std;
using namespace fail;

bool SerialOutputLogger::run()
{
	IOPortListener ev_ioport(m_port, m_out);
	while (true) {
		simulator.addListener(&ev_ioport);
		simulator.resume();
		if (m_limit == 0 || m_output.size() < m_limit) {
			m_output += ev_ioport.getData();
		}
	}
	return true;
}

void SerialOutputLogger::resetOutput()
{
	m_output.clear();
}

string SerialOutputLogger::getOutput()
{
	return m_output;
}
