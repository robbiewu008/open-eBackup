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
#include "ObjectDeleteReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
}

ObjectDeleteReader::ObjectDeleteReader(const ReaderParams &deleteReaderParams)
    : ReaderBase(deleteReaderParams)
{
    m_advParams = dynamic_pointer_cast<ObjectBackupAdvanceParams>(m_backupParams.srcAdvParams);
}

ObjectDeleteReader::~ObjectDeleteReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    INFOLOG("ObjectDeleteReader Destory");
}

BackupRetCode ObjectDeleteReader::Start()
{
    lock_guard<mutex> lk(m_mtx);
    DBGLOG("ObjectDeleteReader start!");
    try {
        m_thread = thread(&ObjectDeleteReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectDeleteReader::Abort()
{
    lock_guard<mutex> lk(m_mtx);
    INFOLOG("ObjectDeleteReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectDeleteReader::Destroy()
{
    INFOLOG("ObjectDeleteReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or ObjectDeleteReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool ObjectDeleteReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

BackupPhaseStatus ObjectDeleteReader::GetStatus()
{
    lock_guard<mutex> lk(m_mtx);
    DBGLOG("Enter ObjectDeleteReader GetStatus!");
    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

bool ObjectDeleteReader::IsComplete() const
{
    if ((m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume) &&
        m_readQueue->Empty() && m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("delete reader complete");
        return true;
    }
    return false;
}

void ObjectDeleteReader::HandleComplete() const
{
    INFOLOG("Complete ObjectDeleteReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void ObjectDeleteReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("ObjectDeleteReader main thread start!");
    while (true) {
        if (IsComplete()) {
            break;
        }
        if (IsAbort()) {
            WARNLOG("ObjectDeleteReader main thread abort!");
            break;
        }
        DBGLOG("request read from m_readQueue.");
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_fileName.c_str());
            ++m_controlInfo->m_readConsume;
            PushToAggregator(fileHandle);
        }
    }
    HandleComplete();
    INFOLOG("ObjectDeleteReader main thread end!");
    return;
}

void ObjectDeleteReader::PushToAggregator(FileHandle& fileHandle)
{
    if (m_backupParams.commonParams.writeDisable) {
        if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
            m_controlInfo->m_noOfDirDeleted++;
        } else {
            m_controlInfo->m_noOfFilesDeleted++;
        }
        return;
    }

    while (!m_aggregateQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
        if (IsAbort()) {
            WARNLOG("ObjectDeleteReader abort!");
            return;
        }
    }
    ++m_controlInfo->m_readProduce;
    return;
}

int ObjectDeleteReader::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Open file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int ObjectDeleteReader::ReadData(FileHandle& fileHandle)
{
    DBGLOG("Read data for file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int ObjectDeleteReader::ReadMeta(FileHandle& fileHandle)
{
    DBGLOG("Read meta for file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int ObjectDeleteReader::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Close file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}
