#include "RandomGenerator.hpp"
#include "sal/Listener.hpp"
#include "sal/Memory.hpp"

using namespace std;
using namespace fail;

bool RandomGenerator::run()
{
	m_log << "RandomGenerator started. @ " << m_symbol << std::endl;

	MemoryManager& mm = simulator.getMemoryManager();
	MemReadListener l_mem(m_symbol.getAddress());

	uint32_t value = m_seed;

	while (true) {
		simulator.addListenerAndResume(&l_mem);

		value = value * 1103515245 + 12345;

		mm.setBytes(m_symbol.getAddress(), sizeof(value), &value);
	}
	return true;
}
