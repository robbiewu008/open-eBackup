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
#include "LibnfsDeleteReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    
    const int MP_SUCCESS = 0;
}

LibnfsDeleteReader::LibnfsDeleteReader(const ReaderParams &deleteReaderParams)
    : ReaderBase(deleteReaderParams)
{
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams);

    m_commonData.controlInfo = m_controlInfo;
    m_commonData.abort = &m_abort;
}

LibnfsDeleteReader::~LibnfsDeleteReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    INFOLOG("LibnfsDeleteReader Destory()");
}

BackupRetCode LibnfsDeleteReader::Start()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("LibnfsDeleteReader start!");
    try {
        m_thread = thread(&LibnfsDeleteReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDeleteReader::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsDeleteReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDeleteReader::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsDeleteReader::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsDeleteReader GetStatus!");

    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

bool LibnfsDeleteReader::IsComplete() const
{
    if ((m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume) &&
        m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("delete reader complete");
        return true;
    }
    return false;
}

void LibnfsDeleteReader::HandleComplete() const
{
    INFOLOG("Complete LibnfsDeleteReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void LibnfsDeleteReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDeleteReader main thread start!");
    while (true) {
        if (IsAbort(m_commonData)) {
            WARNLOG("LibnfsDeleteReader main thread abort!");
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
    INFOLOG("LibnfsDeleteReader main thread end!");
    m_threadDone = true;
    return;
}

void LibnfsDeleteReader::PushToAggregator(FileHandle& fileHandle)
{
    if (!m_backupParams.commonParams.writeDisable) {
        while (!m_aggregateQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
            if (IsAbort(m_commonData)) {
                WARNLOG("LibnfsDeleteReader abort!");
                return;
            }
        }
        ++m_controlInfo->m_readProduce;
    } else {
        fileHandle.m_file->IsFlagSet(IS_DIR) ? m_controlInfo->m_noOfDirDeleted++ : m_controlInfo->m_noOfFilesDeleted++;
    }

    return;
}

int LibnfsDeleteReader::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDeleteReader::ReadData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDeleteReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDeleteReader::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}