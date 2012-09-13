#include <iostream>

#include "QEMUListener.hpp"
#include "../SALInst.hpp"

extern "C" {
#include "qemu/failqemu.h"
}

namespace fail {

bool QEMUMemWriteListener::onAddition()
{
	//std::cout << "QEMUMemWriteListener::onAddition" << std::endl;
	if (failqemu_add_watchpoint(simulator.m_cpuenv, m_WatchAddr, m_WatchWidth, 1) != 0) {
		std::cout << "adding watchpoint failed!" << std::endl;
		return false;
	}
	return true;
}

void QEMUMemWriteListener::onDeletion()
{
	//std::cout << "QEMUMemWriteListener::onDeletion" << std::endl;
	failqemu_remove_watchpoint(simulator.m_cpuenv, m_WatchAddr, m_WatchWidth, 1);
}

bool QEMUTimerListener::onAddition()
{
	//std::cout << "QEMUTimerListener::onAddition" << std::endl;
	setId(failqemu_register_timer(getTimeout(), (void *)this));
	//std::cout << "this = " << std::hex << (unsigned) this << std::endl;
	//std::cout << "id = " << std::hex << (unsigned) getId() << std::endl;
	return true;
}

void QEMUTimerListener::onDeletion()
{
	//std::cout << "QEMUTimerListener::onDeletion" << std::endl;
	//std::cout << "this = " << std::hex << (unsigned) this << std::endl;
	//std::cout << "id = " << std::hex << (unsigned) getId() << std::endl;
	failqemu_unregister_timer(getId());
}

} // end-of-namespace: fail
