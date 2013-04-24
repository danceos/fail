#include "../SALInst.hpp"
#include "Gem5Wrapper.hpp"

#include "sim/system.hh"
#include "mem/packet.hh"
#include "mem/physical.hh"

namespace fail {

// Register-/Memory-related:
regdata_t GetRegisterContent(System* sys, unsigned int id, RegisterType type, size_t idx)
{
	switch (type) {
	case RT_GP: // pass on...
	case RT_FP:	return sys->getThreadContext(id)->readIntReg(idx);
	case RT_ST: return sys->getThreadContext(id)->readMiscReg(idx);
	case RT_IP: return sys->getThreadContext(id)->pcState().pc();
	}
	// This shouldn't be reached if a valid register is passed
	assert(false && "FATAL ERROR: invalid register type (should never be reached)!");
	return 0;
}

void SetRegisterContent(System* sys, unsigned int id, RegisterType type, size_t idx,
                        regdata_t value)
{
	switch (type) {
	case RT_GP: // pass on...
	case RT_FP: sys->getThreadContext(id)->setIntReg(idx, value); return;
	case RT_ST: sys->getThreadContext(id)->setMiscReg(idx, value); return;
	case RT_IP: sys->getThreadContext(id)->pcState().pc(static_cast<Addr>(value)); return;
	}
	// This shouldn't be reached if a valid register is passed
	assert(false && "FATAL ERROR: Invalid register type (should never be reached)!");
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
