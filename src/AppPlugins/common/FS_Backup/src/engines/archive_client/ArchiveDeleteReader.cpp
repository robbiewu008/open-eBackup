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
#include "ArchiveDeleteReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int MP_SUCCESS = 0;
}

ArchiveDeleteReader::ArchiveDeleteReader(const ReaderParams &deleteReaderParams)
    : ReaderBase(deleteReaderParams)
{
    m_advParams = dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(m_backupParams.srcAdvParams);
}

ArchiveDeleteReader::~ArchiveDeleteReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    INFOLOG("ArchiveDeleteReader Destory()");
}

void ArchiveDeleteReader::SetArchiveClient(std::shared_ptr<ArchiveClientBase> client)
{
    m_archiveClient = client;
}

BackupRetCode ArchiveDeleteReader::Start()
{
    lock_guard<std::mutex> lk(mtx);
    DBGLOG("ArchiveDeleteReader start!");
    try {
        m_thread = std::thread(&ArchiveDeleteReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveDeleteReader::Destroy()
{
    INFOLOG("ArchiveDeleteReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread func didn't finish! Check if latency is too big or ArchiveDeleteReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool ArchiveDeleteReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        HandleComplete();
        return true;
    }
    return false;
}

BackupRetCode ArchiveDeleteReader::Abort()
{
    lock_guard<std::mutex> lk(mtx);
    INFOLOG("ArchiveDeleteReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus ArchiveDeleteReader::GetStatus()
{
    lock_guard<std::mutex> lk(mtx);
    DBGLOG("Enter ArchiveDeleteReader GetStatus!");

    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

bool ArchiveDeleteReader::IsComplete() const
{
    if ((m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume) &&
        m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("delete reader complete");
        return true;
    }
    return false;
}

void ArchiveDeleteReader::HandleComplete() const
{
    INFOLOG("Complete ArchiveDeleteReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void ArchiveDeleteReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("ArchiveDeleteReader main thread start!");
    while (true) {
        if (IsAbort()) {
            WARNLOG("ArchiveDeleteReader main thread abort!");
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

        if (IsComplete()) {
            break;
        }
    }
    HandleComplete();
    INFOLOG("ArchiveDeleteReader main thread end!");
}

void ArchiveDeleteReader::PushToAggregator(FileHandle& fileHandle)
{
    if (!m_backupParams.commonParams.writeDisable) {
        while (!m_aggregateQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
            if (IsAbort()) {
                WARNLOG("ArchiveDeleteReader abort!");
                return;
            }
        }
        ++m_controlInfo->m_readProduce;
    } else {
        fileHandle.m_file->IsFlagSet(IS_DIR) ? m_controlInfo->m_noOfDirDeleted++ : m_controlInfo->m_noOfFilesDeleted++;
    }
}

int ArchiveDeleteReader::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int ArchiveDeleteReader::ReadData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int ArchiveDeleteReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int ArchiveDeleteReader::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}