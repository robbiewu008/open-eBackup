/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file BlockStorageDevice.cpp
 * @brief  Contains function declarations block storage device
 * @version 1.0.0
 * @date 2015-02-02
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/BlockStorageDevice.h"
#include <vector>
#include "dataprocess/vmwarenative/Define.h"
#include "dataprocess/vmwarenative/MutexLock.h"
#include "dataprocess/vmwarenative/MutexLockGuard.h"
#include "dataprocess/vmwarenative/CountDown.h"
#include "apps/vmwarenative/VMwareDef.h"


using namespace std;
using namespace AGENT_VMWARENATIVE_MUTEXLOCK;
using namespace AGENT_VMWARENATIVE_COUNTDOWN;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;

BlockStorageDevice::BlockStorageDevice()
{}

BlockStorageDevice::~BlockStorageDevice()
{}

mp_void* BlockStorageDevice::read(DoubleQueue& dQueue, const vmware_volume_info& volumeInfo, mp_int32& condition,
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown, AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex)
{
    auto buffer = std::make_unique<char[]>(VMWARE_DATABLOCK_SIZE);

    vector<dirty_range>::const_iterator iter;
    int numOfBlockRead = 0;

    // open backend storage lun mounted for reading, pls note the open mode 'rb'
    FILE* pDataLunFile = fopen(volumeInfo.strTargetLunPath.c_str(), "rb");
    if (pDataLunFile == NULL || buffer.get() == NULL) {
        COMMLOG(OS_LOG_ERROR, "Unable to malloc memory or open backend lun '%s'.", volumeInfo.strTargetLunPath.c_str());
        goto out;
    }

    for (iter = volumeInfo.vecDirtyRange.begin(); iter != volumeInfo.vecDirtyRange.end(); ++iter) {
        if (memset_s(buffer.get(), VMWARE_DATABLOCK_SIZE, 0, VMWARE_DATABLOCK_SIZE) != EOK) {
            COMMLOG(OS_LOG_ERROR, "memset_s exec failed!");
            break;
        }
        // check dirty range scope
        if (volumeInfo.ulDiskSize <= iter->start) {
            COMMLOG(OS_LOG_ERROR, "Invalid offset value, no data will be written to queue.");
            break;
        }
        if (volumeInfo.ulDiskSize < (iter->start + VMWARE_DATABLOCK_SIZE)) {
            m_sizeToProtect = volumeInfo.ulDiskSize - iter->start;
            COMMLOG(OS_LOG_WARN, "The allocated memory block is small than 4M!");
        } else {
            m_sizeToProtect = VMWARE_DATABLOCK_SIZE;
        }

        // read data from FusionStorage mounted file
        if (-1 == fseek(pDataLunFile, iter->start, SEEK_SET)) {
            COMMLOG(OS_LOG_ERROR, "fseek exec failed.");
            break;
        }
        (void)fread(buffer.get(), m_sizeToProtect, 1, pDataLunFile);

        // push data buffer to queue
        mp_string strTmpBuff(buffer.get(), m_sizeToProtect);
        st_disk_datablock st_dataBlock(iter->start, m_sizeToProtect, strTmpBuff);
        // push data to queue
        dQueue.EnQueue(st_dataBlock);
        numOfBlockRead++;
    }

    if (numOfBlockRead == static_cast<int>(volumeInfo.vecDirtyRange.size())) {
        COMMLOG(OS_LOG_INFO,
            "ReadDataLunFun exec successfully, '%d' data block has been pushed to data block queue!",
            numOfBlockRead);
    }

out:
    if (pDataLunFile != NULL) {
        fclose(pDataLunFile);
        pDataLunFile = NULL;
    }

    MutexLockGuard lock(mutex);
    ++condition;
    countdown.Done();
    return ((void*)0);
}

mp_void* BlockStorageDevice::write(DoubleQueue& dQueue, const vmware_volume_info& volumeInfo, mp_int32& condition,
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown, AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex,
    mp_uint64 backupLevel)
{
    FILE* pDataLunFile = fopen(volumeInfo.strTargetLunPath.c_str(), "wb");
    if (pDataLunFile == NULL) {
        COMMLOG(OS_LOG_ERROR, "Unable to open disk file '%s'.", volumeInfo.strTargetLunPath.c_str());
        goto out;
    }

    dQueue.SetNumberOfDataBlockCompleted(0);

    // loop to read data block from queue and write it to target lun
    while (true) {
        int threadCount = 0;
        {
            MutexLockGuard lock(mutex);
            threadCount = condition;
        }
        if (dQueue.DeQueueToDataLun(pDataLunFile, 1, threadCount)) {
            MutexLockGuard lock(mutex);
            condition = 0;
            break;
        }
    }
    if (dQueue.GetNumberOfDataBlockCompleted() == static_cast<mp_uint64>(volumeInfo.vecDirtyRange.size())) {
        COMMLOG(OS_LOG_INFO,
            "All '%ul' data block(s) have been written to target lun '%s' successfully",
            dQueue.GetNumberOfDataBlockCompleted(),
            volumeInfo.strTargetLunPath.c_str());
    }

out:
    countdown.Done();

    if (pDataLunFile != NULL) {
        fclose(pDataLunFile);
        pDataLunFile = NULL;
    }

    return ((void*)0);
}
