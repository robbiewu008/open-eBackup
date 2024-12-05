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
#include "HostDirReader.h"
#include "log/Log.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace Module;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
}

HostDirReader::HostDirReader(
    const ReaderParams &dirReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : ReaderBase(dirReaderParams)
{
    m_srcAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.srcAdvParams);
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.srcRootPath = m_srcAdvParams->dataPath;
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.srcTrimPrefix = m_backupParams.commonParams.trimReaderPrefix;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_failureRecorder = failureRecorder;
}

HostDirReader::~HostDirReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    INFOLOG("Destruct %sDirReader", OS_PLATFORM_NAME.c_str());
}

BackupRetCode HostDirReader::Start()
{
    INFOLOG("%sDirReader start!", OS_PLATFORM_NAME.c_str());
    try {
        m_thread = std::thread(&HostDirReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDirReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDirReader::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostDirReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool HostDirReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

bool HostDirReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DirReader check is complete: "
            "controlReaderComplete %d readQueueEmpty %llu (controlFileReaderProduce %llu) (readedDir %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(),
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_readConsume.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty() &&
        (m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume)) {
        INFOLOG("DirReader complete: "
            "controlReaderComplete %d readQueueEmpty %llu (controlFileReaderProduce %llu) (readedDir %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(),
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_readConsume.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

void HostDirReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sDirReader ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (IsComplete()) {
            m_threadDone = true;
            return;
        }
        if (IsAbort()) {
            m_threadDone = true;
            return;
        }
        DBGLOG("request read from m_readQueue.");
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_dirName.c_str());
            fileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName;
            // dir pass direct
            m_aggregateQueue->WaitAndPush(fileHandle);
            ++m_controlInfo->m_readConsume;
            ++m_controlInfo->m_readProduce;
        }
    }
    INFOLOG("Finish %sDirReader main thread", OS_PLATFORM_NAME.c_str());
    m_threadDone = true;
    return;
}

int HostDirReader::OpenFile(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

int HostDirReader::ReadData(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

int HostDirReader::ReadMeta(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

int HostDirReader::CloseFile(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}