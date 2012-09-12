#include <iostream>

#include "QEMUListener.hpp"
#include "../SALInst.hpp"

extern "C" {
#include "qemu/failqemu.h"
}

namespace fail {

bool QEMUMemWriteListener::onAddition()
{
	std::cout << "QEMUMemWriteListener::onAddition" << std::endl;
	if (failqemu_add_watchpoint(simulator.m_cpuenv, m_WatchAddr, m_WatchWidth, 1) != 0) {
		std::cout << "adding watchpoint failed!" << std::endl;
		return false;
	}
	return true;
}

void QEMUMemWriteListener::onDeletion()
{
	std::cout << "QEMUMemWriteListener::onDeletion" << std::endl;
	failqemu_remove_watchpoint(simulator.m_cpuenv, m_WatchAddr, m_WatchWidth, 1);
}

} // end-of-namespace: fail
