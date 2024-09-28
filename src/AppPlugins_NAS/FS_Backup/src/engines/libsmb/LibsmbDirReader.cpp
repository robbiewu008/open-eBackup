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
#include "LibsmbDirReader.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
}

LibsmbDirReader::LibsmbDirReader(const ReaderParams &dirReaderParams) : ReaderBase(dirReaderParams)
{
    INFOLOG("Construct LibsmbDirReader!");
    m_srcAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams);
    FillContextParams(m_params.srcSmbContextArgs, m_srcAdvParams);
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.srcRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams)->rootPath;
}

LibsmbDirReader::~LibsmbDirReader()
{
    INFOLOG("Destruct LibsmbDirReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode LibsmbDirReader::Start()
{
    INFOLOG("LibsmbDirReader start!");
    try {
        m_thread = thread(&LibsmbDirReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDirReader::Abort()
{
    INFOLOG("LibsmbDirReader Abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDirReader::Destroy()
{
    INFOLOG("LibsmbDirReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or LibsmbDirReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbDirReader::GetStatus()
{
    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort);
}

bool LibsmbDirReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        HandleComplete();
        return true;
    }
    return false;
}

bool LibsmbDirReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("controlReaderComplete %d (controlFileReaderProduce %d) (readedDir %d)",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_controlInfo->m_controlFileReaderProduce.load(), m_readedDir.load());
    }
    if ((m_controlInfo->m_controlReaderPhaseComplete) &&
        (m_controlInfo->m_controlFileReaderProduce == m_readedDir)) {
        INFOLOG("controlReaderComplete %d (controlFileReaderProduce %d) (readedDir %d)",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_controlInfo->m_controlFileReaderProduce.load(), m_readedDir.load());
        HandleComplete();
        return true;
    }
    return false;
}

void LibsmbDirReader::HandleComplete()
{
    INFOLOG("Complete LibsmbDirReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void LibsmbDirReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbDirReader main thread start!");
    while (true) {
        if (IsComplete()) {
            break;
        }
        if (IsAbort()) {
            WARNLOG("LibsmbDirReader main thread abort!");
            break;
        }
        DBGLOG("request read from m_readQueue.");
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName;
            // dir pass direct
            m_aggregateQueue->Push(fileHandle);
            ++m_controlInfo->m_readProduce;
            ++m_readedDir;
        }
    }
    INFOLOG("LibsmbDirReader main thread end!");
    m_threadDone = true;
    return;
}

int LibsmbDirReader::OpenFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDirReader::ReadData(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDirReader::ReadMeta(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDirReader::CloseFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}
