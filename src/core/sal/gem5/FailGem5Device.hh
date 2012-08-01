#ifndef __FAILGEM5_DEVICE_HH__
#define __FAILGEM5_DEVICE_HH__

#include "dev/io_device.hh"
#include "params/FailGem5Device.hh"

class FailGem5Device : public BasicPioDevice
{
	public:
		typedef FailGem5DeviceParams Params;
		
		FailGem5Device(Params *p);

		virtual Tick read(PacketPtr pkt);

		virtual Tick write(PacketPtr pkt);
};

#endif // __FAILGEM5_DEVICE_HH__
