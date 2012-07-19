#include "SerialOutput.hpp"

using namespace std;
using namespace fail;

bool SerialOutput::run()
{
	IOPortListener ev_ioport(m_port, m_out);
	while (true) {
		simulator.addListener(&ev_ioport);
		simulator.resume();
		m_output += ev_ioport.getData();
	}
	return true;
}

void SerialOutput::resetOutput()
{
	m_output.clear();
}

string SerialOutput::getOutput()
{
	return m_output;
}
