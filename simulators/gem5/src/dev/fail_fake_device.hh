/** @file
 * Declaration of a fake device.
 */

#ifndef __FAIL_FAKE__DEVICE_HH__
#define __FAIL_FAKE__DEVICE_HH__

#include <string>

#include "base/range.hh"
#include "dev/io_device.hh"
#include "mem/packet.hh"
#include "params/FailFakeDevice.hh"

/**
 * This device implements a fake device for an iobus.
 * On a read this device returns on every request the maximum value.
 * On a write nothing happens.
 * This device is needed because the iobus has no default device by default.
 * In the case of an fault injection the memory request could run over
 * membus -> bridge -> iobus.
 * If findport() in mem/bus.cc doesnt find a matching port on the iobus a default device is needed.
 * This Device is configuered in fs.py and Device.py.
 */
class FailFakeDevice : public BasicPioDevice
{
  protected:
    uint8_t retData8;
    uint16_t retData16;
    uint32_t retData32;
    uint64_t retData64;

  public:
    typedef FailFakeDeviceParams Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }
    /**
      * The constructor for FailFakeDevice just registers itself with the MMU.
      * @param p params structure
      */
    FailFakeDevice(Params *p);

    /**
     * This read always returns -1.
     * @param pkt The memory request.
     */
    virtual Tick read(PacketPtr pkt);

    /**
     * All writes are simply ignored.
     * @param pkt The memory request.
     */
    virtual Tick write(PacketPtr pkt);
};

#endif // __FAIL_FAKE__DEVICE_HH__
