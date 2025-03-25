/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file AbstractStorageDevice.h
 * @brief  The implemention about AbstractStorageDevice
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_VMWARENATIVE_ABSTRACTSTORAGEDEVICE_H__
#define __AGENT_VMWARENATIVE_ABSTRACTSTORAGEDEVICE_H__

#include "common/Types.h"
#include "common/Log.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/vmwarenative/DoubleQueue.h"
#include "dataprocess/vmwarenative/MutexLock.h"
#include "dataprocess/vmwarenative/CountDown.h"

namespace STORAGEDEVICE_BACKUPLEVEL {
const mp_uint64 INCR = 1;
const mp_uint64 FULL = 2;
}  // namespace STORAGEDEVICE_BACKUPLEVEL

class AbstractStorageDevice {
public:
    AbstractStorageDevice()
    {
        m_sizeToProtect = 0;
        m_iDirtyRangeBlockNum = 0;
    }
    virtual ~AbstractStorageDevice()
    {}

    // read data from backend storage
    virtual mp_void* read(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex) = 0;

    // write data to backend storage
    virtual mp_void* write(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex, mp_uint64 backupLevel = STORAGEDEVICE_BACKUPLEVEL::FULL) = 0;

protected:
    uint64_t m_sizeToProtect;
    mp_int32 m_iDirtyRangeBlockNum;
};

#endif