#ifndef __VMFS6_DEVICE_H__
#define __VMFS6_DEVICE_H__

namespace Vmfs6IO {
using VmfsDevReadFuncT = std::function<ssize_t(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len)>;
using VmfsDevWriteFuncT = std::function<ssize_t(const VmfsDeviceT *dev, off_t pos, const u_char *buf, size_t len)>;
using VmfsDevReserveFuncT = std::function<int(const VmfsDeviceT *dev, off_t pos)>;
using VmfsDevReleaseFuncT = std::function<int(const VmfsDeviceT *dev, off_t pos)>;
using VmfsDevCloseFuncT = std::function<void(VmfsDeviceT *dev)>;

struct VmfsDeviceS {
    VmfsDevReadFuncT read;
    VmfsDevWriteFuncT write;
    VmfsDevReserveFuncT reserve;
    VmfsDevReleaseFuncT release;
    VmfsDevCloseFuncT close;
    uuid_t *uuid;
};
using VmfsDeviceT = struct VmfsDeviceS;

static inline ssize_t VmfsDeviceRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len)
{
    if (dev) {
        return dev->read(dev, pos, buf, len);
    }
    return -1;
}

static inline ssize_t VmfsDeviceWrite(const VmfsDeviceT *dev, off_t pos, const u_char *buf, size_t len)
{
    if (dev && dev->write) {
        return dev->write(dev, pos, buf, len);
    }
    return -1;
}

static inline int VmfsDeviceReserve(const VmfsDeviceT *dev, off_t pos)
{
    if (dev && dev->reserve) {
        return dev->reserve(dev, pos);
    }
    return 0;
}

static inline int VmfsDeviceRelease(const VmfsDeviceT *dev, off_t pos)
{
    if (dev && dev->release) {
        return dev->release(dev, pos);
    }
    return 0;
}

static inline void VmfsDeviceClose(VmfsDeviceT *dev)
{
    if (dev && dev->close) {
        dev->close(dev);
    }
}
}

#endif
