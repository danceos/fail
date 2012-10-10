#ifndef __FAILGEM5_DEVICE_HH__
#define __FAILGEM5_DEVICE_HH__

#include "dev/io_device.hh"
#include "params/FailGem5Device.hh"
#include "cpu/thread_context.hh"

class Gem5InstructionEvent;

class FailGem5Device : public BasicPioDevice
{
	
public:
	typedef FailGem5DeviceParams Params;
	FailGem5Device(Params *p);
	
	void setNextBreakpoints(ThreadContext *tc);

	virtual void startup();
	virtual Tick read(PacketPtr pkt);
	virtual Tick write(PacketPtr pkt);

private:
	Gem5InstructionEvent* m_BreakpointTaken;
	Gem5InstructionEvent* m_BreakpointNotTaken;

	void activateSingleStepMode();
	void deactivateSingleStepMode();
	void clearBreakpoints();
};

#endif // __FAILGEM5_DEVICE_HH__
