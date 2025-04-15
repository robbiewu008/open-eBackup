/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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