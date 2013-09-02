#include "../SALInst.hpp"
#include "Gem5Wrapper.hpp"

#include "sim/system.hh"
#include "mem/packet.hh"
#include "mem/physical.hh"

namespace fail {

// Register-/Memory-related:
regdata_t GetRegisterContent(System* sys, unsigned int id, size_t idx)
{
	// necessary because gem5 register IDs are not unique
	if (idx < RI_INTREGS_MAX) {
		switch (idx) {
		case RI_IP: return sys->getThreadContext(id)->pcState().pc();
		default: return sys->getThreadContext(id)->readIntReg(idx);
		}
	} else {
		return sys->getThreadContext(id)->readMiscReg(idx - RI_INTREGS_MAX);
	}
	return 0;
}

void SetRegisterContent(System* sys, unsigned int id, size_t idx, regdata_t value)
{
	// necessary because gem5 register IDs are not unique
	if (idx < RI_INTREGS_MAX) {
		switch (idx) {
		case RI_IP: sys->getThreadContext(id)->pcState().pc(static_cast<Addr>(value)); break;
		default: sys->getThreadContext(id)->setIntReg(idx, value);
		}
	} else {
		sys->getThreadContext(id)->setMiscReg(idx - RI_INTREGS_MAX, value);
	}
}

void ReadMemory(System* sys, guest_address_t addr, size_t cnt, void *dest)
{
	Request req(addr, cnt, Request::PHYSICAL, 0);

	Packet pkt(&req, MemCmd::ReadReq);
	pkt.dataStatic(dest);

	sys->getPhysMem().functionalAccess(&pkt);
}

void WriteMemory(System* sys, guest_address_t addr, size_t cnt, void const *src)
{
	Request req(addr, cnt, Request::PHYSICAL, 0);

	Packet pkt(&req, MemCmd::WriteReq);
	pkt.dataStatic(src);

	sys->getPhysMem().functionalAccess(&pkt);
}

size_t GetPoolSize(System* sys) { return sys->getPhysMem().totalSize(); }

// Controller-related:
unsigned int GetCPUId(System* sys, int context)
{
	return sys->getThreadContext(context)->cpuId();
}

System* GetSystemObject()
{
	return System::systemList.front();
}

int GetNumberOfContexts(System* sys)
{
	return sys->numContexts();
}

} // end-of-namespace: fail
