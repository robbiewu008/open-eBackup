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
#include "ArchiveDirReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
}

ArchiveDirReader::ArchiveDirReader(const ReaderParams &dirReaderParams) : ReaderBase(dirReaderParams)
{
    INFOLOG("Construct ArchiveDirReader!");
    m_srcAdvParams = dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(m_backupParams.srcAdvParams);
    m_params.srcRootPath = m_srcAdvParams->dataPath;
    m_params.backupType = m_backupParams.backupType;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
}

ArchiveDirReader::~ArchiveDirReader()
{
    INFOLOG("Destruct ArchiveDirReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode ArchiveDirReader::Start()
{
    INFOLOG("ArchiveDirReader start!");
    try {
        m_thread = std::thread(&ArchiveDirReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveDirReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveDirReader::Destroy()
{
    INFOLOG("ArchiveDirReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread func didn't finish! Check if latency is too big or ArchiveDirReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool ArchiveDirReader::IsComplete()
{
    if (m_controlInfo->m_controlReaderFailed) {
        ERRLOG("Control reader failed, exit ArchiveDirReader!");
        HandleComplete();
        return true;
    }
    DBGLOG("ArchiveDirReader check complete: controlReaderComplete - %d, controlFileReaderProduce - %d, "
        "readedDir - %d",
        m_controlInfo->m_controlReaderPhaseComplete.load(), m_controlInfo->m_controlFileReaderProduce.load(),
        m_readedDir.load());
    if ((m_controlInfo->m_controlFileReaderProduce == m_readedDir) &&
        m_controlInfo->m_controlReaderPhaseComplete) {
        HandleComplete();
        return true;
    }
    return false;
}

void ArchiveDirReader::HandleComplete() const
{
    INFOLOG("Complete ArchiveDirReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void ArchiveDirReader::ThreadFunc()
{
    INFOLOG("Start ArchiveDirReader main thread!");
    while (true) {
        if (m_abort) {
            WARNLOG("Abort ArchiveDirReader main thread!");
            m_threadDone = true;
            return;
        }
        DBGLOG("request read from m_readQueue.");
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName;
            ++m_readedDir;
            // dir pass direct
            m_aggregateQueue->WaitAndPush(fileHandle);
            ++m_controlInfo->m_readProduce;
        }

        if (IsComplete()) {
            m_threadDone = true;
            return;
        }
    }
    INFOLOG("End ArchiveDirReader main thread!");
    m_threadDone = true;
    return;
}

int ArchiveDirReader::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int ArchiveDirReader::ReadData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int ArchiveDirReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int ArchiveDirReader::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}
