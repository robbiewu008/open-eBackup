/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file StorageDeviceFactory.h
 * @brief  Contains function declarations for StorageDeviceFactory
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
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