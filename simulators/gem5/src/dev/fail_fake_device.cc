#include "base/trace.hh"
#include "debug/Fail.hh"
#include "dev/fail_fake_device.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

#include "config/FailConfig.hpp"
#include "sal/SALInst.hpp"

using namespace std;

FailFakeDevice::FailFakeDevice(Params *p)
    : BasicPioDevice(p)
{
    if (!p->ret_bad_addr)
        pioSize = p->pio_size;

    retData8 = p->ret_data8;
    retData16 = p->ret_data16;
    retData32 = p->ret_data32;
    retData64 = p->ret_data64;
}

Tick
FailFakeDevice::read(PacketPtr pkt)
{
    pkt->allocate();
    pkt->makeAtomicResponse();

    DPRINTF(Fail, "[FAIL] dev/fail_fake_device.cc read(): fake response\n");

    switch (pkt->getSize()) {
        case sizeof(uint64_t):
            pkt->set(retData64);
            break;
        case sizeof(uint32_t):
            pkt->set(retData32);
            break;
        case sizeof(uint16_t):
            pkt->set(retData16);
            break;
        case sizeof(uint8_t):
            pkt->set(retData8);
            break;
        default:
            if (params()->fake_mem)
                std::memset(pkt->getPtr<uint8_t>(), 0, pkt->getSize());
            else
                panic("invalid access size! Device being accessed by cache?\n");
    }

    return pioDelay;
}

Tick
FailFakeDevice::write(PacketPtr pkt)
{
    pkt->makeAtomicResponse();

    DPRINTF(Fail, "[FAIL] dev/fail_fake_device.cc write(): fake response\n");

    return pioDelay;
}

FailFakeDevice *
FailFakeDeviceParams::create()
{
    return new FailFakeDevice(this);
}
