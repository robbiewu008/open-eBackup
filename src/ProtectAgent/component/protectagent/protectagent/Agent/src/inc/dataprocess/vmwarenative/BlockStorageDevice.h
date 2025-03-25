/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file BlockStorageDevice.h
 * @brief  The implemention about BlockStorageDevice
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_H__
#define __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_H__

#include "common/Types.h"
#include "AbstractStorageDevice.h"

class BlockStorageDevice : public AbstractStorageDevice {
public:
    BlockStorageDevice();
    virtual ~BlockStorageDevice();
    mp_void* read(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex);
    mp_void* write(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex, mp_uint64 backupLevel = STORAGEDEVICE_BACKUPLEVEL::FULL);
};

#endif