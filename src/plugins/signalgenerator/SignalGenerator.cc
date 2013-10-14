#include "SignalGenerator.hpp"
#include "sal/Listener.hpp"
#include "sal/Memory.hpp"

using namespace std;
using namespace fail;

bool SignalGenerator::run()
{
	m_log << "Signalgenerator started. @ " << m_symbol << std::endl;

	MemoryManager& mm = simulator.getMemoryManager();
	MemReadListener l_mem(m_symbol.getAddress());

	/**
	 * for EZS we currently use 8bit ADCs.
	 * @todo Make more versatile for future use.
	 */
	uint8_t value;

	while (true) {
		simulator.addListenerAndResume(&l_mem);
		value = handleEvent();
		mm.setBytes(m_symbol.getAddress(), sizeof(value), &value);
	}
	return true;
}

uint8_t SignalGenerator::handleEvent(void)
{
	double val = m_signal->calculate();
	// Scale to uint8: 0 .. 255
	val = val * 127; // -1: -127 .. +1:  +127
	val = val + 127; // -127: 0 .. +127 : 254
	return (uint8_t) val;
}



Sine::Sine(const SineParams_t param)
{
	m_params.push_back(param);
}

double Sine::calculate() const
{
	simtime_t tps = ticksPerSecond();
	if (tps == 0) {
		// Simulator speed not valid.
		return 0;
	}

	// Get simulation time in seconds.
	double sec = (double)simulator.getTimerTicks() / tps;

	// Sum up all sine waves
	double val = 0;
	for (Sine::SineParamsList_t::const_iterator it = m_params.begin();
			it != m_params.end(); it++)
	{
		val += it->amplitude * sinus(it->freq_in_hz, sec);
	}
	return val;
}
