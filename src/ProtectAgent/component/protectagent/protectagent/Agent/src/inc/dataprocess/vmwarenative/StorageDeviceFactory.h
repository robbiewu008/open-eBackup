#ifndef __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_H__
#define __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_H__

#include <memory>
#include "common/Types.h"
#include "AbstractStorageDevice.h"

class StorageDeviceFactory {
public:
    StorageDeviceFactory();
    virtual ~StorageDeviceFactory();
    static std::shared_ptr<AbstractStorageDevice> CreateStorageDevice(mp_int32 storageProtocol);
};

#endif