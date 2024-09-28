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
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "LibnfsDirMetaReader.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    
    const int MP_SUCCESS = 0;
}

LibnfsDirMetaReader::LibnfsDirMetaReader(const ReaderParams &dirReaderParams) : ReaderBase(dirReaderParams)
{
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams);

    m_commonData.controlInfo = m_controlInfo;
    m_commonData.abort = &m_abort;
}

LibnfsDirMetaReader::~LibnfsDirMetaReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    INFOLOG("LibnfsDirMetaReader Destory()");
}

BackupRetCode LibnfsDirMetaReader::Start()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsDirMetaReader start!");
    try {
        m_thread = thread(&LibnfsDirMetaReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDirMetaReader::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsDirMetaReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDirMetaReader::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsDirMetaReader::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsDirMetaReader GetStatus!");

    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

bool LibnfsDirMetaReader::IsComplete() const
{
    DBGLOG("posix dir reader check complete");
    if ((m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume) &&
        m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("dir reader complete");
        return true;
    }
    return false;
}

void LibnfsDirMetaReader::HandleComplete() const
{
    INFOLOG("Complete LibnfsDirMetaReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void LibnfsDirMetaReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDirMetaReader main thread start!");
    while (true) {
        if (IsAbort(m_commonData)) {
            WARNLOG("LibnfsDirMetaReader main thread abort!");
            break;
        }
        DBGLOG("request read from m_readQueue.");
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName;
            ++m_controlInfo->m_readConsume;
            PushToAggregator(fileHandle);
        }

        if (IsComplete()) {
            break;
        }
    }
    HandleComplete();
    INFOLOG("LibnfsDirMetaReader main thread end!");
    m_threadDone = true;
    return;
}

void LibnfsDirMetaReader::PushToAggregator(FileHandle& fileHandle)
{
    // dir pass direct
    if (!m_backupParams.commonParams.writeDisable) {
        while (!m_aggregateQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
            if (IsAbort(m_commonData)) {
                WARNLOG("LibnfsDirMetaReader abort!");
                return;
            }
        }
        ++m_controlInfo->m_readProduce;
    } else {
        m_controlInfo->m_noOfDirCopied++;
    }

    return;
}

int LibnfsDirMetaReader::OpenFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDirMetaReader::ReadData(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDirMetaReader::ReadMeta(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDirMetaReader::CloseFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}
