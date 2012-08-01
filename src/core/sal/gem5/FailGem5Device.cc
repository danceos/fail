#include "FailGem5Device.hh"
#include "debug/Fail.hh"

#include "../SALInst.hpp"

FailGem5Device::FailGem5Device(Params *p)
	: BasicPioDevice(p)
{
	pioSize = 0x60;
	DPRINTF(Fail, "Fail startup()\n");
	fail::simulator.startup();
}

Tick FailGem5Device::read(PacketPtr pkt)
{
	return pioDelay;
}

Tick FailGem5Device::write(PacketPtr pkt)
{
	return pioDelay;
}

FailGem5Device* FailGem5DeviceParams::create()
{
	return new FailGem5Device(this);
}
