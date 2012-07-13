#include <iostream>

#include "SerialOutput.hpp"

using namespace std;
using namespace fail;

bool SerialOutput::run()
{
	IOPortListener ev_ioport(m_port, m_out);
	BaseListener *ev;
	while (true) {
		simulator.addListener(&ev_ioport);
		ev = simulator.resume();
		simulator.removeListener(&ev_ioport);
		if (ev == &ev_ioport) {
			m_output += ev_ioport.getData();
		} else {
			break;
		}
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
