/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DoubleQueue.cpp
 * @brief  Contains function declarations DoubleQueue
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/DoubleQueue.h"
#include <pthread.h>
#include <fstream>
#include "dataprocess/vmwarenative/MutexLockGuard.h"
#include "common/Log.h"
#include "common/Types.h"

using namespace std;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
namespace {
const mp_int32 QUEUE_SIZE = 100;
}

// Considering the perforcemance issue, pls pay attation to any code change

DoubleQueue::DoubleQueue()
{
    m_readQueue.reserve(QUEUE_SIZE);
    m_writeQueue.reserve(QUEUE_SIZE);
    m_iNumOfBlockCompleted = 0;
}

DoubleQueue::~DoubleQueue()
{
    // release vector resources
    m_readQueue.shrink_to_fit();
    m_writeQueue.shrink_to_fit();
    m_iNumOfBlockCompleted = 0;
}

// enter double queue
void DoubleQueue::EnQueue(st_disk_datablock &datablock)
{
    if (m_writeQueue.size() >= QUEUE_SIZE) {
        /*
         * Memory consumption issue:
         * Should sleep 1(s) to let datablock to be consumed by another thread, but
         * the backup performance maybe effected
         */
        sleep(1);  // sleep 1s
    }

    MutexLockGuard lock(m_mutex_write);
    m_writeQueue.push_back(datablock);
}

// pop double queue
// single thread writes to file, no lock used temporary
bool DoubleQueue::DeQueueToDataLun(FILE* file, int done, int &condition)
{
    // check file pointer
    if (NULL == file) {
        COMMLOG(OS_LOG_ERROR, "The operator handler of data lun mounted is null.");
        return true;
    }

    // if the read queue is empty, then swap the two queues
    if (m_readQueue.empty()) {
        MutexLockGuard lock(m_mutex_write);
        m_writeQueue.swap(m_readQueue);
    }

    for (vector<st_disk_datablock>::iterator iter = m_readQueue.begin(); iter != m_readQueue.end(); ++iter) {
        if (-1 == fseek(file, iter->index, SEEK_SET)) {
            COMMLOG(OS_LOG_ERROR, "fseek exec failed, data writing will be terminated!");
            // release vector resources
            m_readQueue.shrink_to_fit();
            m_writeQueue.shrink_to_fit();
            return true;
        }
        if (fwrite(iter->dataBuff.c_str(), iter->size, 1, file) != 1) {
            COMMLOG(OS_LOG_ERROR, "fwrite exec failed, data writing will be terminated!");
            m_readQueue.shrink_to_fit();
            m_writeQueue.shrink_to_fit();
            return true;
        }
        fflush(file);
        m_iNumOfBlockCompleted++;
    }

    // both the two queues are empty, it means all content has been written
    if (done == condition && m_readQueue.empty()) {
        // release vector resources
        m_readQueue.shrink_to_fit();
        m_writeQueue.shrink_to_fit();
        COMMLOG(OS_LOG_DEBUG,
            "All datablocks are writen to the backend storage, both the read and write thread will exit!");
        return true;
    }

    m_readQueue.clear();

    return false;
}

bool DoubleQueue::DeQueueToVMwareDisk(const std::shared_ptr<VMwareDiskApi> &sPtr, int done, int condition)
{
    if (NULL == sPtr) {
        COMMLOG(OS_LOG_ERROR, "The VMwareDiskAPi pointer is null.");
        return false;
    }

    // if the read queue is empty, then swap the two queues
    if (m_readQueue.empty()) {
        MutexLockGuard lock(m_mutex_write);
        m_writeQueue.swap(m_readQueue);
    }

    mp_int32 iRet = MP_FAILED;
    mp_string strErrorDesc;
    uint64 sizeToRestore = 0;
    for (vector<st_disk_datablock>::iterator iter = m_readQueue.begin(); iter != m_readQueue.end(); ++iter) {
        strErrorDesc = "";
        sizeToRestore = iter->size;
        iRet = sPtr->Write(iter->index, sizeToRestore, (unsigned char *)iter->dataBuff.c_str(), strErrorDesc);
        // return value check of interface 'Write', but the restore performance will be effected
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Write vm disk failed: '%s'!", strErrorDesc.c_str());
            // release vector resources
            m_readQueue.shrink_to_fit();
            m_writeQueue.shrink_to_fit();
            return true;
        }
        m_iNumOfBlockCompleted++;
    }

    // both the two queues are empty, it means all content has been written
    if (done == condition && m_readQueue.empty()) {
        // release vector resources
        m_readQueue.shrink_to_fit();
        m_writeQueue.shrink_to_fit();
        COMMLOG(OS_LOG_DEBUG,
            "All datablocks are writen to the target vCenter vm disk, both the read and write thread will exit!");
        return true;
    }

    m_readQueue.clear();

    return false;
}
bool DoubleQueue::DeQueue()
{
    // if the read queue is empty, then swap the two queues
    if (m_readQueue.empty()) {
        MutexLockGuard lock(m_mutex_write);
        m_writeQueue.swap(m_readQueue);
    }
    mp_bool ret = true;
    return ret;
}

vector<st_disk_datablock> DoubleQueue::GetReadQueue()
{
    return m_readQueue;
}
vector<st_disk_datablock> DoubleQueue::GetWriteQueue()
{
    return m_writeQueue;
}
mp_uint64 DoubleQueue::GetNumberOfDataBlockCompleted()
{
    return m_iNumOfBlockCompleted;
}

void DoubleQueue::SetNumberOfDataBlockCompleted(mp_uint64 datablockNum)
{
    m_iNumOfBlockCompleted = datablockNum;
}
