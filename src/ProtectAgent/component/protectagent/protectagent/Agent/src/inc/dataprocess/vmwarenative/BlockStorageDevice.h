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